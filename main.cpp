#include "./ReadConfigurations/configurate.h"
#include "./Service/WebService/webservice.h"
#include "./Service/KafkaService/kafkaservice.h"

int main()
{
    Configurate* ConfigurateObj = Configurate::getInstance();
    // auto DatabaseConfig = ConfigurateObj->getDatabaseConfig();
    // SHOW_IMPORTANTLOG2(DatabaseConfig.DatabaseIP);
    // SHOW_IMPORTANTLOG2(DatabaseConfig.DatabasePort);
    // SHOW_IMPORTANTLOG2(DatabaseConfig.DatabaseUsername);
    // SHOW_IMPORTANTLOG2(DatabaseConfig.DatabasePassword);
    // SHOW_IMPORTANTLOG2(DatabaseConfig.DETAIL);
    // SHOW_IMPORTANTLOG2(DatabaseConfig.Enable);
    // SHOW_IMPORTANTLOG2(DatabaseConfig.DatabaseName);
    // SHOW_IMPORTANTLOG2(DatabaseConfig.CollectionName);

    // SHOW_WARNING("**********");
    // auto DatabaseInsert = ConfigurateObj->getDatabaseInsert();
    // SHOW_IMPORTANTLOG2(DatabaseInsert.DatabaseIP);
    // SHOW_IMPORTANTLOG2(DatabaseInsert.DatabasePort);
    // SHOW_IMPORTANTLOG2(DatabaseInsert.DatabaseUsername);
    // SHOW_IMPORTANTLOG2(DatabaseInsert.DatabasePassword);
    // SHOW_IMPORTANTLOG2(DatabaseInsert.DETAIL);
    // SHOW_IMPORTANTLOG2(DatabaseInsert.Enable);
    // SHOW_IMPORTANTLOG2(DatabaseInsert.DatabaseName);
    // SHOW_IMPORTANTLOG2(DatabaseInsert.CollectionName);

    // SHOW_WARNING("**********");
    // auto DatabaseFailed = ConfigurateObj->getDatabaseFailed();
    // SHOW_IMPORTANTLOG2(DatabaseFailed.DatabaseIP);
    // SHOW_IMPORTANTLOG2(DatabaseFailed.DatabasePort);
    // SHOW_IMPORTANTLOG2(DatabaseFailed.DatabaseUsername);
    // SHOW_IMPORTANTLOG2(DatabaseFailed.DatabasePassword);
    // SHOW_IMPORTANTLOG2(DatabaseFailed.DETAIL);
    // SHOW_IMPORTANTLOG2(DatabaseFailed.Enable);
    // SHOW_IMPORTANTLOG2(DatabaseFailed.DatabaseName);
    // SHOW_IMPORTANTLOG2(DatabaseFailed.CollectionName);

    // auto WebServiceConfig = ConfigurateObj->getWebServiceConfig();
    // SHOW_IMPORTANTLOG2(WebServiceConfig.URI);
    // SHOW_IMPORTANTLOG2(WebServiceConfig.Port);
    // SHOW_IMPORTANTLOG2(WebServiceConfig.CheckToken);
    // SHOW_IMPORTANTLOG2(WebServiceConfig.TokenTimeAllowed);

    // auto StoreImageConfig = ConfigurateObj->getStoreImageConfig();
    // SHOW_IMPORTANTLOG2(StoreImageConfig.StorePath);
    // SHOW_IMPORTANTLOG2(StoreImageConfig.ColorImageMaxSize);
    // SHOW_IMPORTANTLOG2(StoreImageConfig.PlateImageMaxSize);
    // SHOW_IMPORTANTLOG2(StoreImageConfig.AddBanner);

    // auto InputKafkaConfig = ConfigurateObj->getInputKafkaConfig();
    // SHOW_IMPORTANTLOG2(InputKafkaConfig.BootstrapServers);
    // SHOW_IMPORTANTLOG2(InputKafkaConfig.Topic);
    // SHOW_IMPORTANTLOG2(InputKafkaConfig.GroupID);

    // auto OutputKafkaConfig = ConfigurateObj->getOutputKafkaConfig();
    // SHOW_IMPORTANTLOG2(OutputKafkaConfig.BootstrapServers);
    // SHOW_IMPORTANTLOG2(OutputKafkaConfig.Topic);
    // SHOW_IMPORTANTLOG2(OutputKafkaConfig.GroupID);

    std::shared_ptr<WebService> service{std::make_shared<WebService>()};
    auto ServiceInitResponse = service->init();
    if(ServiceInitResponse.Code == Service::ServiceStatus::InitSuccessful)
    {
        boost::thread WebServiceThread(&WebService::run, service);
        WebServiceThread.detach();
    }

    std::shared_ptr<KafkaService> service2{std::make_shared<KafkaService>()};
    ServiceInitResponse = service2->init();
    if(ServiceInitResponse.Code == Service::ServiceStatus::InitSuccessful)
    {
        auto InputKafkaConfig = ConfigurateObj->getInputKafkaConfig();
        for(int i = 0; i < InputKafkaConfig.PartitionNumber; i++)
        {
            boost::thread KafkaServiceThread(&KafkaService::run, service2);
            KafkaServiceThread.detach();
        }
    }

    while(true)
    {
        sleep(10000);
    }

    return 0;
}