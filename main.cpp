#include "./ReadConfigurations/configurate.h"
#include "./Service/WebService/webservice.h"
#include "./Service/KafkaService/kafkaservice.h"

int main(int argc, char *argv[])
{
    bool DebugMode = false;
    if(argc > 1)
    {
        if((argc == 2) && ((!strcmp(argv[1], "d")) || (!strcmp(argv[1], "D")) || (!strcmp(argv[1], "-d")) || (!strcmp(argv[1], "-D"))))
        {
            SHOW_WARNING("***************** Debug Mode *****************");
            DebugMode = true;
        }else if((argc == 2) && ((!strcmp(argv[1], "v")) || (!strcmp(argv[1], "V")) || (!strcmp(argv[1], "-v")) || (!strcmp(argv[1], "-V"))))
        {
            SHOW_LOG("0.0.0");
        }else
        {
            SHOW_ERROR("Invalid Argument.");
        }
    }
    
    Configurate* ConfigurateObj = Configurate::getInstance();
    boost::thread UpdateRouteThread(&Configurate::RunUpdateService, ConfigurateObj);
    UpdateRouteThread.detach();
    
#ifdef WEBSERVICE
    auto WebServicesConfig = ConfigurateObj->getWebServiceConfig();
    for(auto& config : WebServicesConfig)
    {
        config.DebugMode = DebugMode;
        std::shared_ptr<WebService> service{std::make_shared<WebService>(config)};
        boost::thread WebServiceThread(&WebService::run, service);
        WebServiceThread.detach();
    }
#endif // WEBSERVICE

#ifdef KAFKASERVICE
    auto InputKafkaConfig = ConfigurateObj->getInputKafkaConfig();
    InputKafkaConfig.DebugMode = DebugMode;
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