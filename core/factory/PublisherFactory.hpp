#include <memory>
#include <string>
#include <unordered_map>
#include "IPublisher.hpp"

class PublisherFactory {
public:
    using CreateFunc = std::unique_ptr<IPublisher>(*)();

    static void registerPublisher(const std::string& name, CreateFunc func);

    static std::unique_ptr<IPublisher> create(const std::string& name);

private:
    static std::unordered_map<std::string, CreateFunc>& getRegistry(){
        static std::unordered_map<std::string, CreateFunc> registry;
        return registry;
    }
};
