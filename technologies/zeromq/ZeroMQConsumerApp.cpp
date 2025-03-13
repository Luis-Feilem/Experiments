#include "../../core/interfaces/IConsumerApp.hpp"

class ZeroMQConsumerApp : public IConsumerApp {};

int main() {
    std::cout << "[ZeroMQConsumerApp] Start" << std::endl << std::flush;

    try {
        ZeroMQConsumerApp app;
        std::cout << "[ZeroMQConsumerApp] Creating consumer" << std::endl;
        app.create_consumer();
        std::cout << "[ZeroMQConsumerApp] Running consumer" << std::endl;
        app.run();
        std::cout << "[ZeroMQConsumerApp] Finished execution" << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "[ZeroMQPublisherApp] Exception caught: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "[ZeroMQPublisherApp] Unknown exception caught!" << std::endl;
    }
    
    std::cout << "[ZeroMQConsumerApp] End" << std::endl << std::flush;
    return 0;
}

