#include "./ReadConfigurations/configurate.h"
#include "./Service/WebService/webservice.h"
#include "./Service/KafkaService/kafkaservice.h"

#define AGGREGATION_VERSION "1.0.0 : TestVersion"

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
            std::cout << AGGREGATION_VERSION << std::endl;
            return 0;
        }else if((argc == 2) && ((!strcmp(argv[1], "vv")) || (!strcmp(argv[1], "V")) || (!strcmp(argv[1], "-vv")) || (!strcmp(argv[1], "-VV"))))
        {
            SHOW_IMPORTANTLOG2("Aggregation Version = " << AGGREGATION_VERSION);
            SHOW_IMPORTANTLOG2("Inference Version = " << inference::getVersion() << " Using ONNX Runtime " << std::to_string(ORT_API_VERSION));
            SHOW_IMPORTANTLOG2("Database Version = " << DATABASEVERSION);
            SHOW_IMPORTANTLOG2("Check Operator Version = " << ChOp::getVersion());
            return 0;
        }else
        {
            SHOW_ERROR("Invalid Argument.");
            return 0;
        }
    }
    
    Configurate* ConfigurateObj = Configurate::getInstance();
    boost::thread UpdateServiceThread(&Configurate::RunUpdateService, ConfigurateObj);
    UpdateServiceThread.detach();
    
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