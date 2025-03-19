#ifndef IPUBLISHER_APP_HPP
#define IPUBLISHER_APP_HPP

#include <string>
#include <memory>
#include <thread>
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <IPublisher.hpp>
#include "../../technologies/zeromq_p2p/ZeroMQP2PPublisher.hpp" // Add new publishers here

class IPublisherApp {
protected:
    std::string id;
    std::string topics;
    int message_count;
    int update_every;

    std::shared_ptr<spdlog::logger> console = spdlog::stdout_color_mt("console");

    std::unique_ptr<IPublisher> publisher;

public:
    IPublisherApp(spdlog::level::level_enum log_level = spdlog::level::info);
    virtual ~IPublisherApp() = default;


    // Loads values from environment variables into the attributes
    virtual void load_from_env();

    // Factory Method to Create Publisher
    virtual void create_publisher();

    // Runs the publisher logic (can now be fully generalized)
    virtual void run();
};

#endif // IPUBLISHER_APP_HPP
