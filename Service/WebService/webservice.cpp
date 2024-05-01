#include "webservice.h"

WebService::WebService(Configurate::WebServiceConfigStruct ServiceConfig)
{
    this->app = std::make_shared<crow::SimpleApp>();
    this->app->loglevel(crow::LogLevel::Error);
    this->WebServiceConfig = ServiceConfig;
}

void WebService::run()
{
    try 
    {   
        this->InsertRoute();
        if(this->WebServiceConfig.Authentication)
            this->TokenRoute();

        SHOW_IMPORTANTLOG3("Runinng Aggregation on port " + std::to_string(this->WebServiceConfig.WebServiceInfo.Port));
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

void WebService::InsertRoute()
{
    std::string Route = this->WebServiceConfig.WebServiceInfo.URI;
    if(Route.back() != '/')
        Route += "/";
    if(Route[0] != '/')
        Route = "/" + Route;
    Route += "insert";
    
    this->app->route_dynamic(Route.c_str()).methods(crow::HTTPMethod::POST)([&](const crow::request& req ) {
        
        //! "startTime" for Computing process time for this request
        auto requstStartTime = std::chrono::high_resolution_clock::now();
        
        std::shared_ptr<DataHandler::DataHandlerStruct> DH = std::make_shared<DataHandler::DataHandlerStruct>();
        Configurate* ConfigurateObj = Configurate::getInstance();
        DH->hasInputFields = ConfigurateObj->getInputFields();
        DH->hasOutputFields = ConfigurateObj->getOutputFields();
        DH->StoreImageConfig = ConfigurateObj->getStoreImageConfig();
        DH->ViolationMap = ConfigurateObj->getViolationMap();
        DH->Cameras = ConfigurateObj->getCameras();
        DH->DaysforPassedTimeAcceptable = this->WebServiceConfig.DaysforPassedTimeAcceptable;
        DH->InsertDatabase = ConfigurateObj->getInsertDatabase();
        DH->InsertDatabaseInfo = ConfigurateObj->getInsertDatabaseInfo();
        DH->Modules = ConfigurateObj->getModules();
        DH->DebugMode = this->WebServiceConfig.DebugMode;
        DH->InsertRoute = true;
        DH->WebServiceAuthentication = this->WebServiceConfig.Authentication;
        
        crow::json::wvalue Response;

        DH->Request.remoteIP = req.ipAddress;
        SHOW_IMPORTANTLOG("Recived Insert request from IP -> " + DH->Request.remoteIP);

        std::shared_ptr<Validator> Validatorobj = std::make_shared<Validator>();
        // 0- Check Authentication
        auto AuthenticationStartTime = std::chrono::high_resolution_clock::now();
        if(this->WebServiceConfig.Authentication)
        {
            DH->DecryptedData = false;
            DH->Request.body = req.body;
            // Validation Input data
            if(!(Validatorobj->run(DH)))
            {
                Response["Status"] = DH->Response.errorCode;
                Response["Description"] = DH->Response.Description;
                if(DH->DebugMode)
                    SHOW_ERROR(crow::json::dump(Response));
                return crow::response{DH->Response.HTTPCode , Response};
            }

            // Check Token
            bool TokenFind = false;
            std::string Token = DH->Request.JsonRvalue["Token"].s();
            for(int i = 0; i < DH->Cameras.size(); i++)
            {
                if(DH->Cameras[i].TokenInfo.Token == Token)
                {
                    TokenFind = true;
                    DH->CameraIndex = i;
                    std::time_t currentTime = std::time(nullptr);
                    std::time_t TokenExpirationTime = DH->Cameras[i].TokenInfo.CreatedAt + this->WebServiceConfig.TokenTimeAllowed;
                    if(currentTime > TokenExpirationTime)
                    {
                        Response["Status"] = EXPIREDTOKEN;
                        Response["Description"] = "Expired Token.";
                        if(DH->DebugMode)
                            SHOW_ERROR(crow::json::dump(Response));
                        return crow::response{401, Response};   
                    }
                    break;
                }
            }

            if(!TokenFind)
            {
                Response["Status"] = INVALIDTOKEN;
                Response["Description"] = "Invalid Token.";
                if(DH->DebugMode)
                    SHOW_ERROR(crow::json::dump(Response));
                return crow::response{401, Response};
            }

            std::string EncryptedData = DH->Request.JsonRvalue["Data"].s();
            std::string ClientPublicKeyAddress = this->WebServiceConfig.KeysPath + "/" + DH->Cameras[DH->CameraIndex].CompanyID;
            std::string DecryptedData = decryptString(EncryptedData, "ServerPri.pem",  ClientPublicKeyAddress).DecryptedMessage;
            DH->Request.body = DecryptedData;
        }else
        {
            DH->Request.body = req.body;
        } 
        auto AuthenticationFinishTime = std::chrono::high_resolution_clock::now();
        auto AuthenticationTime =  std::chrono::duration_cast<std::chrono::nanoseconds>(AuthenticationFinishTime - AuthenticationStartTime);

        DH->DecryptedData = true;
        // 1- Validation Input data
        auto validationStartTime = std::chrono::high_resolution_clock::now();
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

        auto ChecRecordIDStartTime = std::chrono::high_resolution_clock::now();
#ifdef INSERTDATABASE
        
        // 2- Check RecordID exist in database or not
        std::vector<MongoDB::Field> filter = {
            // equal
            {"RecordID", DH->ProcessedInputData.MongoID, MongoDB::FieldType::ObjectId, "$gte"},
            {"RecordID", DH->ProcessedInputData.MongoID, MongoDB::FieldType::ObjectId, "$lte"}

        };
        MongoDB::FindOptionStruct Option;
        std::vector<std::string> ResultDoc;
        auto FindReturn = DH->InsertDatabase->Find(DH->InsertDatabaseInfo.DatabaseName, DH->InsertDatabaseInfo.CollectionName, filter, Option, ResultDoc);
        if(FindReturn.Code == MongoDB::MongoStatus::FindSuccessful)
        {
            if(!(ResultDoc.empty()))
            {
                Response["Status"] = DUPLICATERECORD;
                Response["Description"] = "Duplicate Record.";
                if(DH->DebugMode)
                    SHOW_ERROR(crow::json::dump(Response));
                return crow::response{400 , Response};
            }
        }else
        {
            SHOW_ERROR(FindReturn.Description);
            Response["Status"] = DATABASEERROR;
            Response["Description"] = "Network Internal Service Error.";
            if(DH->DebugMode)
                SHOW_ERROR(crow::json::dump(Response));
            return crow::response{500 , Response};
        }
#endif // INSERTDATABASE
        auto ChecRecordIDFinishTime = std::chrono::high_resolution_clock::now();
        auto ChecRecordIDTime =  std::chrono::duration_cast<std::chrono::nanoseconds>(ChecRecordIDFinishTime - ChecRecordIDStartTime);

        auto CheckOpStartTime = std::chrono::high_resolution_clock::now();
        // 3- Run Check Operator Module
        if(DH->Modules.CheckOperator.active && DH->hasInputFields.PlateImage)
        {
            int CheckOpObjectIndex = this->getCheckOpIndex();
            auto CheckOpResult = this->CheckOPObjects[CheckOpObjectIndex]->run(DH->ProcessedInputData.PlateImageMat, DH->Input.PlateValue);
            DH->Input.MasterPlate = DH->Input.PlateValue;
            DH->Input.PlateValue = CheckOpResult.NewPlateValue;
            DH->Input.CodeType = CheckOpResult.CodeType;
            DH->Input.Probability = CheckOpResult.Probability;
            this->releaseCheckOpIndex(CheckOpObjectIndex);
        }
        auto CheckOpFinishTime = std::chrono::high_resolution_clock::now();
        auto CheckOpTime =  std::chrono::duration_cast<std::chrono::nanoseconds>(CheckOpFinishTime - CheckOpStartTime);

        auto storeImageStartTime = std::chrono::high_resolution_clock::now();
#ifdef STOREIMAGE
        // 4- Store Image
        std::shared_ptr<storeimage> storeimageobj = std::make_shared<storeimage>();
        if(!(storeimageobj->run(DH)))
        {
            Response["Status"] = DH->Response.errorCode;
            Response["Description"] = DH->Response.Description;
            if(DH->DebugMode)
                SHOW_ERROR(crow::json::dump(Response));
            return crow::response{DH->Response.HTTPCode , Response};
        }
#endif // STOREIMAGE
        auto storeImageFinishTime = std::chrono::high_resolution_clock::now();
        auto storeImaheTime =  std::chrono::duration_cast<std::chrono::nanoseconds>(storeImageFinishTime - storeImageStartTime);        

        auto saveDataStartTime = std::chrono::high_resolution_clock::now();
#if defined KAFKAOUTPUT || defined INSERTDATABASE
        // 5- Save Data
        std::shared_ptr<savedata> savedataobj = std::make_shared<savedata>();
#ifdef KAFKAOUTPUT
        int OutputKafkaConnectionIndex = this->getKafkaConnectionIndex();
        savedataobj->setOutputKafkaConnection(this->OutputKafkaConnections[OutputKafkaConnectionIndex]);
#endif // KAFKAOUTPUT
        
        if(!(savedataobj->run(DH)))
        {
            Response["Status"] = DH->Response.errorCode;
            Response["Description"] = DH->Response.Description;
            if(DH->DebugMode)
                SHOW_ERROR(crow::json::dump(Response));
            return crow::response{DH->Response.HTTPCode , Response};
        }
#ifdef KAFKAOUTPUT
        this->releaseKafkaIndex(OutputKafkaConnectionIndex);
#endif // KAFKAOUTPUT
#endif // KAFKAOUTPUT || INSERTDATABASE
        auto saveDataFinishTime = std::chrono::high_resolution_clock::now();
        auto saveDataTime =  std::chrono::duration_cast<std::chrono::nanoseconds>(saveDataFinishTime - saveDataStartTime);     

        auto requestFinishTime = std::chrono::high_resolution_clock::now();
        auto requestTime =  std::chrono::duration_cast<std::chrono::nanoseconds>(requestFinishTime - requstStartTime);

        if(DH->DebugMode)
            SHOW_IMPORTANTLOG3("ProccessTime(ns) = " << std::to_string(requestTime.count()) << std::endl << "0- Authentication ProccessTime(ns) = " << std::to_string(AuthenticationTime.count())
                           << std::endl << "1- Validation ProccessTime(ns) = " << std::to_string(ValidationTime.count())
                           << std::endl << "2- Check RecordID ProccessTime(ns) = " << std::to_string(ChecRecordIDTime.count()) << std::endl << "3- CheckOp ProccessTime(ns) = " << std::to_string(CheckOpTime.count())
                           << std::endl << "4- Store image ProccessTime(ns) = " << std::to_string(storeImaheTime.count())
                           << std::endl << "5- Save data ProccessTime(ns) = " << std::to_string(saveDataTime.count()));
        
        Response["Status"] = SUCCESSFUL;
        if(DH->hasInputFields.DeviceID && DH->hasInputFields.ViolationID && DH->hasInputFields.PassedTime && DH->hasInputFields.PlateValue)
            Response["RecordID"] = DH->ProcessedInputData.MongoID;
        SHOW_LOG(crow::json::dump(Response));
        return crow::response{200 , Response};
    });
}

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

        // 2- Generate Token
        auto GenerateTokenStartTime = std::chrono::high_resolution_clock::now();
        std::string Token = boost::lexical_cast<std::string>(boost::uuids::random_generator()());
        
        std::ostringstream oss;
        std::time_t currentTime = std::time(nullptr);
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
            Response["Description"] = UpdateReturn.Description;
            if(DH->DebugMode)
                SHOW_ERROR(crow::json::dump(Response));
            return crow::response{500 , Response};
        }

        ConfigurateObj->SetNewToken(DH->CameraIndex, Token, currentTime);
        if(this->WebServiceConfig.NotifyingOtherServicesTokenUpdate)
        {
            for(auto& Service : this->WebServiceConfig.OtherService)
            {
                SHOW_IMPORTANTLOG2("Send Camera JSON to = " << Service.IP << ":" << Service.Port << "/" << Service.URI);
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
            SHOW_IMPORTANTLOG3("ProccessTime(ns) = " << std::to_string(requestTime.count()) << std::endl << "1- Validation ProccessTime(ns) = " << std::to_string(ValidationTime.count()) << std::endl << "2- Generate Token ProccessTime(ns) = " << std::to_string(GenerateTokenTime.count()));

        Response["Status"] = SUCCESSFUL;
        Response["Token"] = Token;
        SHOW_LOG(crow::json::dump(Response));
        return crow::response{200 , Response};
    });
}