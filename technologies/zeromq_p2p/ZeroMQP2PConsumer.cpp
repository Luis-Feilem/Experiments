#include "ZeroMQP2PConsumer.hpp"
#include <iostream>
#include <thread>
#include <cstdlib>
#include <sstream>
#include "ConsumerFactory.hpp"

Payload ZeroMQP2PConsumer::deserialize(const std::string& raw_message) {
    const char* data = raw_message.data();
    size_t offset = 0;

    // 1. Topic length and content (skip over it)
    uint8_t topic_len = static_cast<uint8_t>(data[offset]);
    offset += 1;

    if (raw_message.size() < offset + topic_len) {
        throw std::runtime_error("Invalid message: incomplete topic");
    }

    std::string topic(data + offset, topic_len);
    offset += topic_len;

    // 2. Message ID length and content
    uint16_t id_len;
    std::memcpy(&id_len, data + offset, sizeof(uint16_t));
    offset += sizeof(uint16_t);

    if (raw_message.size() < offset + id_len) {
        throw std::runtime_error("Invalid message: incomplete message_id");
    }

    std::string message_id(data + offset, id_len);
    offset += id_len;

    // 3. Data size
    uint64_t data_size;
    std::memcpy(&data_size, data + offset, sizeof(uint64_t));
    offset += sizeof(uint64_t);

    if (raw_message.size() < offset + data_size) {
        throw std::runtime_error("Invalid message: data section incomplete");
    }

    std::vector<uint8_t> payload_data(data + offset, data + offset + data_size);

    Payload p;
    p.message_id = message_id;
    p.data_size = data_size;
    p.data = std::move(payload_data);
    p.kind = PayloadKind::FLAT; // Hardcoded until dynamic handling is needed

    return p;
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
        if (!vtopics) {
            subscribed_streams.insert({"zeromq_p2p-P" + consumer_id.substr(1), consumer_id.substr(1)});
            subscribe(consumer_id.substr(1));
            console.log_debug("[ZeroMQP2P Consumer] TOPICS not set, default to publisher with same numerical id: " + consumer_id.substr(1));
        }
        else{
            std::istringstream topics(vtopics);
            std::string topic;
            std::istringstream publishers(vendpoint);
            std::string publisher;
            // console.log_debug("[ZeroMQP2P Consumer] Subscribing to topic list: " + std::to_string(vtopics));
            while(std::getline(topics, topic, ',')){
                std::getline(publishers, publisher, ',');
                if(!publisher.empty()){
                    unique_publishers.insert(publisher);
                }
                console.log_debug("[ZeroMQP2P Consumer] Handling subscription to topic " + topic);
                if(!topic.empty()){
                    console.log_info("[ZeroMQP2P Consumer] Connecting to stream ("+ publisher + "," + topic + ")");
                    subscribe(topic);
                    subscribed_streams.insert({publisher, topic});
                }
            }
        }
    }

    try {
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
    Payload message;

    try {
        auto result = subscriber.recv(zmq_message, zmq::recv_flags::none);
        if (!result) {
            console.log_error("[ZeroMQP2P Consumer] Failed to receive message!");
            return message;
        }

        std::string raw(static_cast<const char*>(zmq_message.data()), zmq_message.size());
        message = deserialize(raw);

        console.log_study("[ZeroMQP2P Consumer] Received message ID: " + message.message_id +
                         ", Size: " + std::to_string(message.data_size) + " bytes");

        // Poison pill handling based on ID
        if (message.message_id.find("__END__") != std::string::npos) {
            std::string source = message.message_id.substr(0, message.message_id.find(":"));
            std::string topic = message.message_id.substr(message.message_id.find(":") + 1);
            terminated_streams.insert({source, topic});

            console.log_info("[ZeroMQP2P Consumer] Termination signal from source: " + source +
                             " on topic: " + topic);
            console.log_debug("[ZeroMQP2P Consumer] Streams closed: " +
                              std::to_string(terminated_streams.size()) + "/" +
                              std::to_string(subscribed_streams.size()));

            if (terminated_streams.size() == subscribed_streams.size()) {
                return Payload{"__END__", {}};  // Final poison pill
            } else {
                return Payload{"__ENDTOPIC__", {}};  // Intermediate termination
            }
        }

        return message;

    } catch (const zmq::error_t& e) {
        console.log_error("[ZeroMQP2P Consumer] Receive failed: " + std::string(e.what()));
        return message;
    }
}
