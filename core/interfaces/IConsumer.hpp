#ifndef ICONSUMER_HPP
#define ICONSUMER_HPP

#include <string>
#include <set>
#include "Logger.hpp"
#include "Payload.h"

class IConsumer {
protected:
    Logger console;
    std::set<std::string> terminated_topics;
    std::set<std::string> subscribed_topics;

public:
    inline IConsumer(const Logger& logger) {
        console = logger;
    }
    virtual ~IConsumer() = default;

    // Initializes the consumer (e.g., connects to a broker, subscribes to a topic)
    virtual void initialize() = 0;

    // Subscribes to a topic (if applicable)
    virtual void subscribe(const std::string &topic) = 0;

    // Receives a message (blocking or non-blocking depending on implementation)
    virtual Payload receive_message() = 0;
};

#endif // ICONSUMER_HPP
