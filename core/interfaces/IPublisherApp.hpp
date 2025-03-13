#ifndef IPUBLISHER_APP_HPP
#define IPUBLISHER_APP_HPP

#include <string>
#include <memory>
#include <thread>
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>

#include "../../technologies/zeromq/ZeroMQPublisher.hpp" // Add new publishers here

class IPublisherApp {
protected:
    std::string id;
    std::string topics;
    int message_count;
    int update_every;

    std::unique_ptr<IPublisher> publisher;

public:
    virtual ~IPublisherApp() = default;

    // Loads values from environment variables into the attributes
    virtual void load_from_env();

    // Factory Method to Create Publisher
    virtual void create_publisher();

    // Runs the publisher logic (can now be fully generalized)
    virtual void run();
};

#endif // IPUBLISHER_APP_HPP
