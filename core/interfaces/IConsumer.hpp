#ifndef ICONSUMER_HPP
#define ICONSUMER_HPP

#include <string>

class IConsumer {
public:
    virtual ~IConsumer() = default;

    // Initializes the consumer (e.g., connects to a broker, subscribes to a topic)
    virtual void initialize() = 0;

    // Subscribes to a topic (if applicable)
    virtual void subscribe(const std::string &topic) = 0;

    // Receives a message (blocking or non-blocking depending on implementation)
    virtual std::string receive_message() = 0;
};

#endif // ICONSUMER_HPP
