#ifndef ZEROMQP2P_PUBLISHER_HPP
#define ZEROMQP2P_PUBLISHER_HPP

#include "../../core/interfaces/IPublisher.hpp"
#include <zmq.hpp>
#include <string>
#include "Logger.hpp"

class ZeroMQP2PPublisher : public IPublisher {
private:
    zmq::context_t context;
    zmq::socket_t publisher;
    Logger console;

public:
    ZeroMQP2PPublisher();
    ~ZeroMQP2PPublisher();

    void initialize() override;
    void send_message(const std::string &message) override;
};

#endif // ZEROMQ_PUBLISHER_HPP
