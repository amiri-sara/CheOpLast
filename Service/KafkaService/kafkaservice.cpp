#include "kafkaservice.h"
KafkaService::KafkaService(Configurate::KafkaConfigStruct ServiceConfig)
{
    this->configuration = std::shared_ptr<RdKafka::Conf>(RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL));
    this->InputKafkaConfig = ServiceConfig;

    configuration->set("bootstrap.servers", this->InputKafkaConfig.BootstrapServers, this->error);
    configuration->set("group.id", this->InputKafkaConfig.GroupID, this->error);
    configuration->set("auto.offset.reset", "earliest", this->error);
    configuration->set("enable.auto.commit", "true", this->error);
    configuration->set("auto.commit.interval.ms", "500", this->error);
}
void KafkaService::run()
{
    std::shared_ptr<KafkaProsumer> kp = std::make_shared<KafkaProsumer>(this->configuration.get(), this->InputKafkaConfig.Topic);
    
    while(true)
    {
        try 
        {   
            DummyData InputData;
            bool kafkard = kp->read(&InputData, this->error);
            if(kafkard)
            {
                SHOW_IMPORTANTLOG3(InputData.text);
            }else
            {
                SHOW_ERROR(this->error);
            }

        }
        catch (std::exception e)
        {

            SHOW_ERROR("Error Code 0x" + std::to_string(__LINE__) + "JBF_3K92XS543272" + e.what());
        }
    }
}