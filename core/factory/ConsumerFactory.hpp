// core/factory/ConsumerFactory.hpp
#include <memory>
#include <string>
#include "../interfaces/IConsumer.hpp"

class ConsumerFactory {
public:
    using CreateFunc = std::unique_ptr<IConsumer>(*)();

    static void registerConsumer(const std::string& name, CreateFunc func) {
        getRegistry()[name] = func;
    }

    static std::unique_ptr<IConsumer> create(const std::string& name) {
        auto it = getRegistry().find(name);
        if (it != getRegistry().end()) {
            return it->second();
        }
        throw std::runtime_error("Consumer type not registered");
    }

private:
    static std::unordered_map<std::string, CreateFunc>& getRegistry() {
        static std::unordered_map<std::string, CreateFunc> registry;
        return registry;
    }
};
