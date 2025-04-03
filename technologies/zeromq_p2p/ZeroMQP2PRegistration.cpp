#include "PublisherFactory.hpp"
#include "./ZeroMQP2PPublisher.hpp"
#include "ConsumerFactory.hpp"
#include "./ZeroMQP2PConsumer.hpp"
#include "Logger.hpp"

extern "C" void register_technology(Logger logger) {
    PublisherFactory::registerPublisher("zeromq_p2p", [](Logger logger) -> std::unique_ptr<IPublisher> {
        logger.log_info("[Kafka Registration] Registering publisher in factory");
        return std::make_unique<ZeroMQP2PPublisher>(logger);
    });
    ConsumerFactory::registerConsumer("zeromq_p2p", [](Logger logger) -> std::unique_ptr<IConsumer> {
        logger.log_info("[Kafka Registration] Registering consumer in factory");
        return std::make_unique<ZeroMQP2PConsumer>(logger);
    });
    logger.log_debug("[Kafka Registration] Registered creators in factories");
    PublisherFactory::debug_print_registry(logger);
    ConsumerFactory::debug_print_registry(logger);
}
