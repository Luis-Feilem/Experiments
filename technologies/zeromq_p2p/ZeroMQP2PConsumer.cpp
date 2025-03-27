#include "ZeroMQP2PConsumer.hpp"
#include <iostream>
#include <thread>
#include <cstdlib>
#include <sstream>
#include "ConsumerFactory.hpp"

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
    std::set<std::string> unique_topics;
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
        unique_topics.insert(consumer_id.substr(1));
        console.log_debug("[ZeroMQP2P Consumer] TOPICS not set, default to publisher with same numerical id: " + consumer_id.substr(1));
    }
    else{
        std::istringstream topics(vtopics);
        std::string topic;
        while(std::getline(topics, topic, ',')){
            if(!topic.empty()){
                unique_topics.insert(topic);
            }
        }
    }

    try {
        for (const auto& topic : unique_topics){
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