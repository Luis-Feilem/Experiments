#ifndef IPUBLISHER_HPP
#define IPUBLISHER_HPP

#include <string>

class IPublisher {
public:
    virtual ~IPublisher() = default;

    // Initializes the publisher (e.g., connects to a broker)
    virtual void initialize() = 0;

    // Sends a message
    virtual void send_message(const std::string &message) = 0;
};

#endif // IPUBLISHER_HPP
