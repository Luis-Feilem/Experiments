#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include "IPublisher.hpp"

class PublisherFactory {
public:
    using CreateFunc = std::unique_ptr<IPublisher>(*)(Logger logger);

    static void registerPublisher(const std::string& name, CreateFunc func);

    static std::unique_ptr<IPublisher> create(const std::string& name, Logger logger);

    static void debug_print_registry(Logger& logger);
private:
    static std::unordered_map<std::string, CreateFunc>& getRegistry();
};
