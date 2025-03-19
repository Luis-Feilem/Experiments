#ifndef ICONSUMER_HPP
#define ICONSUMER_HPP

#include <string>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

class IConsumer {
protected:
    std::shared_ptr<spdlog::logger> console = spdlog::stdout_color_mt("console");

public:
    IConsumer(spdlog::level::level_enum log_level = spdlog::level::info);
    virtual ~IConsumer() = default;

    // Initializes the consumer (e.g., connects to a broker, subscribes to a topic)
    virtual void initialize() = 0;

    // Subscribes to a topic (if applicable)
    virtual void subscribe(const std::string &topic) = 0;

    // Receives a message (blocking or non-blocking depending on implementation)
    virtual std::string receive_message() = 0;
};

#endif // ICONSUMER_HPP
