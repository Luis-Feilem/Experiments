#include "ZeroMQP2PPublisher.hpp"
#include <iostream>
#include <thread>
#include <cstdlib>
#include <sstream>

ZeroMQP2PPublisher::ZeroMQP2PPublisher()
    : context(1), publisher(context, ZMQ_PUB) {
    std::cerr << "Constructing Publisher..." << std::endl << std::flush;
}

ZeroMQP2PPublisher::~ZeroMQP2PPublisher() {
    publisher.close();
    context.close();
}

void ZeroMQP2PPublisher::initialize() {
    const char* vendpoint = std::getenv("PUBLISHER_ENDPOINT");
    if (!vendpoint) {
        throw std::runtime_error("PUBLISHER_ENDPOINT environment variable not set.");
    }

    std::string endpoint = "tcp://" + std::string(std::getenv("PUBLISHER_ENDPOINT")) + ":5555";
    std::cout << "[ZeroMQP2PPublisher] Binding to " << endpoint << std::endl;
    try {
        publisher.bind(endpoint);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "[ZeroMQP2PPublisher] Bound to " << endpoint << std::endl;
    } catch (const zmq::error_t &e) {
        std::cerr << "[ZeroMQP2PPublisher] Initialization failed: " << e.what() << std::endl;
    }
}

void ZeroMQP2PPublisher::send_message(const std::string &message) {
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
            std::cout << "[ZeroMQP2PPublisher] Sent to topic: " << topic << std::endl;
            std::cout << "[Publisher] Socket connected clients: " << publisher.getsockopt<int>(ZMQ_EVENTS) << std::endl;
        } catch (const zmq::error_t &e) {
            std::cerr << "[ZeroMQP2PPublisher] Send failed: " << e.what() << std::endl;
        }
    }
}