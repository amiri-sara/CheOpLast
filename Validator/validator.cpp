#include "validator.h"

bool Validator::run(const std::shared_ptr<DataHandler::DataHandlerStruct> &DH)
{
    // Check Request Format is JSON
    if(!(this->CheckRequstFormatJSON(DH)))
        return false;

    // Check Input fields exist in request or not
    if(DH->InsertRoute)
    {
        if(DH->DecryptedData)
        {
            if(!(this->checkDataExistOrNo(DH)))
                return false;
        }else
        {
            if(!(this->checkEncryptedDataExistOrNo(DH)))
                return false;
        }
    } else
    {
        if(!(this->checkTokenDataExistOrNo(DH)))
            return false;
    }

    // Check Number of Input fields
    if(!(this->CheckNumberOfJSONFields(DH)))
        return false;

    // Check Request values
    if(DH->InsertRoute)
    {
        if(DH->DecryptedData)
        {
            if(!(this->CheckRequestValues(DH)))
                return false;
        }else
        {
            if(!(this->CheckEncryptedRequestValues(DH)))
                return false;
        }
    } else
    {
        if(!(this->CheckTokenRequestValues(DH)))
            return false;
    }

    return true;
}

bool Validator::CheckRequstFormatJSON(const std::shared_ptr<DataHandler::DataHandlerStruct> &DH)
{
    bool isJson  = false;
    try {
        if(DH->DecryptedData)
        {
            DH->Request.JsonRvalue = crow::json::load(DH->Request.body);
        } else 
        {   
            DH->Request.enJsonRvalue = crow::json::load(DH->Request.body);
        }
        isJson = true;
    }catch (...) 
    {
        DH->Response.HTTPCode = 400;
        DH->Response.errorCode = INVALIDJSON;
        DH->Response.Description = "Body of request is invalid";
    }
    return isJson;
}

