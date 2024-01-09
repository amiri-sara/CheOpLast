#include "configurate.h"

Configurate* Configurate::InstancePtr = NULL;

Configurate::Configurate()
{
    SystemKeys SysKeys;
    std::string ConfigFilePath      = std::string(CONFIG_FILE_LOCATION);
    std::string ServerPubKey  = SysKeys.GetKeyConfigServerPublic().Key;
    std::string ClientPubKey  = SysKeys.GetKeyConfigClientPublic().Key;
    std::string ConfigDecryptedFile = decryptFile(ConfigFilePath, ServerPubKey, ClientPubKey).DecryptedMessage;

    if(ConfigDecryptedFile.size() == 0) //NO CONFIG FILE
    {
        SHOW_ERROR("Configurate: "<<std::to_string(__LINE__)<<"_There is no config file");
        throw 0;
    }

    try
    {        
        auto ConfigInputJson = crow::json::load(ConfigDecryptedFile);

        this->DatabaseConfig.DatabaseIP       = ConfigInputJson["Aggregation_Config_Database_IP"].s();
        this->DatabaseConfig.DatabasePort     = ConfigInputJson["Aggregation_Config_Database_PORT"].s();
        this->DatabaseConfig.DatabaseUsername = ConfigInputJson["Aggregation_Config_Database_USER"].s();
        this->DatabaseConfig.DatabasePassword = ConfigInputJson["Aggregation_Config_Database_PASS"].s();
        this->DatabaseConfig.DETAIL           = ConfigInputJson["Aggregation_Config_Database_DETAIL"].s();
        this->DatabaseConfig.DatabaseName     = ConfigInputJson["Aggregation_Config_Database_Name"].s();

        this->DatabaseInsert.DatabaseIP       = ConfigInputJson["Aggregation_Insert_Database_IP"].s();
        this->DatabaseInsert.DatabasePort     = ConfigInputJson["Aggregation_Insert_Database_PORT"].s();
        this->DatabaseInsert.DatabaseUsername = ConfigInputJson["Aggregation_Insert_Database_USER"].s();
        this->DatabaseInsert.DatabasePassword = ConfigInputJson["Aggregation_Insert_Database_PASS"].s();
        this->DatabaseInsert.DETAIL           = ConfigInputJson["Aggregation_Insert_Database_DETAIL"].s();
        this->DatabaseInsert.DatabaseName     = ConfigInputJson["Aggregation_Insert_Database_Name"].s();
        this->DatabaseInsert.CollectionName   = ConfigInputJson["Aggregation_Insert_Collection_Name"].s();

#ifdef FAILEDDATABASE
        this->DatabaseFailed.Enable           = ConfigInputJson["Aggregation_Failed_Database_Enable"].s();
        this->DatabaseFailed.DatabaseIP       = ConfigInputJson["Aggregation_Failed_Database_IP"].s();
        this->DatabaseFailed.DatabasePort     = ConfigInputJson["Aggregation_Failed_Database_PORT"].s();
        this->DatabaseFailed.DatabaseUsername = ConfigInputJson["Aggregation_Failed_Database_USER"].s();
        this->DatabaseFailed.DatabasePassword = ConfigInputJson["Aggregation_Failed_Database_PASS"].s();
        this->DatabaseFailed.DETAIL           = ConfigInputJson["Aggregation_Failed_Database_DETAIL"].s();
        this->DatabaseFailed.DatabaseName     = ConfigInputJson["Aggregation_Failed_Database_Name"].s();
        this->DatabaseFailed.CollectionName   = ConfigInputJson["Aggregation_Failed_Collection_Name"].s();
#endif // FAILEDDATABASE
    }
    catch(...)
    {
        SHOW_ERROR("Configurate: "<<std::to_string(__LINE__)<<"_Invalid config file");
        throw 0;
    }

    SHOW_LOG("CONFIG FILE LOADED SUCCESSFULLY");

    auto AggregationConfigconf       {MongoDB::DatabaseConfig()};
    AggregationConfigconf.IP = DatabaseConfig.DatabaseIP;
    AggregationConfigconf.Port = DatabaseConfig.DatabasePort;
    AggregationConfigconf.User = DatabaseConfig.DatabaseUsername;
    AggregationConfigconf.Password = DatabaseConfig.DatabasePassword;

    auto AggregationConfigDatabase = std::make_shared<MongoDB>(AggregationConfigconf);

    std::vector<MongoDB::Field> filter = {};
    MongoDB::FindOptionStruct Option;

    MongoDB::ResponseStruct FindReturn;

    std::vector<std::string> ConfigDoc;
    FindReturn = AggregationConfigDatabase->Find(this->DatabaseConfig.DatabaseName, "Config", filter, Option, ConfigDoc);
    if(FindReturn.Code == MongoDB::MongoStatus::FindSuccessful)
    {
        for(auto& doc : ConfigDoc)
        {
            crow::json::rvalue AggregationConfigJSON = crow::json::load(doc);
            
            this->ReadConfigServiceConfig.URI = AggregationConfigJSON["ReadConfigService"]["URI"].s(); 
            this->ReadConfigServiceConfig.Port = AggregationConfigJSON["ReadConfigService"]["Port"].i(); 
            this->ReadConfigServiceConfig.threadNumber = AggregationConfigJSON["ReadConfigService"]["threadNumber"].i();
        }
    }else
    {
        SHOW_ERROR(FindReturn.Description);
        throw;
    }

    std::vector<std::string> InputDoc;
    FindReturn = AggregationConfigDatabase->Find(this->DatabaseConfig.DatabaseName, "Input", filter, Option, InputDoc);
    if(FindReturn.Code == MongoDB::MongoStatus::FindSuccessful)
    {
        for(auto& doc : InputDoc)
        {
            crow::json::rvalue AggregationInputJSON = crow::json::load(doc);

            crow::json::rvalue InputFieldsJSON = AggregationInputJSON["InputFields"];
            this->InputFields.DeviceID        = InputFieldsJSON["DeviceID"].b();
            this->InputFields.UserID          = InputFieldsJSON["UserID"].b();
            this->InputFields.StreetID        = InputFieldsJSON["StreetID"].b();
            this->InputFields.ViolationID     = InputFieldsJSON["ViolationID"].b();
            this->InputFields.Direction       = InputFieldsJSON["Direction"].b();
            this->InputFields.PlateValue      = InputFieldsJSON["PlateValue"].b();
            this->InputFields.PlateType       = InputFieldsJSON["PlateType"].b();
            this->InputFields.Suspicious      = InputFieldsJSON["Suspicious"].b();
            this->InputFields.Speed           = InputFieldsJSON["Speed"].b();
            this->InputFields.VehicleType     = InputFieldsJSON["VehicleType"].b();
            this->InputFields.VehicleColor    = InputFieldsJSON["VehicleColor"].b();
            this->InputFields.VehicleModel    = InputFieldsJSON["VehicleModel"].b();
            this->InputFields.Lane            = InputFieldsJSON["Lane"].b();
            this->InputFields.PassedTime      = InputFieldsJSON["PassedTime"].b();
            this->InputFields.ColorImage      = InputFieldsJSON["ColorImage"].b();
            this->InputFields.GrayScaleImage  = InputFieldsJSON["GrayScaleImage"].b();
            this->InputFields.PlateImage      = InputFieldsJSON["PlateImage"].b();
            this->InputFields.Latitude        = InputFieldsJSON["Latitude"].b();
            this->InputFields.Longitude       = InputFieldsJSON["Longitude"].b();
            this->InputFields.Accuracy        = InputFieldsJSON["Accuracy"].b();
            this->InputFields.PlateRect       = InputFieldsJSON["PlateRect"].b();
            this->InputFields.CarRect         = InputFieldsJSON["CarRect"].b();
            this->InputFields.CodeType        = InputFieldsJSON["CodeType"].b();
            this->InputFields.MasterPlate     = InputFieldsJSON["MasterPlate"].b();
            this->InputFields.Probability     = InputFieldsJSON["Probability"].b();
            this->InputFields.RecordID        = InputFieldsJSON["RecordID"].b();
            this->InputFields.ReceivedTime    = InputFieldsJSON["ReceivedTime"].b();

#ifdef KAFKASERVICE
            crow::json::rvalue KafkaDocConfigJSON = AggregationInputJSON["KafkaService"];
            this->InputKafkaConfig.BootstrapServers = KafkaDocConfigJSON["BootstrapServers"].s();
            this->InputKafkaConfig.Topic = KafkaDocConfigJSON["Topic"].s();
            this->InputKafkaConfig.GroupID = KafkaDocConfigJSON["GroupID"].s();
            this->InputKafkaConfig.PartitionNumber = KafkaDocConfigJSON["PartitionNumber"].i();
#endif // KAFKASERVICE
#ifdef WEBSERVICE   
            crow::json::rvalue WebServiceArray = AggregationInputJSON["WebService"];
            std::size_t arraySize = WebServiceArray.size();
            for(std::size_t i = 0; i < arraySize; ++i)
            {
                crow::json::rvalue WebserviceConfigJSON = WebServiceArray[i];

                Configurate::WebServiceConfigStruct WebServiceConf;
                WebServiceConf.URI = WebserviceConfigJSON["URI"].s();
                WebServiceConf.Port = WebserviceConfigJSON["Port"].i();
                WebServiceConf.Authentication = WebserviceConfigJSON["Authentication"].b();
                WebServiceConf.TokenTimeAllowed = WebserviceConfigJSON["TokenTimeAllowed"].i();
                WebServiceConf.threadNumber = WebserviceConfigJSON["threadNumber"].i();
                this->WebServiceConfig.push_back(WebServiceConf);
            }
#endif // WEBSERVICE 
        }
    }else
    {
        SHOW_ERROR(FindReturn.Description);
        throw;
    }

    std::vector<std::string> OutputDoc;
    FindReturn = AggregationConfigDatabase->Find(DatabaseConfig.DatabaseName, "Output", filter, Option, OutputDoc);
    if(FindReturn.Code == MongoDB::MongoStatus::FindSuccessful)
    {
        for(auto& doc : OutputDoc)
        {
            crow::json::rvalue OutputConfigJSON = crow::json::load(doc);
#ifdef STOREIMAGE
            crow::json::rvalue StoreImageConfigJSON = OutputConfigJSON["StoreImage"];
            this->StoreImageConfig.StorePath = StoreImageConfigJSON["StorePath"].s();
            this->StoreImageConfig.ColorImageMaxSize = StoreImageConfigJSON["ColorImageMaxSize"].i();
            this->StoreImageConfig.PlateImageMaxSize = StoreImageConfigJSON["PlateImageMaxSize"].i();
            this->StoreImageConfig.AddBanner = StoreImageConfigJSON["AddBanner"].b();
#endif // STOREIMAGE
#ifdef KAFKAOUTPUT
            crow::json::rvalue KafkaConfigJSON = OutputConfigJSON["Kafka"];
            this->OutputKafkaConfig.BootstrapServers = KafkaConfigJSON["BootstrapServers"].s();
            this->OutputKafkaConfig.Topic = KafkaConfigJSON["Topic"].s();
            this->OutputKafkaConfig.GroupID = KafkaConfigJSON["GroupID"].s();
            this->OutputKafkaConfig.PartitionNumber = KafkaConfigJSON["PartitionNumber"].i();
#endif // KAFKAOUTPUT
        }
    }else
    {
        SHOW_ERROR(FindReturn.Description);
        throw;
    }

    this->ReadCamerasCollection();
    
    AggregationConfigDatabase.reset();
}

