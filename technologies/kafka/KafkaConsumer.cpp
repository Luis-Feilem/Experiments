#include "KafkaConsumer.hpp"
#include "ConsumerFactory.hpp"
#include <cstdlib>
#include <cstring>
#include <sstream>


KafkaConsumer::KafkaConsumer(const Logger& logger)
    : IConsumer(logger),
    consumer_(nullptr),
    conf_(nullptr),
    subscription_list_(nullptr),
    initialized_(false) {
    console.log_info("[Kafka Consumer] KafkaConsumer created.");
}

KafkaConsumer::~KafkaConsumer() {
    if (consumer_) {
        rd_kafka_consumer_close(consumer_);
        rd_kafka_destroy_flags(consumer_, RD_KAFKA_DESTROY_F_NO_CONSUMER_CLOSE);
        // rd_kafka_destroy(consumer_);
        consumer_ = nullptr;
        // Wait up to 5 seconds for all Kafka threads to stop
        int remaining = rd_kafka_wait_destroyed(5000);
        if (remaining != 0) {
            console.log_error("[Kafka Consumer] Kafka still has " + std::to_string(remaining) + " references after destroy.");
        } else {
            console.log_debug("[Kafka Consumer] Kafka destroyed cleanly.");
        }
    }

    if (conf_) {
        rd_kafka_conf_destroy(conf_);
    }

    if (subscription_list_) {
        rd_kafka_topic_partition_list_destroy(subscription_list_);
    }
    console.log_debug("[Kafka Consumer] Destructor finished");
}

void KafkaConsumer::subscribe(const std::string& topic) {
    if (initialized_) {
        console.log_error("[Kafka Consumer] Cannot subscribe to new topics after initialization.");
        return;
    }

    if (subscribed_streams.emplace(topic, "default").second) {
        console.log_info("[Kafka Consumer] Queued subscription for topic: " + topic);
        topic_names_.insert(topic);
    }
}

