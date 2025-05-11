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
        ConfigDatabaseConnect.TLSMode          = ConfigInputJson["Aggregation_Config_Database_TLS"].i();
        ConfigDatabaseConnect.DETAIL           = ConfigInputJson["Aggregation_Config_Database_DETAIL"].s();
        this->ConfigDatabaseInfo.DatabaseName     = ConfigInputJson["Aggregation_Config_Database_Name"].s();

        InsertDatabaseConnect.DatabaseIP       = ConfigInputJson["Aggregation_Insert_Database_IP"].s();
        InsertDatabaseConnect.DatabasePort     = ConfigInputJson["Aggregation_Insert_Database_PORT"].s();
        InsertDatabaseConnect.DatabaseUsername = ConfigInputJson["Aggregation_Insert_Database_USER"].s();
        InsertDatabaseConnect.DatabasePassword = ConfigInputJson["Aggregation_Insert_Database_PASS"].s();
        InsertDatabaseConnect.TLSMode          = ConfigInputJson["Aggregation_Insert_Database_TLS"].i();
        InsertDatabaseConnect.DETAIL           = ConfigInputJson["Aggregation_Insert_Database_DETAIL"].s();
        this->InsertDatabaseInfo.DatabaseName     = ConfigInputJson["Aggregation_Insert_Database_Name"].s();
        this->InsertDatabaseInfo.CollectionName   = ConfigInputJson["Aggregation_Insert_Collection_Name"].s();

#ifdef FAILEDDATABASE
        this->FailedDatabaseInfo.Enable           = ConfigInputJson["Aggregation_Failed_Database_Enable"].i();
        FailedDatabaseConnect.DatabaseIP       = ConfigInputJson["Aggregation_Failed_Database_IP"].s();
        FailedDatabaseConnect.DatabasePort     = ConfigInputJson["Aggregation_Failed_Database_PORT"].s();
        FailedDatabaseConnect.DatabaseUsername = ConfigInputJson["Aggregation_Failed_Database_USER"].s();
        FailedDatabaseConnect.DatabasePassword = ConfigInputJson["Aggregation_Failed_Database_PASS"].s();
        FailedDatabaseConnect.TLSMode          = ConfigInputJson["Aggregation_Failed_Database_TLS"].i();
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
    AggregationConfigconf.TLSMode  = ConfigDatabaseConnect.TLSMode;
    AggregationConfigconf.Detail   = ConfigDatabaseConnect.DETAIL;
    this->ConfigDatabase = std::make_shared<MongoDB>(AggregationConfigconf);

    auto AggregationInsertconf       {MongoDB::DatabaseConfig()};
    AggregationInsertconf.IP = InsertDatabaseConnect.DatabaseIP;
    AggregationInsertconf.Port = InsertDatabaseConnect.DatabasePort;
    AggregationInsertconf.User = InsertDatabaseConnect.DatabaseUsername;
    AggregationInsertconf.Password = InsertDatabaseConnect.DatabasePassword;
    AggregationInsertconf.TLSMode  = InsertDatabaseConnect.TLSMode;
    AggregationInsertconf.Detail   = InsertDatabaseConnect.DETAIL;
    this->InsertDatabase = std::make_shared<MongoDB>(AggregationInsertconf);

#ifdef FAILEDDATABASE
    auto AggregationFailedconf       {MongoDB::DatabaseConfig()};
    AggregationFailedconf.IP = FailedDatabaseConnect.DatabaseIP;
    AggregationFailedconf.Port = FailedDatabaseConnect.DatabasePort;
    AggregationFailedconf.User = FailedDatabaseConnect.DatabaseUsername;
    AggregationFailedconf.Password = FailedDatabaseConnect.DatabasePassword;
    AggregationFailedconf.TLSMode  = FailedDatabaseConnect.TLSMode;
    AggregationFailedconf.Detail   = FailedDatabaseConnect.DETAIL;
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
            
            this->ReadConfigServiceConfig.ReadCamerasCollectionServiceInfo.URI = AggregationConfigJSON["ReadConfigService"]["ReadCamerasCollectionURI"].s(); 
            this->ReadConfigServiceConfig.ReadCamerasCollectionServiceInfo.Port = AggregationConfigJSON["ReadConfigService"]["Port"].i(); 
            this->ReadConfigServiceConfig.SetNewTokenServiceInfo.URI = AggregationConfigJSON["ReadConfigService"]["SetNewTokenURI"].s(); 
            this->ReadConfigServiceConfig.SetNewTokenServiceInfo.Port = AggregationConfigJSON["ReadConfigService"]["Port"].i(); 
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
            this->InputFields.SystemCode      = InputFieldsJSON["SystemCode"].b();
            this->InputFields.CompanyCode     = InputFieldsJSON["CompanyCode"].b();

