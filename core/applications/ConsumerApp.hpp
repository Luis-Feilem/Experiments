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

    std::unique_ptr<IConsumer> consumer;

public:
    ConsumerApp(Logger::LogLevel log_level = Logger::LogLevel::INFO) : console(log_level) {};
    virtual ~ConsumerApp() = default;

    // Factory Method to Create Consumer
    virtual void create_consumer();

    // Initializes and runs the consumer logic
    virtual void run();
};

#endif // ICONSUMER_APP_HPP
