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

size_t RahdariService::getSize() {
    batchMetaData_mutex.lock();
    size_t sizeQueue = batchMetaData_vec.size();
    batchMetaData_mutex.unlock();
    return sizeQueue;
}

using namespace cv;
using namespace std;

std::atomic<uint32_t> RahdariService::totalInfoFetchingTime(0);
std::atomic<uint32_t> RahdariService::InfofetchingCount(0);
boost::mutex RahdariService::mtx_Info;

std::atomic<uint32_t> RahdariService::totalImageFetchingTime(0);
std::atomic<uint32_t> RahdariService::ImagefetchingCount(0);
std::atomic<uint32_t> RahdariService::validImagesCount(0);
boost::mutex RahdariService::mtx_Image;
boost::mutex RahdariService::mtx_ValidImage;
boost::condition_variable RahdariService::batchMetaData_cv;

void RahdariService::monitor() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(monitorInterval));

        {
            size_t totalQueueSize = 0;
            size_t batchInfoQueue = 0;
            ChOp::mtx_ChOp.lock();
            ChOp::mtx_Saving.lock();
            RahdariService::mtx_Image.lock();
            RahdariService::mtx_Info.lock();
            RahdariService::mtx_ValidImage.lock();

            uint32_t avgSavingTime        = ChOp::savingCount > 0 ? ChOp::totalSavingTime / ChOp::savingCount : 0;
            uint32_t avgProcessingTime    = ChOp::ChOpCount > 0 ? ChOp::totalChOpTime / ChOp::ChOpCount : 0;

            uint32_t avgInfoFetchingTime  = RahdariService::InfofetchingCount > 0 ? RahdariService::totalInfoFetchingTime / RahdariService::InfofetchingCount : 0;
            uint32_t avgImageFetchingTime = RahdariService::ImagefetchingCount > 0 ? RahdariService::totalImageFetchingTime / RahdariService::ImagefetchingCount : 0;

            uint32_t numProcessedRecordsPerSec = ChOp::ChOpCount > 0 ? ChOp::ChOpCount / monitorInterval : 0;
            uint32_t numBulkInfoPerSec    =  RahdariService::InfofetchingCount > 0 ? RahdariService::InfofetchingCount / monitorInterval  : 0;
            uint32_t numBulkImagePerSec   =  RahdariService::ImagefetchingCount > 0 ? RahdariService::ImagefetchingCount * 100 / monitorInterval  : 0;
            uint32_t numValidImagePerSec  =  RahdariService::validImagesCount > 0 ? RahdariService::validImagesCount / monitorInterval  : 0;
            
            for(int i =0 ; i<this->CheckOpNumberOfObjectPerService; i++)
            {
                totalQueueSize +=this->m_pChOpObjects[i]->getSize();
            }
            batchInfoQueue = this->getSize();
            Logger::getInstance().logImportant3("\n ****************************************** \n");
            Logger::getInstance().logImportant3("Average Saving Time In " 
                            + std::to_string(monitorInterval) 
                            + " Second: " + std::to_string(avgSavingTime) 
                            + " (ms)");


            Logger::getInstance().logImportant3("Average Processing Time In " 
                            + std::to_string(monitorInterval) 
                            + " Second: " + std::to_string(avgProcessingTime) 
                            + " (ms)");

            Logger::getInstance().logImportant3("Average Info Fetching Time In " 
                            + std::to_string(monitorInterval) 
                            + " Second: " + std::to_string(avgInfoFetchingTime) 
                            + " (ms)");

            Logger::getInstance().logImportant3("Average Image Fetching Time In " 
                            + std::to_string(monitorInterval) 
                            + " Second: " + std::to_string(avgImageFetchingTime) 
                            + " (ms)");

            Logger::getInstance().logImportant3("Number of Records Check Operated In " 
                            + std::to_string(monitorInterval) 
                            + " Second: " + std::to_string(ChOp::ChOpCount));

            Logger::getInstance().logImportant3("Total Number of Images In " 
                            + std::to_string(monitorInterval) 
                            + " Second: " + std::to_string(RahdariService::ImagefetchingCount * 100)); // TODO

            Logger::getInstance().logImportant3("Total Number of Valid Images In " 
                            + std::to_string(monitorInterval) 
                            + " Second: " + std::to_string(RahdariService::validImagesCount)); // TODO

            Logger::getInstance().logImportant3("Total Size of Chop Queue In " 
                            + std::to_string(monitorInterval) 
                            + " Second: " + std::to_string(totalQueueSize));
            Logger::getInstance().logImportant3("batchInfoQueue : "+std::to_string(batchInfoQueue));

            Logger::getInstance().logImportant3("\n **************** Per Second ******************* \n");

            Logger::getInstance().logImportant3("Total Number of Images In One Second: " 
                            + std::to_string(numBulkImagePerSec)); // TODO

            Logger::getInstance().logImportant3("Total Number of Valid Images In One Second: " 
                            + std::to_string(numValidImagePerSec));

            Logger::getInstance().logImportant3("Total Size of Chop Queue In One Second: " 
                            + std::to_string(totalQueueSize / 10));

            Logger::getInstance().logImportant3("Number of Records Check Operated In One Second: " 
                            + std::to_string(numProcessedRecordsPerSec)); 

            Logger::getInstance().logImportant3("\n ****************************************** \n");                
    

            // Reset counts for the next interval
            ChOp::totalSavingTime = 0;
            ChOp::savingCount = 0;
            ChOp::totalChOpTime = 0;
            ChOp::ChOpCount = 0;
            RahdariService::totalInfoFetchingTime = 0;
            RahdariService::InfofetchingCount = 0;
            RahdariService::totalImageFetchingTime = 0;
            RahdariService::ImagefetchingCount = 0;
            RahdariService::validImagesCount = 0;

            ChOp::mtx_ChOp.unlock();
            ChOp::mtx_Saving.unlock();
            RahdariService::mtx_Image.unlock();
            RahdariService::mtx_Info.unlock();
            RahdariService::mtx_ValidImage.unlock();
        }
    }
}

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


