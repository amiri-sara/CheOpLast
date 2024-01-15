#include "storeimage.h"

bool storeimage::run(const std::shared_ptr<DataHandler::DataHandlerStruct> &DH)
{
    // Read Violation Info
    if(!(this->ReadViolationInfo(DH)))
        return false;

    // Create Address for save image
    if(!(this->CreateAddress(DH)))
        return false;

    // Create thumbnail image
    if(!(this->CreateThumbnail(DH)))
        return false;

    // Create banner
    if(DH->StoreImageConfig.AddBanner)
    {        
        if(!(this->CreateBanner(DH)))
        {
            return false;
        }
    }

    return true;
}

bool storeimage::ReadViolationInfo(const std::shared_ptr<DataHandler::DataHandlerStruct> &DH)
{
    bool findViolation = false;
    for (const auto& pair : DH->ViolationMap)
    {
        int key = pair.first;
        if(key == DH->Input.ViolationID)
        {
            DH->ViolationInfo = pair.second;
            findViolation = true;
            break;
        }
    }

    if(findViolation)
    {
        return true;
    }else
    {
        DH->Response.HTTPCode = 500;
        DH->Response.errorCode = CANNOTFINDVIOLATIONID;
        DH->Response.Description = "Internal Error.";
        return false;
    }
}

bool storeimage::CreateAddress(const std::shared_ptr<DataHandler::DataHandlerStruct> &DH)
{
    std::string Year = std::to_string(DH->ProcessedInputData.PassedTimeLocal.tm_year + 1900);
    std::string Month = (DH->ProcessedInputData.PassedTimeLocal.tm_mon + 1 < 10) ? "0" + std::to_string(DH->ProcessedInputData.PassedTimeLocal.tm_mon + 1) : std::to_string(DH->ProcessedInputData.PassedTimeLocal.tm_mon + 1);
    std::string Day = (DH->ProcessedInputData.PassedTimeLocal.tm_mday < 10) ? "0" + std::to_string(DH->ProcessedInputData.PassedTimeLocal.tm_mday) : std::to_string(DH->ProcessedInputData.PassedTimeLocal.tm_mday);
    std::string Hour = (DH->ProcessedInputData.PassedTimeLocal.tm_hour < 10) ? "0" + std::to_string(DH->ProcessedInputData.PassedTimeLocal.tm_hour) : std::to_string(DH->ProcessedInputData.PassedTimeLocal.tm_hour);
    std::string Min = (DH->ProcessedInputData.PassedTimeLocal.tm_min < 10) ? "0" + std::to_string(DH->ProcessedInputData.PassedTimeLocal.tm_min) : std::to_string(DH->ProcessedInputData.PassedTimeLocal.tm_min);
    std::string Sec = (DH->ProcessedInputData.PassedTimeLocal.tm_sec < 10) ? "0" + std::to_string(DH->ProcessedInputData.PassedTimeLocal.tm_sec) : std::to_string(DH->ProcessedInputData.PassedTimeLocal.tm_sec);
    
    DH->StoreImageAddress.ImageFolder = std::to_string(DH->Input.DeviceID) + "/" + Year + "/" + Month + "/" + Day + "/" + Hour + "/" + Min + "/" + Sec + "/" + DH->ProcessedInputData.MongoID + "/";
    
    DH->StoreImageAddress.ImageName = DH->Input.PlateValue;
    DH->StoreImageAddress.ImageAddress = DH->StoreImageAddress.ImageFolder + DH->StoreImageAddress.ImageName + DH->ViolationInfo.ImageSuffix;

    return true;
}

bool storeimage::CreateThumbnail(const std::shared_ptr<DataHandler::DataHandlerStruct> &DH)
{
    try 
    {
        auto const ScaleFactor  {160.0 / DH->ProcessedInputData.ColorImageMat.cols};
        cv::resize(DH->ProcessedInputData.ColorImageMat, DH->ProcessedInputData.ThumbnailImage, cv::Size(), ScaleFactor, ScaleFactor);
        return true;
    } catch(boost::exception_detail::clone_impl<boost::exception_detail::error_info_injector<boost::property_tree::ptree_bad_path> > &e)
    {
        DH->Response.HTTPCode = 500;
        DH->Response.errorCode = CANNOTCREATETHUMBNAILIMAGE;
        DH->Response.Description = "Internal Error.";
        return false;
    }
}

