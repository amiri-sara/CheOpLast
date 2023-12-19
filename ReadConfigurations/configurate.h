#ifndef CONFIGURATE_H
#define CONFIGURATE_H

#include "../Cryptography/systemkeys.h"
#include "../Cryptography/cryptotools.h"
// #include "database.h"
#include "../Logs/log.h"
// #include "tools.h"

#define CONFIG_FILE_LOCATION        "/etc/Aggregation.conf"

class Configurate
{
public:
    struct DatabaseStruct
    {
        std::string DatabaseIP          = "";
        std::string DatabasePort        = "";
        std::string DatabaseUsername    = "";
        std::string DatabasePassword    = "";
        std::string DETAIL              = "";
        std::string DatabaseName        = "";
        std::string CollectionName      = "";
        std::string Enable              = "";
    };




    // struct WebServiceConfigStruct
    // {
    //     std::string ServiceURI;
    //     int ServicePort;
    //     std::string InfoTimeAllowed;
    //     std::map<std::string, std::string> ServiceFields;
    //     std::string ServerPubKey;
    //     std::string ClientPubKey;
    //     std::string QAddr;
    //     std::string QGroup;
    //     std::string QTopic;
    // };

    // struct KnownDevicesStruct
    // {
    //     std::string ID;
    //     std::string DeviceID;
    //     std::string Name;
    //     std::string Username;
    //     std::string Password;
    //     std::string Token;
    //     std::string Date;
    //     //std::string ReservedToken;
    // };

    // struct AuthenticateConfigStruct
    // {
    //     std::string TokenTimeAllowed;
    //     int DBObjCount = 1;
    //     std::vector<KnownDevicesStruct> KnownDevicesVec;
    // };

    // Configurate(const Configurate& Obj) = delete;

    static Configurate* getInstance()
    {
        if (InstancePtr == NULL)
        {
            InstancePtr = new Configurate();
            return InstancePtr;
        }
        else
        {
            return InstancePtr;
        }
    }

    DatabaseStruct getDatabaseConfig();
    DatabaseStruct getDatabaseInsert();
    DatabaseStruct getDatabaseFailed();
    // WebServiceConfigStruct getWebServiceConfig();
    // AuthenticateConfigStruct getAuthenticateConfig();

private:
    DatabaseStruct DatabaseConfig;
    DatabaseStruct DatabaseInsert;
    DatabaseStruct DatabaseFailed;
    // WebServiceConfigStruct WebServiceConfig;
    // AuthenticateConfigStruct AuthenticateConfig;
    static Configurate* InstancePtr;
    Configurate();
};

#endif // CONFIGURATE_H
