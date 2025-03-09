#include "ZeroMQPublisher.hpp"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    std::cout << "Starting Publisher..." << std::endl << std::flush;
    ZeroMQPublisher publisher;
    std::string endpoint = "tcp://127.0.0.1:5555";
    publisher.initialize(endpoint);
    std::this_thread::sleep_for(std::chrono::seconds(5));

    // Send test messages
    for (int i = 1; i <= 5; ++i) {
        std::string message = "Hello, this is message " + std::to_string(i);
        publisher.send_message(message);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