bool Validator::checkDataExistOrNo(const std::shared_ptr<DataHandler::DataHandlerStruct> &DH)
{
    bool allDataExist = false;
    DH->Request.NumberofInputFields = 0;

    if(DH->hasInputFields.DeviceID)
    {
        DH->Request.NumberofInputFields++;
        if(!(DH->Request.JsonRvalue.has("DeviceID")))
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDDEVICEID;
            DH->Response.Description = "DeviceID Dont Exist In Body Of JSON";
            return allDataExist;
        }
    }

    if(DH->hasInputFields.UserID)
    {
        DH->Request.NumberofInputFields++;
        if(!(DH->Request.JsonRvalue.has("UserID")))
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDUSERID;
            DH->Response.Description = "UserID Dont Exist In Body Of JSON";
            return allDataExist;
        }
    }

    if(DH->hasInputFields.StreetID)
    {
        DH->Request.NumberofInputFields++;
        if(!(DH->Request.JsonRvalue.has("StreetID")))
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDSTREETID;
            DH->Response.Description = "StreetID Dont Exist In Body Of JSON";
            return allDataExist;
        }
    }

    if(DH->hasInputFields.ViolationID)
    {
        DH->Request.NumberofInputFields++;
        if(!(DH->Request.JsonRvalue.has("ViolationID")))
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDVIOLATIONID;
            DH->Response.Description = "ViolationID Dont Exist In Body Of JSON";
            return allDataExist;
        }
    }

    if(DH->hasInputFields.Direction)
    {
        DH->Request.NumberofInputFields++;
        if(!(DH->Request.JsonRvalue.has("Direction")))
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDDIRECTION;
            DH->Response.Description = "Direction Dont Exist In Body Of JSON";
            return allDataExist;
        }
    }

    if(DH->hasInputFields.PlateValue)
    {
        DH->Request.NumberofInputFields++;
        if(!(DH->Request.JsonRvalue.has("PlateValue")))
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDPLATEVALUE;
            DH->Response.Description = "PlateValue Dont Exist In Body Of JSON";
            return allDataExist;
        }
    }

    if(DH->hasInputFields.PlateType)
    {
        DH->Request.NumberofInputFields++;
        if(!(DH->Request.JsonRvalue.has("PlateType")))
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDPLATETYPE;
            DH->Response.Description = "PlateType Dont Exist In Body Of JSON";
            return allDataExist;
        }
    }

    if(DH->hasInputFields.Suspicious)
    {
        DH->Request.NumberofInputFields++;
        if(!(DH->Request.JsonRvalue.has("Suspicious")))
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDSUSPICIOUS;
            DH->Response.Description = "Suspicious Dont Exist In Body Of JSON";
            return allDataExist;
        }
    }

    if(DH->hasInputFields.Speed)
    {
        DH->Request.NumberofInputFields++;
        if(!(DH->Request.JsonRvalue.has("Speed")))
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDSPEED;
            DH->Response.Description = "Speed Dont Exist In Body Of JSON";
            return allDataExist;
        }
    }

    if(DH->hasInputFields.VehicleType)
    {
        DH->Request.NumberofInputFields++;
        if(!(DH->Request.JsonRvalue.has("VehicleType")))
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDVEHICLETYPE;
            DH->Response.Description = "VehicleType Dont Exist In Body Of JSON";
            return allDataExist;
        }
    }

    if(DH->hasInputFields.VehicleColor)
    {
        DH->Request.NumberofInputFields++;
        if(!(DH->Request.JsonRvalue.has("VehicleColor")))
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDVEHICLECOLOR;
            DH->Response.Description = "VehicleColor Dont Exist In Body Of JSON";
            return allDataExist;
        }
    }

    if(DH->hasInputFields.VehicleModel)
    {
        DH->Request.NumberofInputFields++;
        if(!(DH->Request.JsonRvalue.has("VehicleModel")))
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDVEHICLEMODEL;
            DH->Response.Description = "VehicleModel Dont Exist In Body Of JSON";
            return allDataExist;
        }
    }

    if(DH->hasInputFields.Lane)
    {
        DH->Request.NumberofInputFields++;
        if(!(DH->Request.JsonRvalue.has("Lane")))
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDLANE;
            DH->Response.Description = "Lane Dont Exist In Body Of JSON";
            return allDataExist;
        }
    }

    if(DH->hasInputFields.PassedTime)
    {
        DH->Request.NumberofInputFields++;
        if(!(DH->Request.JsonRvalue.has("PassedTime")))
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDPASSEDTIME;
            DH->Response.Description = "PassedTime Dont Exist In Body Of JSON";
            return allDataExist;
        }
    }

    if(DH->hasInputFields.ColorImage)
    {
        DH->Request.NumberofInputFields++;
        if(!(DH->Request.JsonRvalue.has("ColorImage")))
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDCOLORIMAGE;
            DH->Response.Description = "ColorImage Dont Exist In Body Of JSON";
            return allDataExist;
        }
    }

    if(DH->hasInputFields.GrayScaleImage)
    {
        DH->Request.NumberofInputFields++;
        if(!(DH->Request.JsonRvalue.has("GrayScaleImage")))
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDGRAYSCALEIMAGE;
            DH->Response.Description = "GrayScaleImage Dont Exist In Body Of JSON";
            return allDataExist;
        }
    }

    if(DH->hasInputFields.PlateImage)
    {
        DH->Request.NumberofInputFields++;
        if(!(DH->Request.JsonRvalue.has("PlateImage")))
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDPLATEIMAGE;
            DH->Response.Description = "PlateImage Dont Exist In Body Of JSON";
            return allDataExist;
        }
    }

    if(DH->hasInputFields.Latitude)
    {
        DH->Request.NumberofInputFields++;
        if(!(DH->Request.JsonRvalue.has("Latitude")))
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDLATITUDE;
            DH->Response.Description = "Latitude Dont Exist In Body Of JSON";
            return allDataExist;
        }
    }

    if(DH->hasInputFields.Longitude)
    {
        DH->Request.NumberofInputFields++;
        if(!(DH->Request.JsonRvalue.has("Longitude")))
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDLONGITUDE;
            DH->Response.Description = "Longitude Dont Exist In Body Of JSON";
            return allDataExist;
        }
    }

    if(DH->hasInputFields.Accuracy)
    {
        DH->Request.NumberofInputFields++;
        if(!(DH->Request.JsonRvalue.has("Accuracy")))
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDACCURACY;
            DH->Response.Description = "Accuracy Dont Exist In Body Of JSON";
            return allDataExist;
        }
    }

    if(DH->hasInputFields.PlateRect)
    {
        DH->Request.NumberofInputFields++;
        if(!(DH->Request.JsonRvalue.has("PlateRect")))
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDPLATERECT;
            DH->Response.Description = "PlateRect Dont Exist In Body Of JSON";
            return allDataExist;
        }
    }

    if(DH->hasInputFields.CarRect)
    {
        DH->Request.NumberofInputFields++;
        if(!(DH->Request.JsonRvalue.has("CarRect")))
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDCARRECT;
            DH->Response.Description = "CarRect Dont Exist In Body Of JSON";
            return allDataExist;
        }
    }

    if(DH->hasInputFields.CodeType)
    {
        DH->Request.NumberofInputFields++;
        if(!(DH->Request.JsonRvalue.has("CodeType")))
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDCODETYPE;
            DH->Response.Description = "CodeType Dont Exist In Body Of JSON";
            return allDataExist;
        }
    }

    if(DH->hasInputFields.MasterPlate)
    {
        DH->Request.NumberofInputFields++;
        if(!(DH->Request.JsonRvalue.has("MasterPlate")))
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDMASTERPLATE;
            DH->Response.Description = "MasterPlate Dont Exist In Body Of JSON";
            return allDataExist;
        }
    }

    if(DH->hasInputFields.Probability)
    {
        DH->Request.NumberofInputFields++;
        if(!(DH->Request.JsonRvalue.has("Probability")))
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDPROBABILITY;
            DH->Response.Description = "Probability Dont Exist In Body Of JSON";
            return allDataExist;
        }
    }

    if(DH->hasInputFields.RecordID)
    {
        DH->Request.NumberofInputFields++;
        if(!(DH->Request.JsonRvalue.has("RecordID")))
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDRECORDID;
            DH->Response.Description = "RecordID Dont Exist In Body Of JSON";
            return allDataExist;
        }
    }

    if(DH->hasInputFields.ReceivedTime)
    {
        DH->Request.NumberofInputFields++;
        if(!(DH->Request.JsonRvalue.has("ReceivedTime")))
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDRECEIVEDTIME;
            DH->Response.Description = "ReceivedTime Dont Exist In Body Of JSON";
            return allDataExist;
        }
    }

    // UUID
    if(DH->Request.JsonRvalue.has("UUID"))
    {
        DH->Request.NumberofInputFields++;
    }

    allDataExist = true;
    return allDataExist;
}

bool Validator::checkEncryptedDataExistOrNo(const std::shared_ptr<DataHandler::DataHandlerStruct> &DH)
{
    bool allDataExist = false;
    DH->Request.NumberofInputFields = 2;

    if(!(DH->Request.enJsonRvalue.has("Token")))
    {
        DH->Response.HTTPCode = 401;
        DH->Response.errorCode = INVALIDTOKEN;
        DH->Response.Description = "Token Dont Exist In Body Of JSON.";
        return allDataExist;
    }

    if(!(DH->Request.enJsonRvalue.has("Data")))
    {
        DH->Response.HTTPCode = 400;
        DH->Response.errorCode = INVALIDDATA;
        DH->Response.Description = "Data Dont Exist In Body Of JSON";
        return allDataExist;
    }

    return true;
}

