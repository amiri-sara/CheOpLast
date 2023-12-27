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
    Route += "Insert";
    
    this->app->route_dynamic(Route.c_str()).methods(crow::HTTPMethod::POST)([&](const crow::request& req ) {
        SHOW_IMPORTANTLOG2(req.body);
        
        auto Res    {crow::json::wvalue()};
        Res["res"]  = "Insert Route";

        return crow::response{200 , "Insert Route"};
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