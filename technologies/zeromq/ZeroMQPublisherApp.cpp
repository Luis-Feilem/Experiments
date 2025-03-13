#include "../../core/interfaces/IPublisherApp.hpp"

class ZeroMQPublisherApp : public IPublisherApp {};

int main() {
    std::cout << "[ZeroMQPublisherApp] Start" << std::endl << std::flush;
    try {
        ZeroMQPublisherApp app;
        std::cout << "[ZeroMQPublisherApp] Creating publisher" << std::endl;
        app.create_publisher();
        std::cout << "[ZeroMQPublisherApp] Running publisher" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        app.run();
        std::cout << "[ZeroMQPublisherApp] Finished execution" << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "[ZeroMQPublisherApp] Exception caught: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "[ZeroMQPublisherApp] Unknown exception caught!" << std::endl;
    }
    std::cout << "[ZeroMQPublisherApp] End" << std::endl << std::flush;
    return 0;
}
