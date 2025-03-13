#include "ZeroMQConsumer.hpp"
#include <iostream>
#include <thread>
#include <cstdlib>
#include <sstream>

ZeroMQConsumer::ZeroMQConsumer()
    : context(1), subscriber(context, ZMQ_SUB) {
    std::cerr << "Constructing Consumer..." << std::endl << std::flush;
}

ZeroMQConsumer::~ZeroMQConsumer() {
    subscriber.close();
    context.close();
}

void ZeroMQConsumer::initialize() {
    const char* network = std::getenv("NETWORK");
    const char* topics = std::getenv("TOPICS");
    if (!network || !topics) {
        throw std::runtime_error("NETWORK or TOPICS environment variable not set.");
    }
    std::string endpoint = "tcp://" + std::string(std::getenv("NETWORK")) + ":5555";

    std::cout << "[ZeroMQ Consumer] Connecting to " << endpoint << std::endl;
    try {
        subscriber.connect(endpoint);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Subscribe to multiple topics
        std::istringstream ss(topics);
        std::string topic;
        while (std::getline(ss, topic, ',')) {
            subscribe(topic);
        }
        std::cout << "[ZeroMQ Consumer] Connected and subscribed." << std::endl;
    } catch (const zmq::error_t &e) {
        std::cerr << "[ZeroMQ Consumer] Initialization failed: " << e.what() << std::endl;
    }
}

void ZeroMQConsumer::subscribe(const std::string &topic) {
    std::cout << "[ZeroMQ Consumer] Subscribing to topic: " << topic << std::endl;
    subscriber.set(zmq::sockopt::subscribe, topic);
    // Set a timeout for receiving messages (10s)
    subscriber.set(zmq::sockopt::rcvtimeo, 10000);
}

std::string ZeroMQConsumer::receive_message() {
    zmq::message_t zmq_message;
    try {
        auto result = subscriber.recv(zmq_message, zmq::recv_flags::none);
        if (!result) {
            std::cerr << "[ZeroMQ Consumer] Failed to receive message!" << std::endl;
            return "";
        }

        std::string message(static_cast<char *>(zmq_message.data()), zmq_message.size());
        size_t space_pos = message.find(' ');
        std::string topic = message.substr(0, space_pos);
        std::string payload = message.substr(space_pos + 1);

        std::cout << "[ZeroMQ Consumer] Received on topic: " << topic << " -> " << payload << std::endl;

        // Handle poison pill termination
        if (payload == "__END__") {
            std::cout << "[ConsumerApp] Received termination signal. Stopping." << std::endl;
            exit(0);
        }

        return payload;
    } catch (const zmq::error_t &e) {
        std::cerr << "[ZeroMQ Consumer] Receive failed: " << e.what() << std::endl;
        return "";
    }
}