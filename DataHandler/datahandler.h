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
        std::tm PassedTimeLocal;
        std::tm ReceivedTimeLocal;
    };

    struct RequestStruct
    {
        std::string Host = "";
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
        Configurate::InputFieldsStruct hasInputFields;
        std::vector<Configurate::CameraStruct> Cameras;
        InputStruct Input;
        ProcessedInputDataStruct ProcessedInputData;
        RequestStruct Request;
        ResponseStruct Response;
        int DaysforPassedTimeAcceptable;
    };
}


#endif // DATAHANDLER_H