#ifdef KAFKASERVICE
            crow::json::rvalue KafkaDocConfigJSON = AggregationInputJSON["KafkaService"];
            this->InputKafkaConfig.BootstrapServers = KafkaDocConfigJSON["BootstrapServers"].s();
            this->InputKafkaConfig.Topic = KafkaDocConfigJSON["Topic"].s();
            this->InputKafkaConfig.GroupID = KafkaDocConfigJSON["GroupID"].s();
            this->InputKafkaConfig.PartitionNumber = KafkaDocConfigJSON["PartitionNumber"].i();
            this->InputKafkaConfig.DaysforPassedTimeAcceptable = KafkaDocConfigJSON["DaysforPassedTimeAcceptable"].i();
#endif // KAFKASERVICE

#ifdef CLIENTSERVICE
            crow::json::rvalue ClientServiceArray = AggregationInputJSON["ClientService"];
            std::size_t clientArraySize = ClientServiceArray.size();
            for(std::size_t i =0 ; i < clientArraySize; ++i)
            {
                crow::json::rvalue ClientServiceConfigJSON = ClientServiceArray[i];
                Configurate::ClientServiceConfigStruct ClientServiceConf;
                ClientServiceConf.ClientServiceInfo.URI = ClientServiceConfigJSON["URI"].s();
                ClientServiceConf.ClientServiceInfo.Port = ClientServiceConfigJSON["Port"].i();
                ClientServiceConf.ThreadNumber = ClientServiceConfigJSON["ThreadNumber"].i();
                ClientServiceConf.ThresholdFetchedRecors = ClientServiceConfigJSON["ThresholdFetchedRecors"].i();
                this->ClientServiceConfig.push_back(ClientServiceConf);

            }
