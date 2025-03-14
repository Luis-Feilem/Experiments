#include "IConsumerApp.hpp"
#include "cstdlib"


void IConsumerApp::create_consumer() {
    std::cout << "[IConsumerApp] Creating consumer" << std::endl;
    std::string technology = std::getenv("TECHNOLOGY");
    if (technology == "zeromq_p2p") {
        consumer = std::make_unique<ZeroMQP2PConsumer>();
        std::cout << "[IConsumerApp] Created ZeroMQ consumer" << std::endl;
    } 
    // Extend here for new technologies
    else {
        throw std::runtime_error("Unsupported technology: " + technology);
    }
}

// Initializes and runs the consumer logic
void IConsumerApp::run() {
    std::cout << "[IConsumerApp] Starting consumer" << std::endl;
    std::string endpoint = std::getenv("CONSUMER_ENDPOINT") ?
                            "tcp://" + std::string(std::getenv("CONSUMER_ENDPOINT")) + ":5555" :
                            "tcp://127.0.0.1:5555";

    std::cout << "[IConsumerApp] Initializing consumer with endpoint: " << endpoint 
                << " and topics: " << topics << std::endl;
    consumer->initialize();
    std::cout << "[IConsumerApp] Initialized consumer" << std::endl;

    while (true) {
        std::cout << "[IConsumerApp] Waiting for message..." << std::endl;
        std::string message = consumer->receive_message();
        if (message == ""){
            std::cout << "[IConsumerApp] Received empty message -> Stopping." << std::endl;
            break;
        }
        std::cout << "[ConsumerApp] Received: " << message << std::endl;
        if (message == "__END__") {
            std::cout << "[ConsumerApp] Received termination signal. Stopping." << std::endl;
            break;
        }
    }
}
