#ifndef ICONSUMER_HPP
#define ICONSUMER_HPP

#include <string>
#include "Logger.hpp"

class IConsumer {
protected:
    Logger console;

public:
    inline IConsumer(Logger::LogLevel log_level = Logger::LogLevel::INFO) {
        console.set_level(log_level);
    }
    virtual ~IConsumer() = default;

    // Initializes the consumer (e.g., connects to a broker, subscribes to a topic)
    virtual void initialize() = 0;

    // Subscribes to a topic (if applicable)
    virtual void subscribe(const std::string &topic) = 0;

    // Receives a message (blocking or non-blocking depending on implementation)
    virtual std::string receive_message() = 0;
};

#endif // ICONSUMER_HPP