#endif // CLIENTSERVICE
#ifdef WEBSERVICE   
            crow::json::rvalue WebServiceArray = AggregationInputJSON["WebService"];
            std::size_t arraySize = WebServiceArray.size();
            for(std::size_t i = 0; i < arraySize; ++i)
            {
                crow::json::rvalue WebserviceConfigJSON = WebServiceArray[i];

                Configurate::WebServiceConfigStruct WebServiceConf;
                WebServiceConf.WebServiceInfo.URI = WebserviceConfigJSON["URI"].s();
                WebServiceConf.WebServiceInfo.Port = WebserviceConfigJSON["Port"].i();
                WebServiceConf.threadNumber = WebserviceConfigJSON["threadNumber"].i();
                WebServiceConf.DaysforPassedTimeAcceptable = WebserviceConfigJSON["DaysforPassedTimeAcceptable"].i();
                WebServiceConf.Authentication = WebserviceConfigJSON["Authentication"]["active"].b();
                WebServiceConf.TokenTimeAllowed = WebserviceConfigJSON["Authentication"]["TokenTimeAllowed"].i();
                WebServiceConf.NotifyingOtherServicesTokenUpdate = WebserviceConfigJSON["Authentication"]["NotifyingOtherServicesTokenUpdate"].b();
                WebServiceConf.KeysPath = WebserviceConfigJSON["Authentication"]["KeysPath"].s();
                crow::json::rvalue OtherServiceArray = WebserviceConfigJSON["Authentication"]["OtherServices"];
                std::size_t OtherServicearraySize = OtherServiceArray.size();
                for(int j = 0; j < OtherServicearraySize; j++)
                {
                    crow::json::rvalue OtherserviceInfoJSON = OtherServiceArray[j];
                    
                    Configurate::WebServiceInfoStruct WebServiceInfo;
                    WebServiceInfo.IP   = OtherserviceInfoJSON["IP"].s();
                    WebServiceInfo.URI  = OtherserviceInfoJSON["URI"].s();
                    WebServiceInfo.Port = OtherserviceInfoJSON["Port"].i();

                    WebServiceConf.OtherService.push_back(WebServiceInfo);
                }
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
            this->OutputFields.PassedVehicleRecordsId = OutputFieldsJSON["PassedVehicleRecordsId"].b();
            this->OutputFields.CompanyCode       = OutputFieldsJSON["CompanyCode"].b();
            this->OutputFields.SystemCode        = OutputFieldsJSON["SystemCode"].b();


            crow::json::rvalue StoreImageConfigJSON = OutputConfigJSON["StoreImage"];
            this->StoreImageConfig.ColorImageMaxSize = StoreImageConfigJSON["ColorImageMaxSize"].i();
            this->StoreImageConfig.PlateImageMaxSize = StoreImageConfigJSON["PlateImageMaxSize"].i();
            this->StoreImageConfig.PlateImagePercent = StoreImageConfigJSON["PlateImagePercent"].i();
#ifdef STOREIMAGE
            this->StoreImageConfig.StorePath = StoreImageConfigJSON["StorePath"].s();
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

    std::vector<std::string> MetaDoc;
    FindReturn = this->ConfigDatabase->Find(this->ConfigDatabaseInfo.DatabaseName, "Meta", filter, Option, MetaDoc);
    if(FindReturn.Code == MongoDB::MongoStatus::FindSuccessful)
    {
        for(auto &doc : MetaDoc)
        {
            crow::json::rvalue AggregationConfigJSON = crow::json::load(doc);
            if(AggregationConfigJSON["_id"] == "last_processed_id")
                this->Meta.last_processed_id = static_cast<uint64_t>(AggregationConfigJSON["PassedVehicleRecordsId"].i());

        }

    }else{
        SHOW_ERROR(FindReturn.Description);
        throw;
    }

    std::vector<std::string> ModulesDoc;
    FindReturn = this->ConfigDatabase->Find(this->ConfigDatabaseInfo.DatabaseName, "Modules", filter, Option, ModulesDoc);
    if(FindReturn.Code == MongoDB::MongoStatus::FindSuccessful)
    {
        for(auto& doc : ModulesDoc)
        {   
            crow::json::rvalue ModulesConfigJSON = crow::json::load(doc);

            crow::json::rvalue CheckOperatorJSON = ModulesConfigJSON["CheckOperator"];
            this->Modules.CheckOperator.active = CheckOperatorJSON["active"].b();
            this->Modules.CheckOperator.NumberOfObjectPerService = CheckOperatorJSON["NumberOfObjectPerService"].i();
            this->Modules.CheckOperator.NumberOfThreadPerService = CheckOperatorJSON["NumberOfThreadPerService"].i();
            this->Modules.CheckOperator.ModelsPath = CheckOperatorJSON["ModelsPath"].s();
            this->Modules.CheckOperator.IgnoreInputPlateType = CheckOperatorJSON["IgnoreInputPlateType"].b();
            if(this->Modules.CheckOperator.active)
            {
                MongoDB::FindOptionStruct Option;
                
                crow::json::rvalue PDConfig = CheckOperatorJSON["PD"];
                std::string PDid = PDConfig["model"]["$oid"].s();
                std::vector<MongoDB::Field> Modelfilter = {
                    // equal
                    {"_id", PDid, MongoDB::FieldType::ObjectId, "$gte"},
                    {"_id", PDid, MongoDB::FieldType::ObjectId, "$lte"}

                };
                std::vector<std::string> PDResultDoc;
                FindReturn = this->ConfigDatabase->Find(this->ConfigDatabaseInfo.DatabaseName, "Models", Modelfilter, Option, PDResultDoc);
                if(FindReturn.Code == MongoDB::MongoStatus::FindSuccessful)
                {
                    if(PDResultDoc.size() != 1)
                    {
                        SHOW_ERROR(PDid << " Model does not exist");
                        throw;
                    }
                    for(auto& doc : PDResultDoc)
                    {
                        crow::json::rvalue ModelsConfigJSON = crow::json::load(doc);
                        this->Modules.CheckOperator.PD.model = ModelsConfigJSON["Name"].s();
                        this->Modules.CheckOperator.PD.modelConfigPath = ModelsConfigJSON["Config"].s(); 
                        this->Modules.CheckOperator.PD.active = PDConfig["active"].b();
                    }
                }else
                {
                    SHOW_ERROR(FindReturn.Description);
                    throw;
                }

                crow::json::rvalue PCConfig = CheckOperatorJSON["PC"];
                std::string PCid = PCConfig["model"]["$oid"].s();
                Modelfilter = {
                    // equal
                    {"_id", PCid, MongoDB::FieldType::ObjectId, "$gte"},
                    {"_id", PCid, MongoDB::FieldType::ObjectId, "$lte"}

                };
                std::vector<std::string> PCResultDoc;
                FindReturn = this->ConfigDatabase->Find(this->ConfigDatabaseInfo.DatabaseName, "Models", Modelfilter, Option, PCResultDoc);
                if(FindReturn.Code == MongoDB::MongoStatus::FindSuccessful)
                {
                    if(PCResultDoc.size() != 1)
                    {
                        SHOW_ERROR(PDid << " Model does not exist");
                        throw;
                    }
                    for(auto& doc : PCResultDoc)
                    {
                        crow::json::rvalue ModelsConfigJSON = crow::json::load(doc);
                        this->Modules.CheckOperator.PC.model = ModelsConfigJSON["Name"].s();
                        this->Modules.CheckOperator.PC.modelConfigPath = ModelsConfigJSON["Config"].s(); 
                        this->Modules.CheckOperator.PC.active = PCConfig["active"].b();
                    }
                }else
                {
                    SHOW_ERROR(FindReturn.Description);
                    throw;
                }

                crow::json::rvalue IROCRConfig = CheckOperatorJSON["IROCR"];
                std::string IROCRid = IROCRConfig["model"]["$oid"].s();
                Modelfilter = {
                    // equal
                    {"_id", IROCRid, MongoDB::FieldType::ObjectId, "$gte"},
                    {"_id", IROCRid, MongoDB::FieldType::ObjectId, "$lte"}
                };
                std::vector<std::string> IROCRResultDoc;
                FindReturn = this->ConfigDatabase->Find(this->ConfigDatabaseInfo.DatabaseName, "Models", Modelfilter, Option, IROCRResultDoc);
                if(FindReturn.Code == MongoDB::MongoStatus::FindSuccessful)
                {
                    if(IROCRResultDoc.size() != 1)
                    {
                        SHOW_ERROR(PDid << " Model does not exist");
                        throw;
                    }
                    for(auto& doc : IROCRResultDoc)
                    {
                        crow::json::rvalue ModelsConfigJSON = crow::json::load(doc);
                        this->Modules.CheckOperator.IROCR.model = ModelsConfigJSON["Name"].s();
                        this->Modules.CheckOperator.IROCR.modelConfigPath = ModelsConfigJSON["Config"].s(); 
                        this->Modules.CheckOperator.IROCR.active = IROCRConfig["active"].b();
                    }
                }else
                {
                    SHOW_ERROR(FindReturn.Description);
                    throw;
                }

                crow::json::rvalue MBOCRConfig = CheckOperatorJSON["MBOCR"];
                std::string MBOCRid = MBOCRConfig["model"]["$oid"].s();
                Modelfilter = {
                    // equal
                    {"_id", MBOCRid, MongoDB::FieldType::ObjectId, "$gte"},
                    {"_id", MBOCRid, MongoDB::FieldType::ObjectId, "$lte"}
                };
                std::vector<std::string> MBOCRResultDoc;
                FindReturn = this->ConfigDatabase->Find(this->ConfigDatabaseInfo.DatabaseName, "Models", Modelfilter, Option, MBOCRResultDoc);
                if(FindReturn.Code == MongoDB::MongoStatus::FindSuccessful)
                {
                    if(MBOCRResultDoc.size() != 1)
                    {
                        SHOW_ERROR(PDid << " Model does not exist");
                        throw;
                    }
                    for(auto& doc : MBOCRResultDoc)
                    {
                        crow::json::rvalue ModelsConfigJSON = crow::json::load(doc);
                        this->Modules.CheckOperator.MBOCR.model = ModelsConfigJSON["Name"].s();
                        this->Modules.CheckOperator.MBOCR.modelConfigPath = ModelsConfigJSON["Config"].s(); 
                        this->Modules.CheckOperator.MBOCR.active = MBOCRConfig["active"].b();
                    }
                }else
                {
                    SHOW_ERROR(FindReturn.Description);
                    throw;
                }

                crow::json::rvalue TZOCRConfig = CheckOperatorJSON["TZOCR"];
                std::string TZOCRid = TZOCRConfig["model"]["$oid"].s();
                Modelfilter = {
                    // equal
                    {"_id", TZOCRid, MongoDB::FieldType::ObjectId, "$gte"},
                    {"_id", TZOCRid, MongoDB::FieldType::ObjectId, "$lte"}
                };
                std::vector<std::string> TZOCRResultDoc;
                FindReturn = this->ConfigDatabase->Find(this->ConfigDatabaseInfo.DatabaseName, "Models", Modelfilter, Option, TZOCRResultDoc);
                if(FindReturn.Code == MongoDB::MongoStatus::FindSuccessful)
                {
                    if(TZOCRResultDoc.size() != 1)
                    {
                        SHOW_ERROR(PDid << " Model does not exist");
                        throw;
                    }
                    for(auto& doc : TZOCRResultDoc)
                    {
                        crow::json::rvalue ModelsConfigJSON = crow::json::load(doc);
                        this->Modules.CheckOperator.TZOCR.model = ModelsConfigJSON["Name"].s();
                        this->Modules.CheckOperator.TZOCR.modelConfigPath = ModelsConfigJSON["Config"].s(); 
                        this->Modules.CheckOperator.TZOCR.active = TZOCRConfig["active"].b();
                    }
                }else
                {
                    SHOW_ERROR(FindReturn.Description);
                    throw;
                }

                crow::json::rvalue FZOCRConfig = CheckOperatorJSON["FZOCR"];
                std::string FZOCRid = FZOCRConfig["model"]["$oid"].s();
                Modelfilter = {
                    // equal
                    {"_id", FZOCRid, MongoDB::FieldType::ObjectId, "$gte"},
                    {"_id", FZOCRid, MongoDB::FieldType::ObjectId, "$lte"}
                };
                std::vector<std::string> FZOCRResultDoc;
                FindReturn = this->ConfigDatabase->Find(this->ConfigDatabaseInfo.DatabaseName, "Models", Modelfilter, Option, FZOCRResultDoc);
                if(FindReturn.Code == MongoDB::MongoStatus::FindSuccessful)
                {
                    if(FZOCRResultDoc.size() != 1)
                    {
                        SHOW_ERROR(PDid << " Model does not exist");
                        throw;
                    }
                    for(auto& doc : FZOCRResultDoc)
                    {
                        crow::json::rvalue ModelsConfigJSON = crow::json::load(doc);
                        this->Modules.CheckOperator.FZOCR.model = ModelsConfigJSON["Name"].s();
                        this->Modules.CheckOperator.FZOCR.modelConfigPath = ModelsConfigJSON["Config"].s(); 
                        this->Modules.CheckOperator.FZOCR.active = FZOCRConfig["active"].b();
                    }
                }else
                {
                    SHOW_ERROR(FindReturn.Description);
                    throw;
                }

                crow::json::rvalue FROCRConfig = CheckOperatorJSON["FROCR"];
                std::string FROCRid = FROCRConfig["model"]["$oid"].s();
                Modelfilter = {
                    // equal
                    {"_id", FROCRid, MongoDB::FieldType::ObjectId, "$gte"},
                    {"_id", FROCRid, MongoDB::FieldType::ObjectId, "$lte"}
                };
                std::vector<std::string> FROCRResultDoc;
                FindReturn = this->ConfigDatabase->Find(this->ConfigDatabaseInfo.DatabaseName, "Models", Modelfilter, Option, FROCRResultDoc);
                if(FindReturn.Code == MongoDB::MongoStatus::FindSuccessful)
                {
                    if(FROCRResultDoc.size() != 1)
                    {
                        SHOW_ERROR(PDid << " Model does not exist");
                        throw;
                    }
                    for(auto& doc : FROCRResultDoc)
                    {
                        crow::json::rvalue ModelsConfigJSON = crow::json::load(doc);
                        this->Modules.CheckOperator.FROCR.model = ModelsConfigJSON["Name"].s();
                        this->Modules.CheckOperator.FROCR.modelConfigPath = ModelsConfigJSON["Config"].s(); 
                        this->Modules.CheckOperator.FROCR.active = FROCRConfig["active"].b();
                    }
                }else
                {
                    SHOW_ERROR(FindReturn.Description);
                    throw;
                }
            }
        
            crow::json::rvalue ClassifierJSON = ModulesConfigJSON["Classifier"];
            this->Modules.Classifier.active = ClassifierJSON["active"].b();
            this->Modules.Classifier.NumberOfObjectPerService = ClassifierJSON["NumberOfObjectPerService"].i();
            this->Modules.Classifier.ModelsPath = ClassifierJSON["ModelsPath"].s();

            if(this->Modules.Classifier.active)
            {
                crow::json::rvalue ModelsArray = ClassifierJSON["Models"];
                std::size_t arraySize = ModelsArray.size();
                for(std::size_t i = 0; i < arraySize; ++i)
                {
                    crow::json::rvalue ClassifierModelConfigJSON = ModelsArray[i];
                    Configurate::ClassifierModelConfigStruct ModelConfig;

                    ModelConfig.active = ClassifierModelConfigJSON["active"].b();
                    if(ModelConfig.active)
                    {
                        std::string Modelid = ClassifierModelConfigJSON["model"]["$oid"].s();
                        std::vector<MongoDB::Field> Modelfilter = {
                            // equal
                            {"_id", Modelid, MongoDB::FieldType::ObjectId, "$gte"},
                            {"_id", Modelid, MongoDB::FieldType::ObjectId, "$lte"}

                        };
                        std::vector<std::string> modelResultDoc;
                        FindReturn = this->ConfigDatabase->Find(this->ConfigDatabaseInfo.DatabaseName, "Models", Modelfilter, Option, modelResultDoc);
                        if(FindReturn.Code == MongoDB::MongoStatus::FindSuccessful)
                        {
                            if(modelResultDoc.size() != 1)
                            {
                                SHOW_ERROR(Modelid << " Model does not exist");
                                throw;
                            }
                            for(auto& doc : modelResultDoc)
                            {
                                crow::json::rvalue ModelsConfigJSON = crow::json::load(doc);
                                ModelConfig.model = ModelsConfigJSON["Name"].s();
                                ModelConfig.modelConfigPath = ModelsConfigJSON["Config"].s();
                                ModelConfig.InputImageType = ClassifierModelConfigJSON["InputImageType"].i();
                                ModelConfig.UseRect = ClassifierModelConfigJSON["UseRect"].b();
                                ModelConfig.InputRectField = ClassifierModelConfigJSON["InputRectField"].i();
                                this->Modules.Classifier.Models.push_back(ModelConfig);
                            }
                        }else
                        {
                            SHOW_ERROR(FindReturn.Description);
                            throw;
                        }
                    }
                } 
            }  
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

    std::map<std::string , std::string> CompaniesMap;
    std::vector<std::string> CompaniesDoc;
    MongoDB::ResponseStruct CompaniesFindReturn = this->InsertDatabase->Find(this->InsertDatabaseInfo.DatabaseName, "companies", filter, Option, CompaniesDoc);
    if(CompaniesFindReturn.Code == MongoDB::MongoStatus::FindSuccessful)
    {
        for(auto& doc : CompaniesDoc)
        {
            crow::json::rvalue CompaniesJSON = crow::json::load(doc);
            std::string ID = CompaniesJSON["_id"]["$oid"].s();
            std::string faName = CompaniesJSON["faName"].s();
            CompaniesMap[ID] = faName;
        }
    } else
    {
        SHOW_ERROR(CompaniesFindReturn.Description);
        throw;
    }

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
                    Camera.DeviceID            = CameraJSON["deviceId"].i();
                    Camera.Username            = CameraJSON["username"].s();
                    Camera.Password            = CameraJSON["password"].s();
                    Camera.TokenInfo.Counter   = CameraJSON["tokenCounter"].i();
                    Camera.TokenInfo.Token     = CameraJSON["token"].s();
                    Camera.Location            = SystemJSON["location"].s();
                    Camera.PoliceCode          = SystemJSON["policeCode"].i();
                    Camera.AllowedSpeed        = SystemJSON["allowedSpeed"].i();
                    Camera.subMode             = SystemJSON["subMode"].s();
                    Camera.addBanner           = SystemJSON["addBanner"].b();
                    Camera.addCrop             = SystemJSON["addCrop"].b();
                    Camera.CompanyID           = SystemJSON["companyId"]["$oid"].s();
                    Camera.CompanyName         = CompaniesMap[SystemJSON["companyId"]["$oid"].s()];
                    
                    ConvertISO8601TimeToUnix(CameraJSON["tokenCreatedAt"]["$date"].s(), Camera.TokenInfo.CreatedAt);

                    this->Cameras.push_back(Camera);
                }
            }else
            {
                SHOW_ERROR(DeviceFindReturn.Description);
                throw;
            }
        }
    } else
    {
        SHOW_ERROR(FindReturn.Description);
        throw;
    }
}

