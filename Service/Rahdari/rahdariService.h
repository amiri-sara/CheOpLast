#ifndef RAHDARISERVICE_H
#define RAHDARISERVICE_H


#include "../Service.h"
#include <condition_variable>
// Include a faster JSON library, e.g., RapidJSON or simdjson
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#define batchMetaData_vec_sizeThresold 100
#define monitorInterval 10
class RahdariService : public Service
{
public:
    RahdariService(Configurate::CurlServiceConfigStruct ServiceConfig);
    void monitor();
    int init();
    void run() override;

    // std::shared_ptr<DataHandler::DataHandlerStruct> DH;
    std::string getCurrentTimeSec();
    size_t getSize();

    static std::atomic<uint32_t> totalInfoFetchingTime; // Total fetching time
    static std::atomic<uint32_t> InfofetchingCount; // Count of fetching operations
    static std::atomic<uint32_t> validImagesCount ;
    static boost::mutex mtx_Info; // Mutex for synchronizing access to shared data
    static boost::mutex mtx_ValidImage;
    static std::atomic<uint32_t> totalImageFetchingTime; // Total fetching time
    static std::atomic<uint32_t> ImagefetchingCount; // Count of fetching operations
    static boost::condition_variable batchMetaData_cv; // Use Boost condition variable


    
    static boost::mutex mtx_Image; // Mutex for synchronizing access to shared data
private:
    int InfoCount = 100;
    // std::string Url = "http://172.30.8.170:8000";
    std::string Url = "";//"http://172.20.6.180:50001";
    std::atomic<uint64_t> MinId{0};
    boost::mutex MinId_mtx; // Mutex for MinId access
    uint64_t MaxId = 0;
    uint64_t TmpMaxId = 0;
    //std::string StorePath = "/home/c-anpr/Images/";
    struct TTOInfo
    {
        std::string Allowed            = "";
        int CompanyCode                      ;
        int DeviceId                       ;
        int LineNumber                     ;
        std::string PassDateTime       = "";
        std::string ReceiveDateTime    = "";
        std::string RegisterDateTime   = "";
        int SystemCode                     ;
        uint64_t    TTOInfoId              ;
        int VehicleClass                   ;
        std::string VehiclePlate       = "";
        int  VehicleSpeed       ;
        std::string WrongDirection     = "";
        std::string HasColorImage      = "";
        std::string HasPlateImage      = "";
        std::string HasCrime           = "";
        std::string CrimeCode          = "";
        std::string PoliceCode         = "";
        std::string PlateImageBase64   = "";
        std::string error              = "";
    };

    struct ImageInfo
    {
        cv::Mat PlateImage;
        std::string ColorImage       = "";
        std::string PlateImageBase64 = "";
        std::string TTOInfoId        = "";
    };

    struct ResponseValue
    {
        uint64_t MaxIdResult;
        std::vector<TTOInfo> TTOInfoResult;
        std::string Error = "";
        ImageInfo ImageResult;
    };
    
    struct getImagesResultStruct {
    std::unordered_map<uint64_t, std::string> PlateImages_map; // Parsed data
    std::string Error; // Error message, if any
    };

    bool batchMetaData_ready = false;
    boost::condition_variable cvar;
    boost::mutex batchMetaData_mutex;
    boost::mutex batchImages_mutex;

    Configurate::CurlServiceConfigStruct CurlServiceConfig;
    std::vector<std::vector<TTOInfo>> batchMetaData_vec;
    std::vector<std::unordered_map<uint64_t, std::string>> batchImagesMap_vec;
    uint64_t getMaxId(const std::string& Url);
    std::vector<TTOInfo> getInfo(const std::string& Url, uint64_t MinId, uint64_t MaxId);
    ImageInfo getImageBase64(std::string Url, uint64_t Id);
    getImagesResultStruct parsePlateImages(const std::string& jsonString);
    std::unordered_map<uint64_t, std::string> getImageBase64_bulk(const std::string& Url, uint64_t MinId, uint64_t MaxId);
    std::string exec(std::string cmd);
    bool isExistsFile(std::string FilePath);
    static size_t writeToString(void *ptr, size_t size, size_t count, void *stream);
    void getInfoHandler();
    ResponseValue getResponseValue(const std::string& jsonString, std::string ResType);
    int& convertVehcileType(int& InputVehicleType);


protected:

};

#endif //RAHDARISERVICE_H