bool storeimage::CreateBanner(const std::shared_ptr<DataHandler::DataHandlerStruct> &DH)
{
    if(DH->ProcessedInputData.ColorImageMat.cols < 300)
    {
        DH->Response.HTTPCode = 400;
        DH->Response.errorCode = INVALIDCOLORIMAGESIZE;
        DH->Response.Description = "The width of the ColorImage for banner printing must be at least 300 pixels.";
        return false;
    }

    // Initialize Banner
    BannerAPI BA;
    BannerAPI::Config Config;
    Config.FontFamily = DH->StoreImageConfig.FontAddress;
    Config.thickness = -1;
    Config.linestyle = 16;
    Config.baseline = 0;

    int ImageWidth = DH->ProcessedInputData.ColorImageMat.cols;

    if(ImageWidth > 720)
    {
        Config.BannerHeight = 110;
        Config.fontHeight = 17;
    } else if(ImageWidth <= 720  && ImageWidth > 560)
    {
        Config.BannerHeight = 100;
        Config.fontHeight = 16;
    } else if(ImageWidth <= 560  && ImageWidth > 400)
    {
        Config.BannerHeight = 90;
        Config.fontHeight = 14;
    }
    else if(ImageWidth <= 400  && ImageWidth > 340)
    {
        Config.BannerHeight = 85;
        Config.fontHeight = 14;
    }
    else if(ImageWidth <= 340  && ImageWidth > 240)
    {
        Config.BannerHeight = 80;
        Config.fontHeight = 12;
    }
    else if(ImageWidth <= 240  && ImageWidth > 200)
    {
        Config.BannerHeight = 120;
        Config.fontHeight = 12;
    }

    BA.init(Config);

    // Create Lines
    std::vector<BannerAPI::LineStruct> Lines;

    // Plate Value
    BannerAPI::LineStruct line1;
    std::string PlateValurStr;
    if(DH->Input.PlateType == 1)
    {
        long int PlateValueInt = std::stol(DH->Input.PlateValue);
        PlateValurStr = BA.TypePlate(PlateValueInt);
    }else
    {
        PlateValurStr = DH->Input.PlateValue;
    }

    std::string PlateLine =  "پلاک : " + PlateValurStr + " " ;
    line1.Text = PlateLine;
    line1.LineAllignment = BannerAPI::RTL;
    Lines.push_back(line1);

    // Date
    BannerAPI::LineStruct line2;
    std::string JalaliDate = convertGregorianToJalali(DH->ProcessedInputData.PassedTimeLocal.tm_year + 1900, DH->ProcessedInputData.PassedTimeLocal.tm_mon + 1, DH->ProcessedInputData.PassedTimeLocal.tm_mday);
    std::string DateLine = "تاریخ : " + JalaliDate + " ";
    line2.Text = DateLine;
    line2.LineAllignment = BannerAPI::LTR;
    Lines.push_back(line2);

    // Location
    BannerAPI::LineStruct line3;

    std::string streetLocation;
    int AllowedSpeed;
    int PoliceCode;
    for(auto& camera : DH->Cameras)
    {
        if(camera.DeviceID == DH->Input.DeviceID)
        {
            streetLocation = camera.Location;
            AllowedSpeed = camera.AllowedSpeed;
            PoliceCode = camera.PoliceCode;
            break;
        }
    }
    
    std::string LocationLine = "محل تردد/تخلف : " + streetLocation + " ";
    line3.Text = LocationLine;
    line3.LineAllignment = BannerAPI::RTL;
    Lines.push_back(line3);
    
    // Time
    BannerAPI::LineStruct line4;
    std::string Hour = (DH->ProcessedInputData.PassedTimeLocal.tm_hour < 10) ? "0" + std::to_string(DH->ProcessedInputData.PassedTimeLocal.tm_hour) : std::to_string(DH->ProcessedInputData.PassedTimeLocal.tm_hour);
    std::string Min = (DH->ProcessedInputData.PassedTimeLocal.tm_min < 10) ? "0" + std::to_string(DH->ProcessedInputData.PassedTimeLocal.tm_min) : std::to_string(DH->ProcessedInputData.PassedTimeLocal.tm_min);
    std::string Sec = (DH->ProcessedInputData.PassedTimeLocal.tm_sec < 10) ? "0" + std::to_string(DH->ProcessedInputData.PassedTimeLocal.tm_sec) : std::to_string(DH->ProcessedInputData.PassedTimeLocal.tm_sec);
    std::string time = Hour + ":" + Min + ":" + Sec;
    std::string TimeLine = "ساعت : " + time + " ";
    line4.Text = TimeLine;
    line4.LineAllignment = BannerAPI::LTR;
    Lines.push_back(line4);

    if(DH->Input.ViolationID == 0) //2002/
    {
        // Allowed Speed
        BannerAPI::LineStruct line5;
        std::string AllowedSpeedLine = "سرعت مجاز : " + std::to_string(AllowedSpeed) + " ";
        line5.Text = AllowedSpeedLine;
        line5.LineAllignment = BannerAPI::RTL;
        Lines.push_back(line5);

        // Speed
        BannerAPI::LineStruct line6;
        std::string SpeedLine = "           سرعت : " + std::to_string(DH->Input.Speed) + " ";
        line6.Text = SpeedLine;
        line6.LineAllignment = BannerAPI::LTR;
        Lines.push_back(line6);
    }

    // Police Code
    BannerAPI::LineStruct line7;
    std::string PoliceCodeLine = "کد پلیس : " + std::to_string(PoliceCode) + " ";
    line7.Text = PoliceCodeLine;
    line7.LineAllignment = BannerAPI::RTL;
    Lines.push_back(line7);

    // Violation
    BannerAPI::LineStruct line8;
    std::string ViolationLine = DH->ViolationInfo.Description + " ";
    line8.Text = ViolationLine;
    line8.LineAllignment = BannerAPI::RTL;
    Lines.push_back(line8);

    DH->ProcessedInputData.Banner = BA.Run(Lines);
    return true;
}