void Configurate::SetNewToken(int CameraIndex, std::string Token, std::time_t TokenTime)
{
    this->Cameras[CameraIndex].TokenInfo.Token = Token;
    this->Cameras[CameraIndex].TokenInfo.CreatedAt = TokenTime;
    this->Cameras[CameraIndex].TokenInfo.Counter += 1;
}

void Configurate::RunUpdateService()
{
    try 
    {   

        std::cout<<"DEbug"<<std::endl;
        this->app = std::make_shared<crow::SimpleApp>();
        this->app->loglevel(crow::LogLevel::Error);
        this->ReadCamerasCollectionRoute();
        this->SetNewTokenRoute();

        SHOW_IMPORTANTLOG3("Runinng Update Config Service on port " + std::to_string(this->ReadConfigServiceConfig.ReadCamerasCollectionServiceInfo.Port));
        try{
            //! run object of crow by specific port and many rout in multithread status
            this->app->port(this->ReadConfigServiceConfig.ReadCamerasCollectionServiceInfo.Port).concurrency(this->ReadConfigServiceConfig.threadNumber).run();

        }  catch (...) {
            SHOW_ERROR("port " + std::to_string(this->ReadConfigServiceConfig.ReadCamerasCollectionServiceInfo.Port) + " is busy . Check ports in server table in config database .");
            exit(0);
        }

    }
    catch(boost::exception_detail::clone_impl<boost::exception_detail::error_info_injector<boost::property_tree::ptree_bad_path> > &e)
    {

        SHOW_ERROR("Error Code 0x" + std::to_string(__LINE__) + "JBF_3K92XS543272" + e.what());

        SHOW_ERROR("Can't Run Crow on port "  + std::to_string(this->ReadConfigServiceConfig.ReadCamerasCollectionServiceInfo.Port) + " please check this port ." );

    }
}

