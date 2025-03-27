#include "PublisherFactory.hpp"


void PublisherFactory::registerPublisher(const std::string& name, CreateFunc func) {
    getRegistry()[name] = func;
}

std::unique_ptr<IPublisher> PublisherFactory::create(const std::string& name, Logger console) {
    auto it = getRegistry().find(name);
    if (it != getRegistry().end()) {
        return it->second(console);
    }
    throw std::runtime_error("Publisher type not registered");
}

