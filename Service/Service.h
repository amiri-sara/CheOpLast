#ifndef SERVICE_H
#define SERVICE_H

#include "../Logs/log.h"
#include "../ReadConfigurations/configurate.h"
#include "../DataHandler/datahandler.h"
#include "../Validator/validator.h"
#include "../Cryptography/cryptotools.h"
#include "../StoreImage/storeimage.h"
#include "../SaveData/savedata.h"
#include "../crow.h"

#if defined KAFKAOUTPUT || defined KAFKASERVICE
    #include "../Kafka/KafkaProsumer.h"
    #include "../Kafka/Utility.h"
#endif // KAFKAOUTPUT || KAFKASERVICE

#include "./checkop/checkop.h"

class Service
{
public:
    enum  ServiceStatus {
        InitSuccessful
    };
    
    struct ServiceResponseStruct {
        enum Service::ServiceStatus Code;
        std::string Description;
    };
    
    Service();
    virtual ~Service() = default; 			
    virtual void run() = 0;
protected:

#ifdef KAFKAOUTPUT
    std::vector<std::shared_ptr<KafkaProsumer>> OutputKafkaConnections;
#endif // KAFKAOUTPUT
    Configurate::KafkaConfigStruct OutputKafkaConfig; 
    std::vector<bool> FreeKafkaVec;
    int getKafkaConnectionIndex();
    void releaseKafkaIndex(int Index);
    boost::mutex FreeKafkaMutex;

    std::vector<std::shared_ptr<CheckOP>> CheckOPObjects;
    std::vector<bool> FreeCheckOpVec;
    int CheckOpNumberOfObjectPerService;
    int getCheckOpIndex();
    void releaseCheckOpIndex(int Index);
    boost::mutex FreeCheckOpMutex;
};

#endif //SERVICE_H