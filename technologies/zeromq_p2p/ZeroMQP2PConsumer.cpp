#include "ZeroMQP2PConsumer.hpp"
#include <iostream>
#include <thread>
#include <cstdlib>
#include <sstream>
#include "ConsumerFactory.hpp"

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
    : IConsumer(Logger::LogLevel::INFO), context(1), subscriber(context, ZMQ_SUB) {
}

ZeroMQP2PConsumer::~ZeroMQP2PConsumer() {
    subscriber.close();
    context.close();
}

void ZeroMQP2PConsumer::initialize() {
    const char* vendpoint = std::getenv("CONSUMER_ENDPOINT");
    const char* vtopics = std::getenv("TOPICS");
    std::string endpoint = "";
    std::string topics = "";
    std::string consumer_id = std::getenv("CONTAINER_ID");
    if (!vendpoint) {
        // throw std::runtime_error("PUBLISHER_ENDPOINT environment variable not set.");
        endpoint = "tcp://zeromq_p2p_P" + consumer_id.substr(1) + ":5555";
        console.log_debug("[ZeroMQP2P Consumer] CONSUMER_ENDPOINT not set, default to publisher with same numerical id: " + endpoint);
    }
    else{
        endpoint = "tcp://" + std::string(std::getenv("CONSUMER_ENDPOINT")) + ":5555";
    }
    if (!vtopics) {
        topics = consumer_id.substr(1);
        throw std::runtime_error("TOPICS environment variable not set, default to same id as the consumer: ");
    }
    else {
        topics = vtopics;
    }

    console.log_debug("[ZeroMQP2P Consumer] Connecting to " + endpoint);
    try {
        subscriber.connect(endpoint);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Subscribe to multiple topics
        std::istringstream ss(topics);
        std::string topic;
        while (std::getline(ss, topic, ',')) {
            subscribe(topic);
        }
        console.log_debug("[ZeroMQP2P Consumer] Connected and subscribed.");
    } catch (const zmq::error_t &e) {
        console.log_error("[ZeroMQP2P Consumer] Initialization failed: " + std::string(e.what()));
    }
}

void ZeroMQP2PConsumer::subscribe(const std::string &topic) {
    console.log_debug("[ZeroMQP2P Consumer] Subscribing to topic: " + topic);
    subscriber.set(zmq::sockopt::subscribe, topic);
    // Set a timeout for receiving messages (10s)
    subscriber.set(zmq::sockopt::rcvtimeo, 10000);
}

std::string ZeroMQP2PConsumer::receive_message() {
    zmq::message_t zmq_message;
    try {
        auto result = subscriber.recv(zmq_message, zmq::recv_flags::none);
        if (!result) {
            console.log_error("[ZeroMQP2P Consumer] Failed to receive message!");
            return "";
        }

        std::string message(static_cast<char *>(zmq_message.data()), zmq_message.size());
        size_t space_pos = message.find(' ');
        std::string topic = message.substr(0, space_pos);
        std::string payload = message.substr(space_pos + 1);

        console.log_info("[ZeroMQP2P Consumer] Received on topic: " + topic + " -> " + payload);;

        // Handle poison pill termination
        if (payload == "__END__") {
            console.log_info("[ZeroMQP2P Consumer] Received termination signal. Stopping.");
            exit(0);
        }

        return payload;
    } catch (const zmq::error_t &e) {
        console.log_error("[ZeroMQP2P Consumer] Receive failed: " + std::string(e.what()));
        return "";
    }
}