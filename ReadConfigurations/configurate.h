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
    struct ConnectDatabaseStruct
    {
        std::string DatabaseIP          = "";
        std::string DatabasePort        = "";
        std::string DatabaseUsername    = "";
        std::string DatabasePassword    = "";
        std::string DETAIL              = "";
    };

    struct InfoDatabaseStruct
    {
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
        int DaysforPassedTimeAcceptable;
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
        int DaysforPassedTimeAcceptable;
    };

    struct CameraStruct
    {
        int DeviceID;
        std::string Username            = "";
        std::string Password            = "";
    };

    struct InputFieldsStruct
    {
        bool DeviceID       =   false;
        bool UserID         =   false;
        bool StreetID       =   false;
        bool ViolationID    =   false;
        bool Direction      =   false;
        bool PlateValue     =   false;
        bool PlateType      =   false;
        bool Suspicious     =   false;
        bool Speed          =   false;
        bool VehicleType    =   false;
        bool VehicleColor   =   false;
        bool VehicleModel   =   false;
        bool Lane           =   false;
        bool PassedTime     =   false;
        bool ColorImage     =   false;
        bool GrayScaleImage =   false;
        bool PlateImage     =   false;
        bool Latitude       =   false;
        bool Longitude      =   false;
        bool Accuracy       =   false;
        bool PlateRect      =   false;
        bool CarRect        =   false;
        bool CodeType       =   false;
        bool MasterPlate    =   false;
        bool Probability    =   false;
        bool RecordID       =   false;
        bool ReceivedTime   =   false;
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

    void RunUpdateService();

    InfoDatabaseStruct getInsertDatabaseInfo();
    InfoDatabaseStruct getFailedDatabaseInfo();
    std::shared_ptr<MongoDB> getInsertDatabase();
    std::shared_ptr<MongoDB> getFailedDatabase();
    std::vector<WebServiceConfigStruct> getWebServiceConfig();
    StoreImageConfigStruct getStoreImageConfig();
    KafkaConfigStruct getInputKafkaConfig();
    KafkaConfigStruct getOutputKafkaConfig();
    InputFieldsStruct getInputFields();
    std::vector<CameraStruct> getCameras();

private:
    InfoDatabaseStruct ConfigDatabaseInfo;
    InfoDatabaseStruct InsertDatabaseInfo;
    InfoDatabaseStruct FailedDatabaseInfo;
    std::shared_ptr<MongoDB> ConfigDatabase;
    std::shared_ptr<MongoDB> InsertDatabase;
    std::shared_ptr<MongoDB> FailedDatabase;
    WebServiceConfigStruct ReadConfigServiceConfig;
    std::vector<WebServiceConfigStruct> WebServiceConfig;
    StoreImageConfigStruct StoreImageConfig;
    KafkaConfigStruct InputKafkaConfig;
    KafkaConfigStruct OutputKafkaConfig;
    InputFieldsStruct InputFields;
    std::vector<CameraStruct> Cameras;

    void ReadCamerasCollection();

    void UpdateRoute();

    static Configurate* InstancePtr;
    Configurate();

    std::shared_ptr<crow::SimpleApp> app;
    mutable std::shared_mutex UpdateConfig_mutex;
};

#endif // CONFIGURATE_H
