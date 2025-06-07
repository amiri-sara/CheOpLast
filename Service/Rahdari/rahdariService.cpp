#include "rahdariService.h"

#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <random>
#include <csignal>
#include <atomic>

std::atomic<bool> stop(false); // Global stop flag

void handle_signal(int signal) {
    stop = true; // Set the stop flag
}


using namespace cv;
using namespace std;


std::string RahdariService::getCurrentTimeSec() {
    // Get the current time as a time_point
    auto now = std::chrono::system_clock::now();
    
    // Convert to time_t for easy formatting
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    
    // Convert to local time
    std::tm local_tm = *std::localtime(&now_c);
    
    // Create a string stream to format the time
    std::ostringstream oss;
    oss << std::put_time(&local_tm, "%Y-%m-%dT%H:%M:%S");
    
    return oss.str();
}

size_t RahdariService::writeToString(void *ptr, size_t size, size_t count, void *stream)
{
    ((std::string*)stream)->append((char*)ptr, 0, size*count);
    return size*count;
}

void RahdariService::producerThread() {
    while (true) { //TODO running must init with boolan atomic value
        
        uint64_t current_id = this->MinId.fetch_add(InfoCount); // Get current start_id and increment atomically

        // Step 1: Fetch metadata in batches
        std::vector<TTOInfo> Info = getInfo(Url, current_id, current_id+InfoCount-1);

        if(Info.empty()) 
        {
            Logger::getInstance().logError("Info is empty !!!");
            boost::this_thread::sleep_for(boost::chrono::milliseconds(10));
            continue;
        } 
        std::vector<TTOInfo> batch_to_queue;

        if(this->use_bulk_images)
        {

        // Step 2: Fetch images in the same ID range

        std::unordered_map<uint64_t, std::string> Images_map = getImageBase64_bulk(Url, current_id, current_id+InfoCount-1);
        if(Images_map.empty()){
            Logger::getInstance().logError("ImageMap Is Empty !!!");
            continue;  //TODO HANDEL BETTER With try

        }

        // Step 3: Match metadata and images by ID
        for (auto& info : Info)
        {
            auto it = Images_map.find(info.TTOInfoId);
            if(it != Images_map.end()){

                info.PlateImageBase64 = it->second;

                if(it->second == "Image Not Available" )  
                    info.error = "Image Not Available" ;

                else if(it->second == "null" || it->second == "NULL"  )
                    info.error = "Image Is NULL" ;
            }else{
                info.error = "There Is No Image For This RecordId ." ;
            }

            if (this->use_batch_queueing) {
                batch_to_queue.push_back(info);
            } else {
                // Single-record queueing
                std::unique_lock<std::mutex> lock(queue_mutex);
                record_queue.push(info);
                if (record_queue.size() > this->ThresholdFetchedRecors) {
                    lock.unlock();
                    Logger::getInstance().logWarning("queue exceeds the threshold");
                    std::unique_lock<std::mutex> wait_lock(queue_mutex);
                    queue_not_full.wait(wait_lock, [this]() {
                        return record_queue.size() <= this->ThresholdFetchedRecors / 2;
                    });
                }
            }

        }
        }
        else
        {
            // Step 2: Fetch images one by one
            for (auto& info : Info) {
                // Fetch single image for the current record
                std::unordered_map<uint64_t, std::string> Images_map = getSingleImage(Url, info.TTOInfoId);
                if(Images_map.empty()){
                    Logger::getInstance().logError("ImageMap Is Empty !!!");
                    continue;  //TODO HANDEL BETTER With try

                }
                
                auto it = Images_map.find(info.TTOInfoId);
                if(it != Images_map.end()){

                    info.PlateImageBase64 = it->second;

                    if(it->second == "Image Not Available" )  
                        info.error = "Image Not Available" ;

                    else if(it->second == "null" || it->second == "NULL"  )
                        info.error = "Image Is NULL" ;
                }else{
                    info.error = "There Is No Image For This RecordId ." ;
                }

                // Step 3: Push to queue immediately for single mode
                std::unique_lock<std::mutex> lock(queue_mutex);
                record_queue.push(info);
                if (record_queue.size() > this->ThresholdFetchedRecors) {
                    lock.unlock();
                    Logger::getInstance().logWarning("queue exceeds the threshold");
                    std::unique_lock<std::mutex> wait_lock(queue_mutex);
                    queue_not_full.wait(wait_lock, [this]() {
                        return record_queue.size() <= this->ThresholdFetchedRecors / 2;
                    });
                }
            }
        }

        // Batch queueing (if enabled)
        if (this->use_batch_queueing && !batch_to_queue.empty()) {
            std::unique_lock<std::mutex> lock(queue_mutex);
            for (auto& info : batch_to_queue) {
                record_queue.push(info);
            }
            if (record_queue.size() > this->ThresholdFetchedRecors) {
                lock.unlock();
                Logger::getInstance().logWarning("queue exceeds the threshold");
                std::unique_lock<std::mutex> wait_lock(queue_mutex);
                queue_not_full.wait(wait_lock, [this]() {
                    return record_queue.size() <= this->ThresholdFetchedRecors / 2;
                });
            }
            batch_to_queue.clear();
        }

        // }

        boost::this_thread::sleep_for(boost::chrono::milliseconds(1));
    }
}