void Configurate::ReadCamerasCollection()
{
    std::lock_guard<std::shared_mutex> lock(this->UpdateConfig_mutex);
    
    auto Aggregationconf       {MongoDB::DatabaseConfig()};
    Aggregationconf.IP = this->DatabaseInsert.DatabaseIP;
    Aggregationconf.Port = this->DatabaseInsert.DatabasePort;
    Aggregationconf.User = this->DatabaseInsert.DatabaseUsername;
    Aggregationconf.Password = this->DatabaseInsert.DatabasePassword;

    auto AggregationDatabase = std::make_shared<MongoDB>(Aggregationconf);
    
    std::vector<MongoDB::Field> filter = {};
    MongoDB::FindOptionStruct Option;

    MongoDB::ResponseStruct FindReturn;

    std::vector<std::string> CamerasDoc;
    FindReturn = AggregationDatabase->Find(this->DatabaseInsert.DatabaseName, "cameras", filter, Option, CamerasDoc);
    if(FindReturn.Code == MongoDB::MongoStatus::FindSuccessful)
    {
        this->Cameras.clear();
        for(auto& doc : CamerasDoc)
        {
            crow::json::rvalue CameraJSON = crow::json::load(doc);
            
            CameraStruct Camera;
            Camera.DeviceID = CameraJSON["deviceId"].i();
            Camera.Username = CameraJSON["username"].s();
            Camera.Password = CameraJSON["password"].s();

            this->Cameras.push_back(Camera);
        }
    }else
    {
        SHOW_ERROR(FindReturn.Description);
        throw;
    }
    AggregationDatabase.reset();
}

