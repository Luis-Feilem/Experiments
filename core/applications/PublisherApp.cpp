#include "PublisherApp.hpp"
#include "cstdlib"
#include "TechnologyLoader.hpp"

template<typename T>
T from_string(const std::string& str, T default_value) {
    std::istringstream iss(str);
    T result;
    if (!(iss >> result)) {
        return default_value;
    }
    return result;
}

// Generate termination message
Payload PublisherApp::generate_termination_message(){
    Payload payload;

    payload.message_id = id + ":__END__"; // add "source" information to termination signal
    payload.data.reserve(0);
    return payload;
}

// Batch generation of payloads across size range
void PublisherApp::generate_payloads(size_t target_size, size_t num_samples) {
    for (size_t i = 0; i < num_samples; ++i) {
        payloads.push_back(Payload::make(id, i, target_size, payload_kind));
    }
}

// Pick a random payload from the pool
const Payload& PublisherApp::pick_random_payload() {
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<> dist(0, payloads.size() - 1);
    return payloads[dist(rng)];
}

PublisherApp::PublisherApp(Logger::LogLevel log_level){
    console = Logger(log_level);
    load_from_env();
    generate_payloads(payload_size, payload_samples);
}

void PublisherApp::load_from_env() {
    const char* env_id = std::getenv("CONTAINER_ID");
    const char* env_topics = std::getenv("TOPICS");
    const char* env_update = std::getenv("UPDATE_EVERY");
    const char* env_psize = std::getenv("PAYLOAD_SIZE");
    const char* env_psamp = std::getenv("PAYLOAD_SAMPLES");
    const char* env_pkind = std::getenv("PAYLOAD_KIND");


    if (!env_id || !env_topics || !env_update || !env_psize || !env_psamp) {
        std::string err_msg;
        if (std::string(env_id).empty()){
            err_msg = "[PublisherApp] Missing required environment variable CONTAINER_ID";
            console.log_error(err_msg);
            throw std::runtime_error(err_msg);
        }
        else if(std::string(env_topics).empty()){
            err_msg = "[PublisherApp] Missing required environment variable TOPICS";
            console.log_error(err_msg);
            throw std::runtime_error(err_msg);
        }
        else if(std::string(env_update).empty()){
            err_msg = "[PublisherApp] Missing required environment variable UPDATE_EVERY";
            console.log_error(err_msg);
            throw std::runtime_error(err_msg);
        }
        else if(std::string(env_psize).empty()){
            err_msg = "[PublisherApp] Missing required environment variable PAYLOAD_SIZE";
            console.log_error(err_msg);
            throw std::runtime_error(err_msg);
        }
        else if(std::string(env_psamp).empty()){
            err_msg = "[PublisherApp] Missing required environment variable PAYLOAD_SAMPLES";
            console.log_error(err_msg);
            throw std::runtime_error(err_msg);
        }
        else if(std::string(env_pkind).empty()){
            err_msg = "[PublisherApp] Missing required environment variable PAYLOAD_KIND";
            console.log_error(err_msg);
            throw std::runtime_error(err_msg);
        }
        else{
            err_msg = "[PublisherApp] Unknown error related to environment variables";
            console.log_error(err_msg);
            throw std::runtime_error(err_msg);
        }
    }

    id = std::string(env_id);
    topics = std::string(env_topics);
    message_count = std::getenv("MESSAGES")? from_string<int>(std::getenv("MESSAGES"), 0) : 0;
    duration = std::getenv("DURATION")? from_string<int>(std::getenv("DURATION"), 0) : 0;
    update_every = from_string(env_update,5000000);
    payload_size = std::atoi(std::getenv("PAYLOAD_SIZE")); 
    payload_samples = std::atoi(std::getenv("PAYLOAD_SAMPLES"));
    payload_kind = Payload::string_to_payloadkind(std::getenv("PAYLOAD_KIND"));

    console.log_debug("[PublisherApp] Loaded from environment: ID=" + id 
        + ", TOPICS=" + topics 
        + ", MESSAGES=" + std::to_string(message_count)
        + ", DURATION=" + std::to_string(duration)
        + ", UPDATE_EVERY=" + std::to_string(update_every) + " us"
        + ", PAYLOAD_SIZE=" + std::to_string(payload_size)
        + ", PAYLOAD_SAMPLES=" + std::to_string(payload_samples)
        + ", PAYLOAD_KIND=" + Payload::payloadkind_to_string(payload_kind)
    );
}


