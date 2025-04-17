#include "KafkaPublisher.hpp"
#include "PublisherFactory.hpp"
#include <cstdlib>
#include <cstring>

static void kafka_log_callback(const rd_kafka_t* rk, int level,
    const char* fac, const char* buf) {
    std::cerr << "[librdkafka][" << fac << "] " << buf << std::endl;
}

static void dr_msg_cb(rd_kafka_t* rk, const rd_kafka_message_t* rkmessage, void*) {
    if (rkmessage->err) {
        std::cerr << "[Kafka Publisher] Delivery failed: " << rd_kafka_err2str(rkmessage->err) << std::endl;
    } else {
        std::cerr << "[Kafka Publisher] Delivered message '" << rkmessage << "' of " << rkmessage->len << " bytes" << std::endl;
    }
}

KafkaPublisher::KafkaPublisher(const Logger& logger)
    try : IPublisher(logger), producer_(nullptr), conf_(nullptr) {
        console.log_info("[Kafka Publisher] KafkaPublisher created.");
    } catch (const std::exception &e){
        console.log_error("[Kafka Publisher] Constructor failed: " + std::string(e.what()));
}

KafkaPublisher::~KafkaPublisher() {
    console.log_debug("[Kafka Publisher] Cleaning up Kafka topic handles...");

    for (auto& [topic, handle] : topic_handles_) {
        console.log_debug("[Kafka Publisher] Destroying topic handle for: " + topic);
        destroy_topic_handle(topic);
        console.log_debug("[Kafka Publisher] Topic handle for '" + topic + "' has been destroyed");
    }
    console.log_debug("[Kafka Publisher] Destroyed topic handles");
    topic_handles_.clear();

    if (producer_) {
        console.log_debug("[Kafka Publisher] Polling before flush...");
        while (rd_kafka_outq_len(producer_) > 0) {
            rd_kafka_poll(producer_, 100);  // wait up to 100ms
        }
        console.log_debug("[Kafka Publisher] Flushing");
        rd_kafka_flush(producer_, 10 * 1000);  // Wait for delivery
        console.log_debug("[Kafka Publisher] Destroying");
        int remaining = rd_kafka_wait_destroyed(5000);
        if (remaining != 0) {
            console.log_error("[Kafka Publisher] Kafka still has " + std::to_string(remaining) + " references after destroy.");
        } else {
            console.log_debug("[Kafka Publisher] Kafka destroyed cleanly.");
        }
    }
    if (conf_) {
        rd_kafka_conf_destroy(conf_);
        conf_ = nullptr;
    }
    console.log_debug("[Kafka Publisher] Kafka destructor finished.");
}

void KafkaPublisher::initialize() {
    const char* vendpoint = std::getenv("PUBLISHER_ENDPOINT");
    broker_ = vendpoint ? std::string(vendpoint) + ":9092" : "localhost:9092";
    console.log_info("[Kafka Publisher] Using broker: " + broker_);

    char errstr[512];
    conf_ = rd_kafka_conf_new();

    rd_kafka_conf_set_log_cb(conf_, kafka_log_callback);
    rd_kafka_conf_set_dr_msg_cb(conf_, dr_msg_cb);

    if (rd_kafka_conf_set(conf_, "bootstrap.servers", broker_.c_str(), errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) {
        throw std::runtime_error("Failed to set bootstrap.servers: " + std::string(errstr));
    }

    rd_kafka_conf_t* snapshot_conf = rd_kafka_conf_dup(conf_);

    if (!(producer_ = rd_kafka_new(RD_KAFKA_PRODUCER, snapshot_conf, errstr, sizeof(errstr)))) {
        throw std::runtime_error("Failed to create producer: " + std::string(errstr));
    }
    snapshot_conf = nullptr;

    console.log_info("[Kafka Publisher] Publisher initialized successfully.");
    log_configuration();
}

std::string KafkaPublisher::serialize(const Payload& payload){
    std::vector<char> buffer;

    // Message ID
    uint16_t id_len = static_cast<uint16_t>(payload.message_id.size());
    buffer.insert(buffer.end(),
                  reinterpret_cast<const char*>(&id_len),
                  reinterpret_cast<const char*>(&id_len) + sizeof(id_len));
    buffer.insert(buffer.end(), payload.message_id.begin(), payload.message_id.end());

    // Kind
    uint8_t kind = static_cast<uint8_t>(payload.kind);
    buffer.insert(buffer.end(),
                  reinterpret_cast<const char*>(&kind),
                  reinterpret_cast<const char*>(&kind) + sizeof(kind));

    // Data size
    size_t size = static_cast<size_t>(payload.data_size);
    buffer.insert(buffer.end(),
                  reinterpret_cast<const char*>(&size),
                  reinterpret_cast<const char*>(&size) + sizeof(size));

    // Data
    // todo: variety of PayloadKind may require different serialization methods
    buffer.insert(buffer.end(), payload.data.begin(), payload.data.end());

    return std::string(buffer.begin(), buffer.end());
}

void KafkaPublisher::send_message(const Payload& message, std::string topic) {
    std::string serialized = serialize(message);

    rd_kafka_topic_t* topic_handle = get_or_create_topic_handle(topic);
    if (!topic_handle) {
        console.log_error("[Kafka Publisher] Failed to obtain topic handle for " + topic);
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
    rd_kafka_poll(producer_, 0);

    if (err != 0) {
        console.log_error("[Kafka Publisher] Produce failed: " + std::string(rd_kafka_err2str(rd_kafka_last_error())));
    } else {
        console.log_debug("[Kafka Publisher] Message queued for topic: " + topic);
    }
    // If message is end-of-stream, destroy the topic handle now
    if (message.message_id.find(TERMINATION_SIGNAL) != std::string::npos) {
        console.log_info("[Kafka Publisher] Received termination signal — destroying topic handle for: " + topic);
        destroy_topic_handle(topic);
    }
}

inline rd_kafka_topic_t* KafkaPublisher::get_or_create_topic_handle(const std::string& topic) {
    auto it = topic_handles_.find(topic);
    if (it != topic_handles_.end()) {
        return it->second;
    }

    rd_kafka_topic_t* handle = rd_kafka_topic_new(producer_, topic.c_str(), nullptr);
    if (!handle) {
        console.log_error("[Kafka Publisher] Failed to create topic handle for: " + topic);
        return nullptr;
    }

    topic_handles_[topic] = handle;
    console.log_debug("[Kafka Publisher] Created new topic handle for: " + topic);
    return handle;
}

inline void KafkaPublisher::destroy_topic_handle(const std::string& topic) {
    auto it = topic_handles_.find(topic);
    if (it != topic_handles_.end()) {
        rd_kafka_topic_destroy(it->second);
        topic_handles_.erase(it);
        console.log_debug("[Kafka Publisher] Destroyed topic handle for: " + topic);
    }
}

void KafkaPublisher::log_configuration() {
    size_t cnt;
    const char** conf = rd_kafka_conf_dump(conf_, &cnt);

    console.log_info("[Kafka Publisher] [CONFIG_BEGIN]");
    for (size_t i = 0; i < cnt; i += 2) {
        console.log_info("[CONFIG] " + std::string(conf[i]) + "=" + std::string(conf[i+1]));
    }
    console.log_info("[Kafka Publisher] [CONFIG_END]");

    rd_kafka_conf_dump_free(conf, cnt);
}
