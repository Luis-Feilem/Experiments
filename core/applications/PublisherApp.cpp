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

// Random alphanumeric string
std::string generate_random_string(size_t length) {
    static const std::string chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<> dist(0, chars.size() - 1);

    std::string result;
    result.reserve(length);
    for (size_t i = 0; i < length; ++i) {
        result += chars[dist(rng)];
    }
    return result;
}
// Generate termination message
Payload generate_termination_message(){
    Payload payload;

    payload.label = "__END__";
    payload.values.reserve(0);
    return payload;
}

// Generate one Payload with roughly target_bytes in memory
Payload generate_payload_in_memory(size_t target_bytes) {
    Payload payload = {"",std::vector<double>()};

    size_t label_length = 5 + (std::rand() % 15);  // 5–19 chars
    payload.label = generate_random_string(label_length);

    size_t label_size_bytes = payload.label.size(); // 1 byte per char
    size_t remaining_bytes = (target_bytes > label_size_bytes) ? (target_bytes - label_size_bytes -1) : 0;
    size_t num_values = remaining_bytes / sizeof(double); // 8 bytes per double

    static std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<double> value_dist(0.0, 1000.0);
    payload.values.reserve(num_values);

    for (size_t i = 0; i < num_values; ++i) {
        payload.values.push_back(value_dist(rng));
    }

    return payload;
}

// Batch generation of payloads across size range
std::vector<Payload> generate_payloads(size_t min_size, size_t max_size, size_t num_samples) {
    std::vector<Payload> payloads;
    if (num_samples <= 1) {
        payloads.push_back(generate_payload_in_memory(min_size));
        return payloads;
    }

    size_t step = (max_size - min_size) / (num_samples - 1);
    for (size_t i = 0; i < num_samples; ++i) {
        size_t target_size = min_size + i * step;
        payloads.push_back(generate_payload_in_memory(target_size));
    }

    return payloads;
}

// Pick a random payload from the pool
const Payload& pick_random_payload(const std::vector<Payload>& payloads) {
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<> dist(0, payloads.size() - 1);
    return payloads[dist(rng)];
}

PublisherApp::PublisherApp(Logger::LogLevel log_level){
    log_level = log_level;
    console = Logger(log_level);
    load_from_env();
    payloads = generate_payloads(payload_min_size, payload_max_size, payload_samples);
}

void PublisherApp::load_from_env() {
    const char* env_id = std::getenv("CONTAINER_ID");
    const char* env_topics = std::getenv("TOPICS");
    const char* env_update = std::getenv("UPDATE_EVERY");
    const char* env_pmins = std::getenv("PAYLOAD_MIN_SIZE");
    const char* env_pmaxs = std::getenv("PAYLOAD_MAX_SIZE");
    const char* env_psamp = std::getenv("PAYLOAD_SAMPLES");


    if (!env_id || !env_topics || !env_update || !env_pmins || !env_pmaxs || !env_psamp) {
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
        else if(std::string(env_pmins).empty()){
            err_msg = "[PublisherApp] Missing required environment variable PAYLOAD_MIN_SIZE";
            console.log_error(err_msg);
            throw std::runtime_error(err_msg);
        }
        else if(std::string(env_pmaxs).empty()){
            err_msg = "[PublisherApp] Missing required environment variable PAYLOAD_MAX_SIZE";
            console.log_error(err_msg);
            throw std::runtime_error(err_msg);
        }
        else if(std::string(env_psamp).empty()){
            err_msg = "[PublisherApp] Missing required environment variable PAYLOAD_SAMPLES";
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
    payload_min_size = std::atoi(std::getenv("PAYLOAD_MIN_SIZE")); 
    payload_max_size = std::atoi(std::getenv("PAYLOAD_MAX_SIZE")); 
    payload_samples = std::atoi(std::getenv("PAYLOAD_SAMPLES"));

    console.log_debug("[PublisherApp] Loaded from environment: ID=" + id 
        + ", TOPICS=" + topics 
        + ", MESSAGES=" + std::to_string(message_count)
        + ", DURATION=" + std::to_string(duration)
        + ", UPDATE_EVERY=" + std::to_string(update_every) + " us"
        + ", PAYLOAD_MIN_SIZE+" + std::to_string(payload_min_size)
        + ", PAYLOAD_MAX_SIZE+" + std::to_string(payload_max_size)
        + ", PAYLOAD_SAMPLES+" + std::to_string(payload_samples)
    );
}


// Factory Method to Create Publisher
void PublisherApp::create_publisher() {
    std::string technology = std::getenv("TECHNOLOGY");
    console.log_debug("[PublisherApp] Creating publisher for technology " + technology + ", log_level: " + Logger::level_to_string(console.get_level()));
    
    publisher = PublisherFactory::create(technology, console);
    console.log_debug("[PublisherApp] Created " + technology + " publisher");
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
    publisher->send_message(generate_termination_message());
    console.log_debug("[PublisherApp] Sent termination signal");
}

void PublisherApp::run_messages(){
    int i = 0;
    while (i < message_count) {
        console.log_info("[PublisherApp] Sending message " + std::to_string(i + 1) + " on topics " + topics);
        // std::string message = "Message " + std::to_string(i + 1) + " [END] to topics: " + topics;
        const Payload& message = pick_random_payload(payloads);
        publisher->send_message(message);
        console.log_info("[PublisherApp] Sent message (" + std::to_string(i + 1) + ") with " + std::to_string(message.values.size()) + " values");
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
        // std::string message = "Message " + std::to_string(i + 1) + " [END] to topics: " + topics;
        const Payload& message = pick_random_payload(payloads);
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