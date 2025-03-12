#ifndef IPUBLISHER_APP_HPP
#define IPUBLISHER_APP_HPP

#include <string>
#include <memory>
#include <thread>
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>

#include "../../technologies/zeromq/ZeroMQPublisher.hpp" // Add new publishers here

class IPublisherApp {
protected:
    std::string id;
    std::string topic;
    int message_count;
    int update_every;

    std::unique_ptr<IPublisher> publisher;

public:
    virtual ~IPublisherApp() = default;

    // // Parses configuration details from JSON file
    // virtual void parse_config(const std::string& config_path) {
    //     std::cout << "[IPublisherApp] Parsing config file: " << config_path << std::endl;
    //     std::ifstream config_file(config_path);
    //     if (!config_file.is_open()) {
    //         throw std::runtime_error("Failed to open config file");
    //     }

    //     nlohmann::json config;
    //     config_file >> config;

    //     std::string container_id = std::getenv("CONTAINER_ID") ? std::getenv("CONTAINER_ID") : "P1";
    //     if (container_id.empty()) {
    //         throw std::runtime_error("CONTAINER_ID environment variable is not set");
    //     }

    //     for (const auto& pub : config["publishers"]) {
    //         std::cout << "[IPublisherApp] Checking config for " << pub["id"] << std::endl;
    //         if (pub["id"] == container_id) {
    //             id = pub["id"];
    //             topic = pub["topic"];
    //             message_count = pub["messages"];
    //             update_every = pub.value("update_every", 1000000);
    //             std::cout << "[IPublisherApp] Loaded config for " << container_id 
    //                       << " - Topic: " << topic 
    //                       << ", Messages: " << message_count 
    //                       << ", Update every: " << update_every << "us" << std::endl;
    //             break;
    //         }
    //     }
    // }

    // Factory Method to Create Publisher
    virtual void create_publisher() {
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
        message_count = std::stoi(std::getenv("MESSAGES"));
        update_every = std::stoi(std::getenv("UPDATE_EVERY"));
    }

    // Runs the publisher logic (can now be fully generalized)
    virtual void run() {
        std::cout << "[IPublisherApp] Running publisher" << std::endl;
        std::string endpoint = std::getenv("DOCKER_ENDPOINT") ? 
                               std::getenv("DOCKER_ENDPOINT") : 
                               "tcp://127.0.0.1:5555";

        std::cout << "[IPublisherApp] Initializing publisher with endpoint: " << endpoint << std::endl;
        publisher->initialize();
        std::cout << "[IPublisherApp] Initialized publisher. It will send " << message_count << " messages every " << update_every << " us" << std::endl;

        for (int i = 0; i < message_count; ++i) {
            std::cout << "[IPublisherApp] Sending message " << i + 1 << " on topic " << topic << std::endl;
            std::string message = "Message " + std::to_string(i + 1) + " [END] to topic: " + topic;
            publisher->send_message(message);
            std::cout << "[IPublisherApp] Sent message " << i + 1 << ". Now sleeping for " << update_every << "us" << std::endl;
            std::this_thread::sleep_for(std::chrono::microseconds(update_every));
        }

        // Send termination signal (poison pill)
        publisher->send_message("__END__");
        std::cout << "[IPublisherApp] Sent termination signal" << std::endl;
    }
};

#endif // IPUBLISHER_APP_HPP
