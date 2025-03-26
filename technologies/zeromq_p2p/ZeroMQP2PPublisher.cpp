#include "ZeroMQP2PPublisher.hpp"
#include <iostream>
#include <thread>
#include <cstdlib>
#include <sstream>
#include "PublisherFactory.hpp"

namespace {
    struct Register {
        Register() {
            PublisherFactory::registerPublisher("zeromq_p2p", []() -> std::unique_ptr<IPublisher> {
                return std::make_unique<ZeroMQP2PPublisher>();
            });
        }
    };

    static Register reg;
}

ZeroMQP2PPublisher::ZeroMQP2PPublisher()
    : context(1), publisher(context, ZMQ_PUB) {
}

ZeroMQP2PPublisher::~ZeroMQP2PPublisher() {
    publisher.close();
    context.close();
}

void ZeroMQP2PPublisher::initialize() {
    console.log_debug("[ZeroMQP2P Publisher] initializing");
    const char* vendpoint = std::getenv("PUBLISHER_ENDPOINT");
    std::string endpoint = "";
    if (!vendpoint) {
        console.log_debug("[ZeroMQP2P Publisher] PUBLISHER_ENDPOINT not set, default to 0.0.0.0");
        // throw std::runtime_error("PUBLISHER_ENDPOINT environment variable not set.");
        endpoint = "0.0.0.0:5555";  
    }
    else{
        endpoint = "tcp://" + std::string(std::getenv("PUBLISHER_ENDPOINT")) + ":5555";
    }

    console.log_debug("[ZeroMQP2P Publisher] Binding to " + endpoint);
    try {
        publisher.bind(endpoint);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        console.log_debug("[ZeroMQP2P Publisher] Bound to " + endpoint);
    } catch (const zmq::error_t &e) {
        console.log_error("[ZeroMQP2P Publisher] Initialization failed: " + std::string(e.what()));
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
            console.log_info("[ZeroMQP2P Publisher] Sent to topic: " + topic);
            console.log_debug("[ZeroMQP2P Publisher] Socket connected clients: " + publisher.get(zmq::sockopt::events));
        } catch (const zmq::error_t &e) {
            console.log_error("[ZeroMQP2P Publisher] Send failed: " + std::string(e.what()));
        }
    }
}