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

    // Add Plate crop
    if(!(this->AddPlateCrop(DH)))
        return false;

    // Create banner
    if(DH->StoreImageConfig.AddBanner)
    {        
        if(!(this->CreateBanner(DH)))
        {
            return false;
        }
    }

    // Store Color Images
    std::string FolderName = DH->StoreImageConfig.StorePath + "/" + DH->StoreImageAddress.ImageFolder;
    std::string ColorImageName = DH->StoreImageAddress.ImageName + DH->ViolationInfo.ImageSuffix + ".jpg";
    if(!(this->StoreImage(FolderName, ColorImageName, DH->ProcessedInputData.ColorImageMat, true, DH->StoreImageConfig.ColorImageMaxSize)))
    {
        DH->Response.HTTPCode = 500;
        DH->Response.errorCode = CANNOTSAVECOLORIMAGE;
        DH->Response.Description = "Internal Error.";
        return false;
    }

    // Store plate Images
    std::string PlateImageName = DH->StoreImageAddress.ImageName + "_P" + ".jpg";
    if(!(this->StoreImage(FolderName, PlateImageName, DH->ProcessedInputData.PlateImageMat, true, DH->StoreImageConfig.PlateImageMaxSize)))
    {
        DH->Response.HTTPCode = 500;
        DH->Response.errorCode = CANNOTSAVEPLATEIMAGE;
        DH->Response.Description = "Internal Error.";
        return false;
    }

    // Store Thumbnail Image
    std::string ThumbnailImageName = DH->StoreImageAddress.ImageName + "_IT" + ".jpg";
    if(!(this->StoreImage(FolderName, ThumbnailImageName, DH->ProcessedInputData.ThumbnailImage, true, 5)))
    {
        DH->Response.HTTPCode = 500;
        DH->Response.errorCode = CANNOTSAVETHUMBNAILIMAGE;
        DH->Response.Description = "Internal Error.";
        return false;
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
    DH->StoreImageAddress.ImageAddress = DH->StoreImageAddress.ImageFolder + DH->StoreImageAddress.ImageName + DH->ViolationInfo.ImageSuffix + ".jpg";

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

bool storeimage::AddPlateCrop(const std::shared_ptr<DataHandler::DataHandlerStruct> &DH)
{
    bool addCrop;
    for(auto& camera : DH->Cameras)
    {
        if(camera.DeviceID == DH->Input.DeviceID)
        {
            addCrop = camera.addCrop;
            break;
        }
    }
    
    if(!addCrop)
        return true;

    try
    {
        cv::Mat largerImg;
        float plateToColorImage = DH->StoreImageConfig.PlateImagePercent / 100.0 + 1;
        cv::resize(DH->ProcessedInputData.PlateImageMat, largerImg, cv::Size(), plateToColorImage, plateToColorImage);
        cv::Rect roi(0, 0, largerImg.cols, largerImg.rows);
        cv::Mat target = DH->ProcessedInputData.ColorImageMat(roi);
        largerImg.copyTo(target);
    } 
    catch(...)
    {
        DH->Response.HTTPCode = 500;
        DH->Response.errorCode = CANNOTADDPLATECROP;
        DH->Response.Description = "Internal Error.";
        return false;
    }

     return true;
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

    // Read information related to the camera
    std::string streetLocation;
    std::string subMode;
    int AllowedSpeed;
    int PoliceCode;
    bool addBanner;
    for(auto& camera : DH->Cameras)
    {
        if(camera.DeviceID == DH->Input.DeviceID)
        {
            streetLocation = camera.Location;
            AllowedSpeed = camera.AllowedSpeed;
            PoliceCode = camera.PoliceCode;
            subMode = camera.subMode;
            addBanner = camera.addBanner;
            break;
        }
    }

    if(subMode == "redLight" || subMode == "cctv" || (!addBanner))
        return true;

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

    Config.BannerWidth = ImageWidth;
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
    std::string LocationLine = "";
    if(subMode == "vehicle") // Vehicle ANPR
    {
        std::string Response = "";
        for(auto i = 0; i < 3; i++)
        {
            int Ret = getLocationName(std::to_string(DH->Input.Latitude), std::to_string(DH->Input.Longitude), Response);
            if(Ret == CURLE_OK)
            {
                break;
            }
            else if(Ret != CURLE_OK && (Ret == CURLE_COULDNT_CONNECT || Ret == CURLE_OPERATION_TIMEDOUT))
            {
                if(DH->DebugMode)
                    SHOW_ERROR("getLocationName service : Auth Connection Failed. Retry Again!");
            }
            else if(Ret != CURLE_OK)
            {
                if(DH->DebugMode)
                    SHOW_ERROR("getLocationName service : Auth Connection Failed.");
                break;
            }
        }

        if(Response == "")
        {
            LocationLine = "محل تردد/تخلف : " + std::to_string(DH->Input.Latitude) + " , " + std::to_string(DH->Input.Longitude) + " ";
        }
        else
        {
            LocationLine = "محل تردد/تخلف : " + Response + " ";
        }
    }
    else
    {
        LocationLine = "محل تردد/تخلف : " + streetLocation + " "; 
    }
    
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

    if(subMode == "speed") // Speed Camera
    {
        // Allowed Speed
        BannerAPI::LineStruct line5;
        std::string AllowedSpeedLine = "سرعت مجاز : " + std::to_string(AllowedSpeed) + " ";
        line5.Text = AllowedSpeedLine;
        line5.LineAllignment = BannerAPI::RTL;
        Lines.push_back(line5);

        // Speed
        BannerAPI::LineStruct line6;
        std::string SpeedLine = "          سرعت : " + std::to_string(DH->Input.Speed) + " ";
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
    line8.LineAllignment = BannerAPI::LTR;
    Lines.push_back(line8);

    cv::Mat Banner = BA.Run(Lines);

    try
    {
        cv::vconcat(Banner, DH->ProcessedInputData.ColorImageMat, DH->ProcessedInputData.ColorImageMat);
    }catch(...)
    {
        DH->Response.HTTPCode = 500;
        DH->Response.errorCode = CANNOTCREATEBANNER;
        DH->Response.Description = "Internal Error.";
        return false;
    }
    
    return true;
}

bool storeimage::StoreImage(std::string FolderName, std::string ImageName, cv::Mat Image, bool EnableResize, int MaxSize)
{
    setPermissionsDir(FolderName+"/");

    std::vector<uchar> ImgData;
    std::vector<int> Param;
    Param.push_back(cv::IMWRITE_JPEG_QUALITY);
    Param.push_back(100);

    std::string OutputImageName =  FolderName+ "/" +ImageName;

    if(EnableResize)
    {
        do
        {
            Param[1] -= 10;
            cv::imencode(".jpg", Image, ImgData, Param);
            if(Param[1] == 0)
                break;
        }
        while(ImgData.size()/1024 >MaxSize);

        std::string DataArray = std::string(reinterpret_cast<const char*>(&ImgData[0]),ImgData.size());
        std::ofstream MyImage(OutputImageName, std::ios::out);

        if(MyImage.is_open())
        {
            MyImage.write(DataArray.c_str(),DataArray.size());
            MyImage.close();
        }
        else
        {
            return false;
        }
    }
    else
    {
        if(!cv::imwrite(OutputImageName, Image, Param))
        {
            return false;
        }
    }
    return true;
}

void storeimage::setPermissionsDir(std::string Path)
{
    std::string SupPath = "";
    std::vector<std::string> SubPathVec;
    if(boost::filesystem::is_directory(Path))
    {
        return;
    }
    do
    {
        size_t Indx = Path.find_last_of('/');
        if(Indx == std::string::npos)
        {
            return;
        }

        SupPath = Path.substr(0, Indx);
        std::string SubPath = Path.substr(Indx);
        SubPathVec.push_back(SubPath);
        Path = SupPath;
    }
    while(!boost::filesystem::is_directory(SupPath));

    SubPathVec.erase(SubPathVec.begin());
    while(SubPathVec.size() > 0)
    {
        std::string SPath = SupPath+SubPathVec.back();
        SubPathVec.erase(SubPathVec.end());
        boost::filesystem::create_directories(SPath);
        boost::filesystem::permissions(SPath, boost::filesystem::owner_all |boost::filesystem::group_all | boost::filesystem::others_all);
        SupPath = SPath;
    }

    return;
}

size_t writeToString(void *ptr, size_t size, size_t count, void *stream)
{
    ((std::string*)stream)->append((char*)ptr, 0, size*count);
    return size*count;
}

int storeimage::getLocationName(std::string Lat, std::string Lon, std::string &Response)
{
    int Ret = -1;
    CURL * curl = curl_easy_init();
    if(curl)
    {
        std::string URL = "http://127.0.0.1:82/reverse?lat="+Lat+"&lon="+Lon+"&format=json&zoom=18&accept-language=fa";
        std::string response;
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
        curl_easy_setopt(curl, CURLOPT_URL, URL.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToString);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3L);
        CURLcode res = curl_easy_perform(curl);
        Ret = res;
        try
        {
            auto Loc = crow::json::load(response);
            std::string city = Loc["address"]["city"].s();
            std::string road = Loc["address"]["road"].s();
            Response = city+"-"+road;
        }
        catch(...)
        {
            SHOW_ERROR("Invalid Json In getLocationName");
        }

        curl_easy_cleanup(curl);
    }

    return Ret;
}