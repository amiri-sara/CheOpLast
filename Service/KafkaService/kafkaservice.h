#ifndef KAFKASERVICE_H
#define KAFKASERVICE_H

#include "../Service.h"
#include "../../Kafka/KafkaProsumer.h"
#include "../../Kafka/Utility.h"

class KafkaService : public Service
{
public:
    KafkaService(Configurate::KafkaConfigStruct ServiceConfig);
    void run() override;

private:
    std::shared_ptr<RdKafka::Conf> configuration;
    Configurate::KafkaConfigStruct InputKafkaConfig; 
    std::string error;

protected:
};

#endif //KAFKASERVICE_H