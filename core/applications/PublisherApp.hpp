#ifndef IPUBLISHER_APP_HPP
#define IPUBLISHER_APP_HPP

#include <string>
#include <memory>
#include <thread>
#include <iostream>
#include <fstream>
#include <sstream>

#include "Logger.hpp"
#include "PublisherFactory.hpp"


class PublisherApp {
protected:
    std::string id;
    std::string topics;
    int message_count;
    int duration;
    int update_every;

    Logger console;
    Logger::LogLevel log_level;

    std::unique_ptr<IPublisher> publisher;

public:
    PublisherApp(Logger::LogLevel log_level = Logger::LogLevel::INFO) {
        log_level = log_level;
        console = Logger(log_level);
    }
    virtual ~PublisherApp() = default;


    // Loads values from environment variables into the attributes
    virtual void load_from_env();

    // Factory call to Create Publisher
    virtual void create_publisher();

    // Runs the publisher logic (can now be fully generalized)
    virtual void run();
};

#endif // IPUBLISHER_APP_HPP
