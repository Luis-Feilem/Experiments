#ifndef IPUBLISHER_APP_HPP
#define IPUBLISHER_APP_HPP

#include <string>
#include <memory>
#include <thread>
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>

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
    ~PublisherApp() = default;


    // Loads values from environment variables into the attributes
    void load_from_env();

    // Factory call to Create Publisher
    void create_publisher();

    // Runs the publisher logic (can now be fully generalized)
    void run();

private:
    // Runs to send a number of messages
    void run_messages();

    // Runs for a set duration
    void run_duration();
};

#endif // IPUBLISHER_APP_HPP
