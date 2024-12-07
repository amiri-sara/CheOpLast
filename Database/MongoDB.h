#include "../Logs/log.h"

#define DATABASEVERSION "0.1.1"

class MongoDB
{
public:
    enum  FieldType {
        ObjectId,
        String,
        Integer,
        Date,
        Double,
        Int64
    };

    enum  MongoStatus {
        Connection,
        Database,
        Collection,
        InvalidMongoType,
        InsertSuccessful,
        InsertFailed,
        InsertError,
        FailedParseDataFromString,
        FindSuccessful,
        UpdateSuccessful,
        UpdateFailed,
        DeleteSuccessful
    };
    
    struct DatabaseConfig {
        std::string IP                           =   "";
        std::string Port                         =   "";
        std::string User                         =   "";
        std::string Password                     =   "";
        std::string Detail                       =   "";
        bool TLSMode                             =false;
    };

    struct Field {
        std::string key                          =   "";
        std::string value                        =   "";
        FieldType type;
        std::string condition                    =   "";
    };

    struct FindOptionStruct
    {
        int limit = 0;
        int skip  = 0;
    };

    struct ResponseStruct {
        enum MongoStatus Code;
        std::string Description;
    };

    MongoDB() = delete;
    MongoDB(const MongoDB&) = delete;
    MongoDB(MongoDB&&) = delete;
    MongoDB(const DatabaseConfig &DBC);

    ResponseStruct Insert(const std::string &DatabaseName, const std::string &CollectionName, const std::vector<Field>& fields);
    ResponseStruct Find(const std::string &DatabaseName, const std::string &CollectionName, const std::vector<Field>& fields, FindOptionStruct& FindOption, std::vector<std::string>& Documents);
    ResponseStruct Update(const std::string &DatabaseName, const std::string &CollectionName, const std::vector<Field>& findfields, const std::vector<Field>& updatefields);
    ResponseStruct Update_one(const std::string &DatabaseName, const std::string &CollectionName, const std::vector<Field>& findfields, const std::vector<Field>& updatefields);
    ResponseStruct Delete(const std::string &DatabaseName, const std::string &CollectionName, const std::vector<Field>& findfields);
    

private:
    std::shared_ptr<mongocxx::pool> mongoPool;
    std::string GetPassword(const std::string &Password);
};