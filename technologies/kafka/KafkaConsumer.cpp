#include "KafkaConsumer.hpp"
#include "../../core/factory/ConsumerFactory.hpp"
#include <cstdlib> // std::getenv


namespace {
    struct Register {
        Register() {
            ConsumerFactory::registerConsumer("kafka", [](Logger console) -> std::unique_ptr<IConsumer> {
                return std::make_unique<KafkaConsumer>(console);
            });
        }
    };

    static Register reg;
}

inline Payload deserialize_payload(const std::string& data){
    Payload payload;
    size_t offset = 0;

    if (data.size() < sizeof(uint32_t) * 2) {
        throw std::runtime_error("Payload too small to contain metadata");
    }

    // Read label length
    uint32_t label_len;
    std::memcpy(&label_len, data.data() + offset, sizeof(label_len));
    offset += sizeof(label_len);

    if (offset + label_len > data.size()) {
        throw std::runtime_error("Invalid label length in payload");
    }

    payload.label.assign(data.data() + offset, label_len);
    offset += label_len;

    // Read number of doubles
    uint32_t num_values;
    std::memcpy(&num_values, data.data() + offset, sizeof(num_values));
    offset += sizeof(num_values);

    if (offset + num_values * sizeof(double) > data.size()) {
        throw std::runtime_error("Payload too small for declared number of values");
    }

    payload.values.resize(num_values);
    std::memcpy(payload.values.data(), data.data() + offset, num_values * sizeof(double));

    return payload;
}


KafkaConsumer::KafkaConsumer(const Logger& logger)
    : IConsumer(logger) {
    console.log_info("[Kafka Consumer] Constructor finished.");
}

void KafkaConsumer::initialize() {
    const char* vendpoint = std::getenv("CONSUMER_ENDPOINT");

    if (!vendpoint) {
        console.log_debug("[KafkaConsumer] CONSUMER_ENDPOINT not set, defaulting to " + BROKER_ADDRESS + ":" + BROKER_PORT);
        broker_ = BROKER_ADDRESS + ":" + BROKER_PORT;
    } else {
        broker_ = std::string(vendpoint);
    }

    console.log_info("[Kafka Consumer] Using broker: " + broker_);

    config_ = cppkafka::Configuration{
        { "bootstrap.servers", broker_ },
        { "group.id", "benchmark_group" },
        { "enable.auto.commit", "false" },
        { "auto.offset.reset", "earliest" }
    };

    consumer_ = std::make_unique<cppkafka::Consumer>(config_);

    const char* vtopics = std::getenv("TOPICS");
    std::string consumer_id = std::getenv("CONTAINER_ID");
    if (!vtopics) {
        subscribed_streams.insert({broker_, consumer_id.substr(1)});
        // subscribe(consumer_id.substr(1));
        console.log_debug("[Kakfa Consumer] TOPICS not set, default to topic with same numerical id: " + consumer_id.substr(1));
    }
    else{
        std::istringstream topics(vtopics);
        std::string topic;
        while(std::getline(topics, topic, ',')){
            console.log_debug("[Kafka Consumer] Handling subscription to topic " + topic);
            if(!topic.empty()){
                subscribe(topic);
            }
        }
    }
    
    // Deduplicate topic list before subscribing
    std::set<std::string> unique_topics(topics_.begin(), topics_.end());
    std::vector<std::string> deduplicated_topics(unique_topics.begin(), unique_topics.end());

    consumer_->subscribe(deduplicated_topics);
    
    console.log_info("[Kafka Consumer] Initialized and subscribed.");
}

void KafkaConsumer::subscribe(const std::string& topic) {
    if (consumer_) {
        console.log_error("[Kafka Consumer] Cannot subscribe after initialization.");
        return;
    }

    subscribed_streams.insert({broker_, topic}); // todo revise insertion logic
    console.log_info("[Kafka Consumer] Queued subscription for topic: " + topic);
    // subscribed_streams.insert({broker_, topic});
    topics_.push_back(topic);
}

Payload KafkaConsumer::receive_message() {
    cppkafka::Message msg = consumer_->poll();

    if (!msg) {
        return {};  // todo Empty/default payload if no message?
    }

    if (msg.get_error()) {
        console.log_error("[Kafka Consumer] Kafka error: " + msg.get_error().to_string());
        return {}; // todo
    }

    try {
        return deserialize_payload(msg.get_payload());
    } catch (const std::exception& ex) {
        console.log_error("Failed to deserialize payload: " + std::string(ex.what()));
        return {};
    }
    
}
