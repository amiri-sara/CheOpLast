#include "./ReadConfigurations/configurate.h"
#include "./Service/WebService/webservice.h"
#include "./Service/KafkaService/kafkaservice.h"

int main()
{
    Configurate* ConfigurateObj = Configurate::getInstance();

    auto ServiceFields = ConfigurateObj->getServiceFields().servicefields;
    
    for(auto it = ServiceFields.begin(); it != ServiceFields.end(); it++)
    {
        SHOW_IMPORTANTLOG2(it->first << " : " << it->second);
    }
    
    auto WebServicesConfig = ConfigurateObj->getWebServiceConfig();
    for(auto& config : WebServicesConfig)
    {
        std::shared_ptr<WebService> service{std::make_shared<WebService>(config)};
        boost::thread WebServiceThread(&WebService::run, service);
        WebServiceThread.detach();
    }

    auto InputKafkaConfig = ConfigurateObj->getInputKafkaConfig();
    std::shared_ptr<KafkaService> service2{std::make_shared<KafkaService>(InputKafkaConfig)};
    for(int i = 0; i < InputKafkaConfig.PartitionNumber; i++)
    {
        boost::thread KafkaServiceThread(&KafkaService::run, service2);
        KafkaServiceThread.detach();
    }

    while(true)
    {
        sleep(10000);
    }

    return 0;
}