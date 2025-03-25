#include "PublisherApp.hpp"
#include "cstdlib"


void PublisherApp::load_from_env() {
    const char* env_id = std::getenv("CONTAINER_ID");
    const char* env_topics = std::getenv("TOPICS");
    const char* env_messages = std::getenv("MESSAGES");
    const char* env_update = std::getenv("UPDATE_EVERY");

    if (!env_id || !env_topics || !env_messages || !env_update) {
        throw std::runtime_error("[PublisherApp] Missing required environment variables");
    }

    id = std::string(env_id);
    topics = std::string(env_topics);
    message_count = std::stoi(env_messages);
    update_every = std::stoi(env_update);

    console.log_debug("[PublisherApp] Loaded from environment: ID=" + id 
        + ", TOPICS=" + topics 
        + ", MESSAGES=" + env_messages 
        + ", UPDATE_EVERY=" + env_update + " us"
    );
}


// Factory Method to Create Publisher
void PublisherApp::create_publisher() {
    std::string technology = std::getenv("TECHNOLOGY");
    console.log_debug("[PublisherApp] Creating publisher for technology " + technology);
    
    publisher = PublisherFactory::create(technology);
    load_from_env();
    console.log_debug("[PublisherApp] Created " + technology + " publisher");
}

// Runs the publisher logic (can now be fully generalized)
void PublisherApp::run() {
    console.log_debug("[PublisherApp] Running publisher");
    std::string endpoint = std::getenv("PUBLISHER_ENDPOINT") ?
                            "tcp://" + std::string(std::getenv("PUBLISHER_ENDPOINT")) + ":5555" :
                            "tcp://127.0.0.1:5555";

    console.log_debug("[PublisherApp] Initializing publisher with endpoint: " + endpoint);
    publisher->initialize();
    console.log_debug("[PublisherApp] Initialized publisher. It will send " + std::to_string(message_count) 
        + " messages every " + std::to_string(update_every) + " us"
    );

    int i = 0;
    while (i < message_count - 1) {
        console.log_info("[PublisherApp] Sending message " + std::to_string(i + 1) + " on topics " + topics);
        std::string message = "Message " + std::to_string(i + 1) + " [END] to topics: " + topics;
        publisher->send_message(message);
        console.log_info("[PublisherApp] Sent message " + std::to_string(i + 1) + ". Now sleeping for " + std::to_string(update_every) + "us");
        std::this_thread::sleep_for(std::chrono::microseconds(update_every));
        i++;
    }
    console.log_info("[PublisherApp] Sending message " + std::to_string(i + 1) + " on topics " + topics);
    std::string message = "Message " + std::to_string(i + 1) + " [END] to topics: " + topics;
    publisher->send_message(message);
    console.log_info("[PublisherApp] Sent message " + std::to_string(i + 1));
    i++;

    // Send termination signal (poison pill)
    publisher->send_message("__END__");
    console.log_debug("[PublisherApp] Sent termination signal");
}


int main(int argc, char * argv[]) {
    std::ios::sync_with_stdio(false); // Disable stream buffering
    std::cout << "[PublisherApp] Start" << std::endl << std::flush;
    try {
        Logger::LogLevel log_level;
        if (argc >= 2 && argv[1] != nullptr){
            log_level = Logger::string_to_level(argv[1]);
        }
        PublisherApp app = PublisherApp(log_level);
        app.create_publisher();
        app.run();
    } catch (const std::exception &e) {
        std::cerr << "[PublisherApp] Exception caught: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "[PublisherApp] Unknown exception caught!" << std::endl;
    }
    return 0;
}