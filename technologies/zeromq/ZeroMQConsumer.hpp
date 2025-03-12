#ifndef ZEROMQ_CONSUMER_HPP
#define ZEROMQ_CONSUMER_HPP

#include "../../core/interfaces/IConsumer.hpp"
#include <zmq.hpp>
#include <string>

class ZeroMQConsumer : public IConsumer {
private:
    zmq::context_t context;
    zmq::socket_t subscriber;

public:
    ZeroMQConsumer();
    ~ZeroMQConsumer();

    void initialize() override;
    void subscribe(const std::string &topic) override;
    std::string receive_message() override;
};

#endif // ZEROMQ_CONSUMER_HPP
