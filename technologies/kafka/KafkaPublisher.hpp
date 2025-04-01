#pragma once

#include "../../core/interfaces/IPublisher.hpp"
#include <cppkafka/cppkafka.h>
#include <memory>
#include <string>

const std::string BROKER_ADDRESS = "benchmark_kafka_broker";
const std::string BROKER_PORT = "9092";

class KafkaPublisher : public IPublisher {
public:
    KafkaPublisher(const Logger& logger, std::string broker_address = BROKER_ADDRESS);
    ~KafkaPublisher() override = default;

    void initialize() override;
    void send_message(const Payload& message, std::string topic) override;

private:
    std::string broker_;
    cppkafka::Configuration config_;
    std::unique_ptr<cppkafka::Producer> producer_;
};
