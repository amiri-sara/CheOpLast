#ifndef KAFKA_PROSUMER_H
#define KAFKA_PROSUMER_H

#include <memory>

#include <librdkafka/rdkafka.h>
#include <librdkafka/rdkafkacpp.h>
#include <iostream>

#include "Utility.h"

class KafkaProsumer
{
public:

    explicit KafkaProsumer(RdKafka::Conf *configuration, std::string topic_string = "");

    ~KafkaProsumer() noexcept;

    /**
     * @brief Writes the data to the broker
     * 
     * @param data Self explanatory
     * @return State of successful data production.
     */
    bool write(DummyData data) noexcept;

    /**
     * @brief Reads data from the broker
     *
     * @param data self explanatory
     * @return State of data existence.
     */
    // bool simpleread(DummyData *data) noexcept;
    bool read(DummyData *data, std::string error) noexcept;
private:

    std::shared_ptr<RdKafka::Consumer> Simpleconsumer;
    std::shared_ptr<RdKafka::KafkaConsumer> consumer;
    std::shared_ptr<RdKafka::Producer> producer;
    std::shared_ptr<RdKafka::Topic> topic;
    std::string topic_string;
};

#endif /* KAFKA_PROSUMER_H */ 