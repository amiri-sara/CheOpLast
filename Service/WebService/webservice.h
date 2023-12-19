#ifndef WEBSERVICE_H
#define WEBSERVICE_H

#include "../Service.h"
#include "../../crow.h"

class WebService : public Service
{
public:
    WebService(){};

    Service::ServiceResponseStruct init() override;
    void run() override;

private:    
    std::shared_ptr<crow::SimpleApp> app;
    Configurate::WebServiceConfigStruct WebServiceConfig;

    void InsertRoute();
    void TokenRoute();

protected:
};

#endif //WEBSERVICE_H