#ifndef ICONSUMER_APP_HPP
#define ICONSUMER_APP_HPP

#include <string>
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>

#include "../../technologies/zeromq/ZeroMQConsumer.hpp" // Add new consumers here

class IConsumerApp {
protected:
    int id;
    std::string subscribed_topic;

    std::unique_ptr<IConsumer> consumer;

public:
    virtual ~IConsumerApp() = default;

    // Parses configuration details from JSON file
    virtual void parse_config(const std::string& config_path) {
        std::ifstream config_file(config_path);
        if (!config_file.is_open()) {
            throw std::runtime_error("Failed to open config file");
        }

        nlohmann::json config;
        config_file >> config;

        std::string container_id = std::getenv("CONTAINER_ID") ? std::getenv("CONTAINER_ID") : "1";
        if (container_id.empty()) {
            throw std::runtime_error("CONTAINER_ID environment variable is not set");
        }
        

        for (const auto& con : config["consumers"]) {
            if (con["id"] == container_id) {
                id = con["id"];
                subscribed_topic = con["subscribed_topic"];
                std::cout << "[ConsumerApp] Loaded config for " << container_id 
                          << " - Subscribed to: " << subscribed_topic << std::endl;
                break;
            }
        }
    }

    // Factory Method to Create Consumer
    virtual void create_consumer() {
        std::string technology = std::getenv("TECHNOLOGY");
        if (technology == "ZeroMQ") {
            consumer = std::make_unique<ZeroMQConsumer>();
        } 
        // Extend here for new technologies
        else {
            throw std::runtime_error("Unsupported technology: " + technology);
        }
    }

    // Initializes and runs the consumer logic
    virtual void run() {
        std::string endpoint = std::getenv("DOCKER_ENDPOINT") ? 
                               std::getenv("DOCKER_ENDPOINT") : 
                               "tcp://127.0.0.1:5555";

        consumer->initialize(endpoint, subscribed_topic);

        while (true) {
            std::string message = consumer->receive_message();
            if (message == "__END__") {
                std::cout << "[ConsumerApp] Received termination signal. Stopping." << std::endl;
                break;
            }
        }
    }
};

#endif // ICONSUMER_APP_HPP
