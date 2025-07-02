#include "webservice.h"

// Assuming these macros exist for logging
#define SHOW_LOG(msg) std::cout << msg << std::endl
#define SHOW_ERROR(msg) std::cerr << msg << std::endl
#define SHOW_IMPORTANTLOG(msg) std::cout << msg << std::endl
#define SHOW_IMPORTANTLOG3(msg) std::cout << msg << std::endl

// Assuming these constants exist
// enum StatusCode { SUCCESSFUL, EXPIREDTOKEN, INVALIDTOKEN, DUPLICATERECORD, DATABASEERROR, CHECKOPERROR, CLASSIFIERERROR };

// WebService::ThreadPool::ThreadPool(size_t numThreads) {
//     for (size_t i = 0; i < numThreads; ++i) {
//         workers.emplace_back([this] {
//             while (true) {
//                 std::function<void()> task;
//                 {
//                     std::unique_lock<std::mutex> lock(queueMutex);
//                     condition.wait(lock, [this] { return stopFlag || !tasks.empty(); });
//                     if (stopFlag && tasks.empty()) return;
//                     task = std::move(tasks.front());
//                     tasks.pop();
//                 }
//                 task();
//             }
//         });
//     }
// }

// WebService::ThreadPool::~ThreadPool() {
//     stop();
// }

// void WebService::ThreadPool::enqueue(std::function<void()> task) {
//     {
//         std::unique_lock<std::mutex> lock(queueMutex);
//         tasks.push(std::move(task));
//     }
//     condition.notify_one();
// }

// void WebService::ThreadPool::stop() {
//     {
//         std::unique_lock<std::mutex> lock(queueMutex);
//         stopFlag = true;
//     }
//     condition.notify_all();
//     for (std::thread& worker : workers) {
//         if (worker.joinable()) worker.join();
//     }
// }

std::string rectToString(const cv::Rect& rect) {
    return std::to_string(rect.x) + "," + 
           std::to_string(rect.y) + "," + 
           std::to_string(rect.width) + "," + 
           std::to_string(rect.height);
}



// std::shared_ptr<DataHandler::DataHandlerStruct> createClassifierSaveData(const std::shared_ptr<DataHandler::DataHandlerStruct>& dh, const Classifier::OutputStruct& classifierItem)
// {
//     auto newCarREct = dh->hasOutputFields.CarRect;
//     dh->hasOutputFields.ColorImage;
//     dh->hasOutputFields.CompanyCode;
//     dh->hasOutputFields.DeviceID;
//     dh->hasOutputFields.
//     dh->hasOutputFields
//     dh->hasOutputFields
    
//     auto newDH = std::make_shared<DataHandler::DataHandlerStruct>(*dh);
    
//     newDH->ProcessedInputData.VehicleModel = classifierItem.vehicleModel;
//     newDH->ProcessedInputData.CarRect = rectToString(classifierItem.box);
    
//     return newDH;
// }

