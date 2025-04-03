#include "KafkaPublisher.hpp"
#include "PublisherFactory.hpp"
#include <cstdlib>
#include <cstring>


KafkaPublisher::KafkaPublisher(const Logger& logger)
    try : IPublisher(logger), producer_(nullptr), conf_(nullptr) {
        console.log_info("[Kafka Publisher] KafkaPublisher created.");
    } catch (const std::exception &e){
        console.log_error("[Kafka Publisher] Constructor failed: " + std::string(e.what()));
}

KafkaPublisher::~KafkaPublisher() {
    console.log_debug("Cleaning up Kafka topic handles...");

    for (auto& [topic, handle] : topic_handles_) {
        console.log_debug("Destroying topic handle for: " + topic);
        rd_kafka_topic_destroy(handle);
    }
    topic_handles_.clear();

    if (producer_) {
        rd_kafka_flush(producer_, 5000);  // Wait for delivery
        rd_kafka_destroy(producer_);
    }
    if (conf_) {
        rd_kafka_conf_destroy(conf_);
    }
}

void KafkaPublisher::initialize() {
    const char* vendpoint = std::getenv("PUBLISHER_ENDPOINT");
    broker_ = vendpoint ? std::string(vendpoint) : "localhost:9092";
    console.log_info("[KafkaPublisher] Using broker: " + broker_);

    char errstr[512];

    conf_ = rd_kafka_conf_new();

    if (!(producer_ = rd_kafka_new(RD_KAFKA_PRODUCER, conf_, errstr, sizeof(errstr)))) {
        throw std::runtime_error("Failed to create producer: " + std::string(errstr));
    }

    if (rd_kafka_brokers_add(producer_, broker_.c_str()) == 0) {
        throw std::runtime_error("No valid brokers added: " + broker_);
    }

    console.log_info("KafkaProducer initialized successfully.");
}

inline std::string serialize_payload(const Payload& message){
    std::string serialized;
    // Simple binary serialization: [label_len][label][num_vals][vals...]
    uint32_t label_len = static_cast<uint32_t>(message.label.size());
    uint32_t num_vals = static_cast<uint32_t>(message.values.size());

    serialized.append(reinterpret_cast<char*>(&label_len), sizeof(label_len));
    serialized.append(message.label);
    serialized.append(reinterpret_cast<char*>(&num_vals), sizeof(num_vals));
    for (double v : message.values) {
        serialized.append(reinterpret_cast<char*>(&v), sizeof(double));
    }
    return serialized;
}

void KafkaPublisher::send_message(const Payload& message, std::string topic) {
    std::string serialized = serialize_payload(message);

    rd_kafka_topic_t* topic_handle = get_or_create_topic_handle(topic);
    if (!topic_handle) {
        console.log_error("Failed to obtain topic handle for " + topic);
        return;
    }

    // todo termination signal -> rd_kafka_topic_destroy?
    int err = rd_kafka_produce(
        topic_handle,                                           // topic
        RD_KAFKA_PARTITION_UA,                                  // partition
        RD_KAFKA_MSG_F_COPY,                                    // copy payload
        const_cast<char*>(serialized.data()),                   // payload ptr
        serialized.size(),                                      // payload len
        nullptr, 0,                                              // key
        nullptr                                                  // msg_opaque
    );

    if (err != 0) {
        console.log_error("Produce failed: " + std::string(rd_kafka_err2str(rd_kafka_last_error())));
    } else {
        console.log_debug("Message queued for topic: " + topic);
    }
    // If message is end-of-stream, destroy the topic handle now
    if (message.label == "__END__") {
        console.log_info("Received __END__ signal — destroying topic handle for: " + topic);
        rd_kafka_topic_destroy(topic_handle);
    }
}

inline rd_kafka_topic_t* KafkaPublisher::get_or_create_topic_handle(const std::string& topic) {
    auto it = topic_handles_.find(topic);
    if (it != topic_handles_.end()) {
        return it->second;
    }

    rd_kafka_topic_t* handle = rd_kafka_topic_new(producer_, topic.c_str(), nullptr);
    if (!handle) {
        console.log_error("Failed to create topic handle for: " + topic);
        return nullptr;
    }

    topic_handles_[topic] = handle;
    console.log_debug("Created new topic handle for: " + topic);
    return handle;
}

inline void KafkaPublisher::destroy_topic_handle(const std::string& topic) {
    auto it = topic_handles_.find(topic);
    if (it != topic_handles_.end()) {
        rd_kafka_topic_destroy(it->second);
        topic_handles_.erase(it);
        console.log_debug("Destroyed topic handle for: " + topic);
    }
}