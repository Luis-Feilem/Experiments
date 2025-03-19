#ifndef IPUBLISHER_HPP
#define IPUBLISHER_HPP

#include <string>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

class IPublisher {
protected:
    std::shared_ptr<spdlog::logger> console = spdlog::stdout_color_mt("console");
    
public:
    IPublisher(spdlog::level::level_enum log_level = spdlog::level::info);
    virtual ~IPublisher() = default;

    // Initializes the publisher (e.g., connects to a broker)
    virtual void initialize() = 0;

    // Sends a message
    virtual void send_message(const std::string &message) = 0;
};

#endif // IPUBLISHER_HPP
