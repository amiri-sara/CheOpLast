#include <iostream>
#include <numeric>
#include <sstream>
#include <string>
#include <iomanip>
#include <ctime>

#include <bsoncxx/v_noabi/bsoncxx/json.hpp>
#include <mongocxx/v_noabi/mongocxx/client.hpp>
#include <mongocxx/v_noabi/mongocxx/stdx.hpp>
#include <mongocxx/v_noabi/mongocxx/uri.hpp>
#include <mongocxx/v_noabi/mongocxx/instance.hpp>
#include <bsoncxx/v_noabi/bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/v_noabi/bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/v_noabi/bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/v_noabi/bsoncxx/builder/stream/array.hpp>
#include <mongocxx/v_noabi/mongocxx/pool.hpp>
#include <mongocxx/v_noabi/mongocxx/exception/exception.hpp>
#include <mongocxx/v_noabi/mongocxx/exception/bulk_write_exception.hpp>

#include "../Logs/log.h"

class MongoDB
{
public:
    enum  FieldType {
        ObjectId,
        String,
        Integer,
        Date,
        Double
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
    ~MongoDB() = default;

    ResponseStruct Insert(const std::string &DatabaseName, const std::string &CollectionName, const std::vector<Field>& fields);
    ResponseStruct Find(const std::string &DatabaseName, const std::string &CollectionName, const std::vector<Field>& fields, FindOptionStruct& FindOption, std::vector<std::string>& Documents);
    ResponseStruct Update(const std::string &DatabaseName, const std::string &CollectionName, const std::vector<Field>& findfields, const std::vector<Field>& updatefields);
    ResponseStruct Delete(const std::string &DatabaseName, const std::string &CollectionName, const std::vector<Field>& findfields);

private:
    std::shared_ptr<mongocxx::client> Conn;
    std::string GetPassword(const std::string &Password);
};