#pragma once

#include "../../core/interfaces/IConsumer.hpp"
#include <cppkafka/cppkafka.h>
#include <memory>
#include <string>
#include <vector>

const std::string BROKER_ADDRESS = "benchmark_kafka_broker";
const std::string BROKER_PORT = "9092";

class KafkaConsumer : public IConsumer {
public:
    KafkaConsumer(const Logger& logger);
    ~KafkaConsumer() override = default;

    void initialize() override;
    void subscribe(const std::string& topic) override;
    Payload receive_message() override;

private:
    std::string broker_;
    cppkafka::Configuration config_;
    std::unique_ptr<cppkafka::Consumer> consumer_;
    std::vector<std::string> topics_;
};