void RahdariService::getInfoHandler()
{
    int sleep_duration = 1000; // Start with 1 second
    std::chrono::steady_clock::time_point last_check_time = std::chrono::steady_clock::now();
    const std::chrono::seconds check_interval(5); // Check every 5 seconds
    const size_t threshold = this->CurlServiceConfig.ThresholdFetchedRecors; // Set threshold
    

    while (true)
    {
        MinId_mtx.lock();
        uint64_t minIdLocal = this->MinId.fetch_add(InfoCount);
        MinId_mtx.unlock();
        std::vector<TTOInfo> Info = getInfo(Url, minIdLocal, minIdLocal+InfoCount-1);

        if(Info.empty())
        {
            Logger::getInstance().logError("Info is empty !!!");
            boost::this_thread::sleep_for(boost::chrono::milliseconds(sleep_duration));
            sleep_duration = std::min(sleep_duration * 2, 5000); 
            continue;
        } 
        sleep_duration = 1000; // Reset on success

        //getplateimages

        std::unordered_map<uint64_t, std::string> Images_map = getImageBase64_bulk(Url, minIdLocal, minIdLocal+InfoCount-1);
        if(Images_map.empty())
            continue;

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

        }

        if(Info.size()>0)
        {
            batchMetaData_mutex.lock();
            batchMetaData_vec.push_back(Info);
            batchMetaData_mutex.unlock();
        }
        else
        {
            Logger::getInstance().logError("Info is empty.");
            boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
            continue;
        }

    //A mutex
        // // Check every 10 seconds
        // auto now = std::chrono::steady_clock::now();
        // if (now - last_check_time >= check_interval)
        // {
        //     last_check_time = now; // Reset last check time

        //     batchMetaData_mutex.lock();

        //     // If batchMetaData_vec size exceeds the threshold, wait for it to become empty
        //     if (batchMetaData_vec.size() > threshold)
        //     {
        //         SHOW_IMPORTANTLOG3("Waiting for batchMetaData_vec to be empty... Size: " << batchMetaData_vec.size());

        //         while (!batchMetaData_vec.empty())
        //         {
        //             batchMetaData_mutex.unlock();
        //             boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));  // Sleep for 1 second
        //             batchMetaData_mutex.lock();  // Re-lock after waking up
        //         }

        //         SHOW_IMPORTANTLOG3("batchMetaData_vec is now empty.");
        //     }

        //     batchMetaData_mutex.unlock();
        // }



    // B condition variable
            // Check every 10 seconds
        auto now = std::chrono::steady_clock::now();
        if (now - last_check_time >= check_interval)
        {
            last_check_time = now; // Reset last check time

            // If batchMetaData_vec size exceeds the threshold, wait for it to become empty
            boost::unique_lock<boost::mutex> lock(batchMetaData_mutex);
            if (batchMetaData_vec.size() > threshold)
            {
                Logger::getInstance().logImportant2("Waiting for batchMetaData_vec to be empty...");

                
                // Wait until batchMetaData_vec is empty
                batchMetaData_cv.wait(lock, [this]{ return batchMetaData_vec.empty(); });

                Logger::getInstance().logImportant2("batchMetaData_vec is now empty."); 
            }
        }


        boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
    }
    
}

