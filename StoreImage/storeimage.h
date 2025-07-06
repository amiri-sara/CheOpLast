#ifndef STOREIMAGE_H
#define STOREIMAGE_H

#include "../Logs/log.h"
#include "../DataHandler/datahandler.h"
#include "../Time/timetools.h"

// #ifdef BUILDUBUNTU22
#include "./banner22/bannerapi.h"
// #endif // BUILDUBUNTU22
#ifdef BUILDUBUNTU20
// #include "./banner20/bannerapi.h"
#endif // BUILDUBUNTU20


class storeimage
{
public:
    storeimage() = default;
    bool run(const std::shared_ptr<DataHandler::DataHandlerStruct> &DH);

private:  
    bool ReadViolationInfo(const std::shared_ptr<DataHandler::DataHandlerStruct> &DH);

    bool CreateAddress(const std::shared_ptr<DataHandler::DataHandlerStruct> &DH);

    bool CreateThumbnail(const std::shared_ptr<DataHandler::DataHandlerStruct> &DH);
    bool AddPlateCrop(const std::shared_ptr<DataHandler::DataHandlerStruct> &DH);
    bool CreateBanner(const std::shared_ptr<DataHandler::DataHandlerStruct> &DH);
    
    // --- تغییر امضای متد StoreImage ---
    bool StoreImage(std::string FolderName, std::string ImageName, const cv::Mat& Image, bool EnableResize, int MaxSize);
    void setPermissionsDir(std::string Path);
    int getLocationName(std::string Lat, std::string Lon, std::string &Response);
};



#endif //STOREIMAGE_H