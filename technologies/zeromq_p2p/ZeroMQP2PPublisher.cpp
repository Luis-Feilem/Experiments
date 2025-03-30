#include "ZeroMQP2PPublisher.hpp"
#include <iostream>
#include <thread>
#include <cstdlib>
#include <sstream>
#include "PublisherFactory.hpp"

namespace {
    struct Register {
        Register() {
            PublisherFactory::registerPublisher("zeromq_p2p", [](Logger console) -> std::unique_ptr<IPublisher> {
                return std::make_unique<ZeroMQP2PPublisher>(console);
            });
        }
    };

    static Register reg;
}


inline std::vector<char> serialize_payload_with_topic(const std::string& topic, const Payload& payload) {
    std::vector<char> buffer;

    // 1. Prefix with topic length (1 byte)
    uint8_t topic_len = static_cast<uint8_t>(topic.size());
    buffer.push_back(topic_len);

    // 2. Topic bytes
    buffer.insert(buffer.end(), topic.begin(), topic.end());

    // 3. Label length
    uint8_t label_len = static_cast<uint8_t>(payload.label.size());
    buffer.push_back(label_len);

    // 4. Label content
    buffer.insert(buffer.end(), payload.label.begin(), payload.label.end());

    // 5. Double values
    const char* values_bytes = reinterpret_cast<const char*>(payload.values.data());
    buffer.insert(buffer.end(), values_bytes, values_bytes + payload.values.size() * sizeof(double));

    return buffer;
}

ZeroMQP2PPublisher::ZeroMQP2PPublisher(const Logger& logger)
    try : IPublisher(logger), context(1), publisher(context, ZMQ_PUB) {
        console.log_debug("[ZeroMQP2P Publisher] Constructor finished");
    } catch (const zmq::error_t& e) {
        console.log_error("[ZeroMQP2P Publisher] Constructor failed: " + std::string(e.what()));
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
        endpoint = "tcp://0.0.0.0:5555";  
    }
    else{
        endpoint = "tcp://" + std::string(std::getenv("PUBLISHER_ENDPOINT")) + ":5555";
    }

    console.log_debug("[ZeroMQP2P Publisher] Binding to " + endpoint);
    try {
        publisher.bind(endpoint);
        std::this_thread::sleep_for(std::chrono::milliseconds(2000)); // wait for consumer to start and connect
        console.log_debug("[ZeroMQP2P Publisher] Bound to " + endpoint);
    } catch (const zmq::error_t &e) {
        console.log_error("[ZeroMQP2P Publisher] Initialization failed: " + std::string(e.what()));
    }
}

void ZeroMQP2PPublisher::send_message(const Payload &message) {
    const char* topics = std::getenv("TOPICS");
    if (!topics) {
        throw std::runtime_error("TOPICS environment variable not set.");
    }

    std::istringstream ss(topics);
    std::string topic;
    while (std::getline(ss, topic, ',')) {
        try {
            // std::string full_message = topic + " " + message;
            // zmq::message_t zmq_message(full_message.begin(), full_message.end());
            std::vector<char> buffer = serialize_payload_with_topic(topic, message);
            // Debug print to make sure serialization worked:
            console.log_debug("[ZeroMQP2P Publisher] Serialized buffer size: " + std::to_string(buffer.size()));

            if (buffer.empty()) {
                console.log_error("[ZeroMQP2P Publisher] Buffer is EMPTY after serialization!");
            }
            std::ostringstream hex_out;
            for (char c : buffer) {
                hex_out << std::hex << std::setw(2) << std::setfill('0') << (static_cast<int>(c) & 0xff) << " ";
            }
            console.log_debug("[ZeroMQP2P Publisher] Serialized bytes: " + hex_out.str());

            zmq::message_t zmq_message(buffer.begin(), buffer.end());
            console.log_info("[ZeroMQP2P Publisher] [" + topic + "] "+ std::to_string(zmq_message.size()) + " B"); 
            publisher.send(zmq_message, zmq::send_flags::none);
            console.log_debug("[ZeroMQP2P Publisher] Socket connected clients: " + publisher.get(zmq::sockopt::events));
        } catch (const zmq::error_t &e) {
            console.log_error("[ZeroMQP2P Publisher] Send failed: " + std::string(e.what()));
        }
    }
}