bool Validator::checkTokenDataExistOrNo(const std::shared_ptr<DataHandler::DataHandlerStruct> &DH)
{
    bool allDataExist = false;
    DH->Request.NumberofInputFields = 4;

    if(!(DH->Request.JsonRvalue.has("DeviceID")))
    {
        DH->Response.HTTPCode = 400;
        DH->Response.errorCode = INVALIDDEVICEID;
        DH->Response.Description = "DeviceID Dont Exist In Body Of JSON";
        return allDataExist;
    }

    if(!(DH->Request.JsonRvalue.has("User")))
    {
        DH->Response.HTTPCode = 400;
        DH->Response.errorCode = INVALIDUSERPASS;
        DH->Response.Description = "User Dont Exist In Body Of JSON";
        return allDataExist;
    }

    if(!(DH->Request.JsonRvalue.has("Password")))
    {
        DH->Response.HTTPCode = 400;
        DH->Response.errorCode = INVALIDUSERPASS;
        DH->Response.Description = "Password Dont Exist In Body Of JSON";
        return allDataExist;
    }

    if(!(DH->Request.JsonRvalue.has("CompanyName")))
    {
        DH->Response.HTTPCode = 400;
        DH->Response.errorCode = INVALIDCOMPANYNAME;
        DH->Response.Description = "CompanyName Dont Exist In Body Of JSON";
        return allDataExist;
    }

    return true;
}

bool Validator::CheckNumberOfJSONFields(const std::shared_ptr<DataHandler::DataHandlerStruct> &DH)
{
    int SizeOfJson;
    if(DH->DecryptedData)
    {
        SizeOfJson = DH->Request.JsonRvalue.size();
    } else 
    {
        SizeOfJson = DH->Request.enJsonRvalue.size();
    }
    if(SizeOfJson == DH->Request.NumberofInputFields)
        return true;
    
    DH->Response.HTTPCode = 400;
    DH->Response.errorCode = INVALIDJSON;
    DH->Response.Description = "The number of fields sent is incorrect";
    return false;
}

bool Validator::CheckRequestValues(const std::shared_ptr<DataHandler::DataHandlerStruct> &DH)
{
    // DeviceID
    if(DH->hasInputFields.DeviceID)
    {
        int DeviceID;
        try
        {
            DeviceID = DH->Request.JsonRvalue["DeviceID"].i();
        }catch(...)
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDDEVICEID;
            DH->Response.Description = "The type of DeviceID is invalid.";
            return false; 
        }

#ifdef VALUEVALIDATION
        std::string DeviceIDStr = std::to_string(DeviceID);
        if(DeviceIDStr.length() != 7)
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDDEVICEID;
            DH->Response.Description = "The number of digits of the DeviceID value must be equal to 7.";
            return false;    
        }

        bool DeviceIDExist = false;
        for(auto& camera : DH->Cameras)
        {
            if(DeviceID == camera.DeviceID)
            {
                DeviceIDExist = true;
                break;
            }
        }

        if(!DeviceIDExist)
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDDEVICEID;
            DH->Response.Description = "This DeviceID does not exist.";
            return false; 
        }

        if(DH->WebServiceAuthentication)
        {
            if(DH->Cameras[DH->CameraIndex].DeviceID != DeviceID)
            {
                DH->Response.HTTPCode = 401;
                DH->Response.errorCode = INVALIDTOKEN;
                DH->Response.Description = "Invalid Token.";
                return false; 
            }
        }
#endif // VALUEVALIDATION

        DH->Input.DeviceID = DeviceID;
    }

    // UserID
    if(DH->hasInputFields.UserID)
    {
        int UserID;
        try
        {
            UserID = DH->Request.JsonRvalue["UserID"].i();
        }catch(...)
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDUSERID;
            DH->Response.Description = "The type of UserID is invalid.";
            return false; 
        }

#ifdef VALUEVALIDATION
        std::string UserIDStr = std::to_string(UserID);
        if(UserIDStr.length() > 10)
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDUSERID;
            DH->Response.Description = "The number of digits of the UserID value should not be more than 10.";
            return false;
        }
#endif // VALUEVALIDATION
        
        DH->Input.UserID = UserID;
    }

    // StreetID
    if(DH->hasInputFields.StreetID)
    {
        int StreetID;
        try
        {
            StreetID = DH->Request.JsonRvalue["StreetID"].i();
        }catch(...)
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDSTREETID;
            DH->Response.Description = "The type of StreetID is invalid.";
            return false; 
        }

#ifdef VALUEVALIDATION
        std::string StreetIDStr = std::to_string(StreetID);
        if(StreetID != 0 && (StreetIDStr.length() < 1 || StreetIDStr.length() > 12))
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDSTREETID;
            DH->Response.Description = "The number of digits of the StreetID value must be a value between 1 and 12.";
            return false;
        }
#endif // VALUEVALIDATION
        
        DH->Input.StreetID = StreetID;
    }

    // ViolationID
    if(DH->hasInputFields.ViolationID)
    {
        int ViolationID;
        try
        {
            ViolationID = DH->Request.JsonRvalue["ViolationID"].i();
        }catch(...)
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDVIOLATIONID;
            DH->Response.Description = "The type of ViolationID is invalid.";
            return false; 
        }
        
#ifdef VALUEVALIDATION
        if(ViolationID != 0 && (ViolationID < 2002 || ViolationID > 2255))
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDVIOLATIONID;
            DH->Response.Description = "The value of ViolationID is invalid.";
            return false;
        }
#endif // VALUEVALIDATION

        DH->Input.ViolationID = ViolationID;
    }

    // Direction
    if(DH->hasInputFields.Direction)
    {
        int Direction;
        try
        {
            Direction = DH->Request.JsonRvalue["Direction"].i();
        }catch(...)
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDDIRECTION;
            DH->Response.Description = "The type of Direction is invalid.";
            return false; 
        }

#ifdef VALUEVALIDATION
        if(Direction < -1 || Direction > 2)
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDDIRECTION;
            DH->Response.Description = "The value of Direction must be between -1 and 2.";
            return false;
        }
