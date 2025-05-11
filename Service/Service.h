#ifndef SERVICE_H
#define SERVICE_H

#include <atomic>
#include "../Logs/log.h"
#include "../ReadConfigurations/configurate.h"
#include "../DataHandler/datahandler.h"
#include "../Validator/validator.h"
#include "../Cryptography/cryptotools.h"
#include "../StoreImage/storeimage.h"
#include "../SaveData/savedata.h"
#include "../crow.h"

#include "chop.h"

#if defined KAFKAOUTPUT || defined KAFKASERVICE
    #include "../Kafka/KafkaProsumer.h"
    #include "../Kafka/Utility.h"
#endif // KAFKAOUTPUT || KAFKASERVICE

// #include "classifier.h"

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
    // static std::atomic<uint64_t> chopCounter;
    inline static std::atomic<uint64_t> chopCounter = 0;
    inline static std::atomic<uint64_t> chopTime = 0;
    inline static std::atomic<uint64_t> nullImageCounter = 0;
    inline static std::atomic<uint64_t> ImageCounter = 0;
    inline static std::atomic<uint64_t> InfoRequestCounter = 0;
    inline static std::atomic<uint64_t> ImageRequestCounter = 0;

    inline static std::atomic<uint64_t> InfoRequestTime = 0;
    inline static std::atomic<uint64_t> ImageRequestTime = 0;

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

    std::vector<std::shared_ptr<ChOp>> m_pChOpObjects;
    std::vector<bool> FreeCheckOpVec;
    int CheckOpNumberOfObjectPerService;
    int getCheckOpIndex();
    void releaseCheckOpIndex(int Index);
    boost::mutex FreeCheckOpMutex;

    // std::vector<std::shared_ptr<Classifier>> m_pClassifierObjects;
    std::vector<bool> FreeClassifierVec;
    int ClassifierNumberOfObjectPerService;
    int getClassifierIndex();
    void releaseClassifierIndex(int Index);
    boost::mutex FreeClassifierMutex;
};

#endif //SERVICE_H