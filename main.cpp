#include "./ReadConfigurations/configurate.h"
#include "./Service/WebService/webservice.h"
#include "./Service/KafkaService/kafkaservice.h"

int main()
{
    Configurate* ConfigurateObj = Configurate::getInstance();
    boost::thread UpdateRouteThread(&Configurate::RunUpdateService, ConfigurateObj);
    UpdateRouteThread.detach();
    
#ifdef WEBSERVICE
    auto WebServicesConfig = ConfigurateObj->getWebServiceConfig();
    for(auto& config : WebServicesConfig)
    {
        std::shared_ptr<WebService> service{std::make_shared<WebService>(config)};
        boost::thread WebServiceThread(&WebService::run, service);
        WebServiceThread.detach();
    }
#endif // WEBSERVICE

#ifdef KAFKASERVICE
    auto InputKafkaConfig = ConfigurateObj->getInputKafkaConfig();
    std::shared_ptr<KafkaService> service2{std::make_shared<KafkaService>(InputKafkaConfig)};
    for(int i = 0; i < InputKafkaConfig.PartitionNumber; i++)
    {
        boost::thread KafkaServiceThread(&KafkaService::run, service2);
        KafkaServiceThread.detach();
    }
#endif // KAFKASERVICE

    while(true)
    {
        sleep(10000);
    }

    return 0;
}