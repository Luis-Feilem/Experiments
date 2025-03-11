#include "ZeroMQConsumer.hpp"
#include <iostream>
#include <thread>

ZeroMQConsumer::ZeroMQConsumer()
    : context(1), subscriber(context, ZMQ_SUB) {
        std::cerr << "Constructing Consumer..." << std::endl << std::flush;
    }

ZeroMQConsumer::~ZeroMQConsumer() {
    subscriber.close();
    context.close();
}

void ZeroMQConsumer::initialize(const std::string &endpoint, const std::string &topic) {
    std::cout << "[ZeroMQ Consumer] Connecting to " << endpoint << " and subscribing to topic: " << topic << std::endl;
    try {
        subscriber.connect(endpoint);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        subscribe(topic);
        std::cout << "[ZeroMQ Consumer] Connected to " << endpoint << " and subscribed to topic: " << topic << std::endl;
    } catch (const zmq::error_t &e) {
        std::cerr << "[ZeroMQ Consumer] Initialization failed: " << e.what() << std::endl;
    }
}

void ZeroMQConsumer::subscribe(const std::string &topic) {
    std::cout << "[ZeroMQ Consumer] Subscribing to topic: " << topic << std::endl;
    subscriber.set(zmq::sockopt::subscribe, topic);
}

std::string ZeroMQConsumer::receive_message() {
    zmq::message_t zmq_message;
    try {
        std::cout << "[ZeroMQ Consumer] Waiting for message..." << std::endl << std::flush;
        auto result = subscriber.recv(zmq_message, zmq::recv_flags::none);
        if (!result) {
            std::cerr << "[ZeroMQ Consumer] Failed to receive message!" << std::endl << std::flush;
        }

        std::string message(static_cast<char *>(zmq_message.data()), zmq_message.size());

        // Split out the topic
        size_t space_pos = message.find(' ');
        std::string topic = message.substr(0, space_pos);
        std::string payload = message.substr(space_pos + 1);

        std::cout << "[ZeroMQ Consumer] Received on topic: " << topic << " -> " << payload << std::endl << std::flush;

        // Handle poison pill termination
        if (payload == "__END__") {
            std::cout << "[ConsumerApp] Received termination signal. Stopping." << std::endl;
            exit(0); // Clean exit
        }

        return payload;
    } catch (const zmq::error_t &e) {
        std::cerr << "[ZeroMQ Consumer] Receive failed: " << e.what() << std::endl << std::flush;
        return "";
    }
}
