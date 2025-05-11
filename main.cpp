#include "./ReadConfigurations/configurate.h"
#include "./Service/WebService/webservice.h"
// #include "./Service/KafkaService/kafkaservice.h"
#include "./Service/Rahdari/rahdariService.h"

#define CHECKOP_VERSION "2.0.7"

#define monitorInterval 20

// std::atomic<uint64_t> Service::chopCounter(0);

void monitor() {
    while (true) {

        std::this_thread::sleep_for(std::chrono::seconds(monitorInterval));
        uint64_t chopCounter       = Service::chopCounter > 0         ?   Service::chopCounter.exchange(0) : 0;
        uint64_t chopTime          = Service::chopTime > 0            ?   Service::chopTime.exchange(0) : 0;

        uint64_t nullImagecount    = Service::nullImageCounter > 0    ?   Service::nullImageCounter.exchange(0) : 0;
        uint64_t Imagecount        = Service::ImageCounter > 0        ?   Service::ImageCounter.exchange(0) : 0;
        uint64_t ImageRequestCount = Service::ImageRequestCounter > 0 ?   Service::ImageRequestCounter.exchange(0) : 0;
        uint64_t InfoRequestCount  = Service::InfoRequestCounter > 0  ?   Service::InfoRequestCounter.exchange(0) : 0;
        uint64_t InfoRequestTime   = Service::InfoRequestTime > 0     ?   Service::InfoRequestTime.exchange(0) : 0;
        uint64_t ImageRequestTime  = Service::ImageRequestTime > 0    ?   Service::ImageRequestTime.exchange(0) : 0;
        uint64_t InfoRequestTimeAve = InfoRequestCount > 0 ? InfoRequestTime/InfoRequestCount : 0;
        uint64_t ImageRequestTimeAve = ImageRequestCount > 0 ? ImageRequestTime/ImageRequestCount : 0;
        uint64_t ChopTimeAve = chopCounter > 0 ? chopTime/chopCounter : 0;




        Logger::getInstance().logImportant2("chopCounter in last 20 seconds: " + std::to_string(chopCounter) );

        Logger::getInstance().logImportant2("chopCounter in last 1 seconds: " + std::to_string(chopCounter / monitorInterval) );
        Logger::getInstance().logImportant2("nullImagecount in last 20 seconds: " + std::to_string(nullImagecount) );
        Logger::getInstance().logImportant2("Imagecount in last 20 seconds: " + std::to_string(Imagecount) );
        Logger::getInstance().logImportant2("ImageRequestCount in last 20 seconds: " + std::to_string(ImageRequestCount) );

        Logger::getInstance().logImportant2("InfoRequestCount in last 20 seconds: " + std::to_string(InfoRequestCount) );
        Logger::getInstance().logImportant2("InfoRequestTime in last 20 seconds: " + std::to_string(InfoRequestTime) );
        Logger::getInstance().logImportant2("ImageRequestTime in last 20 seconds: " + std::to_string(ImageRequestTime) );
        Logger::getInstance().logImportant2("chopTime in last 20 seconds: " + std::to_string(chopTime) );




        Logger::getInstance().logImportant2("InfoRequestTime (ms) Average in last 20 seconds: " + std::to_string(InfoRequestTimeAve) );
        Logger::getInstance().logImportant2("ImageRequestTime (ms) Average in last 20 seconds: " + std::to_string(ImageRequestTimeAve) );
        Logger::getInstance().logImportant2("chopTime (ms) Average in last 20 seconds: " + std::to_string(ChopTimeAve) );



    }
}



