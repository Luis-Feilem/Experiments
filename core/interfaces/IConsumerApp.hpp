#ifndef ICONSUMER_APP_HPP
#define ICONSUMER_APP_HPP

#include <string>
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>

#include "../../technologies/zeromq/ZeroMQConsumer.hpp" // Add new consumers here

class IConsumerApp {
protected:
    std::string id;
    std::string subscribed_topic;

    std::unique_ptr<IConsumer> consumer;

public:
    virtual ~IConsumerApp() = default;

    // // Parses configuration details from JSON file
    // virtual void parse_config(const std::string& config_path) {
    //     std::cout << "[IConsumerApp] Parsing config file: " << config_path << std::endl;
    //     std::ifstream config_file(config_path);
    //     if (!config_file.is_open()) {
    //         throw std::runtime_error("Failed to open config file");
    //     }

    //     nlohmann::json config;
    //     config_file >> config;

    //     std::string container_id = std::getenv("CONTAINER_ID") ? std::getenv("CONTAINER_ID") : "C1";
    //     if (container_id.empty()) {
    //         throw std::runtime_error("CONTAINER_ID environment variable is not set");
    //     }
        

    //     for (const auto& con : config["consumers"]) {
    //         std::cout << "[IConsumerApp] Checking config for " << con["id"] << std::endl;
    //         if (con["id"] == container_id) {
    //             id = con["id"];
    //             subscribed_topic = con["subscribed_topic"];
    //             std::cout << "[IConsumerApp] Loaded config for " << container_id 
    //                       << " - Subscribed to: " << subscribed_topic << std::endl;
    //             break;
    //         }
    //     }
    // }

    // Factory Method to Create Consumer
    virtual void create_consumer() {
        std::cout << "[IConsumerApp] Creating consumer" << std::endl;
        std::string technology = std::getenv("TECHNOLOGY");
        if (technology == "ZeroMQ") {
            consumer = std::make_unique<ZeroMQConsumer>();
            std::cout << "[IConsumerApp] Created ZeroMQ consumer" << std::endl;
        } 
        // Extend here for new technologies
        else {
            throw std::runtime_error("Unsupported technology: " + technology);
        }
    }

    // Initializes and runs the consumer logic
    virtual void run() {
        std::cout << "[IConsumerApp] Starting consumer" << std::endl;
        std::string endpoint = std::getenv("DOCKER_ENDPOINT") ? 
                               std::getenv("DOCKER_ENDPOINT") : 
                               "tcp://127.0.0.1:5555";

        std::cout << "[IConsumerApp] Initializing consumer with endpoint: " << endpoint 
                  << " and topic: " << subscribed_topic << std::endl;
        consumer->initialize();
        std::cout << "[IConsumerApp] Initialized consumer" << std::endl;

        while (true) {
            std::cout << "[IConsumerApp] Waiting for message..." << std::endl;
            std::string message = consumer->receive_message();
            std::cout << "[ConsumerApp] Received: " << message << std::endl;
            if (message == "__END__") {
                std::cout << "[ConsumerApp] Received termination signal. Stopping." << std::endl;
                break;
            }
        }
    }
};

#endif // ICONSUMER_APP_HPP
