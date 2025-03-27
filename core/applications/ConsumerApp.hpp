#ifndef ICONSUMER_APP_HPP
#define ICONSUMER_APP_HPP

#include <string>
#include <iostream>
#include <fstream>

#include "Logger.hpp"
#include "ConsumerFactory.hpp"

class ConsumerApp {
protected:
    std::string id;
    std::string topics;
    
    Logger console;
    Logger::LogLevel log_level;

    std::unique_ptr<IConsumer> consumer;

public:
    ConsumerApp(Logger::LogLevel log_level = Logger::LogLevel::INFO)  {
        log_level = log_level;
        console = Logger(log_level);
    }
    virtual ~ConsumerApp() = default;

    // Factory call to Create Consumer
    virtual void create_consumer();

    // Initializes and runs the consumer logic
    virtual void run();
};

#endif // ICONSUMER_APP_HPP
