#include "./ReadConfigurations/configurate.h"
#include "./Service/WebService/webservice.h"
#include "./Service/KafkaService/kafkaservice.h"
#include "./Service/Rahdari/rahdariService.h"

#define CHECKOP_VERSION "2.0.4"

int main(int argc, char *argv[])
{   
    bool DebugMode = false;
    bool MonitorMode = false;
    bool ReadFromMinIdTXT = false;
    if(argc > 1)
    {
        if((argc == 2) && ((!strcmp(argv[1], "d")) || (!strcmp(argv[1], "D")) || (!strcmp(argv[1], "-d")) || (!strcmp(argv[1], "-D"))))
        {
            Logger::getInstance().logWarning("***************** Debug Mode *****************");
            DebugMode = true;
        }else if((argc == 2) && ((!strcmp(argv[1], "m")) || (!strcmp(argv[1], "M")) || (!strcmp(argv[1], "-m")) || (!strcmp(argv[1], "-M"))))
        {
            Logger::getInstance().logWarning("***************** Monitoring Mode *****************");
            MonitorMode = true;

        }else if((argc == 2) && ((!strcmp(argv[1], "v")) || (!strcmp(argv[1], "V")) || (!strcmp(argv[1], "-v")) || (!strcmp(argv[1], "-V"))))
        {
            std::cout << CHECKOP_VERSION << std::endl;
            return 0;
        }else if((argc == 2) && ((!strcmp(argv[1], "vv")) || (!strcmp(argv[1], "V")) || (!strcmp(argv[1], "-vv")) || (!strcmp(argv[1], "-VV"))))
        {
            SHOW_IMPORTANTLOG2("Check Operator Version = " << CHECKOP_VERSION);
            SHOW_IMPORTANTLOG2("Inference Version = " << inference::getVersion() << " Using ONNX Runtime " << std::to_string(ORT_API_VERSION));
            SHOW_IMPORTANTLOG2("Database Version = " << DATABASEVERSION);
            // SHOW_IMPORTANTLOG2("Check Operator Version = " << ChOp::getVersion());
            SHOW_IMPORTANTLOG2("Classifier Version = " << Classifier::getVersion());
            return 0;
        }else if((argc == 2) && ((!strcmp(argv[1], "f")) || (!strcmp(argv[1], "F")) || (!strcmp(argv[1], "-f")) || (!strcmp(argv[1], "-F"))))
        {
            SHOW_WARNING("***************** Read From MinId.txt *****************");
            ReadFromMinIdTXT = true;
        }
        else
        {
            Logger::getInstance().logError("Invalid Argument.");
            return 0;
        }
    }
    
    Configurate* ConfigurateObj = Configurate::getInstance();
    // boost::thread UpdateServiceThread(&Configurate::RunUpdateService, ConfigurateObj); #TODO
    // UpdateServiceThread.detach();
#ifdef CURLSERVICE
    auto CurlServiceConfig = ConfigurateObj->getCurlServiceConfig();
    for( auto& config : CurlServiceConfig)
    {
        config.DebugMode = DebugMode;
        config.MonitorMode = MonitorMode;
        config.ReadFromMinIdTXT = ReadFromMinIdTXT;
        std::shared_ptr<RahdariService> service3{std::make_shared<RahdariService>(config)};
        service3->init();
        boost::thread (&RahdariService::run, service3).detach();

    }
#endif //CURLSERVICE

#ifdef WEBSERVICE
    auto WebServicesConfig = ConfigurateObj->getWebServiceConfig();
    for( auto& config : WebServicesConfig)
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