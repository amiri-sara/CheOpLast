#ifndef RAHDARISERVICE_H
#define RAHDARISERVICE_H


#include "../Service.h"

#include <condition_variable>
// Include a faster JSON library, e.g., RapidJSON or simdjson
// #include <rapidjson/document.h>
// #include <rapidjson/error/en.h>
// #include <nlohmann/json.hpp>

// #include "../crow.h"
#define batchMetaData_vec_sizeThresold 100


class RahdariService : public Service
{
public:
    RahdariService(Configurate::ClientServiceConfigStruct ServiceConfig);
    void monitor();
    int init();
    void run() override;

    std::string getCurrentTimeSec();
    size_t getSize();

    
private:
    int InfoCount = 100;
    inline static std::atomic<uint64_t> processCounter = 0;

    // std::string Url = "http://172.30.8.170:8000";
    std::string Url = "";//"http://172.20.6.180:50001";
    std::atomic<uint64_t> MinId{0};
    boost::mutex MinId_mtx; // Mutex for MinId access
    uint64_t MaxId = 0;
    uint64_t TmpMaxId = 0;
    // std::shared_ptr<DataHandler::DataHandlerStruct> DH;
    std::vector<MongoDB::Field> MetaFindFields = { 
    {"_id", "last_processed_id", MongoDB::FieldType::String, "eq"}
};
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
        int ErrorCode = 0;
        ImageInfo ImageResult;
    };
    
    struct getImagesResultStruct {
    std::unordered_map<uint64_t, std::string> PlateImages_map; // Parsed data
    std::string Error; // Error message, if any
    int ErrorCode = 0;
    };

    std::condition_variable queue_not_full;
    int NumberOfProducerThread = 0;
    int ThresholdFetchedRecors = 0;
    bool use_batch_queueing = false;
    bool use_batch_consume = false;
    bool use_bulk_images = false;

        // Global queue for producer-consumer
    std::queue<TTOInfo> record_queue;
    std::mutex queue_mutex;
    // std::atomic<bool> running;

    Configurate::ClientServiceConfigStruct ClientServiceConfig;
    std::vector<std::vector<TTOInfo>> batchMetaData_vec;
    std::vector<std::unordered_map<uint64_t, std::string>> batchImagesMap_vec;
    uint64_t getMaxId(const std::string& Url);
    std::vector<TTOInfo> getInfo(const std::string& Url, uint64_t MinId, uint64_t MaxId);
    ImageInfo getImageBase64(std::string Url, uint64_t Id);
    getImagesResultStruct parsePlateImages(const std::string& jsonString, uint64_t MinId);
    std::unordered_map<uint64_t, std::string> getImageBase64_bulk(const std::string& Url, uint64_t MinId, uint64_t MaxId);
    std::unordered_map<uint64_t, std::string> getSingleImage(const std::string& Url, uint64_t Id);

    std::string exec(std::string cmd);
    bool isExistsFile(std::string FilePath);
    static size_t writeToString(void *ptr, size_t size, size_t count, void *stream);
    void getInfoHandler();
    void producerThread();
    ResponseValue getResponseValue(const std::string& jsonString, std::string ResType);
    int& convertVehcileType(int& InputVehicleType);


    // --- شروع تغییرات برای بهینه‌سازی بارگذاری پیکربندی ---
    // اضافه کردن متغیرهای عضو برای ذخیره پیکربندی‌ها
    Configurate::FieldsStruct m_hasInputFields;
    Configurate::FieldsStruct m_hasOutputFields;
    Configurate::StoreImageConfigStruct m_StoreImageConfig;
    std::unordered_map<int, Configurate::ViolationStruct> m_ViolationMap;
    std::vector<Configurate::CameraStruct> m_Cameras;
    Configurate::InfoDatabaseStruct m_InsertDatabaseInfo;
    Configurate::InfoDatabaseStruct m_FailedDatabaseInfo;
    Configurate::ModulesStruct m_Modules;
    std::shared_ptr<MongoDB> m_InsertDatabase;
    std::shared_ptr<MongoDB> m_FailedDatabase;
    std::shared_ptr<MongoDB> m_ConfigDatabase; // برای به‌روزرسانی MinId در Meta
    Configurate::InfoDatabaseStruct m_ConfigDatabaseInfo;
    // --- پایان تغییرات برای بهینه‌سازی بارگذاری پیکربندی ---


protected:

};

#endif //RAHDARISERVICE_H
