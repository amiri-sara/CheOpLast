#include "webservice.h"

WebService::WebService(Configurate::WebServiceConfigStruct ServiceConfig)
{
    this->app = std::make_shared<crow::SimpleApp>();
    this->app->loglevel(crow::LogLevel::Error);
    this->WebServiceConfig = ServiceConfig;

    Configurate* ConfigurateObj = Configurate::getInstance();
    this->InputFields = ConfigurateObj->getInputFields();
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
    Route += "Insert";
    
    this->app->route_dynamic(Route.c_str()).methods(crow::HTTPMethod::POST)([&](const crow::request& req ) {
        
        //! "startTime" for Computing process time for this request
        auto startTime = std::chrono::high_resolution_clock::now();
        
        std::shared_ptr<DataHandler::DataHandlerStruct> DH = std::make_shared<DataHandler::DataHandlerStruct>();
        DH->Request.body = req.body;

        std::string Host("");
        auto itr = req.headers.find("Host");
        if (itr != req.headers.end())
            Host = itr->second;
        SHOW_IMPORTANTLOG("Recived request from IP -> " + Host);
        DH->Request.Host = Host;

        Configurate* ConfigurateObj = Configurate::getInstance();

        DH->hasInputFields = this->InputFields;
        DH->Cameras = ConfigurateObj->getCameras();
        DH->DaysforPassedTimeAcceptable = this->WebServiceConfig.DaysforPassedTimeAcceptable;

        std::shared_ptr<Validator> Validatorobj = std::make_shared<Validator>();
        if(!(Validatorobj->run(DH)))
            return crow::response{DH->Response.HTTPCode , DH->Response.Description};
        
        return crow::response{DH->Response.HTTPCode , DH->Response.Description};
    });
}

void WebService::TokenRoute()
{
    std::string Route = this->WebServiceConfig.URI;
    if(Route.back() != '/')
        Route += "/";
    if(Route[0] != '/')
        Route = "/" + Route;
    Route += "Token";
    
    this->app->route_dynamic(Route.c_str()).methods(crow::HTTPMethod::POST)([&](const crow::request& req ) {
        SHOW_IMPORTANTLOG2(req.body);
        
        auto Res    {crow::json::wvalue()};
        Res["res"]  = "Token Route";

        return crow::response{200 , "Token Route"};
    });
}