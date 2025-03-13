#include "IPublisherApp.hpp"
#include "cstdlib"

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

    std::cout << "[IPublisherApp] Loaded from environment: "
                << "ID=" << id << ", TOPICS=" << topics
                << ", MESSAGES=" << message_count
                << ", UPDATE_EVERY=" << update_every << " us" << std::endl;
}


// Factory Method to Create Publisher
void IPublisherApp::create_publisher() {
    std::cout << "[IPublisherApp] Creating publisher" << std::endl;
    std::string technology = std::getenv("TECHNOLOGY");
    if (technology == "ZeroMQ") {
        publisher = std::make_unique<ZeroMQPublisher>();
        std::cout << "[IPublisherApp] Created ZeroMQ publisher" << std::endl;
    } 
    // Extend here for new technologies
    else {
        throw std::runtime_error("Unsupported technology: " + technology);
    }
    load_from_env();
}

// Runs the publisher logic (can now be fully generalized)
void IPublisherApp::run() {
    std::cout << "[IPublisherApp] Running publisher" << std::endl;
    std::string endpoint = std::getenv("DOCKER_ENDPOINT") ? 
                            std::getenv("DOCKER_ENDPOINT") : 
                            "tcp://127.0.0.1:5555";

    std::cout << "[IPublisherApp] Initializing publisher with endpoint: " << endpoint << std::endl;
    publisher->initialize();
    std::cout << "[IPublisherApp] Initialized publisher. It will send " << message_count << " messages every " << update_every << " us" << std::endl;

    for (int i = 0; i < message_count; ++i) {
        std::cout << "[IPublisherApp] Sending message " << i + 1 << " on topics " << topics << std::endl;
        std::string message = "Message " + std::to_string(i + 1) + " [END] to topics: " + topics;
        publisher->send_message(message);
        std::cout << "[IPublisherApp] Sent message " << i + 1 << ". Now sleeping for " << update_every << "us" << std::endl;
        std::this_thread::sleep_for(std::chrono::microseconds(update_every));
    }

    // Send termination signal (poison pill)
    publisher->send_message("__END__");
    std::cout << "[IPublisherApp] Sent termination signal" << std::endl;
}