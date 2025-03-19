#include "IPublisherApp.hpp"
#include "cstdlib"


IPublisherApp::IPublisherApp(spdlog::level::level_enum log_level){
    spdlog::set_level(log_level);
    spdlog::debug("Constructing PublisherApp...");
}

void IPublisherApp::load_from_env() {
    const char* env_id = std::getenv("CONTAINER_ID");
    const char* env_topics = std::getenv("TOPICS");
    const char* env_messages = std::getenv("MESSAGES");
    const char* env_update = std::getenv("UPDATE_EVERY");

    if (!env_id || !env_topics || !env_messages || !env_update) {
        throw std::runtime_error("[IPublisherApp] Missing required environment variables");
    }

    id = std::string(env_id);
    topics = std::string(env_topics);
    message_count = std::stoi(env_messages);
    update_every = std::stoi(env_update);

    spdlog::debug("[IPublisherApp] Loaded from environment: ID=" + id 
        + ", TOPICS=" + topics 
        + ", MESSAGES=" + env_messages 
        + ", UPDATE_EVERY=" + env_update + " us"
    );
}


// Factory Method to Create Publisher
void IPublisherApp::create_publisher() {
    spdlog::debug("[IPublisherApp] Creating publisher");
    std::string technology = std::getenv("TECHNOLOGY");
    if (technology == "zeromq_p2p") {
        publisher = std::make_unique<ZeroMQP2PPublisher>();
        spdlog::debug("[IPublisherApp] Created ZeroMQ publisher");
    } 
    // Extend here for new technologies
    else {
        throw std::runtime_error("Unsupported technology: " + technology);
    }
    load_from_env();
}

// Runs the publisher logic (can now be fully generalized)
void IPublisherApp::run() {
    spdlog::debug("[IPublisherApp] Running publisher");
    std::string endpoint = std::getenv("PUBLISHER_ENDPOINT") ?
                            "tcp://" + std::string(std::getenv("PUBLISHER_ENDPOINT")) + ":5555" :
                            "tcp://127.0.0.1:5555";

    spdlog::debug("[IPublisherApp] Initializing publisher with endpoint: " + endpoint);
    publisher->initialize();
    spdlog::debug("[IPublisherApp] Initialized publisher. It will send " + std::to_string(message_count) 
        + " messages every " + std::to_string(update_every) + " us"
    );

    int i = 0;
    while (i < message_count - 1) {
        spdlog::info("[IPublisherApp] Sending message " + std::to_string(i + 1) + " on topics " + topics);
        std::string message = "Message " + std::to_string(i + 1) + " [END] to topics: " + topics;
        publisher->send_message(message);
        spdlog::info("[IPublisherApp] Sent message " + std::to_string(i + 1) + ". Now sleeping for " + std::to_string(update_every) + "us");
        std::this_thread::sleep_for(std::chrono::microseconds(update_every));
        i++;
    }
    spdlog::info("[IPublisherApp] Sending message " + std::to_string(i + 1) + " on topics " + topics);
    std::string message = "Message " + std::to_string(i + 1) + " [END] to topics: " + topics;
    publisher->send_message(message);
    spdlog::info("[IPublisherApp] Sent message " + std::to_string(i + 1));
    i++;

    // Send termination signal (poison pill)
    publisher->send_message("__END__");
    spdlog::debug("[IPublisherApp] Sent termination signal");
}


int main(int argc, char * argv[]) {
    std::cout << "[IPublisherApp] Start" << std::endl << std::flush;

    try {
        spdlog::level::level_enum log_level = spdlog::level::from_str(argv[1]);
        IPublisherApp app = IPublisherApp(log_level);
        spdlog::debug("[IPublisherApp] Creating consumer");
        app.create_publisher();
        spdlog::debug("[IPublisherApp] Running consumer");
        app.run();
        spdlog::debug("[IPublisherApp] Finished execution");
    } catch (const std::exception &e) {
        std::cerr << "[IPublisherApp] Exception caught: {}" << e.what() << std::endl;
    } catch (...) {
        std::cerr << "[IPublisherApp] Unknown exception caught!" << std::endl;
    }
    
    spdlog::debug("[IPublisherApp] End");
    return 0;
}