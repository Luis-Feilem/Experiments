#ifndef IPUBLISHER_HPP
#define IPUBLISHER_HPP

#include <string>
#include "Logger.hpp"
#include "Payload.h"

class IPublisher {
protected:
    Logger console;
    
public:
    IPublisher(const Logger& logger) {
        console = logger;
    }
    virtual ~IPublisher() = default;

    // Initializes the publisher (e.g., connects to a broker)
    virtual void initialize() = 0;

    // Serializes a Payload object to a string format
    virtual std::string serialize(const Payload& message) = 0;

    // Sends a message
    virtual void send_message(const Payload &message, std::string topic) = 0;

    // Log publisher configuration during runtime
    virtual void log_configuration() = 0;
};

#endif // IPUBLISHER_HPP
