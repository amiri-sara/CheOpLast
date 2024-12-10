#ifndef CONFIGURATE_H
#define CONFIGURATE_H

#include "../Cryptography/systemkeys.h"
#include "../Cryptography/cryptotools.h"
#include "../Database/MongoDB.h"
#include "../Logs/log.h"
#include "../Time/timetools.h"

#include "../crow.h"
#include "../crow/json.h"

#define CONFIG_FILE_LOCATION        "/etc/CheckOperator.conf"

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
        bool TLSMode                    =false;
    };

    struct InfoDatabaseStruct
    {
        std::string DatabaseName        = "";
        std::string CollectionName      = "";
        bool Enable                     = false;
    };

    struct WebServiceInfoStruct
    {
        std::string URI                  = "";
        std::string IP                   = "";
        int Port;
    };

    struct CurlServiceInfoStruct
    {
        std::string URI                  = "";
        std::string IP                   = "";
        int Port;
    };

    struct ReadConfigServiceStruct
    {
        WebServiceInfoStruct ReadCamerasCollectionServiceInfo;
        WebServiceInfoStruct SetNewTokenServiceInfo;
        int threadNumber;
    };

    struct CurlServiceConfigStruct
    {
        CurlServiceInfoStruct CurlServiceInfo;
        int ThreadNumber;
        int ThresholdFetchedRecors;
        bool DebugMode;
        bool MonitorMode;
        bool ReadFromMinIdTXT;
        std::vector<Configurate::CurlServiceInfoStruct> OtherService;

    };
    struct WebServiceConfigStruct
    {
        WebServiceInfoStruct WebServiceInfo;
        bool Authentication;
        int TokenTimeAllowed;
        bool NotifyingOtherServicesTokenUpdate;
        int threadNumber;
        int DaysforPassedTimeAcceptable;
        bool DebugMode;
        std::vector<Configurate::WebServiceInfoStruct> OtherService;
        std::string KeysPath;
    };

    struct StoreImageConfigStruct
    {
        std::string StorePath           = "";
        int ColorImageMaxSize;
        int PlateImageMaxSize;
        int PlateImagePercent;
        bool AddBanner;
        std::string FontAddress         = "";
    };

    struct KafkaConfigStruct
    {
        std::string BootstrapServers    = "";
        std::string Topic               = "";
        std::string GroupID             = "";
        int PartitionNumber;
        int DaysforPassedTimeAcceptable;
        bool DebugMode;
    };

    struct TokenStruct
    {
        int Counter;
        std::time_t CreatedAt;
        std::time_t ExpiryAt;
        std::string Token;
    };
    
    struct CameraStruct
    {
        int DeviceID;
        std::string Username            = "";
        std::string Password            = "";
        std::string Location            = "";
        std::string CompanyName         = "";
        std::string CompanyID           = "";
        int PoliceCode;
        int AllowedSpeed;
        std::string subMode             = "";
        bool addBanner                  = false;
        bool addCrop                    = false;
        Configurate::TokenStruct TokenInfo;
    };

    struct FieldsStruct
    {
        bool DeviceID               =   false;
        bool UserID                 =   false;
        bool StreetID               =   false;
        bool ViolationID            =   false;
        bool Direction              =   false;
        bool PlateValue             =   false;
        bool PlateType              =   false;
        bool Suspicious             =   false;
        bool Speed                  =   false;
        bool VehicleType            =   false;
        bool VehicleColor           =   false;
        bool VehicleModel           =   false;
        bool Lane                   =   false;
        bool PassedTime             =   false;
        bool ColorImage             =   false;
        bool ImageAddress           =   false;
        bool GrayScaleImage         =   false;
        bool PlateImage             =   false;
        bool Latitude               =   false;
        bool Longitude              =   false;
        bool Accuracy               =   false;
        bool PlateRect              =   false;
        bool CarRect                =   false;
        bool CodeType               =   false;
        bool MasterPlate            =   false;
        bool Probability            =   false;
        bool RecordID               =   false;
        bool ReceivedTime           =   false;
        bool PassedVehicleRecordsId = false;
        bool CompanyCode            = false;
        bool SystemCode             = false;
    };

    struct ViolationStruct
    {
        std::string Description;
        std::string ImageSuffix;
    };

    struct ModelConfigStruct
    {
        std::string model           = "";
        std::string modelConfigPath = "";
        bool active                 = false;
    };

    struct CheckOperatorStruct
    {
        bool active;
        int NumberOfObjectPerService;
        std::string ModelsPath;
        bool IgnoreInputPlateType;
        Configurate::ModelConfigStruct PD;
        Configurate::ModelConfigStruct PC;
        Configurate::ModelConfigStruct IROCR;
        Configurate::ModelConfigStruct MBOCR;
        Configurate::ModelConfigStruct TZOCR;
        Configurate::ModelConfigStruct FZOCR;
        Configurate::ModelConfigStruct FROCR;
    };

    struct ClassifierModelConfigStruct : public ModelConfigStruct
    {
        int InputRectField         = 0;
        int InputImageType         = 0;
        bool UseRect               = false;
    };

    struct ClassifierStruct
    {
        bool active;
        int NumberOfObjectPerService;
        std::string ModelsPath;
        std::vector<Configurate::ClassifierModelConfigStruct> Models;
    };

    struct ModulesStruct
    {
        Configurate::CheckOperatorStruct CheckOperator;
        Configurate::ClassifierStruct Classifier;
    };
    struct MetaStruct
    {
        uint64_t last_processed_id = 0;
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
    std::vector<CurlServiceConfigStruct> getCurlServiceConfig();
    StoreImageConfigStruct getStoreImageConfig();
    KafkaConfigStruct getInputKafkaConfig();
    KafkaConfigStruct getOutputKafkaConfig();
    FieldsStruct getInputFields();
    FieldsStruct getOutputFields();
    std::vector<CameraStruct> getCameras();
    std::unordered_map<int, ViolationStruct> getViolationMap();
    Configurate::ModulesStruct getModules();
    Configurate::MetaStruct getMeta();
    std::shared_ptr<MongoDB> getConfigDatabase();
    InfoDatabaseStruct getConfigDatabaseInfo();

    void SetNewToken(int CameraIndex, std::string Token, std::time_t TokenTime);

private:
    InfoDatabaseStruct ConfigDatabaseInfo;
    InfoDatabaseStruct InsertDatabaseInfo;
    InfoDatabaseStruct FailedDatabaseInfo;
    std::shared_ptr<MongoDB> ConfigDatabase;
    std::shared_ptr<MongoDB> InsertDatabase;
    std::shared_ptr<MongoDB> FailedDatabase;
    ReadConfigServiceStruct ReadConfigServiceConfig;
    std::vector<WebServiceConfigStruct> WebServiceConfig;
    std::vector<CurlServiceConfigStruct> CurlServiceConfig;
    StoreImageConfigStruct StoreImageConfig;
    KafkaConfigStruct InputKafkaConfig;
    KafkaConfigStruct OutputKafkaConfig;
    FieldsStruct InputFields;
    FieldsStruct OutputFields;
    std::vector<CameraStruct> Cameras;
    std::unordered_map<int, ViolationStruct> ViolationMap;
    Configurate::ModulesStruct Modules;
    Configurate::MetaStruct Meta;

    void ReadCamerasCollection();

    void ReadCamerasCollectionRoute();
    void SetNewTokenRoute();

    static Configurate* InstancePtr;
    Configurate();

    std::shared_ptr<crow::SimpleApp> app;
    mutable std::shared_mutex UpdateConfig_mutex;
};

#endif // CONFIGURATE_H
