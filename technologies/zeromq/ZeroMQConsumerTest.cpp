#include "ZeroMQConsumer.hpp"
#include <iostream>
#include <chrono>
#include <thread>

int main() {
    std::cout << "Starting Consumer..." << std::endl << std::flush;
    ZeroMQConsumer consumer;
    std::string endpoint = "tcp://localhost:5555";
    std::string topic = "";  // Empty string subscribes to all messages

    bool connected = false;
    for (int attempt = 0; attempt < 5; ++attempt) {
        try {
            consumer.initialize(endpoint, topic);
            connected = true;
            break;
        } catch (...) {
            std::cout << "Connection failed, retrying..." << std::endl << std::flush;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    if (!connected) {
        std::cerr << "[ZeroMQ Consumer] Failed to connect after 5 attempts!" << std::endl << std::flush;
    }

    consumer.initialize(endpoint, topic);

    // Receive test messages
    for (int i = 0; i < 5; ++i) {
        std::string message = consumer.receive_message();
        std::cout << "Received: " << message << std::endl << std::flush;
    }

    return 0;
}