void KafkaConsumer::initialize() {
    if (initialized_) {
        console.log_error("[Kafka Consumer] Kafka Consumer already initialized.");
        return;
    }
    std::string consumer_id = std::getenv("CONTAINER_ID");
    const char* vendpoint = std::getenv("CONSUMER_ENDPOINT");
    broker_ = vendpoint ? std::string(vendpoint) + ":9092" : "localhost:9092";

    console.log_info("[Kafka Consumer] Using broker: " + broker_);

    char errstr[512];
    conf_ = rd_kafka_conf_new();

    if (rd_kafka_conf_set(conf_, "bootstrap.servers", broker_.c_str(), errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) {
        throw std::runtime_error("[Kafka Consumer] Failed to set bootstrap.servers: " + std::string(errstr));
    }
    if (rd_kafka_conf_set(conf_, "group.id", "benchmark_group", errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) {
        throw std::runtime_error("[Kafka Consumer] Failed to set group.id: " + std::string(errstr));
    }
    if (rd_kafka_conf_set(conf_, "enable.auto.commit", "false", errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) {
        throw std::runtime_error("[Kafka Consumer] Failed to set enable.auto.commit: " + std::string(errstr));
    }
    if (rd_kafka_conf_set(conf_, "auto.offset.reset", "earliest", errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) {
        throw std::runtime_error("[Kafka Consumer] Failed to set auto.offset.reset: " + std::string(errstr));
    }
    if (!(consumer_ = rd_kafka_new(RD_KAFKA_CONSUMER, conf_, errstr, sizeof(errstr)))) {
        throw std::runtime_error("[Kafka Consumer] Failed to create consumer: " + std::string(errstr));
    }
    // rd_kafka_conf_destroy(conf_);
    conf_ = nullptr;

    rd_kafka_poll_set_consumer(consumer_);

    
    const char* vtopics = std::getenv("TOPICS");
    if (!vtopics) {
        subscribed_streams.insert({broker_, consumer_id.substr(1)});
        subscribe(consumer_id.substr(1));
        console.log_debug("[Kafka Consumer] TOPICS not set, default to topic with same numerical id: " + consumer_id.substr(1));
    }
    else{
        std::istringstream topics(vtopics);
        std::string topic;
        while(std::getline(topics, topic, ',')){
            console.log_debug("[Kafka Consumer] Handling subscription to topic " + topic);
            if(!topic.empty()){
                console.log_info("[Kafka Consumer] Connecting to stream ("+ broker_ + "," + topic + ")");
                subscribe(topic);
            }
        }
    }

    console.log_debug("[KafkaConsumer] Subscription list will have size " + std::to_string(static_cast<int>(topic_names_.size())));
    subscription_list_ = rd_kafka_topic_partition_list_new(static_cast<int>(topic_names_.size()));
    for (const auto& topic : topic_names_) {
        rd_kafka_topic_partition_list_add(subscription_list_, topic.c_str(), -1);
        console.log_debug("[Kafka Consumer] Prepared subscription to topic: " + topic);
    }

    if (rd_kafka_subscribe(consumer_, subscription_list_) != RD_KAFKA_RESP_ERR_NO_ERROR) {
        throw std::runtime_error("[Kafka Consumer] Failed to subscribe to Kafka topics.");
    }

    rd_kafka_topic_partition_list_destroy(subscription_list_);
    subscription_list_ = nullptr;

    initialized_ = true;

    console.log_info("[Kafka Consumer] Consumer initialized and subscribed.");
}


inline Payload deserialize_payload(const std::string& data) {
    Payload result;
    size_t offset = 0;

    // Step 1: Extract label length
    if (data.size() < offset + sizeof(uint32_t))
        throw std::runtime_error("Invalid payload: insufficient data for label length");
    uint32_t label_len;
    std::memcpy(&label_len, data.data() + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Step 2: Extract label
    if (data.size() < offset + label_len)
        throw std::runtime_error("Invalid payload: insufficient data for label string");
    result.label = std::string(data.data() + offset, label_len);
    offset += label_len;

    // Step 3: Extract number of values
    if (data.size() < offset + sizeof(uint32_t))
        throw std::runtime_error("Invalid payload: insufficient data for value count");
    uint32_t num_vals;
    std::memcpy(&num_vals, data.data() + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Step 4: Extract values
    if (data.size() < offset + num_vals * sizeof(double))
        throw std::runtime_error("Invalid payload: insufficient data for values");
    result.values.reserve(num_vals);
    for (uint32_t i = 0; i < num_vals; ++i) {
        double v;
        std::memcpy(&v, data.data() + offset, sizeof(double));
        offset += sizeof(double);
        result.values.push_back(v);
    }

    return result;
}


Payload KafkaConsumer::receive_message() {
    console.log_debug("[Kafka Consumer] Polling for messages...");
    rd_kafka_message_t* msg = rd_kafka_consumer_poll(consumer_, 2000);
    if (!msg) {
        console.log_debug("[Kafka Consumer] Poll returned null message");
        return {};
    }

    Payload payload = {};
    if (msg->err) {
        console.log_error("[Kafka Consumer] Kafka error: " + std::string(rd_kafka_message_errstr(msg)));
    } 
    else if (msg->len > 0) {
        std::string topic = rd_kafka_topic_name(msg->rkt);
        console.log_debug("[Kafka Consumer] Received message on topic '" + topic + "' with " + std::to_string(msg->len) + " bytes");
        // payload = deserialize_payload(std::string(static_cast<const char*>(msg->payload), msg->len));
        try {
            // COPY from message buffer BEFORE destroying
            std::string raw(static_cast<const char*>(msg->payload), msg->len);
            payload = deserialize_payload(raw);
        } catch (const std::exception& e) {
            console.log_error("[Kafka Consumer] Failed to deserialize payload: " + std::string(e.what()));
        }

        if (payload.label.find("__END__")  != std::string::npos) {
            terminated_streams.insert({broker_, topic});
            console.log_info("[Kafka Consumer] Received termination for topic: " + topic);
            console.log_debug("[Kafka Consumer] Streams closed: " + std::to_string(terminated_streams.size()) + "/" + std::to_string(subscribed_streams.size()));
            // rd_kafka_message_destroy(msg);
            if (terminated_streams.size() == subscribed_streams.size()) {
                console.log_info("[Kafka Consumer] All publishers terminated.");
                payload = {"__END__", {}};  // Final poison pill
            } else {
                // Not ready to stop yet, keep listening
                payload = {"__ENDTOPIC__", {}};  // Signal to ignore and continue
            }
        }
    }
    else{
        console.log_error("[Kafka Consumer] Unknown msg handling condition");
    }

    rd_kafka_message_destroy(msg);
    return payload;
}
