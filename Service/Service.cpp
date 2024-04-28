#include "Service.h"

Service::Service()
{
    Configurate* ConfigurateObj = Configurate::getInstance();
#ifdef KAFKAOUTPUT
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
    
    auto Modules = ConfigurateObj->getModules();
    if(Modules.CheckOperator.active)
    {
        this->CheckOpNumberOfObjectPerService = Modules.CheckOperator.NumberOfObjectPerService;
        auto SysKeys {std::make_shared<SystemKeys>()};
        std::string ServerKey = SysKeys->GetKey17().Key + SysKeys->GetKey18().Key;
        std::string ClientKey = SysKeys->GetKey19().Key + SysKeys->GetKey20().Key;

        std::string PDModel = decryptFile(Modules.CheckOperator.ModelsPath + "/" + Modules.CheckOperator.PD.Name, ServerKey, ClientKey).DecryptedMessage;
        std::string PROCRModel = decryptFile(Modules.CheckOperator.ModelsPath + "/" + Modules.CheckOperator.PROCR.Name, ServerKey, ClientKey).DecryptedMessage;
        std::string PCModel = decryptFile(Modules.CheckOperator.ModelsPath + "/" + Modules.CheckOperator.PC.Name, ServerKey, ClientKey).DecryptedMessage;
        std::string MBOCRModel = decryptFile(Modules.CheckOperator.ModelsPath + "/" + Modules.CheckOperator.MBOCR.Name, ServerKey, ClientKey).DecryptedMessage;
        
        CheckOP::CheckOPConfigStruct conf;
        conf.PDModelValue = PDModel;
        conf.PDInputSize = cv::Size(Modules.CheckOperator.PD.Width, Modules.CheckOperator.PD.Height);
        conf.PDPrimaryThreshold = Modules.CheckOperator.PD.PrimaryThreshold;
        conf.PDSecondaryThreshold = Modules.CheckOperator.PD.SecondaryThreshold;

        conf.PCModelValue = PCModel;
        conf.PCInputSize = cv::Size(Modules.CheckOperator.PC.Width, Modules.CheckOperator.PC.Height);
        conf.PCPrimaryThreshold = Modules.CheckOperator.PC.PrimaryThreshold;
        conf.PCSecondaryThreshold = Modules.CheckOperator.PC.SecondaryThreshold;

        conf.PROCRModelValue = PROCRModel;
        conf.PROCRInputSize = cv::Size(Modules.CheckOperator.PROCR.Width, Modules.CheckOperator.PROCR.Height);
        conf.PROCRPrimaryThreshold = Modules.CheckOperator.PROCR.PrimaryThreshold;
        conf.PROCRSecondaryThreshold = Modules.CheckOperator.PROCR.SecondaryThreshold;

        conf.MBOCRModelValue = MBOCRModel;
        conf.MBOCRInputSize = cv::Size(Modules.CheckOperator.MBOCR.Width, Modules.CheckOperator.MBOCR.Height);
        conf.MBOCRPrimaryThreshold = Modules.CheckOperator.MBOCR.PrimaryThreshold;
        conf.MBOCRSecondaryThreshold = Modules.CheckOperator.MBOCR.SecondaryThreshold;

        for(int i = 0; i < this->CheckOpNumberOfObjectPerService; i++)
        {
            this->CheckOPObjects.push_back(std::make_shared<CheckOP>(conf));
            this->FreeCheckOpVec.push_back(true);
        }
    }

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

void Service::releaseKafkaIndex(int Index)
{
    this->FreeKafkaMutex.lock();
    this->FreeKafkaVec[Index] = true;
    this->FreeKafkaMutex.unlock();
}

int Service::getCheckOpIndex()
{
    int FreeCheckOpIndex = -1;
    while(FreeCheckOpIndex < 0)
    {
        bool Found = false;
        this->FreeCheckOpMutex.lock();
        for(int i = 0; i < this->CheckOpNumberOfObjectPerService; i++)
        {
            if(this->FreeCheckOpVec[i])
            {
                FreeCheckOpIndex = i;
                this->FreeCheckOpVec[i] = false;
                Found = true;
                break;
            }
        }
        this->FreeCheckOpMutex.unlock();
        
        if(Found)
            break;
        
        crow::this_thread::sleep_for(crow::chrono::milliseconds(10));
    }
    return FreeCheckOpIndex;
}
    
void Service::releaseCheckOpIndex(int Index)
{
    this->FreeCheckOpMutex.lock();
    this->FreeCheckOpVec[Index] = true;
    this->FreeCheckOpMutex.unlock();
}