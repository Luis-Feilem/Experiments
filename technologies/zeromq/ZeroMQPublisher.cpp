#include "ZeroMQPublisher.hpp"
#include <iostream>
#include <thread>

ZeroMQPublisher::ZeroMQPublisher()
    : context(1), publisher(context, ZMQ_PUB) {
        std::cerr << "Constructing Publisher..." << std::endl << std::flush;
    }

ZeroMQPublisher::~ZeroMQPublisher() {
    publisher.close();
    context.close();
}

void ZeroMQPublisher::initialize(const std::string &endpoint) {
    std::cout << "[ZeroMQ Publisher] Binding to " << endpoint << std::endl << std::flush;
    try {
        publisher.bind(endpoint);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "[ZeroMQ Publisher] Bound to " << endpoint << std::endl << std::flush;
    } catch (const zmq::error_t &e) {
        std::cerr << "[ZeroMQ Publisher] Initialization failed: " << e.what() << std::endl << std::flush;
    }
}

void ZeroMQPublisher::send_message(const std::string &message, const std::string &topic) {
    try {
        std::cout << "[ZeroMQ Publisher] Sending: " << message << " [END] to topic: " << topic <<std::endl << std::flush;
        std::string full_message = topic + " " + message;
        zmq::message_t zmq_message(full_message.begin(), full_message.end());
        publisher.send(zmq_message, zmq::send_flags::none);
        std::cout << "[ZeroMQ Publisher] Sent: " << message << std::endl << std::flush;
    } catch (const zmq::error_t &e) {
        std::cerr << "[ZeroMQ Publisher] Send failed: " << e.what() << std::endl << std::flush;
    }
}
