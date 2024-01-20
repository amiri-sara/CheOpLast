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
    // Configurate::FieldsStruct InputFields;
    // Configurate::FieldsStruct OutputFields;
    // Configurate::InfoDatabaseStruct InsertDatabaseInfo;
    // Configurate::StoreImageConfigStruct StoreImageConfig;
    // std::unordered_map<int, Configurate::ViolationStruct> ViolationMap;
    
    // std::shared_ptr<MongoDB> InsertDatabase;
};

#endif //SERVICE_H