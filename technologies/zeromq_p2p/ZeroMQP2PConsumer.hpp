#ifndef ZEROMQP2P_CONSUMER_HPP
#define ZEROMQP2P_CONSUMER_HPP

#include "IConsumer.hpp"
#include <zmq.hpp>
#include <string>
#include <sstream>
#include <set>
#include <iostream>
#include "Logger.hpp"

class ZeroMQP2PConsumer : public IConsumer {
private:
    zmq::context_t context;
    zmq::socket_t subscriber;

public:
    ZeroMQP2PConsumer(const Logger& logger);
    ~ZeroMQP2PConsumer();

    void initialize() override;
    void subscribe(const std::string &topic) override;
    Payload receive_message() override;
};

#endif // ZEROMQP2P_CONSUMER_HPP
