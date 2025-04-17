#include "ZeroMQP2PPublisher.hpp"
#include <iostream>
#include <thread>
#include <cstdlib>
#include <sstream>
#include "PublisherFactory.hpp"

std::string ZeroMQP2PPublisher::serialize(const Payload& message){
    console.log_error("[ZeroMQP2P Publisher] serialize called without topic. This should not happen.");
    return "ERROR";
}

std::string ZeroMQP2PPublisher::serialize(const Payload& message, std::string topic) {
    std::vector<char> buffer;

    // 1. Topic length and content
    uint8_t topic_len = static_cast<uint8_t>(topic.size());
    buffer.push_back(topic_len);
    buffer.insert(buffer.end(), topic.begin(), topic.end());

    // 2. Message ID length and content
    uint16_t id_len = static_cast<uint16_t>(message.message_id.size());
    buffer.insert(buffer.end(),
                  reinterpret_cast<const char*>(&id_len),
                  reinterpret_cast<const char*>(&id_len) + sizeof(id_len));
    buffer.insert(buffer.end(), message.message_id.begin(), message.message_id.end());

    // 3. Data size
    uint64_t data_size = static_cast<uint64_t>(message.data_size);
    buffer.insert(buffer.end(),
                  reinterpret_cast<const char*>(&data_size),
                  reinterpret_cast<const char*>(&data_size) + sizeof(data_size));

    // 4. Raw data
    buffer.insert(buffer.end(), message.data.begin(), message.data.end());

    return std::string(buffer.begin(), buffer.end());
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
        console.log_debug("[ZeroMQP2P Publisher] Bound to " + endpoint);
    } catch (const zmq::error_t &e) {
        console.log_error("[ZeroMQP2P Publisher] Initialization failed: " + std::string(e.what()));
    }
    log_configuration();
}

void ZeroMQP2PPublisher::send_message(const Payload& message, std::string topic) {
    try {
        std::string raw = serialize(message, topic);

        console.log_debug("[ZeroMQP2P Publisher] Serialized payload ID: " + message.message_id + 
                          " and size: " + std::to_string(message.data_size) + " bytes");

        zmq::message_t zmq_message(raw.begin(), raw.end());
        publisher.send(zmq_message, zmq::send_flags::none);

        console.log_info("[ZeroMQP2P Publisher] Sent " + std::to_string(zmq_message.size()) +
                         " B on topic " + topic);
        console.log_debug("[ZeroMQP2P Publisher] Socket connected clients: " + publisher.get(zmq::sockopt::events));

    } catch (const zmq::error_t& e) {
        console.log_error("[ZeroMQP2P Publisher] Send failed: " + std::string(e.what()));
    }
}

void ZeroMQP2PPublisher::log_configuration(){
    console.log_info("[CONFIG_BEGIN]");

    console.log_info("[CONFIG] socket_type=ZMQ_PUB");
    console.log_info("[CONFIG] endpoint=" + endpoint );

    // Common socket options
    int hwm, linger, snd_buffer;
    size_t sz = sizeof(int);

    zmq_getsockopt(publisher, ZMQ_SNDHWM, &hwm, &sz);
    zmq_getsockopt(publisher, ZMQ_LINGER, &linger, &sz);
    zmq_getsockopt(publisher, ZMQ_SNDTIMEO, &snd_buffer, &sz);

    console.log_info("[CONFIG] ZMQ_SNDHWM=" + std::to_string(hwm));
    console.log_info("[CONFIG] ZMQ_LINGER=" + std::to_string(linger));
    console.log_info("[CONFIG] ZMQ_SNDBUF=" + std::to_string(snd_buffer));

    int major, minor, patch;
    zmq_version(&major, &minor, &patch);
    console.log_info("[CONFIG] zmq_version=" + std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch));

    console.log_info("[CONFIG_END]");
}