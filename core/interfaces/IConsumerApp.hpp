#ifndef ICONSUMER_APP_HPP
#define ICONSUMER_APP_HPP

#include <string>
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include "../factory/ConsumerFactory.hpp"

class IConsumerApp {
protected:
    std::string id;
    std::string topics;
    
    std::shared_ptr<spdlog::logger> console = spdlog::stdout_color_mt("console");

    std::unique_ptr<IConsumer> consumer;

public:
    IConsumerApp(spdlog::level::level_enum log_level = spdlog::level::info);
    virtual ~IConsumerApp() = default;

    // Factory Method to Create Consumer
    virtual void create_consumer();

    // Initializes and runs the consumer logic
    virtual void run();
};

#endif // ICONSUMER_APP_HPP
