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
    std::cout << "ZeroMQConsumerApp started" << std::endl << std::flush;
    ZeroMQConsumerApp app;
    app.parse_config("test_scenarios/1pub1sub1topics10msg5000ms/config.json");
    app.run();
    std::cout << "ZeroMQConsumerApp finished" << std::endl << std::flush;
    return 0;
}


#endif // ZEROMQ_CONSUMER_APP_HPP
