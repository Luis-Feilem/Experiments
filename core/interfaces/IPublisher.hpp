#ifndef IPUBLISHER_HPP
#define IPUBLISHER_HPP

#include <string>

class IPublisher {
public:
    virtual ~IPublisher() = default;

    // Initializes the publisher (e.g., connects to a broker)
    virtual void initialize(const std::string &endpoint) = 0;

    // Sends a message
    virtual void send_message(const std::string &message, const std::string &topic) = 0;
};

#endif // IPUBLISHER_HPP
