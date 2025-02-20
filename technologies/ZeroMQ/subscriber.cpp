#include <zmq.hpp>
#include <iostream>

int main() {
    std::cout << "Starting ZeroMQ Subscriber..." << std::endl;
    
    zmq::context_t context(1);
    zmq::socket_t subscriber(context, ZMQ_SUB);
    
    try {
        subscriber.connect("tcp://localhost:5555"); // Connect to the publisher
        subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0); // Subscribe to all messages
        std::cout << "Subscriber connected to tcp://localhost:5555" << std::endl;
    } catch (const zmq::error_t& e) {
        std::cerr << "Error connecting subscriber: " << e.what() << std::endl;
        return 1;
    }

    while (true) {
        zmq::message_t message;
        subscriber.recv(message, zmq::recv_flags::none);

        std::string receivedMessage(static_cast<char*>(message.data()), message.size());
        std::cout << "Received: " << receivedMessage << std::endl;
    }

    return 0;
}