#endif // VALUEVALIDATION

        DH->Input.Direction = Direction;
    }

    // PlateType
    if(DH->hasInputFields.PlateType)
    {
        int PlateType;
        try
        {
            PlateType = DH->Request.JsonRvalue["PlateType"].i();
        }catch(...)
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDPLATETYPE;
            DH->Response.Description = "The type of PlateType is invalid.";
            return false; 
        }
        
#ifdef VALUEVALIDATION
        if(PlateType < 0 || PlateType > 100)
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDPLATETYPE;
            DH->Response.Description = "The value of PlateType is invalid.";
            return false;
        }
#endif // VALUEVALIDATION

        DH->Input.PlateType = PlateType;
    }
    
    // PlateValue
    if(DH->hasInputFields.PlateValue)
    {
        std::string PlateValue;
        try
        {
            PlateValue = DH->Request.JsonRvalue["PlateValue"].s();
        }catch(...)
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDPLATEVALUE;
            DH->Response.Description = "The type of PlateValue is invalid.";
            return false; 
        }

#ifdef VALUEVALIDATION
        if(PlateValue.empty())
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDPLATEVALUE;
            DH->Response.Description = "The PlateValue should not be empty.";
            return false;
        }

        if(PlateValue == "0")
        {
            // Just to not validate the plate value and accept zero value
        }else if(DH->Input.PlateType == 1)
        {
            if(PlateValue.length() != 9 || !(std::all_of(PlateValue.begin(), PlateValue.end(),[](char c) { return std::isdigit(static_cast<unsigned char>(c));})))
            {
                DH->Response.HTTPCode = 400;
                DH->Response.errorCode = INVALIDPLATEVALUE;
                DH->Response.Description = "The value of PlateValue is invalid.";
                return false;
            }
        }else if(DH->Input.PlateType == 2)
        {
            if(PlateValue.length() != 8 || !(std::all_of(PlateValue.begin(), PlateValue.end(),[](char c) { return std::isdigit(static_cast<unsigned char>(c));})))
            {
                DH->Response.HTTPCode = 400;
                DH->Response.errorCode = INVALIDPLATEVALUE;
                DH->Response.Description = "The value of PlateValue is invalid.";
                return false;
            }
        }else if(DH->Input.PlateType == 3)
        {
            if(PlateValue.length() < 5 || PlateValue.length() > 7 || !(std::all_of(PlateValue.begin(), PlateValue.end(),[](char c) { return std::isdigit(static_cast<unsigned char>(c));})))
            {
                DH->Response.HTTPCode = 400;
                DH->Response.errorCode = INVALIDPLATEVALUE;
                DH->Response.Description = "The value of PlateValue is invalid.";
                return false;
            }
        }else if(DH->Input.PlateType == 4 || DH->Input.PlateType == 5)
        {
            if(PlateValue.length() < 7 || PlateValue.length() > 8)
            {
                DH->Response.HTTPCode = 400;
                DH->Response.errorCode = INVALIDPLATEVALUE;
                DH->Response.Description = "The value of PlateValue is invalid.";
                return false;
            }
        }else
        {
            if(PlateValue.length() > 15)
            {
                DH->Response.HTTPCode = 400;
                DH->Response.errorCode = INVALIDPLATEVALUE;
                DH->Response.Description = "The value of PlateValue is invalid.";
                return false;
            }
        }
#endif // VALUEVALIDATION

        DH->Input.PlateValue = PlateValue;
    }

    // Suspicious
    if(DH->hasInputFields.Suspicious)
    {
        std::string Suspicious;
        try
        {
            Suspicious = DH->Request.JsonRvalue["Suspicious"].s();
        }catch(...)
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDSUSPICIOUS;
            DH->Response.Description = "The type of Suspicious is invalid.";
            return false; 
        }

#ifdef VALUEVALIDATION
        if(Suspicious.length() > 15)
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDSUSPICIOUS;
            DH->Response.Description = "The number of characters of the Suspicious value should not be more than 15.";
            return false;
        }
#endif // VALUEVALIDATION

        DH->Input.Suspicious = Suspicious;
    }

    // Speed
    if(DH->hasInputFields.Speed)
    {
        int Speed;
        try
        {
            Speed = DH->Request.JsonRvalue["Speed"].i();
        }catch(...)
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDSPEED;
            DH->Response.Description = "The type of Speed is invalid.";
            return false; 
        }

#ifdef VALUEVALIDATION
        if(Speed < 0 || Speed > 300)
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDSPEED;
            DH->Response.Description = "The value of Speed must be a value between 0 and 300.";
            return false;
        }
#endif // VALUEVALIDATION 

        DH->Input.Speed = Speed;
    }    
    
    // VehicleType
    if(DH->hasInputFields.VehicleType)
    {
        int VehicleType;
        try
        {
            VehicleType = DH->Request.JsonRvalue["VehicleType"].i();
        }catch(...)
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDVEHICLETYPE;
            DH->Response.Description = "The type of VehicleType is invalid.";
            return false; 
        }

#ifdef VALUEVALIDATION
        if(VehicleType < 0 || VehicleType > 2)
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDVEHICLETYPE;
            DH->Response.Description = "The value of VehicleType must be a value between 0 and 2.";
            return false;
        }
#endif // VALUEVALIDATION

        DH->Input.VehicleType = VehicleType;
    }
    
    // VehicleColor
    if(DH->hasInputFields.VehicleColor)
    {
        int VehicleColor;
        try
        {
            VehicleColor = DH->Request.JsonRvalue["VehicleColor"].i();
        }catch(...)
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDVEHICLECOLOR;
            DH->Response.Description = "The type of VehicleColor is invalid.";
            return false; 
        }

#ifdef VALUEVALIDATION
        if(VehicleColor < -1 || VehicleColor > 20)
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDVEHICLECOLOR;
            DH->Response.Description = "The value of VehicleColor is invalid.";
            return false;
        }
