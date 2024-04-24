#include "Service.h"

Service::Service()
{
#ifdef KAFKAOUTPUT
    Configurate* ConfigurateObj = Configurate::getInstance();
    
    std::shared_ptr<RdKafka::Conf> OutputKafkaconfiguration = std::shared_ptr<RdKafka::Conf>(RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL));
    std::string error;
    this->OutputKafkaConfig = ConfigurateObj->getOutputKafkaConfig();
    OutputKafkaconfiguration->set("bootstrap.servers", this->OutputKafkaConfig.BootstrapServers, error);
    OutputKafkaconfiguration->set("group.id", this->OutputKafkaConfig.GroupID, error);
    OutputKafkaconfiguration->set("auto.offset.reset", "earliest", error);
    OutputKafkaconfiguration->set("enable.auto.commit", "true", error);
    OutputKafkaconfiguration->set("auto.commit.interval.ms", "500", error);
    OutputKafkaconfiguration->set("enable.auto.reconnect", "true", error);

    for(int i = 0; i < this->OutputKafkaConfig.PartitionNumber; i++)
    {
        this->OutputKafkaConnections.push_back(std::make_shared<KafkaProsumer>(OutputKafkaconfiguration.get(), this->OutputKafkaConfig.Topic));
        this->FreeKafkaVec.push_back(true);
    }
    
#endif // KAFKAOUTPUT
}

int Service::getKafkaConnectionIndex()
{
    int FreeKafkaIndex = -1;
    while(FreeKafkaIndex < 0)
    {
        bool Found = false;
        this->FreeKafkaMutex.lock();
        for(int i = 0; i < this->OutputKafkaConfig.PartitionNumber; i++)
        {
            if(this->FreeKafkaVec[i])
            {
                FreeKafkaIndex = i;
                this->FreeKafkaVec[i] = false;
                Found = true;
                break;
            }
        }
        this->FreeKafkaMutex.unlock();
        
        if(Found)
            break;
        
        crow::this_thread::sleep_for(crow::chrono::milliseconds(10));
    }
    return FreeKafkaIndex;
}

void Service::releaseIndex(int Index)
{
    SHOW_IMPORTANTLOG2(Index);
    this->FreeKafkaMutex.lock();
    this->FreeKafkaVec[Index] = true;
    this->FreeKafkaMutex.unlock();
}