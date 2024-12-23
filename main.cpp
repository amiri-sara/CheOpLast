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
        bool hasDebugFlag = false;
        bool hasMonitorFlag = false;

        // Iterate through all provided arguments
        for(int i = 1; i < argc; ++i)
        {
            if((!strcmp(argv[i], "d")) || (!strcmp(argv[i], "D")) || (!strcmp(argv[i], "-d")) || (!strcmp(argv[i], "-D")))
            {
                hasDebugFlag = true;
            }
            else if((!strcmp(argv[i], "m")) || (!strcmp(argv[i], "M")) || (!strcmp(argv[i], "-m")) || (!strcmp(argv[i], "-M")))
            {
                hasMonitorFlag = true;
            }
            else if((!strcmp(argv[i], "v")) || (!strcmp(argv[i], "V")) || (!strcmp(argv[i], "-v")) || (!strcmp(argv[i], "-V")))
            {
                std::cout << CHECKOP_VERSION << std::endl;
                return 0;
            }
            else if((!strcmp(argv[i], "vv")) || (!strcmp(argv[i], "VV")) || (!strcmp(argv[i], "-vv")) || (!strcmp(argv[i], "-VV")))
            {
                SHOW_IMPORTANTLOG2("Check Operator Version = " << CHECKOP_VERSION);
                SHOW_IMPORTANTLOG2("Inference Version = " << inference::getVersion() << " Using ONNX Runtime " << std::to_string(ORT_API_VERSION));
                SHOW_IMPORTANTLOG2("Database Version = " << DATABASEVERSION);
                SHOW_IMPORTANTLOG2("Classifier Version = " << Classifier::getVersion());
                return 0;
            }
            else if((!strcmp(argv[i], "f")) || (!strcmp(argv[i], "F")) || (!strcmp(argv[i], "-f")) || (!strcmp(argv[i], "-F")))
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

        // Activate Debug and/or Monitor Mode based on flags
        if(hasDebugFlag && hasMonitorFlag)
        {
            Logger::getInstance().logWarning("***************** Debug and Monitoring Mode *****************");
            DebugMode = true;
            MonitorMode = true;
        }
        else if(hasDebugFlag)
        {
            Logger::getInstance().logWarning("***************** Debug Mode *****************");
            DebugMode = true;
        }
        else if(hasMonitorFlag)
        {
            Logger::getInstance().logWarning("***************** Monitoring Mode *****************");
            MonitorMode = true;
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