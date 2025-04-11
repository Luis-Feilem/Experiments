#include "ConsumerApp.hpp"
#include "cstdlib"


void ConsumerApp::create_consumer() {
    std::string technology = std::getenv("TECHNOLOGY");
    console.log_debug("[ConsumerApp] Creating consumer for technology " + technology + ", log_level: " + Logger::level_to_string(console.get_level()));
    std::string tech_lib;
#ifdef _WIN32
    tech_lib = technology + "_technology.dll";  // or with full path
#else
    tech_lib = "/app/lib/lib"+ technology + "_technology.so";
#endif

    TechnologyLoader::load_technology(tech_lib, console);

    consumer = ConsumerFactory::create(technology, console);
    console.log_debug("[ConsumerApp] Created " + technology + " consumer");
}

// Initializes and runs the consumer logic
void ConsumerApp::run() {
    int termination_signals = 0;
    console.log_study("[ConsumerApp] Starting consumer");
    consumer->initialize();
    console.log_study("[ConsumerApp] Initialized consumer");

    while (true) {
        console.log_debug("[ConsumerApp] Waiting for message...");
        Payload message = consumer->receive_message();
        console.log_debug("[ConsumerApp] Received message with " + std::to_string(message.values.size()) + " values");
        if (message.label == ""){
            console.log_study("[ConsumerApp] Received message with no label -> Retrying.");
            continue;
        }
        if (message.label == "__ENDTOPIC__") {
            termination_signals++;
            console.log_study("[ConsumerApp] Received termination, total is now " + std::to_string(consumer->get_terminated_streams_size()) + "/" + std::to_string(consumer->get_subscribed_streams_size()));
            continue; // Not a usable payload
        }
        if (message.label == "__END__") {
            termination_signals++;
            console.log_study("[ConsumerApp] Received termination from all sources and topics (" + std::to_string(termination_signals) + ")");
            break;
        }
        console.log_study("[ConsumerApp] Received update on " + message.label + " with " + std::to_string(message.values.size()) + " values");
    }
}


int main(int argc, char * argv[]) {
    std::ios::sync_with_stdio(false); // Disable stream buffering
    std::cout << "[ConsumerApp] Start" << std::endl << std::flush;
    try {
        Logger::LogLevel log_level = Logger::LogLevel::INFO;
        if (argc >= 2 && argv[1] != nullptr){
            log_level = Logger::string_to_level(argv[1]);
        }
        std::cout << "[ConsumerApp] Log level: " << Logger::level_to_string(log_level) << std::endl << std::flush;
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