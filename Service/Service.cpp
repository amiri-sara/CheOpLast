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
    
    auto SysKeys {std::make_shared<SystemKeys>()};
    std::string ServerKey = SysKeys->GetKey17().Key + SysKeys->GetKey18().Key; 
    std::string ClientKey = SysKeys->GetKey19().Key + SysKeys->GetKey20().Key;

    auto Modules = ConfigurateObj->getModules();
    if(Modules.CheckOperator.active)
    {
        this->CheckOpNumberOfObjectPerService = Modules.CheckOperator.NumberOfObjectPerService;
        ChOp::ConfigStruct chopConf;

        if(Modules.CheckOperator.PD.active)
        {
            chopConf.PDConfig.model = decryptFile(Modules.CheckOperator.ModelsPath + "/" + Modules.CheckOperator.PD.model, ServerKey, ClientKey).DecryptedMessage;

            chopConf.PDConfig.modelConfig = decryptFile(Modules.CheckOperator.ModelsPath + "/" + Modules.CheckOperator.PD.modelConfigPath, ServerKey, ClientKey).DecryptedMessage;
        } else
        {
            chopConf.PDConfig.active = Modules.CheckOperator.PD.active;
        }

        if(Modules.CheckOperator.PC.active)
        {
            chopConf.PCConfig.model = decryptFile(Modules.CheckOperator.ModelsPath + "/" + Modules.CheckOperator.PC.model, ServerKey, ClientKey).DecryptedMessage;
            chopConf.PCConfig.modelConfig = decryptFile(Modules.CheckOperator.ModelsPath + "/" + Modules.CheckOperator.PC.modelConfigPath, ServerKey, ClientKey).DecryptedMessage;
        } else
        {
            chopConf.PCConfig.active = Modules.CheckOperator.PC.active;
        }
        
        if(Modules.CheckOperator.IROCR.active)
        {
            chopConf.IROCRConfig.model = decryptFile(Modules.CheckOperator.ModelsPath + "/" + Modules.CheckOperator.IROCR.model, ServerKey, ClientKey).DecryptedMessage;
            chopConf.IROCRConfig.modelConfig = decryptFile(Modules.CheckOperator.ModelsPath + "/" + Modules.CheckOperator.IROCR.modelConfigPath, ServerKey, ClientKey).DecryptedMessage;
        } else
        {
            chopConf.IROCRConfig.active = Modules.CheckOperator.IROCR.active;
        }

        if(Modules.CheckOperator.MBOCR.active)
        {
            chopConf.MBOCRConfig.model = decryptFile(Modules.CheckOperator.ModelsPath + "/" + Modules.CheckOperator.MBOCR.model, ServerKey, ClientKey).DecryptedMessage;
            chopConf.MBOCRConfig.modelConfig = decryptFile(Modules.CheckOperator.ModelsPath + "/" + Modules.CheckOperator.MBOCR.modelConfigPath, ServerKey, ClientKey).DecryptedMessage;
        } else
        {
            chopConf.MBOCRConfig.active = Modules.CheckOperator.MBOCR.active;
        }

        if(Modules.CheckOperator.TZOCR.active)
        {
            chopConf.TZOCRConfig.model = decryptFile(Modules.CheckOperator.ModelsPath + "/" + Modules.CheckOperator.TZOCR.model, ServerKey, ClientKey).DecryptedMessage;
            chopConf.TZOCRConfig.modelConfig = decryptFile(Modules.CheckOperator.ModelsPath + "/" + Modules.CheckOperator.TZOCR.modelConfigPath, ServerKey, ClientKey).DecryptedMessage;
        } else
        {
            chopConf.TZOCRConfig.active = Modules.CheckOperator.TZOCR.active;
        }

        if(Modules.CheckOperator.FZOCR.active)
        {
            chopConf.FZOCRConfig.model = decryptFile(Modules.CheckOperator.ModelsPath + "/" + Modules.CheckOperator.FZOCR.model, ServerKey, ClientKey).DecryptedMessage;
            chopConf.FZOCRConfig.modelConfig = decryptFile(Modules.CheckOperator.ModelsPath + "/" + Modules.CheckOperator.FZOCR.modelConfigPath, ServerKey, ClientKey).DecryptedMessage;
        } else
        {
            chopConf.FZOCRConfig.active = Modules.CheckOperator.FZOCR.active;
        }

        if(Modules.CheckOperator.FROCR.active)
        {
            chopConf.FROCRConfig.model = decryptFile(Modules.CheckOperator.ModelsPath + "/" + Modules.CheckOperator.FROCR.model, ServerKey, ClientKey).DecryptedMessage;
            chopConf.FROCRConfig.modelConfig = decryptFile(Modules.CheckOperator.ModelsPath + "/" + Modules.CheckOperator.FROCR.modelConfigPath, ServerKey, ClientKey).DecryptedMessage;
        } else
        {
            chopConf.FROCRConfig.active = Modules.CheckOperator.FROCR.active;
        }

        chopConf.ignoreInputPlateType = Modules.CheckOperator.IgnoreInputPlateType;

        for(int i = 0; i < this->CheckOpNumberOfObjectPerService; i++)
        {
            this->m_pChOpObjects.push_back(std::make_shared<ChOp>(chopConf));
            boost::thread checkOPTread(&ChOp::process,this->m_pChOpObjects[i]);

            this->FreeCheckOpVec.push_back(true);
        }
    }

    if(Modules.Classifier.active)
    {
        this->ClassifierNumberOfObjectPerService = Modules.Classifier.NumberOfObjectPerService;
        std::vector<Classifier::ConfigStruct> classifierModelsConfig;
        for(int i = 0; i < Modules.Classifier.Models.size(); i++)
        {   
            Classifier::ConfigStruct conf;
            conf.model = decryptFile(Modules.Classifier.ModelsPath + "/" + Modules.Classifier.Models[i].model, ServerKey, ClientKey).DecryptedMessage;
            conf.modelConfig = decryptFile(Modules.Classifier.ModelsPath + "/" + Modules.Classifier.Models[i].modelConfigPath, ServerKey, ClientKey).DecryptedMessage;
            classifierModelsConfig.push_back(conf);
        }
        
        for(int i = 0; i < this->ClassifierNumberOfObjectPerService; i++)
        {
            this->m_pClassifierObjects.push_back(std::make_shared<Classifier>(classifierModelsConfig));
            this->FreeClassifierVec.push_back(true);
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

int Service::getClassifierIndex()
{
    int FreeClassifierIndex = -1;
    while(FreeClassifierIndex < 0)
    {
        bool Found = false;
        this->FreeClassifierMutex.lock();
        for(int i = 0; i < this->ClassifierNumberOfObjectPerService; i++)
        {
            if(this->FreeClassifierVec[i])
            {
                FreeClassifierIndex = i;
                this->FreeClassifierVec[i] = false;
                Found = true;
                break;
            }
        }
        this->FreeClassifierMutex.unlock();
        
        if(Found)
            break;
        
        crow::this_thread::sleep_for(crow::chrono::milliseconds(10));
    }
    return FreeClassifierIndex;
}

void Service::releaseClassifierIndex(int Index)
{
    this->FreeClassifierMutex.lock();
    this->FreeClassifierVec[Index] = true;
    this->FreeClassifierMutex.unlock();
}
