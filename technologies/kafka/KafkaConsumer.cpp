#include "KafkaConsumer.hpp"
#include "ConsumerFactory.hpp"
#include <cstdlib>
#include <cstring>


KafkaConsumer::KafkaConsumer(const Logger& logger)
    : IConsumer(logger),
    consumer_(nullptr),
    conf_(nullptr),
    subscription_list_(nullptr),
    initialized_(false) {
    console.log_info("KafkaConsumer created.");
}

KafkaConsumer::~KafkaConsumer() {
    if (consumer_) {
        rd_kafka_consumer_close(consumer_);
        rd_kafka_destroy(consumer_);
    }

    if (conf_) {
        rd_kafka_conf_destroy(conf_);
    }

    if (subscription_list_) {
        rd_kafka_topic_partition_list_destroy(subscription_list_);
    }
}

void KafkaConsumer::subscribe(const std::string& topic) {
    if (initialized_) {
        console.log_error("Cannot subscribe to new topics after initialization.");
        return;
    }

    if (subscribed_streams.emplace(topic, "default").second) {
        console.log_info("[KafkaConsumer] Queued subscription for topic: " + topic);
        topic_names_.insert(topic);
    }
}

void KafkaConsumer::initialize() {
    if (initialized_) {
        console.log_error("KafkaConsumer already initialized.");
        return;
    }
    const char* vendpoint = std::getenv("CONSUMER_ENDPOINT");
    broker_ = vendpoint ? std::string(vendpoint) + ":9092" : "localhost:9092";

    console.log_info("[KafkaConsumer] Using broker: " + broker_);

    char errstr[512];
    conf_ = rd_kafka_conf_new();

    rd_kafka_conf_set(conf_, "bootstrap.servers", broker_.c_str(), errstr, sizeof(errstr));
    rd_kafka_conf_set(conf_, "group.id", "benchmark_group", errstr, sizeof(errstr));
    rd_kafka_conf_set(conf_, "enable.auto.commit", "false", errstr, sizeof(errstr));
    rd_kafka_conf_set(conf_, "auto.offset.reset", "earliest", errstr, sizeof(errstr));

    consumer_ = rd_kafka_new(RD_KAFKA_CONSUMER, conf_, errstr, sizeof(errstr));
    if (!consumer_) {
        throw std::runtime_error("Failed to create consumer: " + std::string(errstr));
    }

    rd_kafka_poll_set_consumer(consumer_);

    subscription_list_ = rd_kafka_topic_partition_list_new(static_cast<int>(topic_names_.size()));
    for (const auto& topic : topic_names_) {
        rd_kafka_topic_partition_list_add(subscription_list_, topic.c_str(), -1);
        console.log_debug("Prepared subscription to topic: " + topic);
    }

    if (rd_kafka_subscribe(consumer_, subscription_list_) != RD_KAFKA_RESP_ERR_NO_ERROR) {
        throw std::runtime_error("Failed to subscribe to Kafka topics.");
    }

    initialized_ = true;

    console.log_info("KafkaConsumer initialized and subscribed.");
}

Payload KafkaConsumer::receive_message() {
    rd_kafka_message_t* msg = rd_kafka_consumer_poll(consumer_, 1000);

    if (!msg) {
        return {};
    }

    Payload payload;

    if (msg->err) {
        console.log_error("Kafka error: " + std::string(rd_kafka_message_errstr(msg)));
    } else if (msg->payload && msg->len > 0) {
        const char* data = static_cast<const char*>(msg->payload);
        size_t offset = 0;

        uint32_t label_len;
        std::memcpy(&label_len, data + offset, sizeof(label_len));
        offset += sizeof(label_len);

        payload.label.assign(data + offset, label_len);
        offset += label_len;

        uint32_t num_vals;
        std::memcpy(&num_vals, data + offset, sizeof(num_vals));
        offset += sizeof(num_vals);

        payload.values.resize(num_vals);
        std::memcpy(payload.values.data(), data + offset, num_vals * sizeof(double));

        console.log_debug("Received message for topic: " +
            std::string(msg->rkt ? rd_kafka_topic_name(msg->rkt) : "unknown") +
            " | Label: " + payload.label +
            " | Size: " + std::to_string(payload.values.size()));
        
            // Optional: Mark topic as terminated if label is "__END__"
        if (payload.label == "__END__") {
            std::string topic_name = rd_kafka_topic_name(msg->rkt);
            terminated_streams.emplace(topic_name, "default");
            console.log_info("Received __END__ on topic: " + topic_name);
        }
    }

    rd_kafka_message_destroy(msg);
    return payload;
}