RahdariService::RahdariService(Configurate::ClientServiceConfigStruct ServiceConfig)
{
    this->ClientServiceConfig = ServiceConfig;

        // بارگذاری پیکربندی‌ها از Configurate Singleton به متغیرهای عضو
    Configurate* ConfigurateObj = Configurate::getInstance();
    m_hasInputFields = ConfigurateObj->getInputFields(); //
    m_hasOutputFields = ConfigurateObj->getOutputFields(); //
    m_StoreImageConfig = ConfigurateObj->getStoreImageConfig(); //
    m_ViolationMap = ConfigurateObj->getViolationMap(); //
    m_Cameras = ConfigurateObj->getCameras(); //
    m_InsertDatabaseInfo = ConfigurateObj->getInsertDatabaseInfo(); //
    m_FailedDatabaseInfo = ConfigurateObj->getFailedDatabaseInfo(); //
    m_Modules = ConfigurateObj->getModules(); //
    m_InsertDatabase = ConfigurateObj->getInsertDatabase(); //
    m_FailedDatabase = ConfigurateObj->getFailedDatabase(); //
    m_ConfigDatabase = ConfigurateObj->getConfigDatabase(); //
    m_ConfigDatabaseInfo = ConfigurateObj->getConfigDatabaseInfo();
}
int RahdariService::init()
{
    if(!this->ClientServiceConfig.ReadFromMinIdTXT)
    {
        // --- استفاده از متغیر عضو m_ConfigDatabaseInfo ---
        std::vector<MongoDB::Field> MetaFindFields = {
            {"_id", "last_processed_id", MongoDB::FieldType::String, "eq"}
        };
        MongoDB::FindOptionStruct Option;
        std::vector<std::string> MetaDoc;
        MongoDB::ResponseStruct FindReturn = m_ConfigDatabase->Find(m_ConfigDatabaseInfo.DatabaseName, "Meta", MetaFindFields, Option, MetaDoc); //

        if(FindReturn.Code == MongoDB::MongoStatus::FindSuccessful && !MetaDoc.empty()) {
            crow::json::rvalue AggregationConfigJSON = crow::json::load(MetaDoc[0]);
            this->MinId = static_cast<uint64_t>(AggregationConfigJSON["PassedVehicleRecordsId"].i()); //
        } else {
            Logger::getInstance().logError("Could not find 'last_processed_id' in Meta collection. Starting from 0.");
            this->MinId = 0;
        }

        std::cout << "********************" << std::endl;
        std::cout<<"MinId: " <<this->MinId<<std::endl;
        std::cout << "********************" << std::endl;   
    }else{
        if(!isExistsFile("MinId.txt"))
        {
            Logger::getInstance().logError("The File 'MinId.txt' doesn't exist");
            return -1; 
        }
        std::ifstream t("MinId.txt");
        std::cout << "********************" << std::endl;
        std::string minIdStr;
        t.seekg(0, std::ios::end);
        minIdStr.reserve(t.tellg());
        t.seekg(0, std::ios::beg);
        minIdStr.assign((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
        std::cout << "Raw MinId from file: " << minIdStr << std::endl;
        try{
            this->MinId = std::stoll(minIdStr);
            std::cout << "Converted MinId: " << this->MinId << std::endl;
            } catch (const std::invalid_argument& e) {
                Logger::getInstance().logError("Invalid MinId value in file.");
                return -1;
            } catch (const std::out_of_range& e) {
                Logger::getInstance().logError("MinId value is out of range.");
                return -1;
        }
        std::cout << "********************" << std::endl;   
    }
    Url = this->ClientServiceConfig.ClientServiceInfo.URI;

    curl_global_init(CURL_GLOBAL_ALL);

    std::vector<boost::thread> threads;
    this->NumberOfProducerThread = this->ClientServiceConfig.ThreadNumber;
    this->ThresholdFetchedRecors = this->ClientServiceConfig.ThresholdFetchedRecors;
    this->use_batch_consume = this->ClientServiceConfig.UseBatchConsume;
    this->use_batch_queueing = this->ClientServiceConfig.UseBatchProduce;
    this->use_bulk_images = this->ClientServiceConfig.UseBulkImages;
    
    for (int i = 0; i < this->NumberOfProducerThread; ++i) {
        threads.emplace_back(boost::bind(&RahdariService::producerThread, this)  );
    }
    for(auto& thread: threads)
    {
        thread.detach();
    }

    return 0; // Success
}
void RahdariService::run()
{
        // --- تغییرات: ایجاد اشیاء storeimageobj و savedataobj یک بار برای هر رشته Consumer ---
    // این اشیاء در ابتدای اجرای تابع run توسط هر ترد Consumer ساخته می‌شوند
    // و تا پایان عمر آن ترد/تابع پابرجا خواهند بود.
    std::shared_ptr<storeimage> storeimageobj = std::make_shared<storeimage>();
    std::shared_ptr<savedata> savedataobj = std::make_shared<savedata>();
    // --- پایان تغییرات ---

    while(!stop.load())
    {
        TTOInfo record;
        std::vector<TTOInfo> recordsVec;
        
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            if (record_queue.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            continue;
            }

            if (this->use_batch_consume) { // Batch consume: Pop up to batch_size records
                
                size_t count = 0;
                while (!record_queue.empty() && count < InfoCount) {
                    recordsVec.push_back(record_queue.front());
                    record_queue.pop();
                    ++count;
                }
            } else {  // Single-record consume
               
                recordsVec.push_back(record_queue.front());
                record_queue.pop();
            }

           
            if (record_queue.size() <= this->ThresholdFetchedRecors / 2) {
                queue_not_full.notify_one();   // Notify producer if queue size is below threshold
            }
            queueSize = record_queue.size();            

        }
        for (auto& record : recordsVec) {
            auto processStartTime = std::chrono::high_resolution_clock::now();
            std::shared_ptr<DataHandler::DataHandlerStruct> DH = std::make_shared<DataHandler::DataHandlerStruct>();
            // --- شروع تغییرات برای بهینه‌سازی بارگذاری پیکربندی ---
            // استفاده از متغیرهای عضو به جای بارگذاری مجدد از Configurate Singleton
            DH->hasInputFields = m_hasInputFields; //
            DH->hasOutputFields = m_hasOutputFields; //
            DH->StoreImageConfig = m_StoreImageConfig; //
            DH->ViolationMap = m_ViolationMap; //
            DH->Cameras = m_Cameras; //
            DH->InsertDatabase = m_InsertDatabase; //
            DH->InsertDatabaseInfo = m_InsertDatabaseInfo; //
            DH->FailedDatabase = m_FailedDatabase; //
            DH->FailedDatabaseInfo = m_FailedDatabaseInfo; //
            DH->Modules = m_Modules; //
            DH->DebugMode = this->ClientServiceConfig.DebugMode;
            DH->ConfigDatabase = m_ConfigDatabase; //
            DH->ConfigDatabaseInfo = m_ConfigDatabaseInfo; //
            // --- پایان تغییرات برای بهینه‌سازی بارگذاری پیکربندی ---

            DH->Input.DeviceID        =     record.DeviceId;
            DH->Input.Lane            =     record.LineNumber;
            DH->Input.PassedTime      =     record.PassDateTime;
            DH->Input.SystemCode      =     record.SystemCode;
            DH->Input.RecordID        =     record.TTOInfoId ;
            DH->Input.VehicleType     =     record.VehicleClass;
            DH->Input.PlateValue      =     record.VehiclePlate;
            DH->Input.Speed           =     record.VehicleSpeed;
            DH->Input.CompanyCode     =     record.CompanyCode;
            DH->Input.ReceivedTime    =     getCurrentTimeSec();

            std::tm PassedTimeUTC     = {};
            std::tm ReceivedTimeUTC   = {};

            ConvertLocalTimeToUTC(DH->Input.PassedTime, PassedTimeUTC);
            DH->ProcessedInputData.PassedTimeUTC   = PassedTimeUTC;
            DH->ProcessedInputData.PassedTimeLocal = stringToTm(DH->Input.PassedTime);

            ConvertLocalTimeToUTC(DH->Input.ReceivedTime, ReceivedTimeUTC);
            DH->ProcessedInputData.ReceivedTimeUTC   = ReceivedTimeUTC;
            DH->ProcessedInputData.ReceivedTimeLocal = stringToTm(DH->Input.ReceivedTime);
            DH->Input.PlateImageBase64 = record.PlateImageBase64;

            auto CheckOpStartTime = std::chrono::high_resolution_clock::now();
            // 3- Run Check Operator Module
            if(DH->Modules.CheckOperator.active && DH->hasInputFields.PlateImage)
            {
                ChOp::InputStruct inputChOp;
                inputChOp.plateImageBase64 = DH->Input.PlateImageBase64;
                inputChOp.plateValue = DH->Input.PlateValue ;
                // inputChOp.plateType = DH->hasInputFields.PlateType ? DH->Input.PlateType : static_cast<int>(inference::standards::PlateType::UNKNOWN);

                int CheckOpObjectIndex = this->getCheckOpIndex();
                ChOp::OutputStruct ChOpOutput;
                // try
                // {
                    ChOpOutput = this->m_pChOpObjects[CheckOpObjectIndex]->run(inputChOp);
                    this->releaseCheckOpIndex(CheckOpObjectIndex);
                    DH->Input.MasterPlate = DH->Input.PlateValue;
                    DH->Input.PlateValue = ChOpOutput.newPlateValue;
                    DH->Input.CodeType = ChOpOutput.codeType;
                    DH->Input.Probability = ChOpOutput.probability;
                    DH->Input.PlateType = ChOpOutput.newPlateType;
                    DH->ProcessedInputData.croppedPlateImage = ChOpOutput.croppedPlateImage.clone();

                    chopCounter++;

                    // std::cout<<"DH->Input.MasterPlate"<<DH->Input.MasterPlate<<std::endl;
                    // std::cout<<"DH->Input.PlateValue"<<DH->Input.PlateValue<<std::endl;
                    // std::cout<<"DH->Input.CodeType"<<DH->Input.CodeType<<std::endl;
                    // std::cout<<"DH->Input.Probability"<<DH->Input.Probability<<std::endl;
                    // std::cout<<"DH->Input.PlateType"<<DH->Input.PlateType<<std::endl;




            //     } catch (const std::exception& e)
            //     {
            //         Response["Status"] = CHECKOPERROR;
            //         Response["Description"] = e.what();
            //         if(DH->hasInputFields.DeviceID && DH->hasInputFields.ViolationID && DH->hasInputFields.PassedTime && DH->hasInputFields.PlateValue)
            //             Response["RecordID"] = DH->ProcessedInputData.MongoID;
            //         Response["IP"] = DH->Request.remoteIP;
            //         if(DH->FailedDatabaseInfo.Enable)
            //         {
            //             std::vector<MongoDB::Field> fields = {
            //                 {"Status", std::to_string(CHECKOPERROR), MongoDB::FieldType::Integer},
            //                 {"Description", e.what(), MongoDB::FieldType::String},
            //                 {"IP", DH->Request.remoteIP, MongoDB::FieldType::String}
            //             };

            //             if(DH->hasInputFields.DeviceID && DH->hasInputFields.ViolationID && DH->hasInputFields.PassedTime && DH->hasInputFields.PlateValue)
            //             {
            //                 MongoDB::Field RecordIDField = {"RecordID", DH->ProcessedInputData.MongoID, MongoDB::FieldType::ObjectId};
            //                 fields.push_back(RecordIDField);
            //             }

            //             DH->FailedDatabase ->Insert(DH->FailedDatabaseInfo.DatabaseName, DH->FailedDatabaseInfo.CollectionName, fields);
            //         }
            //         if(DH->DebugMode)
            //             SHOW_ERROR(crow::json::dump(Response));
            //         this->releaseCheckOpIndex(CheckOpObjectIndex);
            //         ClientResponse["Status"] = CHECKOPERROR;
            //         ClientResponse["Description"] = "Internal Service Error.";
            //         return crow::response{DH->Response.HTTPCode , ClientResponse};
            //     }
            }
            auto CheckOpFinishTime = std::chrono::high_resolution_clock::now();
            auto CheckOpTime =  std::chrono::duration_cast<std::chrono::milliseconds>(CheckOpFinishTime - CheckOpStartTime).count();
            chopTime += CheckOpTime;

            if(DH->DebugMode)
                SHOW_IMPORTANTLOG3("ProccessTime(ms) = " << std::to_string(CheckOpTime) << std::endl );


        // auto storeImageDH = std::make_shared<DataHandler::DataHandlerStruct>(DH); // Copy DH for thread safety
        threadPool->enqueue([DH, this, storeimageobj, savedataobj]() {
           
#ifdef STOREIMAGE
             auto storeImageStartTime = std::chrono::high_resolution_clock::now();
            // std::shared_ptr<storeimage> storeimageobj = std::make_shared<storeimage>();
            if (!storeimageobj->run(DH)) {

            }
        // });
            storeImageCounter++;
            auto storeImageFinishTime = std::chrono::high_resolution_clock::now();
            auto storeImaheTime =  std::chrono::duration_cast<std::chrono::microseconds>(storeImageFinishTime - storeImageStartTime).count();
            storeImageTime +=storeImaheTime;
            if(DH->DebugMode){
            	SHOW_IMPORTANTLOG3("storeImageTime(microseconds) = " << std::to_string(storeImaheTime) << std::endl );
            	SHOW_IMPORTANTLOG3("storeImageCounter = " << std::to_string(storeImageCounter) << std::endl );
            	}       
#endif //STOREIMAGE
            
#ifdef INSERTDATABASE
            auto saveDataStartTime = std::chrono::high_resolution_clock::now();
        // 6- Save Data
            // std::shared_ptr<savedata> savedataobj = std::make_shared<savedata>();

                if(!(savedataobj->run(DH)))
                {


                }
                processCounter++;
                saveRecordCounter++;
                if(processCounter > 1000)
                {
                    std::vector<MongoDB::Field> MetaUpdateFields = { 
                    {"PassedVehicleRecordsId", std::to_string(DH->Input.RecordID), MongoDB::FieldType::Int64, "set"} //TODO  Think for better 
                    };
                    DH->ConfigDatabase->Update_one(DH->ConfigDatabaseInfo.DatabaseName,"Meta",MetaFindFields,MetaUpdateFields);
                    processCounter.exchange(0);
                }
                auto SaveEndTime = std::chrono::high_resolution_clock::now();
                double durationSaveTime = std::chrono::duration_cast<std::chrono::microseconds>(SaveEndTime - saveDataStartTime).count();
                saveRecordTime += durationSaveTime;
	            if(DH->DebugMode)
                    SHOW_IMPORTANTLOG3("saveRecordTime(microseconds) = " << std::to_string(durationSaveTime) << std::endl );

#endif //Save Data


        
        });
        auto endTime = std::chrono::high_resolution_clock::now();
        double durationTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - processStartTime).count();
        if(DH->DebugMode)
            SHOW_IMPORTANTLOG3("durationTime one Record (milliseconds) = " << std::to_string(durationTime) << std::endl );
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(2));
        

    }
}