#endif // VALUEVALIDATION

        DH->Input.VehicleColor = VehicleColor;
    }

    // VehicleModel
    if(DH->hasInputFields.VehicleModel)
    {
        int VehicleModel;
        try
        {
            VehicleModel = DH->Request.JsonRvalue["VehicleModel"].i();
        }catch(...)
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDVEHICLEMODEL;
            DH->Response.Description = "The type of VehicleModel is invalid.";
            return false; 
        }

#ifdef VALUEVALIDATION
        if(VehicleModel < -1 || VehicleModel > 100)
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDVEHICLEMODEL;
            DH->Response.Description = "The value of VehicleModel is invalid.";
            return false;
        }
#endif // VALUEVALIDATION

        DH->Input.VehicleModel = VehicleModel;
    }

    // Lane
    if(DH->hasInputFields.Lane)
    {
        int Lane;
        try
        {
            Lane = DH->Request.JsonRvalue["Lane"].i();
        }catch(...)
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDLANE;
            DH->Response.Description = "The type of Lane is invalid.";
            return false; 
        }

#ifdef VALUEVALIDATION
        if(Lane < 0 || Lane > 9)
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDLANE;
            DH->Response.Description = "The value of Lane must be a value between 0 and 9.";
            return false;
        }
#endif // VALUEVALIDATION

        DH->Input.Lane = Lane;
    }
    
    // PassedTime
    if(DH->hasInputFields.PassedTime)
    {
        std::string PassedTime;
        try
        {
            PassedTime = DH->Request.JsonRvalue["PassedTime"].s();
        }catch(...)
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDPASSEDTIME;
            DH->Response.Description = "The type of PassedTime is invalid.";
            return false; 
        }

#ifdef VALUEVALIDATION
        if(PassedTime.empty() || (PassedTime.length() != 20 && PassedTime.length() != 24))
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDPASSEDTIME;
            DH->Response.Description = "The value of PassedTime must not be empty and its number of characters must be equal to 20 or 24(YYYY-MM-DDTHH:MM:SSZ or YYYY-MM-DDTHH:MM:SS.sssZ).";
            return false;
        }
#endif // VALUEVALIDATION

        std::tm PassedTimeLocal = {};
        if(!(ConvertISO8601TimeToLocal(PassedTime, PassedTimeLocal)))
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDPASSEDTIME;
            DH->Response.Description = "The value of PassedTime is invalid.";
            return false;
        }

#ifdef VALUEVALIDATION
        std::tm DayAgo = getDaysAgo(DH->DaysforPassedTimeAcceptable);
        if(compareDate(PassedTimeLocal, DayAgo) == -1)
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = OLDDATA;
            DH->Response.Description = "The data is old.";
            return false;
        }
#endif // VALUEVALIDATION

        DH->Input.PassedTime = PassedTime;
        DH->ProcessedInputData.PassedTimeLocal = PassedTimeLocal;
    }

    // ColorImage
    if(DH->hasInputFields.ColorImage)
    {
        std::string ColorImage;
        try
        {
            ColorImage = DH->Request.JsonRvalue["ColorImage"].s();
        }catch(...)
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDCOLORIMAGE;
            DH->Response.Description = "The type of ColorImage is invalid.";
            return false; 
        }

#ifdef VALUEVALIDATION
        if(ColorImage.empty())
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDCOLORIMAGE;
            DH->Response.Description = "The ColorImage should not be empty.";
            return false;
        }
#endif // VALUEVALIDATION

        cv::Mat ColorImageMat = convertBase64ToMatImage(ColorImage);
        if(ColorImageMat.total() == 0)
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDCOLORIMAGE;
            DH->Response.Description = "The value of ColorImage is invalid.";
            return false;
        }
        
#ifdef VALUEVALIDATION
        auto ColorImageLength = ColorImage.length();
        auto sizeInBytes = 4 * std::ceil((ColorImageLength / 3))*0.5624896334383812;
        auto sizeInKb = sizeInBytes/1000;

        if(sizeInKb > DH->StoreImageConfig.ColorImageMaxSize)
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDCOLORIMAGESIZE;
            DH->Response.Description = "The size of the ColorImage should not be more than " + std::to_string(DH->StoreImageConfig.ColorImageMaxSize) + "KB";
            return false;
        }

        if(ColorImageMat.channels() != 3)
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDCOLORIMAGE;
            DH->Response.Description = "ColorImage should have 3 channels.";
            return false;
        }
#endif // VALUEVALIDATION

        DH->Input.ColorImage = ColorImage;
#ifdef STOREIMAGE
        DH->ProcessedInputData.ColorImageMat = ColorImageMat;
#endif // STOREIMAGE
    }

    // ImageAddress
    if(DH->hasInputFields.ImageAddress)
    {
        std::string ImageAddress;
        try
        {
            ImageAddress = DH->Request.JsonRvalue["ImageAddress"].s();
        }catch(...)
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDCOLORIMAGE;
            DH->Response.Description = "The type of ImageAddress is invalid.";
            return false; 
        }

        DH->Input.ImageAddress = ImageAddress;
    }

    // GrayScaleImage
    if(DH->hasInputFields.GrayScaleImage)
    {

    }
    
    // PlateImage
    if(DH->hasInputFields.PlateImage)
    {
        std::string PlateImage;
        try
        {
            PlateImage = DH->Request.JsonRvalue["PlateImage"].s();
        }catch(...)
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDPLATEIMAGE;
            DH->Response.Description = "The type of PlateImage is invalid.";
            return false; 
        }

#ifdef VALUEVALIDATION
        if(PlateImage.empty())
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDPLATEIMAGE;
            DH->Response.Description = "The PlateImage should not be empty.";
            return false;
        }
#endif // VALUEVALIDATION

        cv::Mat PlateImageMat = convertBase64ToMatImage(PlateImage);
        if(PlateImageMat.total() == 0)
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDPLATEIMAGE;
            DH->Response.Description = "The value of PlateImage is invalid.";
            return false;
        }

