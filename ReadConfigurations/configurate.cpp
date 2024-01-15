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
    
    ConnectDatabaseStruct ConfigDatabaseConnect;
    ConnectDatabaseStruct InsertDatabaseConnect;
    ConnectDatabaseStruct FailedDatabaseConnect;

    try
    {        
        auto ConfigInputJson = crow::json::load(ConfigDecryptedFile);

        ConfigDatabaseConnect.DatabaseIP       = ConfigInputJson["Aggregation_Config_Database_IP"].s();
        ConfigDatabaseConnect.DatabasePort     = ConfigInputJson["Aggregation_Config_Database_PORT"].s();
        ConfigDatabaseConnect.DatabaseUsername = ConfigInputJson["Aggregation_Config_Database_USER"].s();
        ConfigDatabaseConnect.DatabasePassword = ConfigInputJson["Aggregation_Config_Database_PASS"].s();
        ConfigDatabaseConnect.DETAIL           = ConfigInputJson["Aggregation_Config_Database_DETAIL"].s();
        this->ConfigDatabaseInfo.DatabaseName     = ConfigInputJson["Aggregation_Config_Database_Name"].s();

        InsertDatabaseConnect.DatabaseIP       = ConfigInputJson["Aggregation_Insert_Database_IP"].s();
        InsertDatabaseConnect.DatabasePort     = ConfigInputJson["Aggregation_Insert_Database_PORT"].s();
        InsertDatabaseConnect.DatabaseUsername = ConfigInputJson["Aggregation_Insert_Database_USER"].s();
        InsertDatabaseConnect.DatabasePassword = ConfigInputJson["Aggregation_Insert_Database_PASS"].s();
        InsertDatabaseConnect.DETAIL           = ConfigInputJson["Aggregation_Insert_Database_DETAIL"].s();
        this->InsertDatabaseInfo.DatabaseName     = ConfigInputJson["Aggregation_Insert_Database_Name"].s();
        this->InsertDatabaseInfo.CollectionName   = ConfigInputJson["Aggregation_Insert_Collection_Name"].s();

#ifdef FAILEDDATABASE
        this->FailedDatabaseInfo.Enable           = ConfigInputJson["Aggregation_Failed_Database_Enable"].s();
        FailedDatabaseConnect.DatabaseIP       = ConfigInputJson["Aggregation_Failed_Database_IP"].s();
        FailedDatabaseConnect.DatabasePort     = ConfigInputJson["Aggregation_Failed_Database_PORT"].s();
        FailedDatabaseConnect.DatabaseUsername = ConfigInputJson["Aggregation_Failed_Database_USER"].s();
        FailedDatabaseConnect.DatabasePassword = ConfigInputJson["Aggregation_Failed_Database_PASS"].s();
        FailedDatabaseConnect.DETAIL           = ConfigInputJson["Aggregation_Failed_Database_DETAIL"].s();
        this->FailedDatabaseInfo.DatabaseName     = ConfigInputJson["Aggregation_Failed_Database_Name"].s();
        this->FailedDatabaseInfo.CollectionName   = ConfigInputJson["Aggregation_Failed_Collection_Name"].s();
#endif // FAILEDDATABASE
    }
    catch(...)
    {
        SHOW_ERROR("Configurate: "<<std::to_string(__LINE__)<<"_Invalid config file");
        throw 0;
    }

    SHOW_LOG("CONFIG FILE LOADED SUCCESSFULLY");

    auto AggregationConfigconf       {MongoDB::DatabaseConfig()};
    AggregationConfigconf.IP = ConfigDatabaseConnect.DatabaseIP;
    AggregationConfigconf.Port = ConfigDatabaseConnect.DatabasePort;
    AggregationConfigconf.User = ConfigDatabaseConnect.DatabaseUsername;
    AggregationConfigconf.Password = ConfigDatabaseConnect.DatabasePassword;
    this->ConfigDatabase = std::make_shared<MongoDB>(AggregationConfigconf);

    auto AggregationInsertconf       {MongoDB::DatabaseConfig()};
    AggregationInsertconf.IP = InsertDatabaseConnect.DatabaseIP;
    AggregationInsertconf.Port = InsertDatabaseConnect.DatabasePort;
    AggregationInsertconf.User = InsertDatabaseConnect.DatabaseUsername;
    AggregationInsertconf.Password = InsertDatabaseConnect.DatabasePassword;
    this->InsertDatabase = std::make_shared<MongoDB>(AggregationInsertconf);

