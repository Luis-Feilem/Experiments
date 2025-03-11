#ifndef ZEROMQ_PUBLISHER_HPP
#define ZEROMQ_PUBLISHER_HPP

#include "../../core/interfaces/IPublisher.hpp"
#include <zmq.hpp>
#include <string>

class ZeroMQPublisher : public IPublisher {
private:
    zmq::context_t context;
    zmq::socket_t publisher;

public:
    ZeroMQPublisher();
    ~ZeroMQPublisher();

    void initialize(const std::string &endpoint) override;
    void send_message(const std::string &message, const std::string &topic) override;
};

#endif // ZEROMQ_PUBLISHER_HPP
