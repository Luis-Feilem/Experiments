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

    std::string endpoint;

public:
    ZeroMQP2PPublisher(const Logger& logger);
    ~ZeroMQP2PPublisher();

    void initialize() override;
    void send_message(const Payload &message, std::string topic) override;

    std::string serialize(const Payload& message);
    std::string serialize(const Payload& message, std::string topic);

    void log_configuration() override;

protected:
    
};

#endif // ZEROMQ_PUBLISHER_HPP
