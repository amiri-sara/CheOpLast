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

        SHOW_IMPORTANTLOG3("Runinng Aggregation on port " + std::to_string(this->WebServiceConfig.Port));
        try{
            //! run object of crow by specific port and many rout in multithread status
            this->app->port(this->WebServiceConfig.Port).concurrency(this->WebServiceConfig.threadNumber).run();

        }  catch (...) {
            SHOW_ERROR("port " + std::to_string(this->WebServiceConfig.Port) + " is busy . Check ports in server table in config database .");
            exit(0);
        }

    }
    catch(boost::exception_detail::clone_impl<boost::exception_detail::error_info_injector<boost::property_tree::ptree_bad_path> > &e)
    {

        SHOW_ERROR("Error Code 0x" + std::to_string(__LINE__) + "JBF_3K92XS543272" + e.what());

        SHOW_ERROR("Can't Run Crow on port "  + std::to_string(this->WebServiceConfig.Port) + " please check this port ." );

    }
}

void WebService::InsertRoute()
{
    std::string Route = this->WebServiceConfig.URI;
    if(Route.back() != '/')
        Route += "/";
    if(Route[0] != '/')
        Route = "/" + Route;
    Route += "insert";
    
    this->app->route_dynamic(Route.c_str()).methods(crow::HTTPMethod::POST)([&](const crow::request& req ) {
        
        //! "startTime" for Computing process time for this request
        auto requstStartTime = std::chrono::high_resolution_clock::now();
        
        std::shared_ptr<DataHandler::DataHandlerStruct> DH = std::make_shared<DataHandler::DataHandlerStruct>();
        DH->Request.body = req.body;
        crow::json::wvalue Response;

        DH->Request.remoteIP = req.ipAddress;
        SHOW_IMPORTANTLOG("Recived request from IP -> " + DH->Request.remoteIP);

        Configurate* ConfigurateObj = Configurate::getInstance();

        DH->hasInputFields = ConfigurateObj->getInputFields();
        DH->hasOutputFields = ConfigurateObj->getOutputFields();
        DH->StoreImageConfig = ConfigurateObj->getStoreImageConfig();
        DH->ViolationMap = ConfigurateObj->getViolationMap();
        DH->Cameras = ConfigurateObj->getCameras();
        DH->DaysforPassedTimeAcceptable = this->WebServiceConfig.DaysforPassedTimeAcceptable;
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

        auto storeImageStartTime = std::chrono::high_resolution_clock::now();
#ifdef STOREIMAGE
        // 3- Store Image
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
        // 4- Save Data
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
        this->releaseIndex(OutputKafkaConnectionIndex);
#endif // KAFKAOUTPUT
#endif // KAFKAOUTPUT || INSERTDATABASE
        auto saveDataFinishTime = std::chrono::high_resolution_clock::now();
        auto saveDataTime =  std::chrono::duration_cast<std::chrono::nanoseconds>(saveDataFinishTime - saveDataStartTime);     

        auto requestFinishTime = std::chrono::high_resolution_clock::now();
        auto requestTime =  std::chrono::duration_cast<std::chrono::nanoseconds>(requestFinishTime - requstStartTime);

        if(DH->DebugMode)
            SHOW_IMPORTANTLOG3("ProccessTime(ns) = " << std::to_string(requestTime.count()) << std::endl << "1- Validation ProccessTime(ns) = " << std::to_string(ValidationTime.count())
                           << std::endl << "2- Check RecordID ProccessTime(ns) = " << std::to_string(ChecRecordIDTime.count()) << std::endl << "3- Store image ProccessTime(ns) = " << std::to_string(storeImaheTime.count())
                           << std::endl << "4- Save data ProccessTime(ns) = " << std::to_string(saveDataTime.count()));
        
        Response["Status"] = SUCCESSFUL;
        Response["Description"] = "Successful";
        SHOW_LOG(crow::json::dump(Response));
        return crow::response{200 , Response};
    });
}

void WebService::TokenRoute()
{
    std::string Route = this->WebServiceConfig.URI;
    if(Route.back() != '/')
        Route += "/";
    if(Route[0] != '/')
        Route = "/" + Route;
    Route += "token";
    
    this->app->route_dynamic(Route.c_str()).methods(crow::HTTPMethod::POST)([&](const crow::request& req ) {
        SHOW_IMPORTANTLOG2(req.body);
        
        auto Res    {crow::json::wvalue()};
        Res["res"]  = "Token Route";

        return crow::response{200 , "Token Route"};
    });
}