#ifndef ZEROMQ_CONSUMER_APP_HPP
#define ZEROMQ_CONSUMER_APP_HPP

#include "../../core/interfaces/IConsumerApp.hpp"

class ZeroMQConsumerApp : public IConsumerApp {
public:
    void create_consumer() override {
        consumer = std::make_unique<ZeroMQConsumer>();
    }
};

int main() {
    std::cout << "[ZeroMQConsumerApp] Start" << std::endl << std::flush;

    try {
        ZeroMQConsumerApp app;
        std::cout << "[ZeroMQConsumerApp] Parsing config file" << std::endl;
        app.parse_config("test_scenarios/1pub1sub1topics10msg5000ms/config.json");
        std::cout << "[ZeroMQConsumerApp] Creating consumer" << std::endl;
        app.create_consumer();
        std::cout << "[ZeroMQConsumerApp] Running consumer" << std::endl;
        app.run();
        std::cout << "[ZeroMQConsumerApp] Finished execution" << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "[ZeroMQPublisherApp] Exception caught: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "[ZeroMQPublisherApp] Unknown exception caught!" << std::endl;
    }
    
    std::cout << "[ZeroMQConsumerApp] End" << std::endl << std::flush;
    return 0;
}


#endif // ZEROMQ_CONSUMER_APP_HPP
