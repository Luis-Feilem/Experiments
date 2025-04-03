#include <memory>
#include <string>
#include <unordered_map>
#include "IConsumer.hpp"

class ConsumerFactory {
public:
    using CreateFunc = std::unique_ptr<IConsumer>(*)(Logger console);

    static void registerConsumer(const std::string& name, CreateFunc func);

    static std::unique_ptr<IConsumer> create(const std::string& name, Logger console);

    static void debug_print_registry(Logger& logger);

private:
    static std::unordered_map<std::string, CreateFunc>& getRegistry();
};