#ifdef VALUEVALIDATION
        auto PlateImageLength = PlateImage.length();
        auto sizeInBytes = 4 * std::ceil((PlateImageLength / 3))*0.5624896334383812;
        auto sizeInKb = sizeInBytes/1000;

        if(sizeInKb > DH->StoreImageConfig.PlateImageMaxSize)
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDPLATEIMAGESIZE;
            DH->Response.Description = "The size of the Plate Image should not be more than " + std::to_string(DH->StoreImageConfig.PlateImageMaxSize) + "KB";
            return false;
        }
#endif // VALUEVALIDATION

        DH->Input.PlateImageBase64 = PlateImage;

#ifdef STOREIMAGE
        DH->ProcessedInputData.PlateImageMat = PlateImageMat;
#else
        if(DH->Modules.CheckOperator.active)
            DH->ProcessedInputData.PlateImageMat = PlateImageMat;
#endif // STOREIMAGE
    }

    // Latitude
    if(DH->hasInputFields.Latitude)
    {
        double Latitude;
        try
        {
            Latitude = DH->Request.JsonRvalue["Latitude"].d();
        }catch(...)
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDLATITUDE;
            DH->Response.Description = "The type of Latitude is invalid.";
            return false; 
        }

#ifdef VALUEVALIDATION
        std::string LatitudeStr = std::to_string(Latitude);
        if(Latitude < 0 || LatitudeStr.length() > 10)
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDLATITUDE;
            DH->Response.Description = "The value of Latitude is invalid.";
            return false;
        }
#endif // VALUEVALIDATION

        DH->Input.Latitude = Latitude;
    }

    // Longitude
    if(DH->hasInputFields.Longitude)
    {
        double Longitude;
        try
        {
            Longitude = DH->Request.JsonRvalue["Longitude"].d();
        }catch(...)
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDLONGITUDE;
            DH->Response.Description = "The type of Longitude is invalid.";
            return false; 
        }

#ifdef VALUEVALIDATION
        std::string LongitudeStr = std::to_string(Longitude);
        if(Longitude < 0 || LongitudeStr.length() > 10)
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDLONGITUDE;
            DH->Response.Description = "The value of Longitude is invalid.";
            return false;
        }
#endif // VALUEVALIDATION

        DH->Input.Longitude = Longitude;
    }

    // Accuracy
    if(DH->hasInputFields.Accuracy)
    {
        int Accuracy;
        try
        {
            Accuracy = DH->Request.JsonRvalue["Accuracy"].i();
        }catch(...)
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDACCURACY;
            DH->Response.Description = "The type of Accuracy is invalid.";
            return false; 
        }

#ifdef VALUEVALIDATION
        if(Accuracy < 0 || Accuracy > 100)
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDACCURACY;
            DH->Response.Description = "The value of Accuracy is invalid.";
            return false;
        }
#endif // VALUEVALIDATION

        DH->Input.Accuracy = Accuracy;
    }   

    // PlateRect
    if(DH->hasInputFields.PlateRect)
    {
        std::string PlateRect;
        try
        {
            PlateRect = DH->Request.JsonRvalue["PlateRect"].s();
        }catch(...)
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDPLATERECT;
            DH->Response.Description = "The type of PlateRect is invalid.";
            return false; 
        }

#ifdef VALUEVALIDATION
        if(PlateRect.empty() || PlateRect.length() > 20)
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDPLATERECT;
            DH->Response.Description = "The value of PlateRect is invalid.";
            return false;
        }
#endif // VALUEVALIDATION

        DH->Input.PlateRect = PlateRect;
        int x = 0, y = 0, width = 0, height = 0;
        char delimiter;
        std::stringstream ss(PlateRect);
        ss >> x >> delimiter >> y >> delimiter >> width >> delimiter >> height;
        DH->ProcessedInputData.PlateRect = cv::Rect(x, y, width, height);
    }

    // CarRect
    if(DH->hasInputFields.CarRect)
    {
        std::string CarRect;
        try
        {
            CarRect = DH->Request.JsonRvalue["CarRect"].s();
        }catch(...)
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDCARRECT;
            DH->Response.Description = "The type of CarRect is invalid.";
            return false; 
        }

#ifdef VALUEVALIDATION
        if(CarRect.empty() || CarRect.length() > 20)
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDCARRECT;
            DH->Response.Description = "The value of CarRect is invalid.";
            return false;
        }
#endif // VALUEVALIDATION

        DH->Input.CarRect = CarRect;
        int x = 0, y = 0, width = 0, height = 0;
        char delimiter;
        std::stringstream ss(CarRect);
        ss >> x >> delimiter >> y >> delimiter >> width >> delimiter >> height;
        DH->ProcessedInputData.CarRect = cv::Rect(x, y, width, height);
    }

    // CodeType
    if(DH->hasInputFields.CodeType)
    {
        int CodeType;
        try
        {
            CodeType = DH->Request.JsonRvalue["CodeType"].i();
        }catch(...)
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDCODETYPE;
            DH->Response.Description = "The type of CodeType is invalid.";
            return false; 
        }

#ifdef VALUEVALIDATION
        if(CodeType < 0 || CodeType > 20)
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDCODETYPE;
            DH->Response.Description = "The value of CodeType is invalid.";
            return false;
        }
#endif // VALUEVALIDATION

        DH->Input.CodeType = CodeType;
    }

    // Probability
    if(DH->hasInputFields.Probability)
    {
        int Probability;
        try
        {
            Probability = DH->Request.JsonRvalue["Probability"].i();
        }catch(...)
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDPROBABILITY;
            DH->Response.Description = "The type of Probability is invalid.";
            return false; 
        }

#ifdef VALUEVALIDATION
        if(Probability < 0 || Probability > 100)
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDPROBABILITY;
            DH->Response.Description = "The value of Probability is invalid.";
            return false;
        }