// Factory Method to Create Publisher
void PublisherApp::create_publisher() {
    std::string technology = std::getenv("TECHNOLOGY");
    console.log_debug("[PublisherApp] Creating publisher for technology " + technology + ", log_level: " + Logger::level_to_string(console.get_level()));
    std::string tech_lib;
#ifdef _WIN32
    tech_lib = technology + "_technology.dll";  // or with full path
#else
    tech_lib = "/app/lib/lib"+ technology + "_technology.so";
#endif

    TechnologyLoader::load_technology(tech_lib, console);
    console.log_debug("[PublisherApp] Factory state before calling 'create'");
    PublisherFactory::debug_print_registry(console);
    
    publisher = PublisherFactory::create(technology, console);
    console.log_debug("[PublisherApp] Created " + technology + " publisher");
}

void PublisherApp::publish_on_topic(std::string topic, int i){
    const Payload& base = pick_random_payload();
    Payload message = Payload::reuse_with_new_id(id, i, base.data, base.kind);
    console.log_study("[PublisherApp] Publishing message " + std::to_string(i) + 
                      " of size " + std::to_string(base.data_size) + 
                      " to topic " + topic);
    publisher->send_message(message, topic);
    console.log_study("[PublisherApp] Published message " + std::to_string(i) + 
                      " of size " + std::to_string(base.data_size) + 
                      " to topic " + topic);
}

void PublisherApp::publish_on_all_topics(int i){
    try {
        std::istringstream ss(topics);
        std::string topic;
        while (std::getline(ss, topic, ',')) {
            publish_on_topic(topic, i);
        }
    } catch (const std::exception& e){
        console.log_error("[Publisher App] Exception during publish: " + std::string(e.what()));
    }
}

void PublisherApp::terminate_topic(std::string topic){
    console.log_info("[PublisherApp] Closing stream for topic " + topic);
    publisher->send_message(generate_termination_message(), topic);
}

void PublisherApp::terminate_all_topics(){
    std::istringstream ss(topics);
    std::string topic;
    while (std::getline(ss, topic, ',')) {
        terminate_topic(topic);
    }
}

// Runs the publisher logic (can now be fully generalized)
void PublisherApp::run() {
    console.log_study("[PublisherApp] Starting publisher");
    publisher->initialize();
    // wait for consumer to start and connect, and to synchronize with metrics gathering
    std::this_thread::sleep_for(std::chrono::milliseconds(4000));

    if (message_count > 0) {
        console.log_study("[PublisherApp] Initialized publisher. It will send a total of " + std::to_string(message_count) 
            + " messages, one every " + std::to_string(update_every) + " us"
        );
        run_messages();
    }
    else if (duration > 0) {
        console.log_study("[PublisherApp] Initialized publisher. It will send messages for " + std::to_string(duration) 
            + " seconds, one every " + std::to_string(update_every) + " us"
        );
        run_duration();
    }
    else{
        console.log_error("[PublisherApp] Neither MESSAGES nor DURATION are positive values. No messages are sent.");
    }
    // Send termination signal (poison pill)
    console.log_study("[PublisherApp] Terminating publisher. Sending termination signal to all topics.");	
    terminate_all_topics();
    console.log_study("[PublisherApp] Terminated publisher. Sending termination signal to all topics.");
}

void PublisherApp::run_messages(){
    int i = 1;
    while (i <= message_count) {
        console.log_info("[PublisherApp] Sending message " + std::to_string(i));
        // std::string message = "Message " + std::to_string(i + 1) + " [END] to topics: " + topics;
        publish_on_all_topics(i);
        console.log_info("[PublisherApp] Sent message " + std::to_string(i));
        i++;
        // If you want to simulate a data generating system, uncomment this section
        // if (i < message_count){
        //     console.log_debug("[PublisherApp] Now sleeping for " + std::to_string(update_every) + "us");
        //     std::this_thread::sleep_for(std::chrono::microseconds(update_every));
        // }
    }
}

void PublisherApp::run_duration(){
    using namespace std::chrono;
    auto start_time = steady_clock::now();
    auto end_time = start_time + seconds(duration);

    int i = 0;
    while (steady_clock::now() < end_time) {
        console.log_info("[PublisherApp] Sending message " + std::to_string(i + 1));
        // std::string message = "Message " + std::to_string(i + 1) + " [END] to topics: " + topics;
        publish_on_all_topics(i);
        console.log_info("[PublisherApp] Sent message " + std::to_string(i + 1));
        console.log_debug("[PublisherApp] Now sleeping for " + std::to_string(update_every) + "us");
        std::this_thread::sleep_for(microseconds(update_every));

        ++i;
    }
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