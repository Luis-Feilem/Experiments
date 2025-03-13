#include "../../core/interfaces/IPublisherApp.hpp"

class ZeroMQPublisherApp : public IPublisherApp {};

int main() {
    std::cout << "[ZeroMQP2PPublisherApp] Start" << std::endl << std::flush;
    try {
        ZeroMQPublisherApp app;
        std::cout << "[ZeroMQP2PPublisherApp] Creating publisher" << std::endl;
        app.create_publisher();
        std::cout << "[ZeroMQP2PPublisherApp] Running publisher" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        app.run();
        std::cout << "[ZeroMQP2PPublisherApp] Finished execution" << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "[ZeroMQP2PPublisherApp] Exception caught: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "[ZeroMQP2PPublisherApp] Unknown exception caught!" << std::endl;
    }
    std::cout << "[ZeroMQP2PPublisherApp] End" << std::endl << std::flush;
    return 0;
}
