#include "PublisherApp.hpp"
#include "cstdlib"

template<typename T>
T from_string(const std::string& str, T default_value) {
    std::istringstream iss(str);
    T result;
    if (!(iss >> result)) {
        return default_value;
    }
    return result;
}

void PublisherApp::load_from_env() {
    const char* env_id = std::getenv("CONTAINER_ID");
    const char* env_topics = std::getenv("TOPICS");
    const char* env_update = std::getenv("UPDATE_EVERY");

    if (!env_id || !env_topics || !env_update) {
        if (std::string(env_id).empty()){
            throw std::runtime_error("[PublisherApp] Missing required environment variable CONTAINER_ID");
        }
        else if(std::string(env_topics).empty()){
            throw std::runtime_error("[PublisherApp] Missing required environment variable TOPICS");
        }
        else if(std::string(env_update).empty()){
            throw std::runtime_error("[PublisherApp] Missing required environment variable UPDATE_EVERY");
        }
        else{
            throw std::runtime_error("[PublisherApp] Unknown error related to environment variables");
        }
    }

    id = std::string(env_id);
    topics = std::string(env_topics);
    message_count = std::getenv("MESSAGES")? from_string<int>(std::getenv("MESSAGES"), 0) : 0;
    duration = std::getenv("DURATION")? from_string<int>(std::getenv("DURATION"), 0) : 0;
    update_every = from_string(env_update,5000000);

    console.log_debug("[PublisherApp] Loaded from environment: ID=" + id 
        + ", TOPICS=" + topics 
        + ", MESSAGES=" + std::to_string(message_count)
        + ", DURATION=" + std::to_string(duration)
        + ", UPDATE_EVERY=" + std::to_string(update_every) + " us"
    );
}


// Factory Method to Create Publisher
void PublisherApp::create_publisher() {
    std::string technology = std::getenv("TECHNOLOGY");
    console.log_debug("[PublisherApp] Creating publisher for technology " + technology + ", log_level: " + Logger::level_to_string(console.get_level()));
    
    publisher = PublisherFactory::create(technology, console);
    console.log_debug("[PublisherApp] Created " + technology + " publisher");
    load_from_env();
}

// Runs the publisher logic (can now be fully generalized)
void PublisherApp::run() {
    console.log_debug("[PublisherApp] Starting publisher");
    publisher->initialize();
    console.log_debug("[PublisherApp] Initialized publisher. It will send " + std::to_string(message_count) 
        + " messages every " + std::to_string(update_every) + " us"
    );

    if (message_count > 0) {
        run_messages();
    }
    else if (duration > 0) {
        run_duration();
    }
    else{
        console.log_error("[PublisherApp] Neither MESSAGES nor DURATION are positive values. No messages are sent.");
    }
    // Send termination signal (poison pill)
    publisher->send_message("__END__");
    console.log_debug("[PublisherApp] Sent termination signal");
}

void PublisherApp::run_messages(){
    int i = 0;
    while (i < message_count) {
        console.log_info("[PublisherApp] Sending message " + std::to_string(i + 1) + " on topics " + topics);
        std::string message = "Message " + std::to_string(i + 1) + " [END] to topics: " + topics;
        publisher->send_message(message);
        console.log_info("[PublisherApp] Sent message " + std::to_string(i + 1));
        if (i < message_count){
            console.log_debug("[PublisherApp] Now sleeping for " + std::to_string(update_every) + "us");
            std::this_thread::sleep_for(std::chrono::microseconds(update_every));
        }
        i++;
    }
}

void PublisherApp::run_duration(){
    using namespace std::chrono;
    auto start_time = steady_clock::now();
    auto end_time = start_time + seconds(duration);

    int i = 0;
    while (steady_clock::now() < end_time) {
        console.log_info("[PublisherApp] Sending message " + std::to_string(i + 1) + " on topics " + topics);
        std::string message = "Message " + std::to_string(i + 1) + " [END] to topics: " + topics;
        publisher->send_message(message);
        console.log_info("[PublisherApp] Sent message " + std::to_string(i + 1));
        console.log_debug("[PublisherApp] Now sleeping for " + std::to_string(update_every) + "us");
        std::this_thread::sleep_for(microseconds(update_every));

        ++i;
    }
    console.log_info("[PublisherApp] Duration-based sending complete after " + std::to_string(duration) + "s");
}

int main(int argc, char * argv[]) {
    std::ios::sync_with_stdio(false); // Disable stream buffering
    std::cout << "[PublisherApp] Start" << std::endl << std::flush;
    try {
        Logger::LogLevel log_level = Logger::LogLevel::INFO;
        if (argc >= 2 && argv[1] != nullptr){
            log_level = Logger::string_to_level(argv[1]);
        }
        std::cout << "[PublisherApp] Log level: " << Logger::level_to_string(log_level) << std::endl << std::flush;
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