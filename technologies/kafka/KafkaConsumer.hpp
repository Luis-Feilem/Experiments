#pragma once

#include "../../core/interfaces/IConsumer.hpp"
#include <librdkafka/rdkafka.h>
#include <string>
#include <vector>
#include <unordered_set>

class KafkaConsumer : public IConsumer {
public:
    KafkaConsumer(const Logger& logger);
    ~KafkaConsumer() override;

    void initialize() override;
    void subscribe(const std::string& topic) override;
    Payload receive_message() override;

private:
    std::string broker_;
    std::unordered_set<std::string> topic_names_;

    rd_kafka_t* consumer_;
    rd_kafka_conf_t* conf_;
    rd_kafka_topic_partition_list_t* subscription_list_;
    bool initialized_;
};
