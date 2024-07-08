#include <stdexcept>

#include "KafkaProsumer.h"

KafkaProsumer::KafkaProsumer(RdKafka::Conf *configuration, std::string topic_string)
: topic_string(topic_string)
{
    std::string error;
//    Simpleconsumer = std::shared_ptr<RdKafka::Consumer>(RdKafka::Consumer::create(configuration, error));
//    /// RAII : resource acquisition is instantiation!
//    if (Simpleconsumer == nullptr)
//        // TODO: handle log
//        throw std::runtime_error(error);

    consumer = std::shared_ptr<RdKafka::KafkaConsumer>(RdKafka::KafkaConsumer::create(configuration, error));
    /// RAII : resource acquisition is instantiation!
    if (consumer == nullptr)
    {
        // TODO: handle log
        std::cout << error << std::endl;
        throw std::runtime_error(error);
    }

    RdKafka::ErrorCode resp = consumer->subscribe({topic_string});
    if (resp != RdKafka::ERR_NO_ERROR) 
        throw std::runtime_error("Failed to subscribe: " + RdKafka::err2str(resp));
    
    // std::shared_ptr<RdKafka::Conf> topic_configuration = std::shared_ptr<RdKafka::Conf>(RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC));
    // topic = std::shared_ptr<RdKafka::Topic>(RdKafka::Topic::create(Simpleconsumer.get(), topic_string, topic_configuration.get(), error));
    // if (topic == nullptr)
    //     // TODO: handle log
    //     throw std::runtime_error(error);
    // RdKafka::ErrorCode consumer_start_error = Simpleconsumer->start(topic.get(), /*partition*/ 0, /*start_offset*/ 0   );
    // if (consumer_start_error != RdKafka::ERR_NO_ERROR)
    //     throw std::runtime_error("Failed to start consumer: " + err2str(consumer_start_error));
    // configuration->set("default_topic_conf", topic_configuration.get(), error);

    producer = std::shared_ptr<RdKafka::Producer>(RdKafka::Producer::create(configuration, error));
    if (producer == nullptr)
        // TODO: handle log
        throw std::runtime_error(error);
}

KafkaProsumer::~KafkaProsumer() noexcept
{
    while (producer->outq_len() > 0)
        // TODO: determine a fitter value for waiting time
        producer->poll(500);
}


// TODO: handle on error callback
bool KafkaProsumer::write(DummyData data) noexcept
{
    RdKafka::ErrorCode produce_response = producer->produce(topic_string, /* partition */ RdKafka::Topic::PARTITION_UA,
                            RdKafka::Producer::RK_MSG_COPY /* Copy payload */,
                            /* Value */
                            const_cast<char *>(data.text.c_str()), data.text.size(),
                            /* Key */
                            const_cast<char *>(data.key.c_str()), data.key.size(),
                            /* Timestamp (defaults to now) */
                            0,
                            /* Message headers, if any */
                            nullptr,
                            /* Per-message opaque value passed to
                             * delivery report */
                            nullptr);
    if (produce_response == RdKafka::ERR_NO_ERROR)
        return true;
    // TODO: Check other types of error.
    return false;
}

// // TODO: handle on error callback
// bool KafkaProsumer::simpleread(DummyData *data) noexcept
// {
//     std::shared_ptr<RdKafka::Message> message = std::shared_ptr<RdKafka::Message>(Simpleconsumer->consume(topic.get(), /*partition*/ 0, 500));
//     if (message->err() == RdKafka::ERR_NO_ERROR) 
//     {
//         // TODO: handle headers and other metadata
//         data->text = std::string(static_cast<const char *>(message->payload()), static_cast<int>(message->len()));
//         return true;
//     }
//     // TODO: Check other types of error.
//     return false;
// }


bool KafkaProsumer::read(DummyData *data, std::string error) noexcept
{
    std::shared_ptr<RdKafka::Message> message = std::shared_ptr<RdKafka::Message>(consumer->consume(10000));
    if (message->err() == RdKafka::ERR_NO_ERROR) 
    {
        // TODO: handle headers and other metadata
        data->text = std::string(static_cast<const char *>(message->payload()), static_cast<int>(message->len()));
        data->partition = message->partition();
        data->offset = message->offset();
        return true;
    }else
    {
        error = RdKafka::err2str(message->err());
        return false;
    }
}
