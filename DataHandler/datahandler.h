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
        int Lane;
        int Accuracy;
        
        double Latitude;
        double Longitude;

        std::string PlateValue; 
        std::string Suspicious;
        std::string PassedTime;
        std::string ColorImage;
        std::string ImageAddress;
        std::string GrayScaleImage;
        std::string PlateImage;
        std::string PlateRect;
        std::string CarRect;
        
        // Check Operator
        int CodeType;
        int Probability;
        std::string MasterPlate;

        std::string RecordID;
        std::string ReceivedTime;
    };

    struct ProcessedInputDataStruct
    {
        cv::Mat ColorImageMat;
        cv::Mat PlateImageMat;
        cv::Mat ThumbnailImage;
        std::tm PassedTimeLocal;
        std::tm ReceivedTimeLocal;
        std::string MongoID;
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
        Configurate::ModulesStruct Modules;
        bool DebugMode;
        bool InsertRoute;
        bool DecryptedData;
        bool WebServiceAuthentication;
    };
}


#endif // DATAHANDLER_H