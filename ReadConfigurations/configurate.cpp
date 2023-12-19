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

    // WebServiceConfig.ServerPubKey = ServerPubKey;
    // WebServiceConfig.ClientPubKey = ClientPubKey;

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

    // DataBase::DataBaseConfigStruct DBConf;
    // DBConf.Server = DatabaseConfig.DatabaseIP;
    // DBConf.User = DatabaseConfig.DatabaseUsername;
    // DBConf.Password = DatabaseConfig.DatabasePassword;
    // DBConf.DBName = DatabaseConfig.DatabaseName;
    // DBConf.EnableLog = true;
    // DataBase DB;
    // if(DB.init(DBConf) != 0)
    // {
    //     SHOW_ERROR("Configurate: "<<std::to_string(__LINE__)<<"_DataBase init failed");
    //     throw 0;
    // }

    // std::vector<std::map<std::string, std::string>> ConfigRec = DB.selectAllM("Configuration", "");
    // if(ConfigRec.size() > 0)
    // {
    //     WebServiceConfig.ServiceURI = ConfigRec[0]["ServiceURI"];
    //     WebServiceConfig.ServicePort = std::stoi(ConfigRec[0]["ServicePort"]);
    //     WebServiceConfig.InfoTimeAllowed = ConfigRec[0]["InfoTimeAllowed"];
    //     WebServiceConfig.QAddr = ConfigRec[0]["QAddress"];
    //     WebServiceConfig.QGroup = ConfigRec[0]["QGroup"];
    //     WebServiceConfig.QTopic = ConfigRec[0]["QTopic"];
    //     AuthenticateConfig.TokenTimeAllowed = ConfigRec[0]["TokenTimeAllowed"];
    //     AuthenticateConfig.DBObjCount = std::stoi(ConfigRec[0]["DBObjCount"]);
    //     ConfigRec.erase(ConfigRec.begin());
    // }
    // else
    // {
    //     SHOW_ERROR("Configurate: "<<std::to_string(__LINE__)<<"_There is no Configuration Record");
    //     throw 0;
    // }

    // std::vector<std::vector<std::string>> Fields = DB.selectAll("ServiceFields", "");
    // int ColCount = Fields[0].size();
    // for(int i = 0; i < ColCount; i++)
    // {
    //     if(Fields[0][i] == "ID")
    //         continue;
    //     std::vector<std::vector<std::string>> Col = DB.select("ServiceFields", Fields[0][i], "");
    //     WebServiceConfig.ServiceFields[Col[0][0]] = Col[1][0];
    // }

    // std::vector<std::map<std::string, std::string>> KnownDevicesRec = DB.selectAllM("KnownDevices", "");
    // if(KnownDevicesRec.size() > 0)
    // {
    //     while(KnownDevicesRec.size() > 0)
    //     {
    //         KnownDevicesStruct KnownDevices;
    //         KnownDevices.ID = KnownDevicesRec[0]["ID"];
    //         KnownDevices.DeviceID = KnownDevicesRec[0]["DeviceID"];
    //         KnownDevices.Name = KnownDevicesRec[0]["Name"];
    //         KnownDevices.Username = KnownDevicesRec[0]["Username"];
    //         KnownDevices.Password = KnownDevicesRec[0]["Password"];
    //         KnownDevices.Token = KnownDevicesRec[0]["Token"];
    //         KnownDevices.Date = KnownDevicesRec[0]["Date"];
    //         //KnownDevices.ReservedToken = boost::lexical_cast<std::string>(boost::uuids::random_generator()());
    //         AuthenticateConfig.KnownDevicesVec.push_back(KnownDevices);
    //         KnownDevicesRec.erase(KnownDevicesRec.begin());
    //     }
    // }
    // else
    // {
    //     SHOW_ERROR("Configurate: "<<std::to_string(__LINE__)<<"_KnownDevices table is empty");
    //     throw 0;
    // }

//    std::vector<std::map<std::string, std::string>> SettingsRec = DB.selectAllM("Settings", "");
//    if(SettingsRec.size() > 0)
//    {
//        Settings.MinPlateImageWidth = std::stoi(SettingsRec[0]["MinPlateImageWidth"]);
//        Settings.MaxPlateImageWidth = std::stoi(SettingsRec[0]["MaxPlateImageWidth"]);
//        Settings.MinPlateImageHeight = std::stoi(SettingsRec[0]["MinPlateImageHeight"]);
//        Settings.MaxPlateImageHeight = std::stoi(SettingsRec[0]["MaxPlateImageHeight"]);
//        Settings.MinPlateImageSize = std::stoi(SettingsRec[0]["MinPlateImageSize"]);
//        Settings.MaxPlateImageSize = std::stoi(SettingsRec[0]["MaxPlateImageSize"]);
//        Settings.MinColorImageWidth = std::stoi(SettingsRec[0]["MinColorImageWidth"]);
//        Settings.MaxColorImageWidth = std::stoi(SettingsRec[0]["MaxColorImageWidth"]);
//        Settings.MinColorImageHeight = std::stoi(SettingsRec[0]["MinColorImageHeight"]);
//        Settings.MaxColorImageHeight = std::stoi(SettingsRec[0]["MaxColorImageHeight"]);
//        Settings.MinColorImageSize = std::stoi(SettingsRec[0]["MinColorImageSize"]);
//        Settings.MaxColorImageSize = std::stoi(SettingsRec[0]["MaxColorImageSize"]);
//        SettingsRec.erase(SettingsRec.begin());
//    }
//    else
//    {
//        SHOW_ERROR("There is no Settings Record!");
//        throw -1
//    }
}

Configurate::DatabaseStruct Configurate::getDatabaseConfig()
{
    return DatabaseConfig;
}

Configurate::DatabaseStruct Configurate::getDatabaseInsert()
{
    return DatabaseInsert;
}

Configurate::DatabaseStruct Configurate::getDatabaseFailed()
{
    return DatabaseFailed;
}