RahdariService::getImagesResultStruct RahdariService::parsePlateImages(const std::string& jsonString, uint64_t MinId) {
    getImagesResultStruct result; // Initialize the result struct

    rapidjson::Document document;
    if (document.Parse(jsonString.c_str()).HasParseError()) {
        std::cerr << "Image JSON Parse Error: " << rapidjson::GetParseError_En(document.GetParseError()) << std::endl;
        result.Error = rapidjson::GetParseError_En(document.GetParseError());
        result.ErrorCode = INVALIDJSON;
        return result;
    }

    // Helper lambda to process a single image object
    auto processImageObject = [&](const rapidjson::Value& item) {
        // Validate that item is a JSON object
        if (!item.IsObject()) {
            std::string errorMsg = item.IsArray() ? "Item is an array, expected a JSON object" : "Item is not a JSON object";
            Logger::getInstance().logWarning(errorMsg + ". Skipping record...[" + std::to_string(MinId) + "]");
            result.Error = errorMsg;
            result.ErrorCode = INVALIDTYPE;
            return false;
        }

        // Validate required fields
        if (!item.HasMember("tid") || !item.HasMember("plate_image")) {
            Logger::getInstance().logWarning("Record missing required fields. Skipping record...[" + std::to_string(MinId) + "]");
            result.Error = "Missing 'tid' or 'plate_image' field in JSON response";
            result.ErrorCode = MISSINGFIELD;
            return false;
        }

        uint64_t tid = item["tid"].GetUint64();
        std::string plate_image;

        if (item["plate_image"].IsString()) {
            plate_image = item["plate_image"].GetString();
            // Efficiently remove newline characters
            plate_image.erase(std::remove(plate_image.begin(), plate_image.end(), '\n'), plate_image.end());
        } else if (item["plate_image"].IsNull()) {
            plate_image = "null";
            nullImageCounter++;
            if (this->ClientServiceConfig.DebugMode)
                Logger::getInstance().logWarning("Image Is NULL With tid: " + std::to_string(tid));
        }

        result.PlateImages_map[tid] = std::move(plate_image); // Move instead of copy
        return true;
    };

    // Check if JSON is an array (batch images) or single object
    if (document.IsArray()) {
        // Handle batch images
        if (document.Empty()) {
            Logger::getInstance().logError("The document array is empty. Skipping record... [" + std::to_string(MinId) + "]");
            result.Error = "Image Array is Empty";
            result.ErrorCode = EMPTYRESPONSE;
            return result;
        }

        // Reserve space in the map
        result.PlateImages_map.reserve(document.Size());
        ImageCounter += document.Size();

        // Process each item in the array
        for (const auto& item : document.GetArray()) {
            processImageObject(item);
        }
    } else if (document.IsObject()) {
        // Handle single image
        result.PlateImages_map.reserve(1);
        ImageCounter += 1;

        if (processImageObject(document)) {
            // Successfully processed single image
        } else {
            // Return with error if single image processing fails
            return result;
        }
    } else {
        Logger::getInstance().logError("The record received is not in the correct format (neither array nor object) [" + std::to_string(MinId) + "]");
        result.Error = "JSON is neither an array nor an object";
        result.ErrorCode = INVALIDTYPE;
        return result;
    }

    return result; // Return the result struct
}