#ifdef FAILEDDATABASE
    auto AggregationFailedconf       {MongoDB::DatabaseConfig()};
    AggregationFailedconf.IP = FailedDatabaseConnect.DatabaseIP;
    AggregationFailedconf.Port = FailedDatabaseConnect.DatabasePort;
    AggregationFailedconf.User = FailedDatabaseConnect.DatabaseUsername;
    AggregationFailedconf.Password = FailedDatabaseConnect.DatabasePassword;
    this->FailedDatabase = std::make_shared<MongoDB>(AggregationFailedconf);
#endif // FAILEDDATABASE

    std::vector<MongoDB::Field> filter = {};
    MongoDB::FindOptionStruct Option;

    MongoDB::ResponseStruct FindReturn;

    std::vector<std::string> ConfigDoc;
    FindReturn = this->ConfigDatabase->Find(this->ConfigDatabaseInfo.DatabaseName, "Config", filter, Option, ConfigDoc);
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
    FindReturn = this->ConfigDatabase->Find(this->ConfigDatabaseInfo.DatabaseName, "Input", filter, Option, InputDoc);
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
            this->InputFields.ImageAddress    = InputFieldsJSON["ImageAddress"].b();
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
            this->InputKafkaConfig.DaysforPassedTimeAcceptable = KafkaDocConfigJSON["DaysforPassedTimeAcceptable"].i();
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
                WebServiceConf.DaysforPassedTimeAcceptable = WebserviceConfigJSON["DaysforPassedTimeAcceptable"].i();
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
    FindReturn = this->ConfigDatabase->Find(this->ConfigDatabaseInfo.DatabaseName, "Output", filter, Option, OutputDoc);
    if(FindReturn.Code == MongoDB::MongoStatus::FindSuccessful)
    {
        for(auto& doc : OutputDoc)
        {
            crow::json::rvalue OutputConfigJSON = crow::json::load(doc);

            crow::json::rvalue OutputFieldsJSON = OutputConfigJSON["OutputFields"];
            this->OutputFields.DeviceID          = OutputFieldsJSON["DeviceID"].b();
            this->OutputFields.UserID            = OutputFieldsJSON["UserID"].b();
            this->OutputFields.StreetID          = OutputFieldsJSON["StreetID"].b();
            this->OutputFields.ViolationID       = OutputFieldsJSON["ViolationID"].b();
            this->OutputFields.Direction         = OutputFieldsJSON["Direction"].b();
            this->OutputFields.PlateValue        = OutputFieldsJSON["PlateValue"].b();
            this->OutputFields.PlateType         = OutputFieldsJSON["PlateType"].b();
            this->OutputFields.Suspicious        = OutputFieldsJSON["Suspicious"].b();
            this->OutputFields.Speed             = OutputFieldsJSON["Speed"].b();
            this->OutputFields.VehicleType       = OutputFieldsJSON["VehicleType"].b();
            this->OutputFields.VehicleColor      = OutputFieldsJSON["VehicleColor"].b();
            this->OutputFields.VehicleModel      = OutputFieldsJSON["VehicleModel"].b();
            this->OutputFields.Lane              = OutputFieldsJSON["Lane"].b();
            this->OutputFields.PassedTime        = OutputFieldsJSON["PassedTime"].b();
            this->OutputFields.ColorImage        = OutputFieldsJSON["ColorImage"].b();
            this->OutputFields.ImageAddress      = OutputFieldsJSON["ImageAddress"].b();
            this->OutputFields.GrayScaleImage    = OutputFieldsJSON["GrayScaleImage"].b();
            this->OutputFields.PlateImage        = OutputFieldsJSON["PlateImage"].b();
            this->OutputFields.Latitude          = OutputFieldsJSON["Latitude"].b();
            this->OutputFields.Longitude         = OutputFieldsJSON["Longitude"].b();
            this->OutputFields.Accuracy          = OutputFieldsJSON["Accuracy"].b();
            this->OutputFields.PlateRect         = OutputFieldsJSON["PlateRect"].b();
            this->OutputFields.CarRect           = OutputFieldsJSON["CarRect"].b();
            this->OutputFields.CodeType          = OutputFieldsJSON["CodeType"].b();
            this->OutputFields.MasterPlate       = OutputFieldsJSON["MasterPlate"].b();
            this->OutputFields.Probability       = OutputFieldsJSON["Probability"].b();
            this->OutputFields.RecordID          = OutputFieldsJSON["RecordID"].b();
            this->OutputFields.ReceivedTime      = OutputFieldsJSON["ReceivedTime"].b();

#ifdef STOREIMAGE
            crow::json::rvalue StoreImageConfigJSON = OutputConfigJSON["StoreImage"];
            this->StoreImageConfig.StorePath = StoreImageConfigJSON["StorePath"].s();
            this->StoreImageConfig.ColorImageMaxSize = StoreImageConfigJSON["ColorImageMaxSize"].i();
            this->StoreImageConfig.PlateImageMaxSize = StoreImageConfigJSON["PlateImageMaxSize"].i();
            
            crow::json::rvalue BannerJSON = StoreImageConfigJSON["Banner"];
            crow::json::wvalue WBannerJSON(BannerJSON);
            this->StoreImageConfig.AddBanner = BannerJSON["active"].b();
            this->StoreImageConfig.FontAddress = BannerJSON["fontAddress"].s();            
            std::vector<std::string> BannerKeys = WBannerJSON.keys();
            for(auto& key : BannerKeys)
            {
                if(key == "active" || key == "fontAddress")
                    continue;
                
                int ViolationID = std::stoi(key);
                Configurate::ViolationStruct VS;
                VS.Description = BannerJSON[key]["Description"].s();
                VS.ImageSuffix = BannerJSON[key]["Suffix"].s();
                
                this->ViolationMap[ViolationID] = VS;
            }
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
}

void Configurate::ReadCamerasCollection()
{
    std::lock_guard<std::shared_mutex> lock(this->UpdateConfig_mutex);
    
    std::vector<MongoDB::Field> filter = {};
    MongoDB::FindOptionStruct Option;

    std::vector<std::string> SystemDoc;
    MongoDB::ResponseStruct FindReturn = this->InsertDatabase->Find(this->InsertDatabaseInfo.DatabaseName, "systems", filter, Option, SystemDoc);
    if(FindReturn.Code == MongoDB::MongoStatus::FindSuccessful)
    {
        this->Cameras.clear();
        for(auto& doc : SystemDoc)
        {
            crow::json::rvalue SystemJSON = crow::json::load(doc);

            std::string ID = SystemJSON["_id"]["$oid"].s();
            std::vector<MongoDB::Field> devicefilter = 
            {
                {"systemId", ID, MongoDB::FieldType::ObjectId, "$gte"},
                {"systemId", ID, MongoDB::FieldType::ObjectId, "$lte"}
            };
                
            std::vector<std::string> DeviceDoc;
            MongoDB::ResponseStruct DeviceFindReturn = this->InsertDatabase->Find(this->InsertDatabaseInfo.DatabaseName, "cameras", devicefilter, Option, DeviceDoc);
            if(DeviceFindReturn.Code == MongoDB::MongoStatus::FindSuccessful)
            {
                for(auto& doc : DeviceDoc)
                {
                    crow::json::rvalue CameraJSON = crow::json::load(doc);
            
                    CameraStruct Camera;
                    Camera.DeviceID     = CameraJSON["deviceId"].i();
                    Camera.Username     = CameraJSON["username"].s();
                    Camera.Password     = CameraJSON["password"].s();
                    Camera.Location     = SystemJSON["location"].s();
                    Camera.PoliceCode   = SystemJSON["policeCode"].i();
                    Camera.AllowedSpeed = SystemJSON["allowedSpeed"].i();

                    this->Cameras.push_back(Camera);
                }
            }else
            {
                SHOW_ERROR(DeviceFindReturn.Description);
                throw;
            }
        }
    }else
    {
        SHOW_ERROR(FindReturn.Description);
        throw;
    }
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

Configurate::InfoDatabaseStruct Configurate::getInsertDatabaseInfo()
{
    return this->InsertDatabaseInfo;
}

Configurate::InfoDatabaseStruct Configurate::getFailedDatabaseInfo()
{
    return this->FailedDatabaseInfo;
}

std::shared_ptr<MongoDB> Configurate::getInsertDatabase()
{
    return this->InsertDatabase;
}

std::shared_ptr<MongoDB> Configurate::getFailedDatabase()
{
    return this->FailedDatabase;
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

Configurate::FieldsStruct Configurate::getInputFields()
{
    return this->InputFields;
}

Configurate::FieldsStruct Configurate::getOutputFields()
{
    return this->OutputFields;
}

std::unordered_map<int, Configurate::ViolationStruct> Configurate::getViolationMap()
{
    return this->ViolationMap;
}
