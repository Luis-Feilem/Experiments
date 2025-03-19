#include "IConsumerApp.hpp"
#include "cstdlib"


IConsumerApp::IConsumerApp(spdlog::level::level_enum log_level){
    spdlog::set_level(log_level);
    spdlog::debug("[IConsumerApp] Constructing ConsumerApp...");
}

void IConsumerApp::create_consumer() {
    spdlog::debug("[IConsumerApp] Creating consumer");
    std::string technology = std::getenv("TECHNOLOGY");
    if (technology == "zeromq_p2p") {
        consumer = std::make_unique<ZeroMQP2PConsumer>();
        spdlog::debug("[IConsumerApp] Created ZeroMQ consumer");
    } 
    // Extend here for new technologies
    else {
        throw std::runtime_error("Unsupported technology: " + technology);
    }
}

// Initializes and runs the consumer logic
void IConsumerApp::run() {
    spdlog::debug("[IConsumerApp] Starting consumer");
    std::string endpoint = std::getenv("CONSUMER_ENDPOINT") ?
                            "tcp://" + std::string(std::getenv("CONSUMER_ENDPOINT")) + ":5555" :
                            "tcp://127.0.0.1:5555";

    spdlog::debug("[IConsumerApp] Initializing consumer with endpoint: " + endpoint 
                + " and topics: " + topics);
    consumer->initialize();
    spdlog::debug("[IConsumerApp] Initialized consumer");

    while (true) {
        spdlog::debug("[IConsumerApp] Waiting for message...");
        std::string message = consumer->receive_message();
        if (message == ""){
            spdlog::info("[IConsumerApp] Received empty message -> Stopping.");
            break;
        }
        spdlog::info("[ConsumerApp] Received: " + message);
        if (message == "__END__") {
            spdlog::info("[ConsumerApp] Received termination signal. Stopping.");
            break;
        }
    }
}


int main(int argc, char * argv[]) {
    std::cout << "[IConsumerApp] Start" << std::endl << std::flush;

    try {
        spdlog::level::level_enum log_level = spdlog::level::from_str(argv[1]);
        IConsumerApp app = IConsumerApp(log_level);
        spdlog::debug("[IConsumerApp] Creating consumer");
        app.create_consumer();
        spdlog::debug("[IConsumerApp] Running consumer");
        app.run();
        spdlog::debug("[IConsumerApp] Finished execution");
    } catch (const std::exception &e) {
        std::cerr << "[IConsumerApp] Exception caught: {}" << e.what() << std::endl;
    } catch (...) {
        std::cerr << "[IConsumerApp] Unknown exception caught!" << std::endl;
    }
    
    spdlog::debug("[IConsumerApp] End");
    return 0;
}