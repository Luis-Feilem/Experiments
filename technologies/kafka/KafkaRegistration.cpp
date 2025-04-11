#include "PublisherFactory.hpp"
#include "./KafkaPublisher.hpp"
#include "ConsumerFactory.hpp"
#include "./KafkaConsumer.hpp"

extern "C" void register_technology(Logger logger) {
    PublisherFactory::registerPublisher("kafka", [](Logger logger) -> std::unique_ptr<IPublisher> {
        return std::make_unique<KafkaPublisher>(logger);
    });
    ConsumerFactory::registerConsumer("kafka", [](const Logger& logger) -> std::unique_ptr<IConsumer> {
        return std::make_unique<KafkaConsumer>(logger);
    });
    logger.log_debug("[Kafka Registration] Registered creators in factories");
    PublisherFactory::debug_print_registry(logger);
    ConsumerFactory::debug_print_registry(logger);
}
