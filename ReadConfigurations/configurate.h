#ifndef CONFIGURATE_H
#define CONFIGURATE_H

#include "../Cryptography/systemkeys.h"
#include "../Cryptography/cryptotools.h"
#include "../Database/MongoDB.h"
#include "../Logs/log.h"

#include "../crow.h"
#include "../crow/json.h"

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
        bool Authentication;
        int TokenTimeAllowed;
        int threadNumber;
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
        int PartitionNumber;
    };

    struct CameraStruct
    {
        int DeviceID;
        std::string Username            = "";
        std::string Password            = "";
    };

    struct ServiceFieldsStruct
    {
        std::map<std::string, std::string> servicefields;
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
    std::vector<WebServiceConfigStruct> getWebServiceConfig();
    StoreImageConfigStruct getStoreImageConfig();
    KafkaConfigStruct getInputKafkaConfig();
    KafkaConfigStruct getOutputKafkaConfig();
    ServiceFieldsStruct getServiceFields();
    std::vector<CameraStruct> getCameras();

private:
    DatabaseStruct DatabaseConfig;
    DatabaseStruct DatabaseInsert;
    DatabaseStruct DatabaseFailed;
    std::vector<WebServiceConfigStruct> WebServiceConfig;
    StoreImageConfigStruct StoreImageConfig;
    KafkaConfigStruct InputKafkaConfig;
    KafkaConfigStruct OutputKafkaConfig;
    ServiceFieldsStruct ServiceFields;
    std::vector<CameraStruct> Cameras;

    static Configurate* InstancePtr;
    Configurate();
};

#endif // CONFIGURATE_H
