#include "KafkaPublisher.hpp"
#include "../../core/factory/PublisherFactory.hpp"

namespace {
    struct Register {
        Register() {
            PublisherFactory::registerPublisher("kafka", [](Logger console) -> std::unique_ptr<IPublisher> {
                return std::make_unique<KafkaPublisher>(console);
            });
        }
    };

    static Register reg;
}

inline std::string serialize_payload(const Payload& payload){
    std::string buffer;

    uint32_t label_len = static_cast<uint32_t>(payload.label.size());
    uint32_t num_values = static_cast<uint32_t>(payload.values.size());

    // Reserve buffer for efficiency
    buffer.reserve(sizeof(label_len) + label_len + sizeof(num_values) + sizeof(double) * num_values);

    // Append label length
    buffer.append(reinterpret_cast<const char*>(&label_len), sizeof(label_len));

    // Append label characters
    buffer.append(payload.label.data(), label_len);

    // Append number of values
    buffer.append(reinterpret_cast<const char*>(&num_values), sizeof(num_values));

    // Append all double values
    for (double value : payload.values) {
        buffer.append(reinterpret_cast<const char*>(&value), sizeof(double));
    }

    return buffer;
}

KafkaPublisher::KafkaPublisher(const Logger& logger, std::string broker_address)
    try : IPublisher(logger), broker_(broker_address) {
    console.log_debug("[Kafka Publisher] Constructor finished");
    } catch (const std::exception& e) { 
        console.log_error("[Kafka Publisher] Constructor failed: " + std::string(e.what()));
}

void KafkaPublisher::initialize() {
    console.log_info("[Kafka Publisher] Initializing");

    const char* vendpoint = std::getenv("PUBLISHER_ENDPOINT");
    std::string endpoint = "";
    if (!vendpoint) {
        console.log_debug("[Kafka Publisher] PUBLISHER_ENDPOINT not set, defaulting to " + BROKER_ADDRESS + ":" + BROKER_PORT);
        broker_ = BROKER_ADDRESS + ":" + BROKER_PORT;
    } else{
        broker_ = std::string(vendpoint);
    }
    console.log_info("[Kafka Publisher] Using broker: " + broker_);
    
    config_ = cppkafka::Configuration{
        { "bootstrap.servers", endpoint }
    };

    producer_ = std::make_unique<cppkafka::Producer>(config_);
    console.log_info("[Kafka Publisher] Kafka producer initialized.");
}

void KafkaPublisher::send_message(const Payload& message, std::string topic) {
    try {
        std::string serialized = serialize_payload(message);
        producer_->produce(cppkafka::MessageBuilder(topic).payload(serialized));
        producer_->flush();  // Optional, can optimize later
    } catch (const std::exception& ex) {
        console.log_error("KafkaPublisher failed to send message: " + std::string(ex.what()));
    }
}