RahdariService::RahdariService(Configurate::CurlServiceConfigStruct ServiceConfig)
{
    this->CurlServiceConfig = ServiceConfig;
}
int RahdariService::init()
{
    if(!this->CurlServiceConfig.ReadFromMinIdTXT)
    {
        Configurate* ConfigurateObj = Configurate::getInstance();
        this->MinId = ConfigurateObj->getMeta().last_processed_id;
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
            // Read the entire file content into a string
        std::string minIdStr;
        t.seekg(0, std::ios::end);
        minIdStr.reserve(t.tellg());
        t.seekg(0, std::ios::beg);
        minIdStr.assign((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
        // Output the raw string read from the file
        std::cout << "Raw MinId from file: " << minIdStr << std::endl;
        // Convert string to long long
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

    curl_global_init(CURL_GLOBAL_ALL);
    
    for(int i = 0;i < this->CurlServiceConfig.ThreadNumber; i++){
        boost::thread(&RahdariService::getInfoHandler, this).detach(); 
    }
        
    if(this->CurlServiceConfig.MonitorMode)
        boost::thread (&RahdariService::monitor, this).detach();


    return 0; // Success
}
void RahdariService::run()
{
    bool cont;
    Url = this->CurlServiceConfig.CurlServiceInfo.URI;

    while(true)
    {
        auto StartTime_getMaxId = std::chrono::high_resolution_clock::now();
        this->MaxId = getMaxId(Url);
        auto EndTime_getMaxId = std::chrono::high_resolution_clock::now();
        double duration_maxId = std::chrono::duration_cast<std::chrono::milliseconds>(EndTime_getMaxId - StartTime_getMaxId).count();
        if(this->MaxId != 0)
        {
            SHOW_IMPORTANTLOG("MaxId: "<< this->MaxId <<", Time: "<<std::to_string(duration_maxId));
            if(this->MaxId == TmpMaxId)
            {
                boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
                continue;
            }
            TmpMaxId = this->MaxId;
            std::string cmd = "echo 'MaxId: "+std::to_string(this->MaxId)+"'>MaxId.txt";
            system(cmd.c_str());
        }
        else
        {
                Logger::getInstance().logError("MaxId is null, Time: ");
    
            // return -1; //TODO ERROR HANDLING FOR VIOD FUNC
        }

        while(true)
        {

            std::vector<TTOInfo> Info;
            batchMetaData_mutex.lock();
            if(batchMetaData_vec.size()>0)
            {
                Info = batchMetaData_vec[0];
                batchMetaData_vec.erase(batchMetaData_vec.begin());
                batchMetaData_mutex.unlock();
            }
            else
            {
                batchMetaData_mutex.unlock();
                continue;
            }

                        // Notify the producer if the vector is now empty
            {
                boost::lock_guard<boost::mutex> lock(batchMetaData_mutex);
                if (batchMetaData_vec.empty()) {
                    batchMetaData_cv.notify_all();  // Notify the producer thread
                }
            }


            if(Info.size() == 0)
            {
                Logger::getInstance().logError("Can not get Info!");
                MinId = MinId+1; //TODO NOT SURE
                boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
                continue;
            }


            int CheckOpObjectIndex = 0;
            int j = 0;
            for(int i = 0; i < Info.size(); i++)
            {
                // InfoIdVec.push_back(Info[i].TTOInfoId);
                if(j >= this->CheckOpNumberOfObjectPerService)
                {
                    j = 0;
                }
                std::shared_ptr<DataHandler::DataHandlerStruct> DH = std::make_shared<DataHandler::DataHandlerStruct>();


                Configurate* ConfigurateObj = Configurate::getInstance();
                DH->hasInputFields       =  ConfigurateObj->getInputFields();
                DH->hasOutputFields      =  ConfigurateObj->getOutputFields();
                DH->StoreImageConfig     =  ConfigurateObj->getStoreImageConfig();
                DH->ViolationMap         =  ConfigurateObj->getViolationMap();
                DH->Cameras              =  ConfigurateObj->getCameras();
                // DH->DaysforPassedTimeAcceptable = this->CurlServiceConfig.DaysforPassedTimeAcceptable;
                DH->InsertDatabase       =  ConfigurateObj->getInsertDatabase();
                DH->InsertDatabaseInfo   =  ConfigurateObj->getInsertDatabaseInfo();
                DH->FailedDatabase       =  ConfigurateObj->getFailedDatabase();
                DH->FailedDatabaseInfo   =  ConfigurateObj->getFailedDatabaseInfo();
                DH->Modules              =  ConfigurateObj->getModules();
                DH->ConfigDatabase       =  ConfigurateObj->getConfigDatabase();
                DH->ConfigDatabaseInfo   =  ConfigurateObj->getConfigDatabaseInfo();
                DH->DebugMode            =  this->CurlServiceConfig.DebugMode;
                DH->MonitorMode          = this->CurlServiceConfig.MonitorMode;

                crow::json::wvalue Response;
                DH->Input.DeviceID        =  Info[i].DeviceId;
                DH->Input.Lane            =  Info[i].LineNumber;
                DH->Input.PassedTime      =  Info[i].PassDateTime;
                DH->Input.SystemCode      =  Info[i].SystemCode;
                DH->Input.PassedVehicleRecordsId = Info[i].TTOInfoId ;
                DH->Input.VehicleType     = Info[i].VehicleClass;
                DH->Input.PlateValue      = Info[i].VehiclePlate;
                DH->Input.Speed           = Info[i].VehicleSpeed;
                DH->Input.CompanyCode     = Info[i].CompanyCode;
                DH->Input.ReceivedTime    = getCurrentTimeSec();

                std::tm PassedTimeUTC     = {};
                std::tm ReceivedTimeUTC   = {};

                ConvertLocalTimeToUTC(DH->Input.PassedTime, PassedTimeUTC);
                DH->ProcessedInputData.PassedTimeUTC   = PassedTimeUTC;
                DH->ProcessedInputData.PassedTimeLocal = stringToTm(DH->Input.PassedTime);

                ConvertLocalTimeToUTC(DH->Input.ReceivedTime, ReceivedTimeUTC);
                DH->ProcessedInputData.ReceivedTimeUTC   = ReceivedTimeUTC;
                DH->ProcessedInputData.ReceivedTimeLocal = stringToTm(DH->Input.ReceivedTime);
                DH->Input.PlateImageBase64 = Info[i].PlateImageBase64;

                if(DH->Input.PlateValue == "000000000")
                {
                    DH->Response.HTTPCode = 400;
                    DH->Response.errorCode = INVALIDMASTERPLATE;
                    DH->Response.Description = "The Value of MasterPlate Is Invalid.";
                }

                //getImages
                if(Info[i].error != "")//TODO
                {
                    DH->Response.HTTPCode = 400;
                    DH->Response.errorCode = CANNOTFINDIMAGE;
                    DH->Response.Description = Info[i].error;

                    this->m_pChOpObjects[j]->ProcessInputVecMutex.lock();
                    this->m_pChOpObjects[j]->ProcessInputVec.push_back(DH);
                    this->m_pChOpObjects[j]->ProcessInputVecMutex.unlock();
                    
                continue;
                } //TODO VALIDATION


                DH->ProcessedInputData.PlateImageMat = createMatImage(DH->Input.PlateImageBase64,std::to_string(DH->Input.PassedVehicleRecordsId)); 

                // 1- Validation Input data
                if(DH->ProcessedInputData.PlateImageMat.total() == 0)
                {
                    DH->Response.HTTPCode = 400;
                    DH->Response.errorCode = INVALIDPLATEIMAGE;
                    DH->Response.Description = "The Value of PlateImage Is Invalid.";
                    
                    // continue;
                }

                this->m_pChOpObjects[j]->ProcessInputVecMutex.lock();
                this->m_pChOpObjects[j]->ProcessInputVec.push_back(DH);
                this->m_pChOpObjects[j]->ProcessInputVecMutex.unlock();


                j++;
               
            }//for info

            if(this->MinId >= this->MaxId)
            {
                this->MinId = this->MaxId;
                break;
            }

            // cont = true;//TODO 
            // while(cont)
            // {
            //     cont = false;
            //     for(int idx=0; idx<this->CheckOpNumberOfObjectPerService; idx++)
            //     {
            //         if(this->m_pChOpObjects[idx]->ProcessInputVec.size() > 50)
            //         {
            //             cont = true;
            //             break;
            //         }
            //     }
            //     boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
            // }

            // boost::this_thread::sleep_for(boost::chrono::milliseconds(100));

        }
    }
    
    curl_global_cleanup();

    // return 0; //TODO ERROR HANDLING FOR VIOD FUNC
}

RahdariService::getImagesResultStruct RahdariService::parsePlateImages(const std::string& jsonString, uint64_t MinId) {

    getImagesResultStruct result; // Initialize the result struct

    rapidjson::Document document;
    if (document.Parse(jsonString.c_str()).HasParseError()) {
        result.Error = "JSON parse error: " + std::string(rapidjson::GetParseError_En(document.GetParseError()));
        return result; // Return early on error
    }

    // if (document.HasMember("Error")) {
    //     result.Error = document["Error"].GetString();
    //     return result; // Return early on error presence //TODO
    // }

    if (!document.IsArray()) {
        Logger::getInstance().logError("The record received is not in the correct format !!! ["+ std::to_string(MinId)+"]");
        result.Error = "Invalid JSON structure";
        return result; // Return early if top-level JSON is not an array
    }

        // Log if the array is empty
    if (document.Empty()) {
        Logger::getInstance().logError("The document array is empty. Skipping record... [" + std::to_string(MinId) + "]");
        result.Error = "Empty document array";
        return result; // Return early if array is empty
    }

    // Reserve space in the map if you have an idea of the number of records
    result.PlateImages_map.reserve(document.Size());

    for (const auto& item : document.GetArray()) {
        // Validate that each element is an json object
        if (!item.IsObject()) {
            Logger::getInstance().logWarning("Invalid record format: Expected an json object. Skipping record...["+ std::to_string(MinId)+"]");
            continue; // Skip non-object elements
        }

        // Validate the presence and types of required fields
        if (!item.HasMember("tid") ||!item.HasMember("plate_image") ){ //|| !item["plate_image"].IsString()) {
            
            Logger::getInstance().logWarning("Record missing required fields or invalid field types. Skipping record...["+ std::to_string(MinId)+"]");
            continue; // Skip records with missing or invalid fields
        }

        uint64_t tid = item["tid"].GetUint64();
        std::string plate_image;

        if(item["plate_image"].IsString()){ 

            plate_image = item["plate_image"].GetString();
            plate_image = "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";

// Efficiently remove newline characters
            plate_image.erase(std::remove(plate_image.begin(), plate_image.end(), '\n'), plate_image.end());

            if(this->CurlServiceConfig.MonitorMode){
                mtx_ValidImage.lock();
                validImagesCount++;
                mtx_ValidImage.unlock();
            }
        

        }else if(item["plate_image"].IsNull()){
            // result.Error = "Image Is NULL";
            plate_image = "null";
            if(this->CurlServiceConfig.DebugMode)
                Logger::getInstance().logWarning("Image Is NULL With tid : " +std::to_string(tid)); //TODO RETURN CODE TYPE;
        }

        result.PlateImages_map[tid] = std::move(plate_image); // Move instead of copy


        }
        
    return result; // Return the result struct
}

std::unordered_map< uint64_t, std::string> RahdariService::getImageBase64_bulk(const std::string& Url, uint64_t MinId, uint64_t MaxId)
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

        // Start timing the request
        auto startTimeRequest = std::chrono::high_resolution_clock::now();

        CURLcode res = curl_easy_perform(curl);

        // End timing the request
        auto endTimeRequest = std::chrono::high_resolution_clock::now();
        double duration_Image = std::chrono::duration_cast<std::chrono::milliseconds>(endTimeRequest - startTimeRequest).count();

        curl_slist_free_all(HList);
        curl_easy_cleanup(curl);
        if(res != CURLE_OK)
        {
            std::cerr << "curl_easy_perform() failed for URL: " << requestUrl << " Error: " << curl_easy_strerror(res) << std::endl;
            std::cerr << "Request time: " << duration_Image << " ms" << std::endl;
 
            return {}; // Return empty map on failure

        }
    
        // Start timing the JSON parsing
        auto parseStartTime = std::chrono::high_resolution_clock::now();

        resVal =  parsePlateImages(response, MinId); // Handle parsing errors within this function

        // End timing the JSON parsing
        auto parseEndTime = std::chrono::high_resolution_clock::now();
        double parseDuration = std::chrono::duration_cast<std::chrono::milliseconds>(parseEndTime - parseStartTime).count();
        if(this->CurlServiceConfig.MonitorMode)
        {
            mtx_Image.lock();
            totalImageFetchingTime += duration_Image; 
            ImagefetchingCount++;
            // Logger.getInstance().logInfo("Images Request time: " + duration_Image + " ms");
            // Logger.getInstance().logInfo("Images JSON parsing time: " + parseDuration + " ms");
            mtx_Image.unlock();

        }

        if(resVal.Error.empty())
        {
            auto result = resVal.PlateImages_map; // Store result to return it
            return result; 
        }
        else{
            std::cerr<<"Could not get Image: "<<resVal.Error<<std::endl;
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

    // Start timing the request
    auto startTimeRequest = std::chrono::high_resolution_clock::now();

    CURLcode res = curl_easy_perform(curl);

    // End timing the request
    auto endTimeRequest = std::chrono::high_resolution_clock::now();
    double duration_Info = std::chrono::duration_cast<std::chrono::milliseconds>(endTimeRequest - startTimeRequest).count();
    curl_slist_free_all(HList);
    curl_easy_cleanup(curl);
    curl_count++;
    if(res != CURLE_OK)
    {
        SHOW_IMPORTANTLOG("curl_count: " <<curl_count);
        Logger::getInstance().logError("curl_easy_perform() failed for URL : " + requestUrl + " Error: "+ curl_easy_strerror(res));

        return Info; // Return empty vector on failure
    }
    auto parseStartTime = std::chrono::high_resolution_clock::now();

    ResponseValue ResVal = getResponseValue(response, "getttoinfo");

        // End timing the JSON parsing
    auto parseEndTime = std::chrono::high_resolution_clock::now();
    double parseDuration = std::chrono::duration_cast<std::chrono::milliseconds>(parseEndTime - parseStartTime).count();

    if(ResVal.TTOInfoResult.size() != 0)
        Info.swap(ResVal.TTOInfoResult);
    else
        std::cerr<<"Could not get TTOInfo "<<ResVal.Error<<std::endl;

    if(this->CurlServiceConfig.MonitorMode)
    {
        mtx_Info.lock();
        totalInfoFetchingTime += duration_Info; // Use regular addition
        InfofetchingCount++;
        // SHOW_LOG("Info Request time: " << duration_Info << " ms");
        // SHOW_LOG("Info JSON parsing time: " << parseDuration << " ms");
        mtx_Info.unlock();
        
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
        return resVal; // Return empty ResponseValue on error
    }

    // if (document.HasMember("Error")) {
    //     resVal.Error = document["Error"].GetString();//TODO
    // }

    if (ResType == "maxid") {
        if (document.HasMember("maxid")) {
            resVal.MaxIdResult = document["maxid"].GetInt64();
        }
    } else if (ResType == "getttoinfo") {
        if (document.HasMember("res") && document["res"].IsArray()) {
            const auto& resArray = document["res"];
            for (const auto& item : resArray.GetArray()) {
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
                info.VehiclePlate = item.HasMember("PLATE_CODE") && !item["PLATE_CODE"].IsNull() ? std::to_string(item["PLATE_CODE"].GetUint64()) : "00000000";

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
    } else if (ResType == "getImage") {
        resVal.ImageResult.TTOInfoId = std::to_string(document["tid"].GetInt());
        std::string tmpplate = document["plate_image"].GetString();
        tmpplate.erase(std::remove(tmpplate.begin(), tmpplate.end(), '\n'), tmpplate.end());
        resVal.ImageResult.PlateImageBase64 = tmpplate;
    }

    return resVal;

}
