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
        auto startTime = std::chrono::high_resolution_clock::now();
        
        std::shared_ptr<DataHandler::DataHandlerStruct> DH = std::make_shared<DataHandler::DataHandlerStruct>();
        DH->Request.body = req.body;
        crow::json::wvalue Response;

        DH->Request.remoteIP = req.ipAddress;
        SHOW_IMPORTANTLOG("Recived request from IP -> " + DH->Request.remoteIP);

        Configurate* ConfigurateObj = Configurate::getInstance();

        DH->hasInputFields = this->InputFields;
        DH->StoreImageConfig = this->StoreImageConfig;
        DH->ViolationMap = this->ViolationMap;
        DH->Cameras = ConfigurateObj->getCameras();
        DH->DaysforPassedTimeAcceptable = this->WebServiceConfig.DaysforPassedTimeAcceptable;

        // 1- Validation Input data
        std::shared_ptr<Validator> Validatorobj = std::make_shared<Validator>();
        if(!(Validatorobj->run(DH)))
        {
            Response["Status"] = DH->Response.errorCode;
            Response["Description"] = DH->Response.Description;
            return crow::response{DH->Response.HTTPCode , Response};
        }

        // 2- Check RecordID exist in database or not
        std::vector<MongoDB::Field> filter = {
            // equal
            {"_id", DH->ProcessedInputData.MongoID, MongoDB::FieldType::ObjectId, "$gte"},
            {"_id", DH->ProcessedInputData.MongoID, MongoDB::FieldType::ObjectId, "$lte"}

        };
        MongoDB::FindOptionStruct Option;
        std::vector<std::string> ResultDoc;
        auto FindReturn = this->InsertDatabase->Find(this->InsertDatabaseInfo.DatabaseName, this->InsertDatabaseInfo.CollectionName, filter, Option, ResultDoc);
        if(FindReturn.Code == MongoDB::MongoStatus::FindSuccessful)
        {
            if(!(ResultDoc.empty()))
            {
                Response["Status"] = DUPLICATERECORD;
                Response["Description"] = "Duplicate Record.";
                return crow::response{400 , Response};
            }
        }else
        {
            SHOW_ERROR(FindReturn.Description);
            Response["Status"] = DATABASEERROR;
            Response["Description"] = "Network Internal Service Error.";
            return crow::response{500 , Response};
        }
        
        // 3- Store Image
        std::shared_ptr<storeimage> storeimageobj = std::make_shared<storeimage>();
        if(!(storeimageobj->run(DH)))
        {
            Response["Status"] = DH->Response.errorCode;
            Response["Description"] = DH->Response.Description;
            return crow::response{DH->Response.HTTPCode , Response};
        }

        // 4- Save Data
        
        Response["Status"] = SUCCESSFUL;
        Response["Description"] = "Successful";
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