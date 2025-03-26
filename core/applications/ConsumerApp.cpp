#include "ConsumerApp.hpp"
#include "cstdlib"


void ConsumerApp::create_consumer() {
    std::string technology = std::getenv("TECHNOLOGY");
    console.log_debug("[ConsumerApp] Creating consumer for technology " + technology);
    
    consumer = ConsumerFactory::create(technology);
    console.log_debug("[ConsumerApp] Created " + technology + " consumer");
    }

// Initializes and runs the consumer logic
void ConsumerApp::run() {
    console.log_debug("[ConsumerApp] Starting consumer");
    std::string endpoint = std::getenv("CONSUMER_ENDPOINT") ?
                            "tcp://" + std::string(std::getenv("CONSUMER_ENDPOINT")) + ":5555" :
                            "tcp://" + std::string(std::getenv("TECHNOLOGY")) + "_broker:5555";

    console.log_debug("[ConsumerApp] Initializing consumer with endpoint: " + endpoint 
                + " and topics: " + topics);
    consumer->initialize();
    console.log_debug("[ConsumerApp] Initialized consumer");

    while (true) {
        console.log_debug("[ConsumerApp] Waiting for message...");
        std::string message = consumer->receive_message();
        if (message == ""){
            console.log_info("[ConsumerApp] Received empty message -> Stopping.");
            break;
        }
        console.log_info("[ConsumerApp] Received: " + message);
        if (message == "__END__") {
            console.log_info("[ConsumerApp] Received termination signal. Stopping.");
            break;
        }
    }
}


int main(int argc, char * argv[]) {
    std::ios::sync_with_stdio(false); // Disable stream buffering
    std::cout << "[ConsumerApp] Start" << std::endl << std::flush;
    try {
        Logger::LogLevel log_level;
        if (argc >= 2 && argv[1] != nullptr){
            log_level = Logger::string_to_level(argv[1]);
        }
        ConsumerApp app = ConsumerApp(log_level);
        app.create_consumer();
        app.run();
    } catch (const std::exception &e) {
        std::cerr << "[ConsumerApp] Exception caught: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "[ConsumerApp] Unknown exception caught!" << std::endl;
    }
    return 0;
}