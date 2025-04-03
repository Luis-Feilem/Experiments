#ifndef ZEROMQP2P_PUBLISHER_HPP
#define ZEROMQP2P_PUBLISHER_HPP

#include "IPublisher.hpp"
#include <zmq.hpp>
#include <string>
#include "Logger.hpp"

class ZeroMQP2PPublisher : public IPublisher {
private:
    zmq::context_t context;
    zmq::socket_t publisher;

public:
    ZeroMQP2PPublisher(const Logger& logger);
    ~ZeroMQP2PPublisher();

    void initialize() override;
    void send_message(const Payload &message, std::string topic) override;

protected:
    
};

#endif // ZEROMQ_PUBLISHER_HPP
