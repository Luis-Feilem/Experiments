#include "ZeroMQP2PConsumer.hpp"
#include <iostream>
#include <thread>
#include <cstdlib>
#include <sstream>
#include "ConsumerFactory.hpp"

inline Payload deserialize_payload(const void* data, size_t size) {
    const char* byte_data = static_cast<const char*>(data);

    if (size < 1) throw std::runtime_error("Message too small to contain payload");

    // Step 1: Read label length
    uint8_t label_len = static_cast<uint8_t>(byte_data[0]);

    if (size < 1 + label_len) throw std::runtime_error("Payload size mismatch with label");

    // Step 2: Extract label
    std::string label(byte_data + 1, label_len);

    // Step 3: Extract values
    size_t values_start = 1 + label_len;
    size_t remaining_bytes = size - values_start;
    size_t num_doubles = remaining_bytes / sizeof(double);

    std::vector<double> values(num_doubles);
    std::memcpy(values.data(), byte_data + values_start, num_doubles * sizeof(double));

    return Payload{label, values};
}

inline Payload deserialize_payload_with_topic(const void* data, size_t size, std::string& out_topic) {
    const char* byte_data = static_cast<const char*>(data);

    if (size < 1) throw std::runtime_error("Message too short to contain topic length");

    // 1. Topic length
    uint8_t topic_len = static_cast<uint8_t>(byte_data[0]);

    if (size < 1 + topic_len + 1)
        throw std::runtime_error("Message too short after topic");

    // 2. Topic string
    out_topic = std::string(byte_data + 1, topic_len);

    // 3. Shift pointer to actual payload
    const char* payload_data = byte_data + 1 + topic_len;
    size_t payload_size = size - (1 + topic_len);

    return deserialize_payload(payload_data, payload_size);
}

namespace {
    struct Register {
        Register() {
            ConsumerFactory::registerConsumer("zeromq_p2p", [](Logger console) -> std::unique_ptr<IConsumer> {
                return std::make_unique<ZeroMQP2PConsumer>(console);
            });
        }
    };

    static Register reg;
}

ZeroMQP2PConsumer::ZeroMQP2PConsumer(const Logger& logger)
    try : IConsumer(logger), context(1), subscriber(context, ZMQ_SUB) {
        console.log_debug("[ZeroMQP2P Consumer] Constructor finished");
    } catch (const zmq::error_t& e) {
        std::cerr << "[ZeroMQP2P Consumer] Constructor failed: " << e.what() << std::endl;
}

ZeroMQP2PConsumer::~ZeroMQP2PConsumer() {
    subscriber.close();
    context.close();
}

void ZeroMQP2PConsumer::initialize() {
    console.log_debug("[ZeroMQP2P Consumer] initializing...");
    const char* vendpoint = std::getenv("CONSUMER_ENDPOINT");
    const char* vtopics = std::getenv("TOPICS");
    std::set<std::string> unique_publishers;
    std::string consumer_id = std::getenv("CONTAINER_ID");
    if (!vendpoint) {
        unique_publishers.insert("zeromq_p2p-P" + consumer_id.substr(1));
        console.log_debug("[ZeroMQP2P Consumer] CONSUMER_ENDPOINT not set, default to publisher with same numerical id: zeromq_p2p-P" + consumer_id.substr(1));
    }
    else{
        std::istringstream publishers(vendpoint);
        std::string publisher;
        while(std::getline(publishers, publisher, ',')){
            if(!publisher.empty()){
                unique_publishers.insert(publisher);
            }
        }
    }
    if (!vtopics) {
        subscribed_topics.insert(consumer_id.substr(1));
        console.log_debug("[ZeroMQP2P Consumer] TOPICS not set, default to publisher with same numerical id: " + consumer_id.substr(1));
    }
    else{
        std::istringstream topics(vtopics);
        std::string topic;
        while(std::getline(topics, topic, ',')){
            if(!topic.empty()){
                subscribed_topics.insert(topic);
                // could move the logic to subscribe to topic here?
            }
        }
    }

    try {
        for (const auto& topic : subscribed_topics){
            console.log_debug("[ZeroMQP2P Consumer] Subscribing to topic " + topic);
            subscribe(topic);
        }
        for(const auto& publisher : unique_publishers){
            console.log_debug("[ZeroMQP2P Consumer] Connecting to publisher " + publisher);
            subscriber.connect("tcp://" + publisher + ":5555");
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        console.log_debug("[ZeroMQP2P Consumer] Connected and subscribed.");
    } catch (const zmq::error_t &e) {
        console.log_error("[ZeroMQP2P Consumer] Initialization failed: " + std::string(e.what()));
    }
}

void ZeroMQP2PConsumer::subscribe(const std::string &topic) {
    console.log_debug("[ZeroMQP2P Consumer] Subscribing to topic: " + topic);
    subscriber.set(zmq::sockopt::subscribe, std::string(1, static_cast<char>(topic.size())) + topic);
    // Set a timeout for receiving messages (10s)
    subscriber.set(zmq::sockopt::rcvtimeo, 10000);
}

Payload ZeroMQP2PConsumer::receive_message() {
    zmq::message_t zmq_message;
    Payload payload = {"",{}};
    try {
        auto result = subscriber.recv(zmq_message, zmq::recv_flags::none);
        if (!result) {
            console.log_error("[ZeroMQP2P Consumer] Failed to receive message!");
            return payload;
        }

        const unsigned char* bytes = static_cast<const unsigned char*>(zmq_message.data());
        std::ostringstream hex_out;
        for (size_t i = 0; i < zmq_message.size(); ++i) {
            hex_out << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(bytes[i]) << " ";
        }
        console.log_debug("[ZeroMQP2P Consumer] Message bytes: " + hex_out.str());

        // std::string message(static_cast<char *>(zmq_message.data()), zmq_message.size());
        // size_t space_pos = message.find(' ');
        // std::string topic = message.substr(0, space_pos);
        // std::string payload = message.substr(space_pos + 1);
        std::string topic;
        payload = deserialize_payload_with_topic(zmq_message.data(), zmq_message.size(), topic);

        console.log_info("[ZeroMQP2P Consumer] Received " + std::to_string(static_cast<unsigned long long> (zmq_message.size())) + "B in topic " + topic);
        if (payload.label == "__END__") {
            terminated_topics.insert(topic);
            console.log_info("[ZeroMQP2P Consumer] Received termination for topic: " + topic);
    
            if (terminated_topics.size() == subscribed_topics.size()) {
                console.log_info("[ZeroMQP2P Consumer] All publishers terminated.");
                return Payload{"__END__", {}};  // Final poison pill
            } else {
                // Not ready to stop yet, keep listening
                return Payload{"__ENDTOPIC__", {}};  // Signal to ignore and continue
            }
        }

        return payload;

    } catch (const zmq::error_t &e) {
        console.log_error("[ZeroMQP2P Consumer] Receive failed: " + std::string(e.what()));
        return payload;
    }
}