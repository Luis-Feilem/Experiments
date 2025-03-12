#include "ZeroMQPublisher.hpp"
#include <iostream>
#include <thread>
#include <cstdlib>
#include <sstream>

ZeroMQPublisher::ZeroMQPublisher()
    : context(1), publisher(context, ZMQ_PUB) {
    std::cerr << "Constructing Publisher..." << std::endl << std::flush;
}

ZeroMQPublisher::~ZeroMQPublisher() {
    publisher.close();
    context.close();
}

void ZeroMQPublisher::initialize() {
    const char* endpoint = std::getenv("DOCKER_ENDPOINT");
    if (!endpoint) {
        throw std::runtime_error("DOCKER_ENDPOINT environment variable not set.");
    }

    std::cout << "[ZeroMQ Publisher] Binding to " << endpoint << std::endl;
    try {
        publisher.bind(endpoint);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "[ZeroMQ Publisher] Bound to " << endpoint << std::endl;
    } catch (const zmq::error_t &e) {
        std::cerr << "[ZeroMQ Publisher] Initialization failed: " << e.what() << std::endl;
    }
}

void ZeroMQPublisher::send_message(const std::string &message) {
    const char* topics = std::getenv("TOPICS");
    if (!topics) {
        throw std::runtime_error("TOPICS environment variable not set.");
    }

    std::istringstream ss(topics);
    std::string topic;
    while (std::getline(ss, topic, ',')) {
        try {
            std::string full_message = topic + " " + message;
            zmq::message_t zmq_message(full_message.begin(), full_message.end());
            publisher.send(zmq_message, zmq::send_flags::none);
            std::cout << "[ZeroMQ Publisher] Sent to topic: " << topic << std::endl;
        } catch (const zmq::error_t &e) {
            std::cerr << "[ZeroMQ Publisher] Send failed: " << e.what() << std::endl;
        }
    }
}