// RahdariService::getImagesResultStruct RahdariService::parsePlateImages(const std::string& jsonString, uint64_t MinId) {

//     getImagesResultStruct result; // Initialize the result struct

//     rapidjson::Document document;
//     if (document.Parse(jsonString.c_str()).HasParseError()) {
//         std::cerr << "Image JSON Parse Error: " << rapidjson::GetParseError_En(document.GetParseError()) << std::endl;
//         result.Error = rapidjson::GetParseError_En(document.GetParseError());
//         result.ErrorCode = INVALIDJSON;
//         return result; // Return empty ResponseValue on error
//     }

// // // Check for "Error" field in the JSON (uncommented and improved)
// //     if (document.HasMember("Error") && document["Error"].IsString()) {
// //         result.Error = document["Error"].GetString();
// //         result.ErrorCode = SERVERERROR; // Or a specific error code for server-reported errors
// //         std::cerr << "Image Server reported error: " << result.Error << std::endl;
// //         return result;
// //     }

//     if (!document.IsArray()) {
//         Logger::getInstance().logError("The record received is not in the correct format !!! ["+ std::to_string(MinId)+"]");
//         result.Error = "JSON is not an array";
//         result.ErrorCode = INVALIDTYPE;
//         return result; // Return early if top-level JSON is not an array
//     }

//         // Log if the array is empty
//     if (document.Empty()) {
//         Logger::getInstance().logError("The document array is empty. Skipping record... [" + std::to_string(MinId) + "]");
//         result.Error = "Image Array is Empty";
//         result.ErrorCode = EMPTYRESPONSE;
//         return result; //s Return early if array is emptys
//     }

//     // Reserve space in the map if you have an idea of the number of records
//     result.PlateImages_map.reserve(document.Size());
//     ImageCounter += document.Size();

//     for (const auto& item : document.GetArray()) {
//         // Validate that each element is an json object
//         if (!item.IsObject()) {
//             Logger::getInstance().logWarning("Invalid record format: Expected an json object. Skipping record...["+ std::to_string(MinId)+"]");
//             result.Error = "Item in 'Images' array is not an object";
//             result.ErrorCode = INVALIDTYPE;
//             std::cerr << result.Error << std::endl;
//             continue; // Skip non-object elements
//         }