#endif // VALUEVALIDATION

        DH->Input.Probability = Probability;
    }

    // MasterPlate
    if(DH->hasInputFields.MasterPlate)
    {
        std::string MasterPlate;
        try
        {
            MasterPlate = DH->Request.JsonRvalue["MasterPlate"].s();
        }catch(...)
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDMASTERPLATE;
            DH->Response.Description = "The type of MasterPlate is invalid.";
            return false; 
        }

#ifdef VALUEVALIDATION
        if(MasterPlate.empty())
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDMASTERPLATE;
            DH->Response.Description = "The MasterPlate should not be empty.";
            return false;
        }

        if(MasterPlate == "0")
        {
            // Just to not validate the plate value and accept zero value
        }else if(DH->Input.PlateType == 1)
        {
            if(MasterPlate.length() != 9 || !(std::all_of(MasterPlate.begin(), MasterPlate.end(),[](char c) { return std::isdigit(static_cast<unsigned char>(c));})))
            {
                DH->Response.HTTPCode = 400;
                DH->Response.errorCode = INVALIDMASTERPLATE;
                DH->Response.Description = "The value of MasterPlate is invalid.";
                return false;
            }
        }else if(DH->Input.PlateType == 2)
        {
            if(MasterPlate.length() != 8 || !(std::all_of(MasterPlate.begin(), MasterPlate.end(),[](char c) { return std::isdigit(static_cast<unsigned char>(c));})))
            {
                DH->Response.HTTPCode = 400;
                DH->Response.errorCode = INVALIDMASTERPLATE;
                DH->Response.Description = "The value of MasterPlate is invalid.";
                return false;
            }
        }else if(DH->Input.PlateType == 3)
        {
            if(MasterPlate.length() < 5 || MasterPlate.length() > 7 || !(std::all_of(MasterPlate.begin(), MasterPlate.end(),[](char c) { return std::isdigit(static_cast<unsigned char>(c));})))
            {
                DH->Response.HTTPCode = 400;
                DH->Response.errorCode = INVALIDMASTERPLATE;
                DH->Response.Description = "The value of MasterPlate is invalid.";
                return false;
            }
        }else if(DH->Input.PlateType == 4 || DH->Input.PlateType == 5)
        {
            if(MasterPlate.length() < 7 || MasterPlate.length() > 8)
            {
                DH->Response.HTTPCode = 400;
                DH->Response.errorCode = INVALIDMASTERPLATE;
                DH->Response.Description = "The value of MasterPlate is invalid.";
                return false;
            }
        }else
        {
            if(MasterPlate.length() > 15)
            {
                DH->Response.HTTPCode = 400;
                DH->Response.errorCode = INVALIDMASTERPLATE;
                DH->Response.Description = "The value of MasterPlate is invalid.";
                return false;
            }
        }
#endif // VALUEVALIDATION

        DH->Input.MasterPlate = MasterPlate;
    }

    // RecordID
    if(DH->hasInputFields.RecordID)
    {
        std::string RecordID;
        try
        {
            RecordID = DH->Request.JsonRvalue["RecordID"].s();
        }catch(...)
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDRECORDID;
            DH->Response.Description = "The type of RecordID is invalid.";
            return false; 
        }

        if(RecordID.empty() || RecordID.length() != 24)
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDRECORDID;
            DH->Response.Description = "The value of RecordID is invalid.";
            return false;
        }

        if(DH->hasInputFields.DeviceID && DH->hasInputFields.ViolationID && DH->hasInputFields.PassedTime && DH->hasInputFields.PlateValue)
        {
            std::string MOngoIdPlate = DH->hasInputFields.MasterPlate ? DH->Input.MasterPlate : DH->Input.PlateValue;
            std::string CalculatedRecordID = this->GeneratMongoIDHash(DH->ProcessedInputData.PassedTimeLocal, MOngoIdPlate, DH->Input.ViolationID, DH->Input.DeviceID);
            
            if(CalculatedRecordID != RecordID)
            {
                DH->Response.HTTPCode = 400;
                DH->Response.errorCode = INVALIDRECORDID;
                DH->Response.Description = "The RecordID value does not match the other values.";
                return false;
            }
        }

        DH->Input.RecordID = RecordID;
        DH->ProcessedInputData.MongoID = RecordID;
    }else
    {
        if(DH->hasInputFields.DeviceID && DH->hasInputFields.ViolationID && DH->hasInputFields.PassedTime && DH->hasInputFields.PlateValue)
        {
            std::string MOngoIdPlate = DH->hasInputFields.MasterPlate ? DH->Input.MasterPlate : DH->Input.PlateValue;
            std::string CalculatedRecordID = this->GeneratMongoIDHash(DH->ProcessedInputData.PassedTimeLocal, MOngoIdPlate, DH->Input.ViolationID, DH->Input.DeviceID);

            DH->ProcessedInputData.MongoID = CalculatedRecordID;
        }
    }

    // ReceivedTime
    if(DH->hasInputFields.ReceivedTime)
    {
        std::string ReceivedTime;
        try
        {
            ReceivedTime = DH->Request.JsonRvalue["ReceivedTime"].s();
        }catch(...)
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDRECEIVEDTIME;
            DH->Response.Description = "The type of ReceivedTime is invalid.";
            return false; 
        }
        
#ifdef VALUEVALIDATION
        if(ReceivedTime.empty() || ReceivedTime.length() != 20)
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDRECEIVEDTIME;
            DH->Response.Description = "The value of ReceivedTime must not be empty and its number of characters must be equal to 20(YYYY-MM-DDTHH:MM:SSZ).";
            return false;
        }
#endif // VALUEVALIDATION

        std::tm ReceivedTimeLocal = {};
        if(!(ConvertISO8601TimeToLocal(ReceivedTime, ReceivedTimeLocal)))
        {
            DH->Response.HTTPCode = 400;
            DH->Response.errorCode = INVALIDRECEIVEDTIME;
            DH->Response.Description = "The value of ReceivedTime is invalid.";
            return false;
        }

        DH->Input.ReceivedTime = ReceivedTime;
        DH->ProcessedInputData.ReceivedTimeLocal = ReceivedTimeLocal;
    }
    return true;
}