int main(int argc, char *argv[])
{   
    bool DebugMode = false;
    bool MonitorMode = false;
    bool ReadFromMinIdTXT = false;
    bool UseBatchProduce = false;
    bool UseBatchConsume = false;

    // if(argc > 1)
    // {
    //     bool hasDebugFlag = false;
    //     bool hasMonitorFlag = false;

    //     // Iterate through all provided arguments
    //     for(int i = 1; i < argc; ++i)
    //     {
    //         if((!strcmp(argv[i], "d")) || (!strcmp(argv[i], "D")) || (!strcmp(argv[i], "-d")) || (!strcmp(argv[i], "-D")))
    //         {
    //             hasDebugFlag = true;
    //         }
    //         else if((!strcmp(argv[i], "m")) || (!strcmp(argv[i], "M")) || (!strcmp(argv[i], "-m")) || (!strcmp(argv[i], "-M")))
    //         {
    //             hasMonitorFlag = true;
    //         }
    //         else if((!strcmp(argv[i], "v")) || (!strcmp(argv[i], "V")) || (!strcmp(argv[i], "-v")) || (!strcmp(argv[i], "-V")))
    //         {
    //             std::cout << CHECKOP_VERSION << std::endl;
    //             return 0;
    //         }
    //         else if((!strcmp(argv[i], "vv")) || (!strcmp(argv[i], "VV")) || (!strcmp(argv[i], "-vv")) || (!strcmp(argv[i], "-VV")))
    //         {
    //             SHOW_IMPORTANTLOG2("Check Operator Version = " << CHECKOP_VERSION);
    //             // SHOW_IMPORTANTLOG2("Inference Version = " << inference::getVersion() << " Using ONNX Runtime " << std::to_string(ORT_API_VERSION));
    //             SHOW_IMPORTANTLOG2("Database Version = " << DATABASEVERSION);
    //             // SHOW_IMPORTANTLOG2("Classifier Version = " << Classifier::getVersion());
    //             return 0;
    //         }
    //         else if((!strcmp(argv[i], "f")) || (!strcmp(argv[i], "F")) || (!strcmp(argv[i], "-f")) || (!strcmp(argv[i], "-F")))
    //         {
    //             SHOW_WARNING("***************** Read From MinId.txt *****************");
    //             ReadFromMinIdTXT = true;
    //         }
    //         else 
    //         {
    //         // Parse command-line arguments
    //             std::vector<std::string> args(argv + 1, argv + argc);
    //             for (size_t i = 0; i < args.size(); ++i) {
    //                 std::string arg = args[i];
    //                 if (arg.find("--use=") == 0) {
    //                     std::string value = arg.substr(6); // Extract after "--use="
    //                     if (value == "batchConsume") {
    //                         UseBatchConsume = true;
    //                     } else if (value == "batchProduce") {
    //                         UseBatchProduce = true;
    //                     } else if("bothBatch"){
    //                         UseBatchConsume = true;
    //                         UseBatchProduce = true;
    //                     }else {
    //                         std::cerr << "Invalid value for --use: " << value
    //                                 << ". Expected 'BatchConsume' or 'BatchProduce'" << std::endl;
    //                         std::cerr << "Usage: " << argv[0]
    //                                 << " [--use=batchConsume] [--use=batchProduce] [--use=bothBatch]" << std::endl;
    //                         return 1;
    //                     }
    //                 } else {
    //                     std::cerr << "Unknown argument: " << arg << std::endl;
    //                     std::cerr << "Usage: " << argv[0]
    //                             << " [--use=batchConsume] [--use=batchProduce] [--use=bothBatch]" << std::endl;
    //                     return 1;
    //                 }
    //             }
    //         }
    //         // else
    //         // {
    //         //     Logger::getInstance().logError("Invalid Argument.");
    //         //     return 0;
    //         // }
    //     }

    //     // Activate Debug and/or Monitor Mode based on flags
    //     if(hasDebugFlag && hasMonitorFlag)
    //     {
    //         Logger::getInstance().logWarning("***************** Debug and Monitoring Mode *****************");
    //         DebugMode = true;
    //         MonitorMode = true;
    //     }
    //     else if(hasDebugFlag)
    //     {
    //         Logger::getInstance().logWarning("***************** Debug Mode *****************");
    //         DebugMode = true;
    //     }
    //     else if(hasMonitorFlag)
    //     {
    //         Logger::getInstance().logWarning("***************** Monitoring Mode *****************");
    //         MonitorMode = true;
    //     }
    // }
    

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
                      << "  -v, -V                Show CheckOp version and exit\n"
                      << "  -vv, -VV              Show detailed version info and exit\n"
                      << "  -f, -F                Read MinId from MinId.txt\n"
                      << "  --use=batchConsume    Enable batch consuming for consumers\n"
                      << "  --use=batchProduce    Enable batch producing for producers\n"
                      << "  --use=bothBatch       Enable batching for both consumers and producers\n"
                      << std::endl;
            return 0;
        } else if (arg == "-d" || arg == "-D") {
            DebugMode = true;
        } else if (arg == "-m" || arg == "-M") {
            MonitorMode = true;
        } else if (arg == "-v" || arg == "-V") {
            std::cout << CHECKOP_VERSION << std::endl;
            return 0;
        } else if (arg == "-vv" || arg == "-VV") {
            SHOW_IMPORTANTLOG2("Check Operator Version = " << CHECKOP_VERSION);
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
    SHOW_IMPORTANTLOG("Starting RahdariService with:"
              << " batch_producer=" << UseBatchProduce
              << ", batch_consumer=" << UseBatchConsume
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
        config.DebugMode = DebugMode;
        config.MonitorMode = MonitorMode;
        config.ReadFromMinIdTXT = ReadFromMinIdTXT;
        config.UseBatchConsume = UseBatchConsume;
        config.UseBatchProduce = UseBatchProduce;

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

        boost::thread (&monitor).detach();

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
        sleep(10000);
    }

    return 0;
}