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

        for (auto& info : Info)
        {
            auto it = Images_map.find(info.TTOInfoId);
            if(it != Images_map.end()){

                info.PlateImageBase64 = it->second;
            }
            else{
                info.error = "There Is No Image For This RecordId : "+std::to_string(info.TTOInfoId);
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

            // ////////////////////////TEST???????????????//
            // TTOInfo info;
            // info.Allowed            = "";
            // info.CompanyID          = "200";
            // info.DeviceId           = "6226"  ;
            // info.LineNumber         = "1";
            // info.PassDateTime       = "2024-10-21T11:06:39";
            // info.ReceiveDateTime    = "";
            // info.RegisterDateTime   = "";
            // info.SystemD            = "";
            // info.TTOInfoId          = 18693441477    ;
            // info.VehicleClass       = "";
            // info.VehiclePlate       = "441536859";
            // info.VehicleSpeed       = "70";
            // info.WrongDirection     = "";
            // info.HasColorImage      = "";
            // info.HasPlateImage      = "";
            // info.HasCrime           = "";
            // info.CrimeCode          = "";
            // info.PoliceCode         = "";
            // info.PlateImageBase64   = "";
            // info.error              = "";
            // std::ifstream file("/home/amiri/data/Image.txt"); // Open the file
            // std::stringstream buffer; // Create a stringstream to hold the content

            // if (file) { // Check if the file opened successfully
            //     buffer << file.rdbuf(); // Read the file content into the stringstream
            //     info.PlateImageBase64 = buffer.str(); // Convert stringstream to 
            // }
            // info.PlateImageBase64.erase(std::remove(info.PlateImageBase64.begin(), info.PlateImageBase64.end(), '\n'), info.PlateImageBase64.end());

            // Info.push_back(info);


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

                //getImages
                if(Info[i].error != "")
                {
                    DH->Response.HTTPCode = 400;
                    DH->Response.errorCode = CANNOTFINDIMAGE;
                    DH->Response.Description = Info[i].error;

                }

                DH->ProcessedInputData.PlateImageMat = createMatImage(DH->Input.PlateImageBase64,std::to_string(DH->Input.PassedVehicleRecordsId)); 

                // 1- Validation Input data
                if(DH->ProcessedInputData.PlateImageMat.total() == 0)
                {
                    DH->Response.HTTPCode = 400;
                    DH->Response.errorCode = INVALIDPLATEIMAGE;
                    DH->Response.Description = "The Value of PlateImage Is Invalid.";
                }

                if(DH->Input.PlateValue == "000000000")
                {
                    DH->Response.HTTPCode = 400;
                    DH->Response.errorCode = INVALIDMASTERPLATE;
                    DH->Response.Description = "The Value of MasterPlate Is Invalid.";
                }

                this->m_pChOpObjects[j]->ProcessInputVecMutex.lock();
                this->m_pChOpObjects[j]->ProcessInputVec.push_back(DH);
                this->m_pChOpObjects[j]->ProcessInputVecMutex.unlock();
                // this->m_pChOpObjects[j]->preprocess();


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

RahdariService::getImagesResultStruct RahdariService::parsePlateImages(const std::string& jsonString) {

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

    if (document.IsArray()) {
        // Reserve space in the map if you have an idea of the number of records
        result.PlateImages_map.reserve(document.Size());

        for (const auto& item : document.GetArray()) {
            if (item.HasMember("tid") && item["tid"].IsUint64()) {
                uint64_t tid = item["tid"].GetUint64();
                std::string plate_image;

                if (item.HasMember("plate_image")){ 
                    if(item["plate_image"].IsString()){
                        plate_image = item["plate_image"].GetString();
                        if(this->CurlServiceConfig.MonitorMode){
                            mtx_ValidImage.lock();
                            validImagesCount++;
                            mtx_ValidImage.unlock();
                        }

                    }else if(item["plate_image"].IsString() && item["plate_image"].GetString() == "The Value of PlateImage Is Invalid." )
                    {
                        result.Error = "Image Not Available";
                        plate_image = "null";
                        if(this->CurlServiceConfig.DebugMode)
                            Logger::getInstance().logWarning(result.Error); //TODO RETURN CODE TYPE;
                    }

                    else if(item["plate_image"].IsNull()){
                        result.Error = "Image Is NULL";
                        plate_image = "null";
                        if(this->CurlServiceConfig.DebugMode)
                            Logger::getInstance().logWarning(result.Error); //TODO RETURN CODE TYPE;
                    }   
                    // Efficiently remove newline characters
                    plate_image.erase(std::remove(plate_image.begin(), plate_image.end(), '\n'), plate_image.end());
                }

                result.PlateImages_map[tid] = std::move(plate_image); // Move instead of copy
            }
        }
    }

    return result; // Return the result struct
}

std::unordered_map< uint64_t, std::string> RahdariService::getImageBase64_bulk(const std::string& Url, uint64_t MinId, uint64_t MaxId)
{
    //    ImageInfo Img;
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

        resVal =  parsePlateImages(response); // Handle parsing errors within this function

        // End timing the JSON parsing
        auto parseEndTime = std::chrono::high_resolution_clock::now();
        double parseDuration = std::chrono::duration_cast<std::chrono::milliseconds>(parseEndTime - parseStartTime).count();
        if(this->CurlServiceConfig.MonitorMode)
        {
            mtx_Image.lock();
            totalImageFetchingTime += duration_Image; // Use regular addition
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