//         // Validate the presence and types of required fields
//         if (!item.HasMember("tid") ||!item.HasMember("plate_image") ){ //|| !item["plate_image"].IsString()) {
            
//             Logger::getInstance().logWarning("Record missing required fields or invalid field types. Skipping record...["+ std::to_string(MinId)+"]");
//             result.Error = "Missing 'tid' or 'plate_image' field in JSON response";
//             result.ErrorCode = MISSINGFIELD;
//             continue; // Skip records with missing or invalid fields
//         }

//         uint64_t tid = item["tid"].GetUint64();
//         std::string plate_image;

//         if(item["plate_image"].IsString()){ 

//             plate_image = item["plate_image"].GetString();

// // Efficiently remove newline characters
//             plate_image.erase(std::remove(plate_image.begin(), plate_image.end(), '\n'), plate_image.end());


//         }else if(item["plate_image"].IsNull()){
//             // result.Error = "Image Is NULL";
//             plate_image = "null";
//             nullImageCounter++;
//             if(this->ClientServiceConfig.DebugMode)
//                 Logger::getInstance().logWarning("Image Is NULL With tid : " +std::to_string(tid)); //TODO RETURN CODE TYPE;
//         }

//         result.PlateImages_map[tid] = std::move(plate_image); // Move instead of copy


//         }
        
//     return result; // Return the result struct
// }

std::unordered_map<uint64_t, std::string> RahdariService::getSingleImage(const std::string& Url, uint64_t Id)
{
    std::unordered_map<uint64_t, std::string> result;
    std::string requestUrl = Url + "/getplateimage/";
    CURL* curl = curl_easy_init();
    if(curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, requestUrl.c_str());
        std::string Packet = (std::string)"{\"username\": \"sharif\",\"password\":\"sharif@@654\",\"tid\":"+std::to_string(Id)+"}";
        // SHOW_IMPORTANTLOG("getplateimage_packet: " <<Packet<<"\n");


        struct curl_slist* HList = NULL;
        HList = curl_slist_append(HList, "Content-Type: application/json");
        HList = curl_slist_append(HList, "Accept: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, HList);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, Packet.c_str());
        std::string response;
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToString);

        long http_code = 0;
        const int maxRetries = 3;
        int retryCount = 0;
        int64_t duration_Image = 0;
        CURLcode res ;
        while(retryCount < maxRetries)
        {
                                // Start timing the request
            auto startTimeRequest = std::chrono::high_resolution_clock::now();
            res = curl_easy_perform(curl);
                    // End timing the request
            auto endTimeRequest = std::chrono::high_resolution_clock::now();
            duration_Image = std::chrono::duration_cast<std::chrono::milliseconds>(endTimeRequest - startTimeRequest).count();
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
            if (res == CURLE_OK) 
                break; // Success, exit retry loop

            else if(http_code == 429) {
            Logger::getInstance().logError("Rate limit hit for URL: " + requestUrl);
            std::this_thread::sleep_for(std::chrono::seconds(5)); // Wait before retry
            continue;
            // Retry logic here
            }else
            {
            Logger::getInstance().logError("Attempt " + std::to_string(retryCount + 1) + " curl_easy_perform() failed for URL: " + requestUrl +
                                " Error: " + curl_easy_strerror(res) +
                                " HTTP Code: " + (http_code ? std::to_string(http_code) : "N/A"));
            std::this_thread::sleep_for(std::chrono::milliseconds(1000 * retryCount)); // Exponential backoff
            }
        retryCount++;

        }
        ImageRequestTime+=duration_Image;
        ImageRequestCounter ++;
        if (res != CURLE_OK) {
            Logger::getInstance().logError("All retries failed for URL: " + requestUrl);

            std::string errorMsg = curl_easy_strerror(res);
            if (res == CURLE_COULDNT_RESOLVE_HOST) {
                Logger::getInstance().logError("DNS resolution failed for URL: " + requestUrl);
                return {}; // No retry for DNS issues
            } else if (res == CURLE_HTTP_RETURNED_ERROR) {
                long http_code = 0;
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
                Logger::getInstance().logError("HTTP error " + std::to_string(http_code) +
                                            " for URL: " + requestUrl);
                // Handle specific HTTP codes (e.g., 429 for rate-limiting)
            }
            Logger::getInstance().logError("curl_easy_perform() failed for URL: " + requestUrl +
                                        " Error: " + errorMsg);

            return {}; // Return empty vector on failure 
        }else{
            //SHOW_WARNING( "Response is : " << response);
            // ResponseValue ResVal = getResponseValue(response,"getPlateImage");
            curl_slist_free_all(HList);
            curl_easy_cleanup(curl);
            auto parseStartTime = std::chrono::high_resolution_clock::now();

            auto ResVal = parsePlateImages(response,Id);
                    // End timing the JSON parsing
            auto parseEndTime = std::chrono::high_resolution_clock::now();
            double parseDuration = std::chrono::duration_cast<std::chrono::milliseconds>(parseEndTime - parseStartTime).count();
            
            if(ResVal.Error == ""){
                result = ResVal.PlateImages_map;
                return result;
            }

            else{
                std::cout<<"Could not get Image: "<<ResVal.Error<<std::endl;
                return {};

            }
        }
        // curl_slist_free_all(HList);
        // curl_easy_cleanup(curl);
    }
    else
    {
        std::cout<<"curl_easy_init() failed: curl is NULL"<<std::endl;
        return {}; // Return empty map on failure
    }
    // return result;
}