WebService::WebService(Configurate::WebServiceConfigStruct ServiceConfig)
{
    std::cout<<"Debug"<<std::endl;
    this->app = std::make_shared<crow::SimpleApp>();
    this->app->loglevel(crow::LogLevel::Error);
    this->WebServiceConfig = ServiceConfig;
    // threadPool = std::make_unique<ThreadPool>(1);
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

void WebService::run()
{
    try 
    {   
        this->InsertRoute();
        if(this->WebServiceConfig.Authentication)
            this->TokenRoute();

        SHOW_IMPORTANTLOG3("Runinng Service on port " + std::to_string(this->WebServiceConfig.WebServiceInfo.Port));
        try{
            //! run object of crow by specific port and many rout in multithread status
            this->app->port(this->WebServiceConfig.WebServiceInfo.Port).concurrency(this->WebServiceConfig.threadNumber).run();

        }  catch (...) {
            SHOW_ERROR("port " + std::to_string(this->WebServiceConfig.WebServiceInfo.Port) + " is busy . Check ports in server table in config database .");
            exit(0);
        }

    }
    catch(boost::exception_detail::clone_impl<boost::exception_detail::error_info_injector<boost::property_tree::ptree_bad_path> > &e)
    {

        SHOW_ERROR("Error Code 0x" + std::to_string(__LINE__) + "JBF_3K92XS543272" + e.what());

        SHOW_ERROR("Can't Run Crow on port "  + std::to_string(this->WebServiceConfig.WebServiceInfo.Port) + " please check this port ." );

    }
}


void WebService::InsertRoute() {
    std::string Route = this->WebServiceConfig.WebServiceInfo.URI;
    if (Route.back() != '/') Route += "/";
    if (Route[0] != '/') Route = "/" + Route;
    Route += "insert";

    this->app->route_dynamic("/insert").methods(crow::HTTPMethod::POST)([&](const crow::request& req) {
        auto requstStartTime = std::chrono::high_resolution_clock::now();
        
        // Initialize DataHandler
        auto DH = std::make_shared<DataHandler::DataHandlerStruct>();
        // Configurate* ConfigurateObj = Configurate::getInstance();
        // DH->hasInputFields = ConfigurateObj->getInputFields();
        // DH->hasOutputFields = ConfigurateObj->getOutputFields();
        // DH->StoreImageConfig = ConfigurateObj->getStoreImageConfig();
        // DH->ViolationMap = ConfigurateObj->getViolationMap();
        // DH->Cameras = ConfigurateObj->getCameras();
        // DH->DaysforPassedTimeAcceptable = this->WebServiceConfig.DaysforPassedTimeAcceptable;
        // DH->InsertDatabase = ConfigurateObj->getInsertDatabase();
        // DH->InsertDatabaseInfo = ConfigurateObj->getInsertDatabaseInfo();
        // DH->FailedDatabase = ConfigurateObj->getFailedDatabase();
        // DH->FailedDatabaseInfo = ConfigurateObj->getFailedDatabaseInfo();
        // DH->Modules = ConfigurateObj->getModules();

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
        DH->ConfigDatabase = m_ConfigDatabase; //
        DH->ConfigDatabaseInfo = m_ConfigDatabaseInfo; //
            // --- پایان تغییرات برای بهینه‌سازی بارگذاری پیکربندی ---
        DH->DebugMode = this->WebServiceConfig.DebugMode;
        DH->InsertRoute = true;
        DH->WebServiceAuthentication = this->WebServiceConfig.Authentication;

        crow::json::wvalue Response;
        crow::json::wvalue ClientResponse;
        DH->Request.remoteIP = req.ipAddress;
        SHOW_IMPORTANTLOG("Received Insert request from IP -> " + DH->Request.remoteIP);

        // Authentication
        auto AuthenticationStartTime = std::chrono::high_resolution_clock::now();
        if (this->WebServiceConfig.Authentication) {
            DH->DecryptedData = false;
            DH->Request.body = req.body;
            if (!(this->m_Validatorobj->run(DH))) {
                Response["Status"] = DH->Response.errorCode;
                Response["Description"] = DH->Response.Description;
                SHOW_ERROR(crow::json::dump(Response));
                return crow::response{DH->Response.HTTPCode, Response};
            }

            bool TokenFind = false;
            std::string Token = DH->Request.enJsonRvalue["Token"].s();
            for (int i = 0; i < DH->Cameras.size(); i++) {
                if (DH->Cameras[i].TokenInfo.Token == Token) {
                    TokenFind = true;
                    DH->CameraIndex = i;
                    std::time_t currentTime = std::time(nullptr);
                    std::time_t TokenExpirationTime = DH->Cameras[i].TokenInfo.CreatedAt + this->WebServiceConfig.TokenTimeAllowed;
                    if (currentTime > TokenExpirationTime) {
                        Response["Status"] = EXPIREDTOKEN;
                        Response["Description"] = "Expired Token.";
                        SHOW_ERROR(crow::json::dump(Response));
                        return crow::response{401, Response};
                    }
                    break;
                }
            }

            if (!TokenFind) {
                Response["Status"] = INVALIDTOKEN;
                Response["Description"] = "Invalid Token.";
                SHOW_ERROR(crow::json::dump(Response));
                return crow::response{401, Response};
            }

            std::string EncryptedData = DH->Request.enJsonRvalue["Data"].s();
            std::string ClientPublicKeyAddress = this->WebServiceConfig.KeysPath + "/" + DH->Cameras[DH->CameraIndex].CompanyID;
            std::string ServerPrivateKeyAddress = this->WebServiceConfig.KeysPath + "/ServerPri.pem";
            std::string DecryptedData = decryptString(EncryptedData, ServerPrivateKeyAddress, ClientPublicKeyAddress).DecryptedMessage;
            DH->Request.body = DecryptedData;
        } else {
            DH->Request.body = req.body;
        }
        auto AuthenticationFinishTime = std::chrono::high_resolution_clock::now();
        auto AuthenticationTime = std::chrono::duration_cast<std::chrono::nanoseconds>(AuthenticationFinishTime - AuthenticationStartTime);

        DH->DecryptedData = true;
        // Validation
        auto validationStartTime = std::chrono::high_resolution_clock::now();
        if (!(this->m_Validatorobj->run(DH))) {
            // Response["Status"] = DH->Response.errorCode;
            // Response["Description"] = DH->Response.Description;
            // if (DH->hasInputFields.DeviceID && DH->hasInputFields.PlateValue) {
            //     Response["DevicedID"] = DH->Input.DeviceID;
            //     Response["PlateValue"] = DH->Input.PlateValue;
            // }
            // Response["IP"] = DH->Request.remoteIP;
            // if (DH->FailedDatabaseInfo.Enable) {
            //     std::vector<MongoDB::Field> fields = {
            //         {"Status", std::to_string(DH->Response.errorCode), MongoDB::FieldType::Integer},
            //         {"Description", DH->Response.Description, MongoDB::FieldType::String},
            //         {"IP", DH->Request.remoteIP, MongoDB::FieldType::String}
            //     };
            //     if (DH->hasInputFields.DeviceID && DH->hasInputFields.PlateValue) {
            //         fields.push_back({"DeviceID", std::to_string(DH->Input.DeviceID), MongoDB::FieldType::Integer});
            //         fields.push_back({"PlateValue", DH->Input.PlateValue, MongoDB::FieldType::String});
            //     }
            //     threadPool->enqueue([DH, fields]() {
            //         DH->FailedDatabase->Insert(DH->FailedDatabaseInfo.DatabaseName, DH->FailedDatabaseInfo.CollectionName, fields);
            //     });
            // }
            // SHOW_ERROR(crow::json::dump(Response));
            // ClientResponse["Status"] = DH->Response.errorCode;
            // ClientResponse["Description"] = DH->Response.Description;
            // return crow::response{DH->Response.HTTPCode, ClientResponse};
        }
        auto validationFinishTime = std::chrono::high_resolution_clock::now();
        auto ValidationTime = std::chrono::duration_cast<std::chrono::nanoseconds>(validationFinishTime - validationStartTime);

        // Check Operator Module
        auto CheckOpStartTime = std::chrono::high_resolution_clock::now();
        if (DH->Modules.CheckOperator.active && DH->hasInputFields.PlateImage) {
            ChOp::InputStruct inputChOp;
            inputChOp.plateImage = DH->ProcessedInputData.PlateImageMat;
            inputChOp.plateValue = DH->hasInputFields.PlateValue ? DH->Input.PlateValue : "";
            int CheckOpObjectIndex = this->getCheckOpIndex();
            ChOp::OutputStruct ChOpOutput;
            try {
                ChOpOutput = this->m_pChOpObjects[CheckOpObjectIndex]->run(inputChOp);
                this->releaseCheckOpIndex(CheckOpObjectIndex);
                DH->Input.MasterPlate = DH->Input.PlateValue;
                DH->Input.PlateValue = ChOpOutput.newPlateValue;
                DH->Input.CodeType = ChOpOutput.codeType;
                DH->Input.Probability = ChOpOutput.probability;
                DH->Input.PlateType = ChOpOutput.newPlateType;
            } catch (const std::exception& e) {
                Response["Status"] = CHECKOPERROR;
                Response["Description"] = e.what();
                if (DH->hasInputFields.DeviceID && DH->hasInputFields.ViolationID && DH->hasInputFields.PassedTime && DH->hasInputFields.PlateValue)
                    Response["RecordID"] = DH->ProcessedInputData.MongoID;
                Response["IP"] = DH->Request.remoteIP;
                if (DH->FailedDatabaseInfo.Enable) {
                    std::vector<MongoDB::Field> fields = {
                        {"Status", std::to_string(CHECKOPERROR), MongoDB::FieldType::Integer},
                        {"Description", e.what(), MongoDB::FieldType::String},
                        {"IP", DH->Request.remoteIP, MongoDB::FieldType::String}
                    };
                    if (DH->hasInputFields.DeviceID && DH->hasInputFields.ViolationID && DH->hasInputFields.PassedTime && DH->hasInputFields.PlateValue) {
                        fields.push_back({"RecordID", DH->ProcessedInputData.MongoID, MongoDB::FieldType::ObjectId});
                    }
                    threadPool->enqueue([DH, fields]() {
                        DH->FailedDatabase->Insert(DH->FailedDatabaseInfo.DatabaseName, DH->FailedDatabaseInfo.CollectionName, fields);
                    });
                }
                if (DH->DebugMode)
                    SHOW_ERROR(crow::json::dump(Response));
                this->releaseCheckOpIndex(CheckOpObjectIndex);
                ClientResponse["Status"] = CHECKOPERROR;
                ClientResponse["Description"] = "Internal Service Error.";
                return crow::response{DH->Response.HTTPCode, ClientResponse};
            }
        }
        auto CheckOpFinishTime = std::chrono::high_resolution_clock::now();
        auto CheckOpTime = std::chrono::duration_cast<std::chrono::nanoseconds>(CheckOpFinishTime - CheckOpStartTime);


        // Classifier Module
        std::vector<Classifier::OutputStruct> ClassifierOutputVec;
        if (DH->Modules.Classifier.active) {
            Classifier::InputStruct inputClassify;
            inputClassify.Image = DH->ProcessedInputData.ColorImageMat.clone();
            int ClassifierObjectIndex = this->getClassifierIndex();
            try {
                auto ClassifierStartTime = std::chrono::high_resolution_clock::now();

                ClassifierOutputVec = this->m_pClassifierObjects[ClassifierObjectIndex]->run(inputClassify);

                auto ClassifierFinishTime = std::chrono::high_resolution_clock::now();
                auto ClassifierTime = std::chrono::duration_cast<std::chrono::nanoseconds>(ClassifierFinishTime - ClassifierStartTime);

                this->releaseClassifierIndex(ClassifierObjectIndex);

                std::vector<crow::json::wvalue> classificationsJson;
                for (auto& output : ClassifierOutputVec) {
                    DH->ProcessedInputData.VehicleModel = output.vehicleModel;

                    auto storeImageStartTime = std::chrono::high_resolution_clock::now();
#ifdef STOREIMAGE
        // 5- Store Image
        std::shared_ptr<storeimage> storeimageobj = std::make_shared<storeimage>();
        if(!(storeimageobj->run(DH)))
        {
            Response["Status"] = DH->Response.errorCode;
            Response["Description"] = DH->Response.Description;
            if(DH->hasInputFields.DeviceID && DH->hasInputFields.ViolationID && DH->hasInputFields.PassedTime && DH->hasInputFields.PlateValue)
                Response["RecordID"] = DH->ProcessedInputData.MongoID;
            Response["IP"] = DH->Request.remoteIP;
            if(DH->FailedDatabaseInfo.Enable)
            {
                std::vector<MongoDB::Field> fields = {
                    {"Status", std::to_string(DH->Response.errorCode), MongoDB::FieldType::Integer},
                    {"Description", DH->Response.Description, MongoDB::FieldType::String},
                    {"IP", DH->Request.remoteIP, MongoDB::FieldType::String}
                };

                if(DH->hasInputFields.DeviceID && DH->hasInputFields.ViolationID && DH->hasInputFields.PassedTime && DH->hasInputFields.PlateValue)
                {
                    MongoDB::Field RecordIDField = {"RecordID", DH->ProcessedInputData.MongoID, MongoDB::FieldType::ObjectId};
                    fields.push_back(RecordIDField);
                }

                DH->FailedDatabase ->Insert(DH->FailedDatabaseInfo.DatabaseName, DH->FailedDatabaseInfo.CollectionName, fields);
            }
            SHOW_ERROR(crow::json::dump(Response));
            ClientResponse["Status"] = DH->Response.errorCode;
            ClientResponse["Description"] = DH->Response.Description;
            return crow::response{DH->Response.HTTPCode , ClientResponse};
        }
#endif // STOREIMAGE
        auto storeImageFinishTime = std::chrono::high_resolution_clock::now();
        auto storeImaheTime =  std::chrono::duration_cast<std::chrono::nanoseconds>(storeImageFinishTime - storeImageStartTime);        

        auto saveDataStartTime = std::chrono::high_resolution_clock::now();
#if defined KAFKAOUTPUT || defined INSERTDATABASE
        // 6- Save Data
        std::shared_ptr<savedata> savedataobj = std::make_shared<savedata>();
#ifdef KAFKAOUTPUT
        int OutputKafkaConnectionIndex = this->getKafkaConnectionIndex();
        savedataobj->setOutputKafkaConnection(this->OutputKafkaConnections[OutputKafkaConnectionIndex]);
#endif // KAFKAOUTPUT
        
        if(!(savedataobj->run(DH)))
        {
            Response["Status"] = DH->Response.errorCode;
            Response["Description"] = DH->Response.Description;
            if(DH->hasInputFields.DeviceID && DH->hasInputFields.ViolationID && DH->hasInputFields.PassedTime && DH->hasInputFields.PlateValue)
                Response["RecordID"] = DH->ProcessedInputData.MongoID;
            Response["IP"] = DH->Request.remoteIP;
            if(DH->FailedDatabaseInfo.Enable)
            {
                std::vector<MongoDB::Field> fields = {
                    {"Status", std::to_string(DH->Response.errorCode), MongoDB::FieldType::Integer},
                    {"Description", DH->Response.Description, MongoDB::FieldType::String},
                    {"IP", DH->Request.remoteIP, MongoDB::FieldType::String}
                };

                if(DH->hasInputFields.DeviceID && DH->hasInputFields.ViolationID && DH->hasInputFields.PassedTime && DH->hasInputFields.PlateValue)
                {
                    MongoDB::Field RecordIDField = {"RecordID", DH->ProcessedInputData.MongoID, MongoDB::FieldType::ObjectId};
                    fields.push_back(RecordIDField);
                }

                DH->FailedDatabase ->Insert(DH->FailedDatabaseInfo.DatabaseName, DH->FailedDatabaseInfo.CollectionName, fields);
            }
            SHOW_ERROR(crow::json::dump(Response));
            ClientResponse["Status"] = DH->Response.errorCode;
            ClientResponse["Description"] = DH->Response.Description;
            return crow::response{DH->Response.HTTPCode , ClientResponse};
        }
#ifdef KAFKAOUTPUT
        this->releaseKafkaIndex(OutputKafkaConnectionIndex);
#endif // KAFKAOUTPUT
#endif // KAFKAOUTPUT || INSERTDATABASE
        auto saveDataFinishTime = std::chrono::high_resolution_clock::now();
        auto saveDataTime =  std::chrono::duration_cast<std::chrono::nanoseconds>(saveDataFinishTime - saveDataStartTime);     

        auto requestFinishTime = std::chrono::high_resolution_clock::now();
        auto requestTime =  std::chrono::duration_cast<std::chrono::nanoseconds>(requestFinishTime - requstStartTime);
                         //TODO


                    
                crow::json::wvalue CResponse;
                CResponse["VehiclModel"] = output.vehicleModel;
                crow::json::wvalue rectJson;
                rectJson["x"] = output.box.x;
                rectJson["y"] = output.box.y;
                rectJson["width"] = output.box.width;
                rectJson["height"] = output.box.height;
                CResponse["CarRect"] = std::move(rectJson);
                classificationsJson.push_back(std::move(CResponse));
                }
                crow::json::wvalue j;
                j["EmsInfoId"] = DH->Input.RecordID;
                j["numDetections"] = ClassifierOutputVec.size();
                j["classification"] = std::move(classificationsJson);
                std::string test = crow::json::dump(j);
                std::cout << test << std::endl;
            } catch (const std::exception& e) {
                std::cerr << e.what() << '\n';
                this->releaseClassifierIndex(ClassifierObjectIndex);
            }
        }



        // if (DH->DebugMode)
        //     SHOW_IMPORTANTLOG3("ProccessTime(ns) = " << std::to_string(requestTime.count()) << std::endl
        //                       << "0- Authentication ProccessTime(ns) = " << std::to_string(AuthenticationTime.count()) << std::endl
        //                       << "1- Validation ProccessTime(ns) = " << std::to_string(ValidationTime.count()) << std::endl
        //                       << "2- Check RecordID ProccessTime(ns) = " << std::to_string(ChecRecordIDTime.count()) << std::endl
        //                       << "3- CheckOp ProccessTime(ns) = " << std::to_string(CheckOpTime.count()) << std::endl
        //                       << "4- Classifier ProccessTime(ns) = " << std::to_string(ClassifierTime.count()) << std::endl
        //                       << "5- Store image ProccessTime(ns) = " << std::to_string(storeImaheTime.count()) << std::endl
        //                       << "6- Save data ProccessTime(ns) = " << std::to_string(saveDataTime.count()));

        Response["Status"] = SUCCESSFUL;
        if (DH->hasInputFields.DeviceID && DH->hasInputFields.ViolationID && DH->hasInputFields.PassedTime && DH->hasInputFields.PlateValue)
            Response["RecordID"] = DH->ProcessedInputData.MongoID;
        SHOW_LOG(crow::json::dump(Response));
        return crow::response{200, Response};
    });
}























// void WebService::InsertRoute()
// {
//     std::string Route = this->WebServiceConfig.WebServiceInfo.URI;
//     if(Route.back() != '/')
//         Route += "/";
//     if(Route[0] != '/')
//         Route = "/" + Route;
//     Route += "insert";
    
//     this->app->route_dynamic("/insert").methods(crow::HTTPMethod::POST)([&](const crow::request& req ) {
        
//         //! "startTime" for Computing process time for this request
//         auto requstStartTime = std::chrono::high_resolution_clock::now();
        
//         std::shared_ptr<DataHandler::DataHandlerStruct> DH = std::make_shared<DataHandler::DataHandlerStruct>();
//         Configurate* ConfigurateObj = Configurate::getInstance();
//         DH->hasInputFields = ConfigurateObj->getInputFields();
//         DH->hasOutputFields = ConfigurateObj->getOutputFields();
//         DH->StoreImageConfig = ConfigurateObj->getStoreImageConfig();
//         DH->ViolationMap = ConfigurateObj->getViolationMap();
//         DH->Cameras = ConfigurateObj->getCameras();
//         DH->DaysforPassedTimeAcceptable = this->WebServiceConfig.DaysforPassedTimeAcceptable;
//         DH->InsertDatabase = ConfigurateObj->getInsertDatabase();
//         DH->InsertDatabaseInfo = ConfigurateObj->getInsertDatabaseInfo();
//         DH->FailedDatabase = ConfigurateObj->getFailedDatabase();
//         DH->FailedDatabaseInfo = ConfigurateObj->getFailedDatabaseInfo();
//         DH->Modules = ConfigurateObj->getModules();
//         DH->DebugMode = this->WebServiceConfig.DebugMode;
//         DH->InsertRoute = true;
//         DH->WebServiceAuthentication = this->WebServiceConfig.Authentication;
        
//         crow::json::wvalue Response;
//         crow::json::wvalue ClientResponse;

//         DH->Request.remoteIP = req.ipAddress;
//         SHOW_IMPORTANTLOG("Recived Insert request from IP -> " + DH->Request.remoteIP);

//         // std::shared_ptr<Validator> Validatorobj = std::make_shared<Validator>("/home/amiri/projects/c++/loadDetection/webserviceLoad/field_mappings.json");
//         // 0- Check Authentication
//         auto AuthenticationStartTime = std::chrono::high_resolution_clock::now();
//         if(this->WebServiceConfig.Authentication)
//         {
//             DH->DecryptedData = false;
//             DH->Request.body = req.body;
//             // Validation Input data
//             if(!(this->m_Validatorobj->run(DH)))
//             {
//                 Response["Status"] = DH->Response.errorCode;
//                 Response["Description"] = DH->Response.Description;
//                 SHOW_ERROR(crow::json::dump(Response));
//                 return crow::response{DH->Response.HTTPCode , Response};
//             }
            
//             // Check Token
//             bool TokenFind = false;
//             std::string Token = DH->Request.enJsonRvalue["Token"].s();
//             for(int i = 0; i < DH->Cameras.size(); i++)
//             {
//                 if(DH->Cameras[i].TokenInfo.Token == Token)
//                 {
//                     TokenFind = true;
//                     DH->CameraIndex = i;
//                     std::time_t currentTime = std::time(nullptr);
//                     std::time_t TokenExpirationTime = DH->Cameras[i].TokenInfo.CreatedAt + this->WebServiceConfig.TokenTimeAllowed;
//                     if(currentTime > TokenExpirationTime)
//                     {
//                         Response["Status"] = EXPIREDTOKEN;
//                         Response["Description"] = "Expired Token.";
//                         SHOW_ERROR(crow::json::dump(Response));
//                         return crow::response{401, Response};   
//                     }
//                     break;
//                 }
//             }

//             if(!TokenFind)
//             {
//                 Response["Status"] = INVALIDTOKEN;
//                 Response["Description"] = "Invalid Token.";
//                 SHOW_ERROR(crow::json::dump(Response));
//                 return crow::response{401, Response};
//             }

//             std::string EncryptedData = DH->Request.enJsonRvalue["Data"].s();
//             std::string ClientPublicKeyAddress = this->WebServiceConfig.KeysPath + "/" + DH->Cameras[DH->CameraIndex].CompanyID;
//             std::string ServerPrivateKeyAddress = this->WebServiceConfig.KeysPath + "/ServerPri.pem";
//             std::string DecryptedData = decryptString(EncryptedData, ServerPrivateKeyAddress,  ClientPublicKeyAddress).DecryptedMessage;
//             DH->Request.body = DecryptedData;
//         }else
//         {
//             DH->Request.body = req.body;
//         } 
//         auto AuthenticationFinishTime = std::chrono::high_resolution_clock::now();
//         auto AuthenticationTime =  std::chrono::duration_cast<std::chrono::nanoseconds>(AuthenticationFinishTime - AuthenticationStartTime);

//         DH->DecryptedData = true;
//         // 1- Validation Input data
//         auto validationStartTime = std::chrono::high_resolution_clock::now();
//         if(!(this->m_Validatorobj->run(DH)))
//         {
//             Response["Status"] = DH->Response.errorCode;
//             Response["Description"] = DH->Response.Description;
//             if(DH->hasInputFields.DeviceID && DH->hasInputFields.PlateValue)
//             {
//                 Response["DevicedID"] = DH->Input.DeviceID;
//                 Response["PlateValue"] = DH->Input.PlateValue;
//             }
//             Response["IP"] = DH->Request.remoteIP;
//             if(DH->FailedDatabaseInfo.Enable)
//             {
//                 std::vector<MongoDB::Field> fields = {
//                     {"Status", std::to_string(DH->Response.errorCode), MongoDB::FieldType::Integer},
//                     {"Description", DH->Response.Description, MongoDB::FieldType::String},
//                     {"IP", DH->Request.remoteIP, MongoDB::FieldType::String}
//                 };

//                 if(DH->hasInputFields.DeviceID && DH->hasInputFields.PlateValue)
//                 {
//                     MongoDB::Field DeviceIDField = {"DeviceID", std::to_string(DH->Input.DeviceID), MongoDB::FieldType::Integer};
//                     MongoDB::Field PlateValueField = {"PlateValue", DH->Input.PlateValue, MongoDB::FieldType::String};
//                     fields.push_back(DeviceIDField);
//                     fields.push_back(PlateValueField);
//                 }

//                 DH->FailedDatabase ->Insert(DH->FailedDatabaseInfo.DatabaseName, DH->FailedDatabaseInfo.CollectionName, fields);
//             }
//             SHOW_ERROR(crow::json::dump(Response));
//             ClientResponse["Status"] = DH->Response.errorCode;
//             ClientResponse["Description"] = DH->Response.Description;
//             return crow::response{DH->Response.HTTPCode , ClientResponse};
//         }
//         auto validationFinishTime = std::chrono::high_resolution_clock::now();
//         auto ValidationTime =  std::chrono::duration_cast<std::chrono::nanoseconds>(validationFinishTime - validationStartTime);

//         auto ChecRecordIDStartTime = std::chrono::high_resolution_clock::now();
// #ifdef INSERTDATABASE
        
//         // 2- Check RecordID exist in database or not
//         std::vector<MongoDB::Field> filter = {
//             // equal
//             {"RecordID", DH->ProcessedInputData.MongoID, MongoDB::FieldType::ObjectId, "$gte"},
//             {"RecordID", DH->ProcessedInputData.MongoID, MongoDB::FieldType::ObjectId, "$lte"}

//         };
//         MongoDB::FindOptionStruct Option;
//         std::vector<std::string> ResultDoc;
//         auto FindReturn = DH->InsertDatabase->Find(DH->InsertDatabaseInfo.DatabaseName, DH->InsertDatabaseInfo.CollectionName, filter, Option, ResultDoc);
//         if(FindReturn.Code == MongoDB::MongoStatus::FindSuccessful)
//         {
//             if(!(ResultDoc.empty()))
//             {
//                 Response["Status"] = DUPLICATERECORD;
//                 Response["Description"] = "Duplicate Record.";
                
//                 if(DH->hasInputFields.DeviceID && DH->hasInputFields.ViolationID && DH->hasInputFields.PassedTime && DH->hasInputFields.PlateValue)
//                     Response["RecordID"] = DH->ProcessedInputData.MongoID;
//                 Response["IP"] = DH->Request.remoteIP;
//                 if(DH->FailedDatabaseInfo.Enable)
//                 {
//                     std::vector<MongoDB::Field> fields = {
//                         {"Status", std::to_string(DUPLICATERECORD), MongoDB::FieldType::Integer},
//                         {"Description", "Duplicate Record.", MongoDB::FieldType::String},
//                         {"IP", DH->Request.remoteIP, MongoDB::FieldType::String}
//                     };

//                     if(DH->hasInputFields.DeviceID && DH->hasInputFields.ViolationID && DH->hasInputFields.PassedTime && DH->hasInputFields.PlateValue)
//                     {
//                         MongoDB::Field RecordIDField = {"RecordID", DH->ProcessedInputData.MongoID, MongoDB::FieldType::ObjectId};
//                         fields.push_back(RecordIDField);
//                     }

//                     DH->FailedDatabase ->Insert(DH->FailedDatabaseInfo.DatabaseName, DH->FailedDatabaseInfo.CollectionName, fields);
//                 }
//                 SHOW_ERROR(crow::json::dump(Response));
//                 ClientResponse["Status"] = DUPLICATERECORD;
//                 ClientResponse["Description"] = "Duplicate Record.";
//                 return crow::response{400 , ClientResponse};
//             }
//         }else
//         {
//             SHOW_ERROR(FindReturn.Description);
//             Response["Status"] = DATABASEERROR;
//             Response["Description"] = "Network Internal Service Error.";
//             if(DH->hasInputFields.DeviceID && DH->hasInputFields.ViolationID && DH->hasInputFields.PassedTime && DH->hasInputFields.PlateValue)
//                 Response["RecordID"] = DH->ProcessedInputData.MongoID;
//             Response["IP"] = DH->Request.remoteIP;
//             if(DH->FailedDatabaseInfo.Enable)
//             {
//                 std::vector<MongoDB::Field> fields = {
//                     {"Status", std::to_string(DATABASEERROR), MongoDB::FieldType::Integer},
//                     {"Description", "Network Internal Service Error.", MongoDB::FieldType::String},
//                     {"IP", DH->Request.remoteIP, MongoDB::FieldType::String}
//                 };

//                 if(DH->hasInputFields.DeviceID && DH->hasInputFields.ViolationID && DH->hasInputFields.PassedTime && DH->hasInputFields.PlateValue)
//                 {
//                     MongoDB::Field RecordIDField = {"RecordID", DH->ProcessedInputData.MongoID, MongoDB::FieldType::ObjectId};
//                     fields.push_back(RecordIDField);
//                 }

//                 DH->FailedDatabase ->Insert(DH->FailedDatabaseInfo.DatabaseName, DH->FailedDatabaseInfo.CollectionName, fields);
//             }
//             SHOW_ERROR(crow::json::dump(Response));
//             ClientResponse["Status"] = DATABASEERROR;
//             ClientResponse["Description"] = "Network Internal Service Error.";
//             return crow::response{500 , ClientResponse};
//         }
// #endif // INSERTDATABASE
//         auto ChecRecordIDFinishTime = std::chrono::high_resolution_clock::now();
//         auto ChecRecordIDTime =  std::chrono::duration_cast<std::chrono::nanoseconds>(ChecRecordIDFinishTime - ChecRecordIDStartTime);

//         auto CheckOpStartTime = std::chrono::high_resolution_clock::now();
//         // 3- Run Check Operator Module
//         if(DH->Modules.CheckOperator.active && DH->hasInputFields.PlateImage)
//         {
//             ChOp::InputStruct inputChOp;
//             inputChOp.plateImage = DH->ProcessedInputData.PlateImageMat;
//             inputChOp.plateValue = DH->hasInputFields.PlateValue ? DH->Input.PlateValue : "";
//             // inputChOp.plateType = DH->hasInputFields.PlateType ? DH->Input.PlateType : static_cast<int>(inference::standards::PlateType::UNKNOWN);

//             int CheckOpObjectIndex = this->getCheckOpIndex();
//             ChOp::OutputStruct ChOpOutput;
//             try
//             {
//                 ChOpOutput = this->m_pChOpObjects[CheckOpObjectIndex]->run(inputChOp);
//                 this->releaseCheckOpIndex(CheckOpObjectIndex);
//                 DH->Input.MasterPlate = DH->Input.PlateValue;
//                 DH->Input.PlateValue = ChOpOutput.newPlateValue;
//                 DH->Input.CodeType = ChOpOutput.codeType;
//                 DH->Input.Probability = ChOpOutput.probability;
//                 DH->Input.PlateType = ChOpOutput.newPlateType;
//             } catch (const std::exception& e)
//             {
//                 Response["Status"] = CHECKOPERROR;
//                 Response["Description"] = e.what();
//                 if(DH->hasInputFields.DeviceID && DH->hasInputFields.ViolationID && DH->hasInputFields.PassedTime && DH->hasInputFields.PlateValue)
//                     Response["RecordID"] = DH->ProcessedInputData.MongoID;
//                 Response["IP"] = DH->Request.remoteIP;
//                 if(DH->FailedDatabaseInfo.Enable)
//                 {
//                     std::vector<MongoDB::Field> fields = {
//                         {"Status", std::to_string(CHECKOPERROR), MongoDB::FieldType::Integer},
//                         {"Description", e.what(), MongoDB::FieldType::String},
//                         {"IP", DH->Request.remoteIP, MongoDB::FieldType::String}
//                     };

//                     if(DH->hasInputFields.DeviceID && DH->hasInputFields.ViolationID && DH->hasInputFields.PassedTime && DH->hasInputFields.PlateValue)
//                     {
//                         MongoDB::Field RecordIDField = {"RecordID", DH->ProcessedInputData.MongoID, MongoDB::FieldType::ObjectId};
//                         fields.push_back(RecordIDField);
//                     }

//                     DH->FailedDatabase ->Insert(DH->FailedDatabaseInfo.DatabaseName, DH->FailedDatabaseInfo.CollectionName, fields);
//                 }
//                 if(DH->DebugMode)
//                     SHOW_ERROR(crow::json::dump(Response));
//                 this->releaseCheckOpIndex(CheckOpObjectIndex);
//                 ClientResponse["Status"] = CHECKOPERROR;
//                 ClientResponse["Description"] = "Internal Service Error.";
//                 return crow::response{DH->Response.HTTPCode , ClientResponse};
//             }
//         }
//         auto CheckOpFinishTime = std::chrono::high_resolution_clock::now();
//         auto CheckOpTime =  std::chrono::duration_cast<std::chrono::nanoseconds>(CheckOpFinishTime - CheckOpStartTime);

//         auto ClassifierStartTime = std::chrono::high_resolution_clock::now();
//         // 4- Run Classifier Module
//         if(DH->Modules.Classifier.active)
//         {   Classifier::InputStruct inputClassify;
//             inputClassify.ImageBase64 = DH->Input.ColorImage;
//             int ClassifierObjectIndex = this->getClassifierIndex();
//             Classifier::OutputStruct ClassifierOutput;
//             try
//             {
//                 auto ClassifierOutputVec = this->m_pClassifierObjects[ClassifierObjectIndex]->run(inputClassify);
//                 this->releaseClassifierIndex(ClassifierObjectIndex);
//                 std::vector<crow::json::wvalue> classificationsJson;
//                 for (auto& output:ClassifierOutputVec)
//                 {
//                     crow::json::wvalue CResponse;
//                     CResponse["VehiclModel"] = output.vehicleModel;
//                     crow::json::wvalue rectJson;
//                     rectJson["x"] = output.box.x;
//                     rectJson["y"] = output.box.y;
//                     rectJson["width"] = output.box.width;
//                     rectJson["height"] = output.box.height;
//                     CResponse["CarRect"] = std::move(rectJson);
//                     classificationsJson.push_back(std::move(CResponse));
//                     //TODO creat response json
//                 }
//                 crow::json::wvalue j;
//                 j["EmsInfoId"]      = DH->Input.RecordID;
//                 j["numDetections"]  = ClassifierOutputVec.size();
//                 j["classification"] = std::move(classificationsJson) ;
//                 std::string test = crow::json::dump(j);
//                 std::cout<<test<<std::endl;

//             }
//             catch(const std::exception& e)
//             {
//                 std::cerr << e.what() << '\n';
//             }
            
//             // } catch (const std::exception& e)
//             // {
//             //     Response["Status"] = CLASSIFIERERROR;
//             //     Response["Description"] = e.what();
//             //     if(DH->hasInputFields.DeviceID && DH->hasInputFields.ViolationID && DH->hasInputFields.PassedTime && DH->hasInputFields.PlateValue)
//             //         Response["RecordID"] = DH->ProcessedInputData.MongoID;
//             //     Response["IP"] = DH->Request.remoteIP;
//             //     if(DH->FailedDatabaseInfo.Enable)
//             //     {
//             //         std::vector<MongoDB::Field> fields = {
//             //             {"Status", std::to_string(CLASSIFIERERROR), MongoDB::FieldType::Integer},
//             //             {"Description", e.what(), MongoDB::FieldType::String},
//             //             {"IP", DH->Request.remoteIP, MongoDB::FieldType::String}
//             //         };

//             //         if(DH->hasInputFields.DeviceID && DH->hasInputFields.ViolationID && DH->hasInputFields.PassedTime && DH->hasInputFields.PlateValue)
//             //         {
//             //             MongoDB::Field RecordIDField = {"RecordID", DH->ProcessedInputData.MongoID, MongoDB::FieldType::ObjectId};
//             //             fields.push_back(RecordIDField);
//             //         }

//             //         DH->FailedDatabase ->Insert(DH->FailedDatabaseInfo.DatabaseName, DH->FailedDatabaseInfo.CollectionName, fields);
//             //     }
//             //     if(DH->DebugMode)
//             //         SHOW_ERROR(crow::json::dump(Response));
//             //     this->releaseClassifierIndex(ClassifierObjectIndex);
//             //     ClientResponse["Status"] = CLASSIFIERERROR;
//             //     ClientResponse["Description"] = "Internal Service Error.";
//             //     return crow::response{DH->Response.HTTPCode , ClientResponse};
//             // }
//         }

//         auto ClassifierFinishTime = std::chrono::high_resolution_clock::now();
//         auto ClassifierTime =  std::chrono::duration_cast<std::chrono::nanoseconds>(ClassifierFinishTime - ClassifierStartTime);

//         auto storeImageStartTime = std::chrono::high_resolution_clock::now();
// #ifdef STOREIMAGE
//         // 5- Store Image
//         std::shared_ptr<storeimage> storeimageobj = std::make_shared<storeimage>();
//         if(!(storeimageobj->run(DH)))
//         {
//             Response["Status"] = DH->Response.errorCode;
//             Response["Description"] = DH->Response.Description;
//             if(DH->hasInputFields.DeviceID && DH->hasInputFields.ViolationID && DH->hasInputFields.PassedTime && DH->hasInputFields.PlateValue)
//                 Response["RecordID"] = DH->ProcessedInputData.MongoID;
//             Response["IP"] = DH->Request.remoteIP;
//             if(DH->FailedDatabaseInfo.Enable)
//             {
//                 std::vector<MongoDB::Field> fields = {
//                     {"Status", std::to_string(DH->Response.errorCode), MongoDB::FieldType::Integer},
//                     {"Description", DH->Response.Description, MongoDB::FieldType::String},
//                     {"IP", DH->Request.remoteIP, MongoDB::FieldType::String}
//                 };

//                 if(DH->hasInputFields.DeviceID && DH->hasInputFields.ViolationID && DH->hasInputFields.PassedTime && DH->hasInputFields.PlateValue)
//                 {
//                     MongoDB::Field RecordIDField = {"RecordID", DH->ProcessedInputData.MongoID, MongoDB::FieldType::ObjectId};
//                     fields.push_back(RecordIDField);
//                 }

//                 DH->FailedDatabase ->Insert(DH->FailedDatabaseInfo.DatabaseName, DH->FailedDatabaseInfo.CollectionName, fields);
//             }
//             SHOW_ERROR(crow::json::dump(Response));
//             ClientResponse["Status"] = DH->Response.errorCode;
//             ClientResponse["Description"] = DH->Response.Description;
//             return crow::response{DH->Response.HTTPCode , ClientResponse};
//         }
// #endif // STOREIMAGE
//         auto storeImageFinishTime = std::chrono::high_resolution_clock::now();
//         auto storeImaheTime =  std::chrono::duration_cast<std::chrono::nanoseconds>(storeImageFinishTime - storeImageStartTime);        

//         auto saveDataStartTime = std::chrono::high_resolution_clock::now();
// #if defined KAFKAOUTPUT || defined INSERTDATABASE
//         // 6- Save Data
//         std::shared_ptr<savedata> savedataobj = std::make_shared<savedata>();
// #ifdef KAFKAOUTPUT
//         int OutputKafkaConnectionIndex = this->getKafkaConnectionIndex();
//         savedataobj->setOutputKafkaConnection(this->OutputKafkaConnections[OutputKafkaConnectionIndex]);
// #endif // KAFKAOUTPUT
        
//         if(!(savedataobj->run(DH)))
//         {
//             Response["Status"] = DH->Response.errorCode;
//             Response["Description"] = DH->Response.Description;
//             if(DH->hasInputFields.DeviceID && DH->hasInputFields.ViolationID && DH->hasInputFields.PassedTime && DH->hasInputFields.PlateValue)
//                 Response["RecordID"] = DH->ProcessedInputData.MongoID;
//             Response["IP"] = DH->Request.remoteIP;
//             if(DH->FailedDatabaseInfo.Enable)
//             {
//                 std::vector<MongoDB::Field> fields = {
//                     {"Status", std::to_string(DH->Response.errorCode), MongoDB::FieldType::Integer},
//                     {"Description", DH->Response.Description, MongoDB::FieldType::String},
//                     {"IP", DH->Request.remoteIP, MongoDB::FieldType::String}
//                 };

//                 if(DH->hasInputFields.DeviceID && DH->hasInputFields.ViolationID && DH->hasInputFields.PassedTime && DH->hasInputFields.PlateValue)
//                 {
//                     MongoDB::Field RecordIDField = {"RecordID", DH->ProcessedInputData.MongoID, MongoDB::FieldType::ObjectId};
//                     fields.push_back(RecordIDField);
//                 }

//                 DH->FailedDatabase ->Insert(DH->FailedDatabaseInfo.DatabaseName, DH->FailedDatabaseInfo.CollectionName, fields);
//             }
//             SHOW_ERROR(crow::json::dump(Response));
//             ClientResponse["Status"] = DH->Response.errorCode;
//             ClientResponse["Description"] = DH->Response.Description;
//             return crow::response{DH->Response.HTTPCode , ClientResponse};
//         }
// #ifdef KAFKAOUTPUT
//         this->releaseKafkaIndex(OutputKafkaConnectionIndex);
// #endif // KAFKAOUTPUT
// #endif // KAFKAOUTPUT || INSERTDATABASE
//         auto saveDataFinishTime = std::chrono::high_resolution_clock::now();
//         auto saveDataTime =  std::chrono::duration_cast<std::chrono::nanoseconds>(saveDataFinishTime - saveDataStartTime);     

//         auto requestFinishTime = std::chrono::high_resolution_clock::now();
//         auto requestTime =  std::chrono::duration_cast<std::chrono::nanoseconds>(requestFinishTime - requstStartTime);

//         if(DH->DebugMode)
//             SHOW_IMPORTANTLOG3("ProccessTime(ns) = " << std::to_string(requestTime.count()) << std::endl << "0- Authentication ProccessTime(ns) = " << std::to_string(AuthenticationTime.count())
//                            << std::endl << "1- Validation ProccessTime(ns) = " << std::to_string(ValidationTime.count())
//                            << std::endl << "2- Check RecordID ProccessTime(ns) = " << std::to_string(ChecRecordIDTime.count()) << std::endl << "3- CheckOp ProccessTime(ns) = " << std::to_string(CheckOpTime.count())
//                            << std::endl << "4- Classifier ProccessTime(ns) = " << std::to_string(ClassifierTime.count())
//                            << std::endl << "5- Store image ProccessTime(ns) = " << std::to_string(storeImaheTime.count())
//                            << std::endl << "6- Save data ProccessTime(ns) = " << std::to_string(saveDataTime.count()));
        
//         Response["Status"] = SUCCESSFUL;
//         if(DH->hasInputFields.DeviceID && DH->hasInputFields.ViolationID && DH->hasInputFields.PassedTime && DH->hasInputFields.PlateValue)
//             Response["RecordID"] = DH->ProcessedInputData.MongoID;
//         SHOW_LOG(crow::json::dump(Response));
//         return crow::response{200 , Response};
//     });
// }

void WebService::TokenRoute()
{
    std::string Route = this->WebServiceConfig.WebServiceInfo.URI;
    if(Route.back() != '/')
        Route += "/";
    if(Route[0] != '/')
        Route = "/" + Route;
    Route += "token";
    
    this->app->route_dynamic(Route.c_str()).methods(crow::HTTPMethod::POST)([&](const crow::request& req ) {
        //! "startTime" for Computing process time for this request
        auto requstStartTime = std::chrono::high_resolution_clock::now();
        
        std::shared_ptr<DataHandler::DataHandlerStruct> DH = std::make_shared<DataHandler::DataHandlerStruct>();
        DH->InsertRoute = false;
        DH->DecryptedData = true;
        DH->Request.body = req.body;
        crow::json::wvalue Response;

        DH->Request.remoteIP = req.ipAddress;
        SHOW_IMPORTANTLOG("Recived Token request from IP -> " + DH->Request.remoteIP);

        Configurate* ConfigurateObj = Configurate::getInstance();
        DH->Cameras = ConfigurateObj->getCameras();
        DH->InsertDatabase = ConfigurateObj->getInsertDatabase();
        DH->InsertDatabaseInfo = ConfigurateObj->getInsertDatabaseInfo();
        DH->DebugMode = this->WebServiceConfig.DebugMode;
        
        // 1- Validation Input data
        auto validationStartTime = std::chrono::high_resolution_clock::now();
        std::shared_ptr<Validator> Validatorobj = std::make_shared<Validator>();
        if(!(Validatorobj->run(DH)))
        {
            Response["Status"] = DH->Response.errorCode;
            Response["Description"] = DH->Response.Description;
            if(DH->DebugMode)
                SHOW_ERROR(crow::json::dump(Response));
            return crow::response{DH->Response.HTTPCode , Response};
        }
        auto validationFinishTime = std::chrono::high_resolution_clock::now();
        auto ValidationTime =  std::chrono::duration_cast<std::chrono::nanoseconds>(validationFinishTime - validationStartTime);

        std::time_t currentTime = std::time(nullptr);
        // 2- Check Current token expiration time
        auto CheckCurrentTokenStartTime = std::chrono::high_resolution_clock::now();
        std::string currentToken = DH->Cameras[DH->CameraIndex].TokenInfo.Token;
        std::time_t currentTokenCreateAt = DH->Cameras[DH->CameraIndex].TokenInfo.CreatedAt;
        std::time_t TokenExpirationTime = currentTokenCreateAt + this->WebServiceConfig.TokenTimeAllowed;
        auto CheckCurrentTokenEndTime = std::chrono::high_resolution_clock::now();
        auto CheckCurrentTokenTime =  std::chrono::duration_cast<std::chrono::nanoseconds>(CheckCurrentTokenEndTime - CheckCurrentTokenStartTime);
        if(currentTime < TokenExpirationTime)
        {
            auto requestFinishTime = std::chrono::high_resolution_clock::now();
            auto requestTime =  std::chrono::duration_cast<std::chrono::nanoseconds>(requestFinishTime - requstStartTime);
            if(DH->DebugMode)
                SHOW_IMPORTANTLOG3("ProccessTime(ns) = " << std::to_string(requestTime.count()) << std::endl << "1- Validation ProccessTime(ns) = " << std::to_string(ValidationTime.count()) << std::endl << 
                                   "2- CheckCurrentToken ProccessTime(ns) = " << std::to_string(CheckCurrentTokenTime.count()));
            
            Response["Status"] = SUCCESSFUL;
            Response["Token"] = currentToken;
            SHOW_LOG(crow::json::dump(Response));
            return crow::response{200 , Response};   
        }

        // 3- Generate Token
        auto GenerateTokenStartTime = std::chrono::high_resolution_clock::now();
        std::string Token = boost::lexical_cast<std::string>(boost::uuids::random_generator()());
        
        std::ostringstream oss;
        std::tm* CurrenttimeInfo = std::localtime(&currentTime);
        oss << std::put_time(CurrenttimeInfo, "%Y-%m-%d %H:%M:%S");
        std::string TokenTime = oss.str();

        int TokenCounter = DH->Cameras[DH->CameraIndex].TokenInfo.Counter + 1;

        std::vector<MongoDB::Field> filter = {
            // equal
            {"deviceId", std::to_string(DH->Cameras[DH->CameraIndex].DeviceID), MongoDB::FieldType::Integer, "$gte"},
            {"deviceId", std::to_string(DH->Cameras[DH->CameraIndex].DeviceID), MongoDB::FieldType::Integer, "$lte"}
        };

        std::vector<MongoDB::Field> Update = {
            {"token", Token, MongoDB::FieldType::String},
            {"tokenCreatedAt", TokenTime, MongoDB::FieldType::Date},
            {"tokenCounter", std::to_string(TokenCounter), MongoDB::FieldType::Integer}
        };

        auto UpdateReturn = DH->InsertDatabase->Update(DH->InsertDatabaseInfo.DatabaseName, "cameras", filter, Update);
        if(UpdateReturn.Code != MongoDB::MongoStatus::UpdateSuccessful)
        {
            Response["Status"] = DATABASEERROR;
            Response["Description"] = "Internal Server Error.";
            if(DH->DebugMode)
                SHOW_ERROR(crow::json::dump(Response));
            return crow::response{500 , Response};
        }

        ConfigurateObj->SetNewToken(DH->CameraIndex, Token, currentTime);
        if(this->WebServiceConfig.NotifyingOtherServicesTokenUpdate)
        {
            for(auto& Service : this->WebServiceConfig.OtherService)
            {
                std::string URL = Service.IP + ":" + std::to_string(Service.Port) + "/" + Service.URI;
                CURL *curl;
                CURLcode res;
                curl = curl_easy_init();
                if(curl) {
                    
                    crow::json::wvalue json;
                    json["CameraIndex"] = DH->CameraIndex;
                    json["Token"] = Token;
                    json["TokenTime"] = currentTime;
                    std::string jsonStr = crow::json::dump(json);

                    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
                    curl_easy_setopt(curl, CURLOPT_URL, URL.c_str());
                    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
                    curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "http");
                    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 1);
                    struct curl_slist *headers = NULL;
                    headers = curl_slist_append(headers, "Content-Type: application/json");
                    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
                    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonStr.c_str());
                    res = curl_easy_perform(curl);
                    if(res != CURLE_OK)
                    {
                        if(DH->DebugMode)
                            SHOW_ERROR("Send Token Update to = " << Service.IP << ":" << Service.Port << "/" << Service.URI 
                                       << "  Error : " <<  curl_easy_strerror(res));
                    }
                    curl_slist_free_all(headers);
                }
                curl_easy_cleanup(curl);
            }
        }

        auto GenerateTokenFinishTime = std::chrono::high_resolution_clock::now();
        auto GenerateTokenTime =  std::chrono::duration_cast<std::chrono::nanoseconds>(GenerateTokenFinishTime - GenerateTokenStartTime);

        auto requestFinishTime = std::chrono::high_resolution_clock::now();
        auto requestTime =  std::chrono::duration_cast<std::chrono::nanoseconds>(requestFinishTime - requstStartTime);

        if(DH->DebugMode)
            SHOW_IMPORTANTLOG3("ProccessTime(ns) = " << std::to_string(requestTime.count()) << std::endl << "1- Validation ProccessTime(ns) = " << std::to_string(ValidationTime.count()) << std::endl << 
                               "2- CheckCurrentToken ProccessTime(ns) = " << std::to_string(CheckCurrentTokenTime.count()) << std::endl << "3- Generate Token ProccessTime(ns) = " << std::to_string(GenerateTokenTime.count()));

        Response["Status"] = SUCCESSFUL;
        Response["Token"] = Token;
        SHOW_LOG(crow::json::dump(Response));
        return crow::response{200 , Response};
    });
}