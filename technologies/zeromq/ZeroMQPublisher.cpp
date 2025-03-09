#include "ZeroMQPublisher.hpp"
#include <iostream>

ZeroMQPublisher::ZeroMQPublisher()
    : context(1), publisher(context, ZMQ_PUB) {
        std::cerr << "Constructing Publisher..." << std::endl << std::flush;
    }

ZeroMQPublisher::~ZeroMQPublisher() {
    publisher.close();
    context.close();
}

void ZeroMQPublisher::initialize(const std::string &endpoint) {
    try {
        publisher.bind(endpoint);
        std::cout << "[ZeroMQ Publisher] Bound to " << endpoint << std::endl << std::flush;
    } catch (const zmq::error_t &e) {
        std::cerr << "[ZeroMQ Publisher] Initialization failed: " << e.what() << std::endl << std::flush;
    }
}

void ZeroMQPublisher::send_message(const std::string &message) {
    zmq::message_t zmq_message(message.begin(), message.end());
    try {
        publisher.send(zmq_message, zmq::send_flags::none);
        std::cout << "[ZeroMQ Publisher] Sent: " << message << std::endl << std::flush;
    } catch (const zmq::error_t &e) {
        std::cerr << "[ZeroMQ Publisher] Send failed: " << e.what() << std::endl << std::flush;
    }
}