void Configurate::RunUpdateService()
{
    try 
    {   
        this->app = std::make_shared<crow::SimpleApp>();
        this->app->loglevel(crow::LogLevel::Error);
        this->UpdateRoute();

        SHOW_IMPORTANTLOG3("Runinng Update Config Service on port " + std::to_string(this->ReadConfigServiceConfig.Port));
        try{
            //! run object of crow by specific port and many rout in multithread status
            this->app->port(this->ReadConfigServiceConfig.Port).concurrency(this->ReadConfigServiceConfig.threadNumber).run();

        }  catch (...) {
            SHOW_ERROR("port " + std::to_string(this->ReadConfigServiceConfig.Port) + " is busy . Check ports in server table in config database .");
            exit(0);
        }

    }
    catch(boost::exception_detail::clone_impl<boost::exception_detail::error_info_injector<boost::property_tree::ptree_bad_path> > &e)
    {

        SHOW_ERROR("Error Code 0x" + std::to_string(__LINE__) + "JBF_3K92XS543272" + e.what());

        SHOW_ERROR("Can't Run Crow on port "  + std::to_string(this->ReadConfigServiceConfig.Port) + " please check this port ." );

    }
}

void Configurate::UpdateRoute()
{
    std::string Route = this->ReadConfigServiceConfig.URI;
    if(Route[0] != '/')
        Route = "/" + Route;

    this->app->route_dynamic(Route.c_str()).methods(crow::HTTPMethod::POST)([&](const crow::request& req ) { 
        crow::json::rvalue Req = crow::json::load(req.body);
        
        if(Req["collection"].s() == "cameras")
        {
            this->ReadCamerasCollection();

            auto Res    {crow::json::wvalue()};
            Res["Code"]  = 0;
            Res["Description"]  = "Reading cameras collection values was successful";
            return crow::response{200 , Res};

        }else
        {
            auto Res    {crow::json::wvalue()};
            Res["Code"]  = 1;
            Res["Description"]  = "Collection name not valid";
            return crow::response{200 , Res};
        }
    });
}

Configurate::DatabaseStruct Configurate::getDatabaseConfig()
{
    return this->DatabaseConfig;
}

Configurate::DatabaseStruct Configurate::getDatabaseInsert()
{
    return this->DatabaseInsert;
}

Configurate::DatabaseStruct Configurate::getDatabaseFailed()
{
    return this->DatabaseFailed;
}

std::vector<Configurate::WebServiceConfigStruct> Configurate::getWebServiceConfig()
{
    return this->WebServiceConfig;
}

Configurate::StoreImageConfigStruct Configurate::getStoreImageConfig()
{
    return this->StoreImageConfig;
}

Configurate::KafkaConfigStruct Configurate::getInputKafkaConfig()
{
    return this->InputKafkaConfig;
}

Configurate::KafkaConfigStruct Configurate::getOutputKafkaConfig()
{
    return this->OutputKafkaConfig;
}

std::vector<Configurate::CameraStruct> Configurate::getCameras()
{
    std::shared_lock<std::shared_mutex> lock(this->UpdateConfig_mutex); 
    return this->Cameras;
}

Configurate::InputFieldsStruct Configurate::getInputFields()
{
    return this->InputFields;
}
