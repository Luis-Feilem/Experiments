#include "ConsumerFactory.hpp"


void ConsumerFactory::registerConsumer(const std::string& name, CreateFunc func) {
    getRegistry()[name] = func;
}

std::unique_ptr<IConsumer> ConsumerFactory::create(const std::string& name, Logger console) {
    auto it = getRegistry().find(name);
    if (it != getRegistry().end()) {
        return it->second(console);
    }
    throw std::runtime_error("Consumer type not registered");
}

