#ifndef SERVICE_H
#define SERVICE_H

#include "../Logs/log.h"
#include "../ReadConfigurations/configurate.h"
#include "../DataHandler/datahandler.h"
#include "../Validator/validator.h"

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
    
    virtual ~Service() = default; 			

    virtual void run() = 0;
};

#endif //SERVICE_H