bool Validator::CheckEncryptedRequestValues(const std::shared_ptr<DataHandler::DataHandlerStruct> &DH)
{
    std::string Token;
    try
    {
        Token = DH->Request.enJsonRvalue["Token"].s();
    }catch(...)
    {
        DH->Response.HTTPCode = 401;
        DH->Response.errorCode = INVALIDTOKEN;
        DH->Response.Description = "The type of Token is invalid.";
        return false; 
    }

    if(Token.length() != 36)
    {
        DH->Response.HTTPCode = 401;
        DH->Response.errorCode = INVALIDTOKEN;
        DH->Response.Description = "Invalid Token.";
        return false;    
    }
    return true;
}

bool Validator::CheckTokenRequestValues(const std::shared_ptr<DataHandler::DataHandlerStruct> &DH)
{
    // DeviceID
    int DeviceID;
    try
    {
        DeviceID = DH->Request.JsonRvalue["DeviceID"].i();
    }catch(...)
    {
        DH->Response.HTTPCode = 400;
        DH->Response.errorCode = INVALIDDEVICEID;
        DH->Response.Description = "The type of DeviceID is invalid.";
        return false; 
    }

    std::string DeviceIDStr = std::to_string(DeviceID);
    if(DeviceIDStr.length() != 7)
    {
        DH->Response.HTTPCode = 400;
        DH->Response.errorCode = INVALIDDEVICEID;
        DH->Response.Description = "The number of digits of the DeviceID value must be equal to 7.";
        return false;    
    }

    bool DeviceIDExist = false;
    for(int i = 0; i < DH->Cameras.size(); i++)
    {
        if(DeviceID == DH->Cameras[i].DeviceID)
        {
            DH->CameraIndex = i; // Cannot use DH->CameraIndex in the insert route because Validation may be disabled
            DeviceIDExist = true;
            break;
        }
    }

    if(!DeviceIDExist)
    {
        DH->Response.HTTPCode = 400;
        DH->Response.errorCode = INVALIDDEVICEID;
        DH->Response.Description = "This DeviceID does not exist.";
        return false; 
    }

    // Company Name
    std::string CompanyName;
    try
    {
        CompanyName = DH->Request.JsonRvalue["CompanyName"].s();
    }catch(...)
    {
        DH->Response.HTTPCode = 400;
        DH->Response.errorCode = INVALIDCOMPANYNAME;
        DH->Response.Description = "The type of CompanyName is invalid.";
        return false; 
    }

    if(CompanyName.length() > 100)
    {
        DH->Response.HTTPCode = 400;
        DH->Response.errorCode = INVALIDCOMPANYNAME;
        DH->Response.Description = "The number of characters of the CompanyName value should not be more than 100.";
        return false;    
    }

    // if(DH->Cameras[DH->CameraIndex].CompanyName != CompanyName)
    // {
    //     DH->Response.HTTPCode = 400;
    //     DH->Response.errorCode = INVALIDCOMPANYNAME;
    //     DH->Response.Description = "Company Name is incorrect.";
    //     return false;
    // }

    // User and Password
    std::string User;
    try
    {
        User = DH->Request.JsonRvalue["User"].s();
    }catch(...)
    {
        DH->Response.HTTPCode = 400;
        DH->Response.errorCode = INVALIDUSERPASS;
        DH->Response.Description = "The type of User is invalid.";
        return false; 
    }

    std::string EncryptedPassword;
    try
    {
        EncryptedPassword = DH->Request.JsonRvalue["Password"].s();
    }catch(...)
    {
        DH->Response.HTTPCode = 400;
        DH->Response.errorCode = INVALIDUSERPASS;
        DH->Response.Description = "The type of Password is invalid.";
        return false; 
    }

    cipher cipherObj;
    std::string Password = cipherObj.base64_decode(EncryptedPassword);
    if((User.length() > 100) || (Password.length() > 100))
    {
        DH->Response.HTTPCode = 400;
        DH->Response.errorCode = INVALIDUSERPASS;
        DH->Response.Description = "The number of characters of the User/Password value should not be more than 100.";
        return false;    
    }

    if((DH->Cameras[DH->CameraIndex].Username != User) || (DH->Cameras[DH->CameraIndex].Password != Password))
    {
        DH->Response.HTTPCode = 400;
        DH->Response.errorCode = INVALIDUSERPASS;
        DH->Response.Description = "Username or Password is incorrect.";
        return false;
    }

    return true;
}

std::string Validator::GeneratMongoIDHash(const std::tm &PassedTime, const std::string &PlateValue, const int &ViolationID, const int &DeviceID)
{
    std::string HashID = "";
    
    std::tm PassedTimeCopy = PassedTime;
    std::string HexUnixTime = GetHex(mktime(&PassedTimeCopy));
    if(HexUnixTime.size()<8)
    {
        for(int i = 0 ; i < (8 - HexUnixTime.size());i++)
            HashID += "0";
    }
    HashID += HexUnixTime;

    std::string HexPlateValue = GetHex(std::stoi(PlateValue));
    if(HexPlateValue.size() < 8)
    {
        for(int i = 0 ; i < (8 - HexPlateValue.size());i++)
            HashID += "0";
    }
    HashID += HexPlateValue;

    int VI = 0;
    if(ViolationID != 0)
        VI = ViolationID - 2000;
    
    std::string HexVID = GetHex(VI);
    if(HexVID.size()< 2)
    {
        for(int i = 0 ; i < (2 - HexVID.size());i++)
            HashID += "0";
    }
    HashID += HexVID;

    std::string HexDeviceID = GetHex(DeviceID);
    if(HexDeviceID.size() < 6) {
        for(int i = 0 ; i < (6 - HexDeviceID.size());i++)
            HashID += "0";
    }
    HashID += HexDeviceID;

    return HashID;
}