#ifndef ZEROMQ_PUBLISHER_APP_HPP
#define ZEROMQ_PUBLISHER_APP_HPP

#include "../../core/interfaces/IPublisherApp.hpp"

class ZeroMQPublisherApp : public IPublisherApp {
public:
    void create_publisher() override {
        publisher = std::make_unique<ZeroMQPublisher>();
    }
};

int main() {
    std::cout << "[ZeroMQPublisherApp] Start" << std::endl << std::flush;
    try {
        ZeroMQPublisherApp app;
        std::cout << "[ZeroMQPublisherApp] Parsing config file" << std::endl;
        app.parse_config("test_scenarios/2pub2sub2topics10msg5000ms/config.json");
        std::cout << "[ZeroMQPublisherApp] Creating publisher" << std::endl;
        app.create_publisher();
        std::cout << "[ZeroMQPublisherApp] Running publisher" << std::endl;
        app.run();
        std::cout << "[ZeroMQPublisherApp] Finished execution" << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "[ZeroMQPublisherApp] Exception caught: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "[ZeroMQPublisherApp] Unknown exception caught!" << std::endl;
    }
    std::cout << "[ZeroMQPublisherApp] End" << std::endl << std::flush;
    return 0;
}


#endif // ZEROMQ_PUBLISHER_APP_HPP
