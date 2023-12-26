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

        DatabaseConfig.DatabaseIP       = ConfigInputJson["Aggregation_Config_Database_IP"].s();
        DatabaseConfig.DatabasePort     = ConfigInputJson["Aggregation_Config_Database_PORT"].s();
        DatabaseConfig.DatabaseUsername = ConfigInputJson["Aggregation_Config_Database_USER"].s();
        DatabaseConfig.DatabasePassword = ConfigInputJson["Aggregation_Config_Database_PASS"].s();
        DatabaseConfig.DETAIL           = ConfigInputJson["Aggregation_Config_Database_DETAIL"].s();
        DatabaseConfig.DatabaseName     = ConfigInputJson["Aggregation_Config_Database_Name"].s();

        DatabaseInsert.DatabaseIP       = ConfigInputJson["Aggregation_Insert_Database_IP"].s();
        DatabaseInsert.DatabasePort     = ConfigInputJson["Aggregation_Insert_Database_PORT"].s();
        DatabaseInsert.DatabaseUsername = ConfigInputJson["Aggregation_Insert_Database_USER"].s();
        DatabaseInsert.DatabasePassword = ConfigInputJson["Aggregation_Insert_Database_PASS"].s();
        DatabaseInsert.DETAIL           = ConfigInputJson["Aggregation_Insert_Database_DETAIL"].s();
        DatabaseInsert.DatabaseName     = ConfigInputJson["Aggregation_Insert_Database_Name"].s();
        DatabaseInsert.CollectionName   = ConfigInputJson["Aggregation_Insert_Collection_Name"].s();

        DatabaseFailed.Enable           = ConfigInputJson["Aggregation_Failed_Database_Enable"].s();
        DatabaseFailed.DatabaseIP       = ConfigInputJson["Aggregation_Failed_Database_IP"].s();
        DatabaseFailed.DatabasePort     = ConfigInputJson["Aggregation_Failed_Database_PORT"].s();
        DatabaseFailed.DatabaseUsername = ConfigInputJson["Aggregation_Failed_Database_USER"].s();
        DatabaseFailed.DatabasePassword = ConfigInputJson["Aggregation_Failed_Database_PASS"].s();
        DatabaseFailed.DETAIL           = ConfigInputJson["Aggregation_Failed_Database_DETAIL"].s();
        DatabaseFailed.DatabaseName     = ConfigInputJson["Aggregation_Failed_Database_Name"].s();
        DatabaseFailed.CollectionName   = ConfigInputJson["Aggregation_Failed_Collection_Name"].s();
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
    
    std::vector<std::string> WebserviceDoc;
    auto FindReturn = AggregationConfigDatabase->Find(DatabaseConfig.DatabaseName, "WebService", filter, Option, WebserviceDoc);
    if(FindReturn.Code == MongoDB::MongoStatus::FindSuccessful)
    {
        for(auto& doc : WebserviceDoc)
        {
            Configurate::WebServiceConfigStruct WebServiceConf;
            crow::json::rvalue WebserviceConfigJSON = crow::json::load(doc);
            WebServiceConf.URI = WebserviceConfigJSON["URI"].s();
            WebServiceConf.Port = WebserviceConfigJSON["Port"].i();
            WebServiceConf.Authentication = WebserviceConfigJSON["Authentication"].b();
            WebServiceConf.TokenTimeAllowed = WebserviceConfigJSON["TokenTimeAllowed"].i();
            WebServiceConf.threadNumber = WebserviceConfigJSON["threadNumber"].i();

            this->WebServiceConfig.push_back(WebServiceConf);
        }
    }else
    {
        SHOW_ERROR(FindReturn.Description);
        throw;
    }

    std::vector<std::string> StoreImageDoc;
    FindReturn = AggregationConfigDatabase->Find(DatabaseConfig.DatabaseName, "StoreImage", filter, Option, StoreImageDoc);
    if(FindReturn.Code == MongoDB::MongoStatus::FindSuccessful)
    {
        for(auto& doc : StoreImageDoc)
        {
            crow::json::rvalue StoreImageConfigJSON = crow::json::load(doc);
            StoreImageConfig.StorePath = StoreImageConfigJSON["StorePath"].s();
            StoreImageConfig.ColorImageMaxSize = StoreImageConfigJSON["ColorImageMaxSize"].i();
            StoreImageConfig.PlateImageMaxSize = StoreImageConfigJSON["PlateImageMaxSize"].i();
            StoreImageConfig.AddBanner = StoreImageConfigJSON["AddBanner"].b();
        }
    }else
    {
        SHOW_ERROR(FindReturn.Description);
        throw;
    }

    std::vector<std::string> KafkaDoc;
    FindReturn = AggregationConfigDatabase->Find(DatabaseConfig.DatabaseName, "Kafka", filter, Option, KafkaDoc);
    if(FindReturn.Code == MongoDB::MongoStatus::FindSuccessful)
    {
        for(auto& doc : KafkaDoc)
        {
            crow::json::rvalue KafkaDocConfigJSON = crow::json::load(doc);
            if(KafkaDocConfigJSON["Input"].b())
            {
                InputKafkaConfig.BootstrapServers = KafkaDocConfigJSON["BootstrapServers"].s();
                InputKafkaConfig.Topic = KafkaDocConfigJSON["Topic"].s();
                InputKafkaConfig.GroupID = KafkaDocConfigJSON["GroupID"].s();
                InputKafkaConfig.PartitionNumber = KafkaDocConfigJSON["PartitionNumber"].i();
            }else
            {
                OutputKafkaConfig.BootstrapServers = KafkaDocConfigJSON["BootstrapServers"].s();
                OutputKafkaConfig.Topic = KafkaDocConfigJSON["Topic"].s();
                OutputKafkaConfig.GroupID = KafkaDocConfigJSON["GroupID"].s();
                OutputKafkaConfig.PartitionNumber = KafkaDocConfigJSON["PartitionNumber"].i();
            }
        }
    }else
    {
        SHOW_ERROR(FindReturn.Description);
        throw;
    }

    std::vector<std::string> ServiceFieldsDoc;
    FindReturn = AggregationConfigDatabase->Find(DatabaseConfig.DatabaseName, "ServiceFields", filter, Option, ServiceFieldsDoc);
    if(FindReturn.Code == MongoDB::MongoStatus::FindSuccessful)
    {
        for(auto& doc : ServiceFieldsDoc)
        {
            crow::json::rvalue ServiceFieldsDocJSON = crow::json::load(doc);
            crow::json::wvalue KeysValue(ServiceFieldsDocJSON);

            for(auto& key: KeysValue.keys())
            {
                if(key == "_id")
                    continue;
                this->ServiceFields.servicefields[key] = ServiceFieldsDocJSON[key].s();
            }
        }
    }else
    {
        SHOW_ERROR(FindReturn.Description);
        throw;
    }

    
    auto Aggregationconf       {MongoDB::DatabaseConfig()};
    Aggregationconf.IP = DatabaseInsert.DatabaseIP;
    Aggregationconf.Port = DatabaseInsert.DatabasePort;
    Aggregationconf.User = DatabaseInsert.DatabaseUsername;
    Aggregationconf.Password = DatabaseInsert.DatabasePassword;

    auto AggregationDatabase = std::make_shared<MongoDB>(Aggregationconf);
    
    std::vector<std::string> CamerasDoc;
    FindReturn = AggregationDatabase->Find(DatabaseInsert.DatabaseName, "cameras", filter, Option, CamerasDoc);
    if(FindReturn.Code == MongoDB::MongoStatus::FindSuccessful)
    {
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

    AggregationConfigDatabase.reset();
    AggregationDatabase.reset();
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
    return this->Cameras;
}

Configurate::ServiceFieldsStruct Configurate::getServiceFields()
{
    return this->ServiceFields;
}