std::unordered_map<uint64_t, std::string> RahdariService::getImageBase64_bulk(const std::string& Url, uint64_t MinId, uint64_t MaxId)
{
    // ImageInfo Img;
    getImagesResultStruct resVal;
    std::string requestUrl = Url + "/getplateimages/";
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "curl_easy_init() failed: curl is NULL" << std::endl;
        return {}; // Return empty map on failure
    }

        curl_easy_setopt(curl, CURLOPT_URL, requestUrl.c_str());
        std::string Packet = (std::string)"{\"username\": \"sharif\",\"password\":\"sharif@@654\",\"tid1\":"+std::to_string(MinId)+", \"tid2\":"+ std::to_string(MaxId) + "}";
        // SHOW_IMPORTANTLOG3("getplateimages_Packet : "<< Packet<<"\n");
        struct curl_slist* HList = NULL;
        HList = curl_slist_append(HList, "Content-Type: application/json");
        HList = curl_slist_append(HList, "Accept: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, HList);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, Packet.c_str());
        std::string response;
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToString);
        // Set connection timeout (in seconds)
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30L); 
        // Set timeout
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60L);



        long http_code = 0;
        const int maxRetries = 3;
        int retryCount = 0;
        int64_t duration_Image = 0;
        CURLcode res ;

        while (retryCount < maxRetries)
        {
                    // Start timing the request
            auto startTimeRequest = std::chrono::high_resolution_clock::now();
            res = curl_easy_perform(curl);
                    // End timing the request
            auto endTimeRequest = std::chrono::high_resolution_clock::now();
            duration_Image = std::chrono::duration_cast<std::chrono::milliseconds>(endTimeRequest - startTimeRequest).count();
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
            if (res == CURLE_OK) 
                break; // Success, exit retry loop

            else if(http_code == 429) {
            Logger::getInstance().logError("Rate limit hit for URL: " + requestUrl);
            std::this_thread::sleep_for(std::chrono::seconds(5)); // Wait before retry
            continue;
            // Retry logic here
            }else
            {
            Logger::getInstance().logError("Attempt " + std::to_string(retryCount + 1) + " curl_easy_perform() failed for URL: " + requestUrl +
                                " Error: " + curl_easy_strerror(res) +
                                " HTTP Code: " + (http_code ? std::to_string(http_code) : "N/A"));
            std::this_thread::sleep_for(std::chrono::milliseconds(1000 * retryCount)); // Exponential backoff
            }
        retryCount++;

        }
        ImageRequestTime+=duration_Image;
        ImageRequestCounter ++;

        if (res != CURLE_OK) {
            Logger::getInstance().logError("All retries failed for URL: " + requestUrl);

            std::string errorMsg = curl_easy_strerror(res);
            if (res == CURLE_COULDNT_RESOLVE_HOST) {
                Logger::getInstance().logError("DNS resolution failed for URL: " + requestUrl);
                return {}; // No retry for DNS issues
            } else if (res == CURLE_HTTP_RETURNED_ERROR) {
                long http_code = 0;
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
                Logger::getInstance().logError("HTTP error " + std::to_string(http_code) +
                                            " for URL: " + requestUrl);
                // Handle specific HTTP codes (e.g., 429 for rate-limiting)
            }
            Logger::getInstance().logError("curl_easy_perform() failed for URL: " + requestUrl +
                                        " Error: " + errorMsg);

            return {}; // Return empty vector on failure 
        }



        curl_slist_free_all(HList);
        curl_easy_cleanup(curl);

        // Start timing the JSON parsing
        auto parseStartTime = std::chrono::high_resolution_clock::now();

        resVal =  parsePlateImages(response, MinId); // Handle parsing errors within this function

        // End timing the JSON parsing
        auto parseEndTime = std::chrono::high_resolution_clock::now();
        double parseDuration = std::chrono::duration_cast<std::chrono::milliseconds>(parseEndTime - parseStartTime).count();

        if(resVal.Error.empty())
        {
            auto result = resVal.PlateImages_map; // Store result to return it
            return result; 
        }
        else{
            std::cerr<<"Could not get Image in Range : "<< std::to_string(MinId) << " TO " << std::to_string(MaxId)<<" " << resVal.Error <<std::endl; //TODO save in failed database
            return {};
        }

}


RahdariService::ImageInfo RahdariService::getImageBase64(std::string Url, uint64_t Id)
{
    ImageInfo Img;
    Url += "/getplateimage/";
    CURL* curl = curl_easy_init();
    if(curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, Url.c_str());
        std::string Packet = (std::string)"{\"username\": \"sharif\",\"password\":\"sharif@@654\",\"tid\":"+std::to_string(Id)+"}";

        struct curl_slist* HList = NULL;
        HList = curl_slist_append(HList, "Content-Type: application/json");
        HList = curl_slist_append(HList, "Accept: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, HList);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, Packet.c_str());
        std::string response;
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToString);
        CURLcode res = curl_easy_perform(curl);
        if(res != CURLE_OK)
        {
            std::cout<<"curl_easy_perform() failed: "<<curl_easy_strerror(res)<<std::endl;
        }
        else
        {
            //SHOW_WARNING( "Response is : " << response);
            ResponseValue ResVal = getResponseValue(response, "getImage");
            if(ResVal.Error == "")
                Img = ResVal.ImageResult;
            else
                std::cout<<"Could not get Image: "<<ResVal.Error<<std::endl;
        }
        curl_easy_cleanup(curl);
    }
    else
    {
        std::cout<<"curl_easy_init() failed: curl is NULL"<<std::endl;
    }
    return Img;
}


