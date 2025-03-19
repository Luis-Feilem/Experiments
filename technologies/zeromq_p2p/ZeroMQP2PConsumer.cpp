#include "ZeroMQP2PConsumer.hpp"
#include <iostream>
#include <thread>
#include <cstdlib>
#include <sstream>
#include "../../core/factory/ConsumerFactory.hpp"

namespace {
    struct Register {
        Register() {
            ConsumerFactory::registerConsumer("zeromq_p2p", []() -> std::unique_ptr<IConsumer> {
                return std::make_unique<ZeroMQP2PConsumer>();
            });
        }
    };

    static Register reg;
}

ZeroMQP2PConsumer::ZeroMQP2PConsumer()
    : context(1), subscriber(context, ZMQ_SUB) {
    std::cerr << "Constructing Consumer..." << std::endl << std::flush;
}

ZeroMQP2PConsumer::~ZeroMQP2PConsumer() {
    subscriber.close();
    context.close();
}

void ZeroMQP2PConsumer::initialize() {
    const char* vendpoint = std::getenv("CONSUMER_ENDPOINT");
    const char* topics = std::getenv("TOPICS");
    if (!vendpoint || !topics) {
        throw std::runtime_error("CONSUMER_ENDPOINT or TOPICS environment variable not set.");
    }
    std::string endpoint = "tcp://" + std::string(std::getenv("CONSUMER_ENDPOINT")) + ":5555";

    std::cout << "[ZeroMQP2P Consumer] Connecting to " << endpoint << std::endl;
    try {
        subscriber.connect(endpoint);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Subscribe to multiple topics
        std::istringstream ss(topics);
        std::string topic;
        while (std::getline(ss, topic, ',')) {
            subscribe(topic);
        }
        std::cout << "[ZeroMQP2P Consumer] Connected and subscribed." << std::endl;
    } catch (const zmq::error_t &e) {
        std::cerr << "[ZeroMQP2P Consumer] Initialization failed: " << e.what() << std::endl;
    }
}

void ZeroMQP2PConsumer::subscribe(const std::string &topic) {
    std::cout << "[ZeroMQP2P Consumer] Subscribing to topic: " << topic << std::endl;
    subscriber.set(zmq::sockopt::subscribe, topic);
    // Set a timeout for receiving messages (10s)
    subscriber.set(zmq::sockopt::rcvtimeo, 10000);
}

std::string ZeroMQP2PConsumer::receive_message() {
    zmq::message_t zmq_message;
    try {
        auto result = subscriber.recv(zmq_message, zmq::recv_flags::none);
        if (!result) {
            std::cerr << "[ZeroMQP2P Consumer] Failed to receive message!" << std::endl;
            return "";
        }

        std::string message(static_cast<char *>(zmq_message.data()), zmq_message.size());
        size_t space_pos = message.find(' ');
        std::string topic = message.substr(0, space_pos);
        std::string payload = message.substr(space_pos + 1);

        std::cout << "[ZeroMQP2P Consumer] Received on topic: " << topic << " -> " << payload << std::endl;

        // Handle poison pill termination
        if (payload == "__END__") {
            std::cout << "[ConsumerApp] Received termination signal. Stopping." << std::endl;
            exit(0);
        }

        return payload;
    } catch (const zmq::error_t &e) {
        std::cerr << "[ZeroMQP2P Consumer] Receive failed: " << e.what() << std::endl;
        return "";
    }
}