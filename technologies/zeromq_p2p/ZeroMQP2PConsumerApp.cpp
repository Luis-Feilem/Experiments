#include "../../core/interfaces/IConsumerApp.hpp"

class ZeroMQP2PConsumerApp : public IConsumerApp {};

int main() {
    std::cout << "[ZeroMQConsumerApp] Start" << std::endl << std::flush;

    try {
        ZeroMQP2PConsumerApp app;
        std::cout << "[ZeroMQP2PConsumerApp] Creating consumer" << std::endl;
        app.create_consumer();
        std::cout << "[ZeroMQP2PConsumerApp] Running consumer" << std::endl;
        app.run();
        std::cout << "[ZeroMQP2PConsumerApp] Finished execution" << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "[ZeroMQP2PPublisherApp] Exception caught: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "[ZeroMQP2PPublisherApp] Unknown exception caught!" << std::endl;
    }
    
    std::cout << "[ZeroMQP2PConsumerApp] End" << std::endl << std::flush;
    return 0;
}