uint64_t RahdariService::getMaxId(const std::string& Url)
{
    
    uint64_t MaxId = 0;
    std::string requestUrl = Url + "/maxid/";

    CURL* curl = curl_easy_init();
    if(curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, requestUrl.c_str());
            // Create a Crow JSON object
        crow::json::wvalue json_obj;

        // Populate the JSON object
        json_obj["username"] = "sharif";
        json_obj["password"] = "sharif@@654";

        // Convert JSON object to string
        std::string Packet = crow::json::dump(json_obj);
        // std::string Packet = (std::string)"{\"username\": \"sharif\",\"password\":\"sharif@@654\"}";

        struct curl_slist* HList = NULL;
        HList = curl_slist_append(HList, "Content-Type: application/json");
        HList = curl_slist_append(HList, "Accept: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, HList);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, Packet.c_str());
        std::string response;
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToString);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        CURLcode res = curl_easy_perform(curl);
        if(res != CURLE_OK)
        {
            Logger::getInstance().logError(
            std::string("curl_easy_perform() failed for MaxId Url: ") + curl_easy_strerror(res)
        );
        }
        else
        {
            std::cout<<"getMaxId response: "<<response<<std::endl;
            ResponseValue ResVal = getResponseValue(response, "maxid");
            if(ResVal.MaxIdResult != -1)
                MaxId = ResVal.MaxIdResult;
            else
                std::cout<<"Could not get max ID "<<ResVal.Error;
        }
        curl_easy_cleanup(curl);
    }
    else
    {
        std::cout<<"curl_easy_init() failed: curl is NULL"<<std::endl;
    }

    return MaxId;
}
int curl_count = 0;
std::vector<RahdariService::TTOInfo> RahdariService::getInfo(const std::string& Url, uint64_t MinId, uint64_t MaxId)
{
    std::vector<TTOInfo> Info;
    std::string requestUrl = Url + "/getttoinfo/";
    CURL* curl = curl_easy_init();
    if (!curl) 
    {
        std::cerr << "curl_easy_init() failed: curl is NULL" << std::endl;
        return Info; // Return empty vector on failure
    }

    std::string Packet = (std::string)"{\"username\": \"sharif\",\"password\":\"sharif@@654\",\"tid1\":"+std::to_string(MinId)+",\"tid2\":"+std::to_string(MaxId)+"}";
    // SHOW_IMPORTANTLOG("getttoinfo_Packet: " <<Packet<<"\n");

    struct curl_slist* HList = NULL;
    curl_easy_setopt(curl, CURLOPT_URL, requestUrl.c_str());
    HList = curl_slist_append(HList, "Content-Type: application/json");
    HList = curl_slist_append(HList, "Accept: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, HList);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, Packet.c_str());
    std::string response = "";
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToString);
    // Set connection timeout (in seconds)
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30L); 
    // Set timeout
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60L);



    long http_code = 0;
    const int maxRetries = 3;
    int retryCount = 0;
    int64_t duration_Info = 0;
    CURLcode res ;
    ResponseValue ResVal;
    while (retryCount < maxRetries)
    {
            // Start timing the request
        auto startTimeRequest = std::chrono::high_resolution_clock::now();

        res = curl_easy_perform(curl);
            // End timing the request
        auto endTimeRequest = std::chrono::high_resolution_clock::now();
        duration_Info = std::chrono::duration_cast<std::chrono::milliseconds>(endTimeRequest - startTimeRequest).count();

        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        if (res == CURLE_OK)
        {
            auto parseStartTime = std::chrono::high_resolution_clock::now();

            ResVal = getResponseValue(response, "getttoinfo");

            auto parseEndTime = std::chrono::high_resolution_clock::now(); // End timing the JSON parsing

            double parseDuration = std::chrono::duration_cast<std::chrono::milliseconds>(parseEndTime - parseStartTime).count();

            if(ResVal.TTOInfoResult.size() != 0)
            {
                Info.swap(ResVal.TTOInfoResult);
                break; // Success, exit retry loop

            }else if (ResVal.ErrorCode == EMPTYRESPONSE){
                std::cerr<<"Could not get TTOInfo in Range : "<< std::to_string(MinId) << " TO " << std::to_string(MaxId)<<" " << ResVal.Error <<std::endl;
                continue;
            }
                
        } 
            
        else if(http_code == 429) {
        Logger::getInstance().logError("Rate limit hit for URL: " + requestUrl);
        std::this_thread::sleep_for(std::chrono::seconds(2)); // Wait before retry
        continue;

        }else
        {
        Logger::getInstance().logError("Attempt " + std::to_string(retryCount + 1) + " curl_easy_perform() failed for URL: " + requestUrl +
                               " Error: " + curl_easy_strerror(res) +
                               " HTTP Code: " + (http_code ? std::to_string(http_code) : "N/A"));
        
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 * retryCount)); // Exponential backoff
        }
        retryCount++;
    }
    InfoRequestTime +=duration_Info;
    InfoRequestCounter ++;

    if (res != CURLE_OK) {
    Logger::getInstance().logError("All retries failed for URL: " + requestUrl);
    
    std::string errorMsg = curl_easy_strerror(res);
    if (res == CURLE_COULDNT_RESOLVE_HOST) {
        Logger::getInstance().logError("DNS resolution failed for URL: " + requestUrl);
        return Info; // No retry for DNS issues
    } else if (res == CURLE_HTTP_RETURNED_ERROR) {
        long http_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        Logger::getInstance().logError("HTTP error " + std::to_string(http_code) +
                                       " for URL: " + requestUrl); // Handle specific HTTP codes (e.g., 429 for rate-limiting)
        
    }

    Logger::getInstance().logError("curl_easy_perform() failed for URL: " + requestUrl +
                                   " Error: " + errorMsg);

    return Info; // Return empty vector on failure 
    }


    curl_slist_free_all(HList);
    curl_easy_cleanup(curl);

    // auto parseStartTime = std::chrono::high_resolution_clock::now();

    // ResponseValue ResVal = getResponseValue(response, "getttoinfo");

    //     // End timing the JSON parsing
    // auto parseEndTime = std::chrono::high_resolution_clock::now();
    // double parseDuration = std::chrono::duration_cast<std::chrono::milliseconds>(parseEndTime - parseStartTime).count();

    // if(ResVal.TTOInfoResult.size() != 0)
    //     Info.swap(ResVal.TTOInfoResult);
    // else if (ResVal.ErrorCode == EMPTYRESPONSE)
    //     std::cerr<<"Could not get TTOInfo in Range : "<< std::to_string(MinId) << " TO " << std::to_string(MaxId)<<" " << ResVal.Error <<std::endl; //TODO save in failed database

    if (ResVal.ErrorCode == EMPTYRESPONSE)
    {
        uint64_t RealMaxid =  getMaxId(Url);
        if(MinId >= RealMaxid)
        {
            std::cout<<"waiting for new records ...";
        }

    }
    return Info;
}


bool RahdariService::isExistsFile(std::string FilePath)
{
    struct stat buffer;
    return (stat(FilePath.c_str(), &buffer) == 0);
}

int& RahdariService::convertVehcileType(int& InputVehicleType)
{
    if(InputVehicleType == 1)
        InputVehicleType = 0;
    else if(InputVehicleType == 2)
        InputVehicleType = 1;
    else if(InputVehicleType == -1)
        return InputVehicleType;

return InputVehicleType;

}