void Configurate::ReadCamerasCollectionRoute()
{
    std::string Route = this->ReadConfigServiceConfig.ReadCamerasCollectionServiceInfo.URI;
    if(Route[0] != '/')
        Route = "/" + Route;

    this->app->route_dynamic(Route.c_str()).methods(crow::HTTPMethod::POST)([&](const crow::request& req ) { 
        crow::json::rvalue Req = crow::json::load(req.body);
        SHOW_IMPORTANTLOG("Recived Read cameras collection Request from IP -> " + req.ipAddress);

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

void Configurate::SetNewTokenRoute()
{
    std::string Route = this->ReadConfigServiceConfig.SetNewTokenServiceInfo.URI;
    if(Route[0] != '/')
        Route = "/" + Route;

    this->app->route_dynamic(Route.c_str()).methods(crow::HTTPMethod::POST)([&](const crow::request& req ) { 
        crow::json::rvalue Req = crow::json::load(req.body);
        SHOW_IMPORTANTLOG("Recived Set New Token Request from IP -> " + req.ipAddress);

        int CameraIndex = Req["CameraIndex"].i();
        std::string Token = Req["Token"].s();
        std::time_t TokenTime = Req["TokenTime"].i();

        this->SetNewToken(CameraIndex, Token, TokenTime);
        
        auto Res    {crow::json::wvalue()};
        Res["Code"]  = 0;
        Res["Description"]  = "Set New Token value was successful";
        return crow::response{200 , Res};
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
Configurate::InfoDatabaseStruct Configurate::getConfigDatabaseInfo()
{
    return this->ConfigDatabaseInfo;
}

std::shared_ptr<MongoDB> Configurate::getInsertDatabase()
{
    return this->InsertDatabase;
}
std::shared_ptr<MongoDB> Configurate::getConfigDatabase()
{
    return this->ConfigDatabase;
}

std::shared_ptr<MongoDB> Configurate::getFailedDatabase()
{
    return this->FailedDatabase;
}

std::vector<Configurate::WebServiceConfigStruct> Configurate::getWebServiceConfig()
{
    return this->WebServiceConfig;
}
std::vector<Configurate::ClientServiceConfigStruct> Configurate::getClientServiceConfig()
{
    return this->ClientServiceConfig;
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

Configurate::ModulesStruct Configurate::getModules()
{
    return this->Modules;
}
Configurate::MetaStruct Configurate::getMeta()
{
    return this->Meta;
}
