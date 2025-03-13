#ifndef ICONSUMER_APP_HPP
#define ICONSUMER_APP_HPP

#include <string>
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>

#include "../../technologies/zeromq_p2p/ZeroMQP2PConsumer.hpp" // Add new consumers here

class IConsumerApp {
protected:
    std::string id;
    std::string topics;

    std::unique_ptr<IConsumer> consumer;

public:
    virtual ~IConsumerApp() = default;

    // Factory Method to Create Consumer
    virtual void create_consumer();

    // Initializes and runs the consumer logic
    virtual void run();
};

#endif // ICONSUMER_APP_HPP
