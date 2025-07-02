#include "./ReadConfigurations/configurate.h"
#include "./SystemMonitor/monitor.hpp" // Include the header containing Monitoring namespace

#ifdef WEBSERVICE
#include "./Service/WebService/webservice.h"
#endif //WEBSERVICE

#ifdef KAFKASERVICE
#include "./Service/KafkaService/kafkaservice.h"
#endif //KAFKASERVICE

#ifdef CLIENTSERVICE
#include "./Service/Rahdari/rahdariService.h"
#endif //CLIENTSERVICE


#define CHECKOP_VERSION "2.1.0"
#define LOAD_DETECTION_VERSION "2.0.0"


int main(int argc, char *argv[])
{   
    bool DebugMode = false;
    bool MonitorMode = false;
    bool ReadFromMinIdTXT = false;
    bool UseBatchProduce = false;
    bool UseBatchConsume = false;
    bool UseBulkImages = false;

    
    // Parse command-line arguments
    std::vector<std::string> args(argv + 1, argv + argc);
    for (size_t i = 0; i < args.size(); ++i) {
        std::string arg = args[i];
        if (arg == "-h" || arg == "--help") {
            std::cout << "Usage: " << argv[0] << " [options]\n"
                      << "Options:\n"
                      << "  -h, --help            Show this help message and exit\n"
                      << "  -d, -D                Enable Debug Mode\n"
                      << "  -m, -M                Enable Monitor Mode\n"
                      << "  -v, -V                Show binary version and exit\n"
                      << "  -vv, -VV              Show detailed version info and exit\n"
                      << "  -f, -F                Read MinId from MinId.txt\n"
                      << "  --use=batchConsume    Enable batch consuming for consumers\n"
                      << "  --use=batchProduce    Enable batch producing for producers\n"
                      << "  --use=bothBatch       Enable batching for both consumers and producers\n"
                      << "  --use=bulkImages       Enable batching for get Images\n"

                      << std::endl;
            return 0;
        } else if (arg == "-d" || arg == "-D") {
            DebugMode = true;
        } else if (arg == "-m" || arg == "-M") {
            MonitorMode = true;
        } else if (arg == "-v" || arg == "-V") {

#ifdef LOADDETECTOR
        std::cout << LOAD_DETECTION_VERSION << std::endl;
#else
        std::cout << CHECKOP_VERSION << std::endl;
#endif //LOADDETECTOR


            return 0;
        } else if (arg == "-vv" || arg == "-VV") {
#ifdef LOADDETECTOR
            SHOW_IMPORTANTLOG2("Load Detection Version = " << LOAD_DETECTION_VERSION);
#else
            SHOW_IMPORTANTLOG2("Check Operator Version = " << CHECKOP_VERSION);
#endif //LOADDETECTOR
            SHOW_IMPORTANTLOG2("Database Version = " << DATABASEVERSION);
            return 0;
        } else if (arg == "-f" || arg == "-F") {
            SHOW_WARNING("***************** Read From MinId.txt *****************");
            ReadFromMinIdTXT = true;
        } else if (arg.find("--use=") == 0) {
            std::string value = arg.substr(6); // Extract after "--use="
            if (value == "batchConsume") {
                UseBatchConsume = true;
            } else if (value == "batchProduce") {
                UseBatchProduce = true;
            } else if (value == "bothBatch") {
                UseBatchConsume = true;
                UseBatchProduce = true;
            } else if(value == "bulkImages")
            {
                UseBulkImages = true;
            } else {
                std::cerr << "Invalid value for --use: " << value
                          << ". Expected 'batchConsume', 'batchProduce', or 'bothBatch'" << std::endl;
                return 1;
            }
        } else {
            std::cerr << "Unknown argument: " << arg << std::endl;
            return 1;
        }
    }

    // Log mode settings
    if (DebugMode && MonitorMode) {
        Logger::getInstance().logWarning("***************** Debug and Monitoring Mode *****************");
    } else if (DebugMode) {
        Logger::getInstance().logWarning("***************** Debug Mode *****************");
    } else if (MonitorMode) {
        Logger::getInstance().logWarning("***************** Monitoring Mode *****************");
    }

        // Log batching configuration
    SHOW_IMPORTANTLOG("Starting Service with:"
              << " batch_producer=" << UseBatchProduce
              << ", batch_consumer=" << UseBatchConsume
              << ", bulkImages=" << UseBulkImages
              << ", DebugMode=" << DebugMode
              << ", MonitorMode=" << MonitorMode
              << ", ReadFromMinIdTXT=" << ReadFromMinIdTXT);

    Configurate* ConfigurateObj = Configurate::getInstance();
    // boost::thread UpdateServiceThread(&Configurate::RunUpdateService, ConfigurateObj); #TODO
    // UpdateServiceThread.detach();
#ifdef CLIENTSERVICE
    auto ClientServiceConfig = ConfigurateObj->getClientServiceConfig();
    int NumberOfConsumeThread = ConfigurateObj->getModules().CheckOperator.NumberOfThreadPerService;
    for( auto& config : ClientServiceConfig)
    {
        config.DebugMode        = DebugMode;
        config.MonitorMode      = MonitorMode;
        config.ReadFromMinIdTXT = ReadFromMinIdTXT;
        config.UseBatchConsume  = UseBatchConsume;
        config.UseBatchProduce  = UseBatchProduce;
        config.UseBulkImages    = UseBulkImages;
        std::shared_ptr<RahdariService> service{std::make_shared<RahdariService>(config)};
        service->init();
        std::vector<boost::thread> processThread;
        for (int i = 0; i < NumberOfConsumeThread; ++i) {
            processThread.emplace_back(boost::bind(&RahdariService::run, service));
        }
        for(auto& thread: processThread)
        {
            thread.detach();
        }

        Monitoring::startMonitoring();           // Start the monitoring thread

        SHOW_LOG("Monitoring thread started." << std::endl) ;

        // boost::thread (&monitor).detach();

    }
#endif //CLIENTSERVICE

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
        SHOW_LOG("Main application running..." << std::endl);
        sleep(10000);
    }

    return 0;
}
