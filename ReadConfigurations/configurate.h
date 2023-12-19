#ifndef CONFIGURATE_H
#define CONFIGURATE_H

#include "../Cryptography/systemkeys.h"
#include "../Cryptography/cryptotools.h"
#include "../Database/MongoDB.h"
#include "../Logs/log.h"

#define CONFIG_FILE_LOCATION        "/etc/Aggregation.conf"

class Configurate
{
public:
    struct DatabaseStruct
    {
        std::string DatabaseIP          = "";
        std::string DatabasePort        = "";
        std::string DatabaseUsername    = "";
        std::string DatabasePassword    = "";
        std::string DETAIL              = "";
        std::string DatabaseName        = "";
        std::string CollectionName      = "";
        std::string Enable              = "";
    };

    struct WebServiceConfigStruct
    {
        std::string URI                 = "";
        int Port;
        bool CheckToken;
        int TokenTimeAllowed;
    };

    struct StoreImageConfigStruct
    {
        std::string StorePath           = "";
        int ColorImageMaxSize;
        int PlateImageMaxSize;
        bool AddBanner;
    };

    struct KafkaConfigStruct
    {
        std::string BootstrapServers    = "";
        std::string Topic               = "";
        std::string GroupID             = "";
    };

    Configurate(const Configurate& Obj) = delete;

    static Configurate* getInstance()
    {
        if (InstancePtr == NULL)
        {
            InstancePtr = new Configurate();
            return InstancePtr;
        }
        else
        {
            return InstancePtr;
        }
    }

    DatabaseStruct getDatabaseConfig();
    DatabaseStruct getDatabaseInsert();
    DatabaseStruct getDatabaseFailed();
    WebServiceConfigStruct getWebServiceConfig();
    StoreImageConfigStruct getStoreImageConfig();
    KafkaConfigStruct getInputKafkaConfig();
    KafkaConfigStruct getOutputKafkaConfig();

private:
    DatabaseStruct DatabaseConfig;
    DatabaseStruct DatabaseInsert;
    DatabaseStruct DatabaseFailed;
    WebServiceConfigStruct WebServiceConfig;
    StoreImageConfigStruct StoreImageConfig;
    KafkaConfigStruct InputKafkaConfig;
    KafkaConfigStruct OutputKafkaConfig;

    static Configurate* InstancePtr;
    Configurate();
};

#endif // CONFIGURATE_H
