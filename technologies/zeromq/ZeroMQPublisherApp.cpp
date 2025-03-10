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
    std::cout << "ZeroMQPublisherApp started" << std::endl << std::flush;
    ZeroMQPublisherApp app;
    app.parse_config("test_scenarios/1pub1sub1topics10msg5000ms/config.json");
    app.run();
    std::cout << "ZeroMQPublisherApp finished" << std::endl << std::flush;
    return 0;
}


#endif // ZEROMQ_PUBLISHER_APP_HPP
