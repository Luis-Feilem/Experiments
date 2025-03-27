#ifndef IPUBLISHER_HPP
#define IPUBLISHER_HPP

#include <string>
#include "Logger.hpp"

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

    // Sends a message
    virtual void send_message(const std::string &message) = 0;
};

#endif // IPUBLISHER_HPP
