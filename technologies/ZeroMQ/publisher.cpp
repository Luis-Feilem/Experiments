#include <zmq.hpp>
#include <iostream>
#include <chrono>
#include <thread>

int main() {
    std::cout << "Starting ZeroMQ Publisher..." << std::endl;
    
    zmq::context_t context(1);
    zmq::socket_t publisher(context, ZMQ_PUB);
    
    try {
        publisher.bind("tcp://*:5555"); // Bind to port 5555
        std::cout << "Publisher bound to tcp://*:5555" << std::endl;
    } catch (const zmq::error_t& e) {
        std::cerr << "Error binding publisher: " << e.what() << std::endl;
        return 1;
    }

    int count = 1;
    while (true) {
        std::string message = "Hello, ZeroMQ! Count: " + std::to_string(count);
        zmq::message_t zmqMessage(message.begin(), message.end());
        publisher.send(zmqMessage, zmq::send_flags::none);

        std::cout << "Published: " << message << std::endl;
        count++;
        std::this_thread::sleep_for(std::chrono::seconds(1)); // Send message every second
    }

    return 0;
}
