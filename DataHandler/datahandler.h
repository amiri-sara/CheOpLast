#ifndef DATAHANDLER_H
#define DATAHANDLER_H

#include "../Logs/log.h"
#include "../ReadConfigurations/configurate.h"
#include "../crow.h"
#include "../crow/json.h"

namespace DataHandler 
{
    struct InputStruct
    {
        int DeviceID;
        int UserID;
        int StreetID;
        int ViolationID;
        int Direction;
        int PlateType;
        int Speed;
        int VehicleType;
        int VehicleColor;
        int VehicleModel;
        int CompanyCode;
        int SystemCode;
        int Lane;
        int Accuracy;
        uint64_t PassedVehicleRecordsId;
        
        double Latitude;
        double Longitude;

        std::string PlateValue; 
        std::string Suspicious;
        std::string PassedTime;
        std::string ColorImage;
        std::string ImageAddress;
        std::string GrayScaleImage;
        std::string PlateImageBase64;
        std::string PlateRect;
        std::string CarRect;
        
        // Check Operator
        int CodeType;
        int Probability;
        std::string MasterPlate;

        uint64_t RecordID;
        std::string ReceivedTime;
    };

    struct ProcessedInputDataStruct
    {
        cv::Mat ColorImageMat;
        cv::Mat PlateImageMat;
        cv::Mat ThumbnailImage;
        cv::Mat croppedPlateImage;
        std::string PlateImageBase64;
        std::tm PassedTimeUTC;
        std::tm ReceivedTimeUTC;
        std::tm PassedTimeLocal;
        std::tm ReceivedTimeLocal;
        std::string MongoID;
        cv::Rect CarRect;
        cv::Rect PlateRect;
        std::unordered_map<std::string,int> ClassifierModuleOutput;
    };

    struct StoreImageAddressStruct
    {
        std::string ImageFolder;
        std::string ImageName;
        std::string ImageAddress;
    };

    struct RequestStruct
    {
        std::string remoteIP = "";
        std::string body = "";
        crow::json::rvalue JsonRvalue;
        crow::json::rvalue enJsonRvalue;
        int NumberofInputFields;
    };

    struct ResponseStruct
    {
        int errorCode   =   0;
        int HTTPCode    =   0;
        std::string Description;
    };

    struct DataHandlerStruct
    {
        Configurate::FieldsStruct hasInputFields;
        Configurate::FieldsStruct hasOutputFields;
        std::vector<Configurate::CameraStruct> Cameras;
        int CameraIndex;
        Configurate::StoreImageConfigStruct StoreImageConfig;
        std::unordered_map<int, Configurate::ViolationStruct> ViolationMap;
        Configurate::ViolationStruct ViolationInfo;
        InputStruct Input;
        ProcessedInputDataStruct ProcessedInputData;
        StoreImageAddressStruct StoreImageAddress; 
        RequestStruct Request;
        ResponseStruct Response;
        int DaysforPassedTimeAcceptable;
        std::shared_ptr<MongoDB> InsertDatabase;
        Configurate::InfoDatabaseStruct InsertDatabaseInfo;
        std::shared_ptr<MongoDB> FailedDatabase;
        std::shared_ptr<MongoDB> ConfigDatabase;
        Configurate::InfoDatabaseStruct ConfigDatabaseInfo;
        Configurate::InfoDatabaseStruct FailedDatabaseInfo;
        Configurate::ModulesStruct Modules;
        Configurate::MetaStruct Meta;
        bool DebugMode;
        bool MonitorMode;
        bool InsertRoute;
        bool DecryptedData;
        bool WebServiceAuthentication;
    };
}


#endif // DATAHANDLER_H