RahdariService::ResponseValue RahdariService::getResponseValue(const std::string& jsonString, std::string ResType)
{

    ResponseValue resVal;
    rapidjson::Document document;

    // Parse the JSON response
    if (document.Parse(jsonString.c_str()).HasParseError()) {
        std::cerr << "JSON parse error: " << rapidjson::GetParseError_En(document.GetParseError()) << std::endl;
        resVal.Error = rapidjson::GetParseError_En(document.GetParseError());
        resVal.ErrorCode = INVALIDJSON;
        return resVal; // Return empty ResponseValue on error
    }

// Check for "Error" field in the JSON (uncommented and improved)
    if (document.HasMember("Error") && document["Error"].IsString()) {
        resVal.Error = document["Error"].GetString();
        resVal.ErrorCode = SERVERERROR; // Or a specific error code for server-reported errors
        std::cerr << "Server reported error: " << resVal.Error << std::endl;
        return resVal;
    }

    if (ResType == "maxid") {
            if (!document.HasMember("maxid")) {
                resVal.Error = "Missing 'maxid' field in JSON response";
                resVal.ErrorCode = MISSINGFIELD;
                std::cerr << resVal.Error << std::endl;
                return resVal;
            }
            if (!document["maxid"].IsInt64()) {
                resVal.Error = "'maxid' field is not an int64";
                resVal.ErrorCode = MISSINGFIELD;
                std::cerr << resVal.Error << std::endl;
                return resVal;
            }
        resVal.MaxIdResult = document["maxid"].GetInt64();
    } else if (ResType == "getttoinfo")
    {
        if (!document.HasMember("res")) {
                resVal.Error = "Missing 'res' field in JSON response";
                resVal.ErrorCode = MISSINGFIELD;
                std::cerr << resVal.Error << std::endl;
                return resVal;
        }
        if (!document["res"].IsArray()) {
            resVal.Error = "'res' field is not an array";
            resVal.ErrorCode = INVALIDTYPE;
            std::cerr << resVal.Error << std::endl;
            return resVal;
        }

        const auto& resArray = document["res"].GetArray();
        if (resArray.Empty()) {
            resVal.Error = "'res' array is empty";
            resVal.ErrorCode = EMPTYRESPONSE;
            std::cerr << resVal.Error << std::endl;
            return resVal;
        }

        for (const auto& item : resArray) 
            {
                if (!item.IsObject()) {
                    resVal.Error = "Item in 'res' array is not an object";
                    resVal.ErrorCode = INVALIDTYPE;
                    std::cerr << resVal.Error << std::endl;
                    continue;
                }

                TTOInfo info;
                
                // Handle ID
                info.TTOInfoId = item.HasMember("ID") && item["ID"].IsUint64() ? item["ID"].GetUint64() : 0;

                // Handle PASS_DATE
                info.PassDateTime = item.HasMember("PASS_DATE") && !item["PASS_DATE"].IsNull() ? item["PASS_DATE"].GetString() : "";

                // Handle DEVICEID
                info.DeviceId = item.HasMember("DEVICEID") && !item["DEVICEID"].IsNull() ? item["DEVICEID"].GetInt() : 0;

                // Handle SYSTEM_CODE
                info.SystemCode = item.HasMember("SYSTEM_CODE") && !item["SYSTEM_CODE"].IsNull() ? item["SYSTEM_CODE"].GetInt() : 0;

                // Handle COMPANY_CODE
                info.CompanyCode = item.HasMember("COMPANY_CODE") && !item["COMPANY_CODE"].IsNull() ? item["COMPANY_CODE"].GetInt() : 0;

                // Handle PLATE_CODE
                info.VehiclePlate = item.HasMember("PLATE_CODE") && !item["PLATE_CODE"].IsNull() ? std::to_string(item["PLATE_CODE"].GetUint64()) : "0";
                if(item.HasMember("PLATE_CODE") && item["PLATE_CODE"]== 0)
                {
                    info.VehiclePlate = "0";
                }

                // Handle LINE_NUMBER
                info.LineNumber = item.HasMember("LINE_NUMBER") && !item["LINE_NUMBER"].IsNull() ? item["LINE_NUMBER"].GetInt() : 0;

                // Handle VEHICLE_SPEED
                info.VehicleSpeed = item.HasMember("VEHICLE_SPEED") && !item["VEHICLE_SPEED"].IsNull() ? item["VEHICLE_SPEED"].GetInt() : 0;

                // Handle ALLOWED
                info.Allowed = item.HasMember("ALLOWED") && !item["ALLOWED"].IsNull() ? std::to_string(item["ALLOWED"].GetInt()) : "0";

                // Handle VEHICLE_CLASS
                info.VehicleClass = item.HasMember("VEHICLE_CLASS") && !item["VEHICLE_CLASS"].IsNull() ? item["VEHICLE_CLASS"].GetInt() : -1;
                info.VehicleClass = convertVehcileType(info.VehicleClass);

                // Handle WRONG_DIRECTION
                info.WrongDirection = item.HasMember("WRONG_DIRECTION") && !item["WRONG_DIRECTION"].IsNull() ? std::to_string(item["WRONG_DIRECTION"].GetInt()) : "0";

                // Handle HAS_CRIME
                info.HasCrime = item.HasMember("HAS_CRIME") && !item["HAS_CRIME"].IsNull() ? std::to_string(item["HAS_CRIME"].GetInt()) : "0";

                // Handle RECIEVE_DATE_TIME
                info.ReceiveDateTime = item.HasMember("RECIEVE_DATE_TIME") && !item["RECIEVE_DATE_TIME"].IsNull() ? item["RECIEVE_DATE_TIME"].GetString() : "";

                // Handle HAS_IMAGE
                info.HasPlateImage = item.HasMember("HAS_IMAGE") && !item["HAS_IMAGE"].IsNull() ? std::to_string(item["HAS_IMAGE"].GetInt()) : "0";

                // Handle REGISTER_DATE_TIME
                info.RegisterDateTime = item.HasMember("REGISTER_DATE_TIME") && !item["REGISTER_DATE_TIME"].IsNull() ? item["REGISTER_DATE_TIME"].GetString() : "";


                resVal.TTOInfoResult.push_back(info);


            }
            
    }
    else if(ResType == "getPlateImage")
    {
        // ResponseValue ResVal;

    //std::cout << "getResponseValue input is : " << Response << std::endl;

    try
    {
        boost::property_tree::ptree InputJson;

        std::stringstream InputJsonStructure;
        InputJsonStructure << jsonString;
        boost::property_tree::read_json(InputJsonStructure, InputJson);

        // std::cout << InputJsonStructure.str() << std::endl;

        resVal.Error = InputJson.get<std::string>("Error", "");
        resVal.ImageResult.TTOInfoId = std::to_string(InputJson.get<int>("tid", -1));

//        std::string tmpcolor = InputJson.get<std::string>("color_image", "");
        std::string tmpplate = InputJson.get<std::string>("plate_image", "");

//        tmpcolor.erase(std::remove(tmpcolor.begin(), tmpcolor.end(), '\n'), tmpcolor.end());
        tmpplate.erase(std::remove(tmpplate.begin(), tmpplate.end(), '\n'), tmpplate.end());

        //std::cout << "plate image is : "  <<  tmpplate << std::endl;

//        ResVal.ImageResult.ColorImage = tmpcolor;
        resVal.ImageResult.PlateImageBase64 = tmpplate;
    }
    catch(...)
    {
        std::cout<<"Invalid Json Packet !!!"<<std::endl;
    }
    }

    return resVal;

}
