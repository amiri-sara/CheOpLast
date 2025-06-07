#include "MongoDB.h"

//! this function for convert "Password by string type" to "password by string but consist of code caracter"
std::string MongoDB::GetPassword(const std::string &Password)
{

    std::string Ex = Password;
    std::string ans = "";
    for(int i=0;i<Ex.size();i++)
    {
        switch (Ex[i]) {
        case '@':
        {
            ans+= "%40";
            break;
        }
        case '=':
        {
            ans+= "%3D";
            break;
        }
        case ':':
        {
            ans+= "%3A";
            break;
        }
        case '+':
        {
            ans+= "%2B";
            break;
        }
        case '^':
        {
            ans+= "%5E";
            break;
        }
        case '<':
        {
            ans+= "%3C";
            break;
        }
        case '!':
        {
            ans+= "%21";
            break;
        }
        case '>':
        {
            ans+= "%3E";
            break;
        }
        case '%':
        {
            ans+= "%25";
            break;
        }
        default:
        {
            ans+=Ex[i];
        }

        }
    }
    return ans;
}

MongoDB::MongoDB(const DatabaseConfig &DBC)
{
    std::string User       = GetPassword(DBC.User);
    std::string Password   = GetPassword(DBC.Password);
    std::string IP         = DBC.IP;
    std::string Port       = DBC.Port;
    std::string Detail     = DBC.Detail;// "?tls=true&tlsAllowInvalidCertificates=true&tlsAllowInvalidHostnames=true";

    std::string UP = "";
    if(User!="" && Password != "")
    {
        UP =  User + ":" + Password + "@"  ;
    }
    std::string MongoIP = "mongodb://" + UP + IP + ":"+ Port + "/";
    
    if(DBC.TLSMode)
        MongoIP += Detail;

    try 
    {
#ifdef CREATEMONGOINSTANCE
        static mongocxx::instance inst{};
#endif // CREATEMONGOINSTANCE
        mongocxx::uri uri{MongoIP};
        this->mongoPool = std::make_shared<mongocxx::pool>(uri);
    } 
    catch (const mongocxx::exception& e) 
    {
        Logger::getInstance().logError(std::string("Error connecting to MongoDB: ") + e.what());

    }
}

MongoDB::ResponseStruct MongoDB::Insert(const std::string &DatabaseName, const std::string &CollectionName, const std::vector<Field>& fields)
{
    std::shared_ptr<mongocxx::pool::entry> client = std::make_shared<mongocxx::pool::entry>(this->mongoPool->acquire());    
    
    std::shared_ptr<mongocxx::database> db = std::make_shared<mongocxx::database>((*client)->database(DatabaseName.c_str()));
    // Check if the database is valid
    if (!db) {
        return ResponseStruct{MongoStatus::Database, "Access Database Error"};
    }

    mongocxx::collection coll = (*(db))[CollectionName.c_str()];
    // Check if the collection is valid
    if (!coll) {
        return ResponseStruct{MongoStatus::Collection, "Access Collection Error"};
    }
    
    try 
    {
        // Create a BSON document
        bsoncxx::builder::basic::document BasicBuilder;
        for (const auto& field : fields) 
        {
            switch (field.type) {
                case FieldType::ObjectId:
                    BasicBuilder.append(bsoncxx::builder::basic::kvp(field.key, bsoncxx::oid(field.value)));
                    break;
                case FieldType::String:
                    BasicBuilder.append(bsoncxx::builder::basic::kvp(field.key, field.value));
                    break;
                case FieldType::Integer:
                    BasicBuilder.append(bsoncxx::builder::basic::kvp(field.key, std::stoi(field.value)));
                    break;
                case FieldType::Double:
                    BasicBuilder.append(bsoncxx::builder::basic::kvp(field.key, std::stod(field.value)));
                    break;
                case FieldType::Int64:{
                    int64_t value_int64 = std::stoll(field.value);
                    BasicBuilder.append(bsoncxx::builder::basic::kvp(field.key, value_int64));//std::strtoll(field.value.c_str(),nullptr,10)));
                    break;
                }

                case FieldType::Date:
                {
                    std::tm tm{};
                    std::istringstream ss(field.value);
                    if (ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S")) {
                        time_t UnixSecond = std::mktime(&tm);
                        std::chrono::system_clock::time_point tp = std::chrono::system_clock::from_time_t(UnixSecond);
                        BasicBuilder.append(bsoncxx::builder::basic::kvp(field.key, bsoncxx::types::b_date(tp)));
                    } else {
                        return ResponseStruct{MongoStatus::FailedParseDataFromString, "Failed to parse date and time from string."};
                    }                  
                    break;
                }
                default:
                    return ResponseStruct{MongoStatus::InvalidMongoType, "Invalid Mongo Type = " + field.type};
            }
        }
        
        bsoncxx::document::value document = BasicBuilder.extract();
        bsoncxx::document::view DocView = document.view();
        
        // Insert the document into the collection
        auto result = coll.insert_one(DocView);
        if (result) {
            return ResponseStruct{MongoStatus::InsertSuccessful, "Document inserted successfully with _id: " + result->inserted_id().get_oid().value.to_string()};
        } else {
            return ResponseStruct{MongoStatus::InsertFailed, "Failed to insert document."};
        }
    } catch (const mongocxx::exception& e) {
        return ResponseStruct{MongoStatus::InsertError, std::string("MongoDB Exception: ") + e.what()};
    } catch (const std::exception& e) {
        return ResponseStruct{MongoStatus::InsertError, std::string("MongoDB Exception: ") + e.what()};
    } catch (...) {
        return ResponseStruct{MongoStatus::InsertError, "An unknown exception occurred during document insertion."};
    }
}

MongoDB::ResponseStruct MongoDB::Find(const std::string &DatabaseName, const std::string &CollectionName, const std::vector<Field>& fields, FindOptionStruct& FindOption, std::vector<std::string>& Documents)
{
    std::shared_ptr<mongocxx::pool::entry> client = std::make_shared<mongocxx::pool::entry>(this->mongoPool->acquire());

    std::shared_ptr<mongocxx::database> db = std::make_shared<mongocxx::database>((*client)->database(DatabaseName.c_str()));
    // Check if the database is valid
    if (!db) {
        return ResponseStruct{MongoStatus::Database, "Access Database Error"};
    }

    mongocxx::collection coll = (*(db))[CollectionName.c_str()];
    // Check if the collection is valid
    if (!coll) {
        return ResponseStruct{MongoStatus::Collection, "Access Collection Error"};
    }
    
    bsoncxx::builder::basic::document QueryBuilder;

    for (const auto& field : fields) 
        {
            switch (field.type) {
                case FieldType::ObjectId:
                {
                    QueryBuilder.append(bsoncxx::builder::basic::kvp(field.key, [value = field.value, condition = field.condition]{
                        bsoncxx::builder::basic::document NestedBuilder;
                        NestedBuilder.append(bsoncxx::builder::basic::kvp(condition, bsoncxx::oid(value)));
                        return NestedBuilder.extract();
                    }()));
                    break;
                }
                case FieldType::String:
                {
                    QueryBuilder.append(bsoncxx::builder::basic::kvp(field.key, [value = field.value, condition = field.condition]{
                        bsoncxx::builder::basic::document NestedBuilder;
                        NestedBuilder.append(bsoncxx::builder::basic::kvp(condition, value));
                        return NestedBuilder.extract();
                    }()));
                    break;
                }
                case FieldType::Integer:
                {
                    QueryBuilder.append(bsoncxx::builder::basic::kvp(field.key, [value = field.value, condition = field.condition]{
                        bsoncxx::builder::basic::document NestedBuilder;
                        NestedBuilder.append(bsoncxx::builder::basic::kvp(condition, std::stoi(value)));
                        return NestedBuilder.extract();
                    }()));
                    break;
                }
                case FieldType::Double:
                {
                    QueryBuilder.append(bsoncxx::builder::basic::kvp(field.key, [value = field.value, condition = field.condition]{
                        bsoncxx::builder::basic::document NestedBuilder;
                        NestedBuilder.append(bsoncxx::builder::basic::kvp(condition, std::stod(value)));
                        return NestedBuilder.extract();
                    }()));
                    break;
                }
                case FieldType::Date:               
                {
                    std::tm tm{};
                    std::istringstream ss(field.value);
                    if (ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S")) {
                        time_t UnixSecond = std::mktime(&tm);
                        std::chrono::system_clock::time_point tp = std::chrono::system_clock::from_time_t(UnixSecond);
                        QueryBuilder.append(bsoncxx::builder::basic::kvp(field.key, [value = tp, condition = field.condition]{
                            bsoncxx::builder::basic::document NestedBuilder;
                            NestedBuilder.append(bsoncxx::builder::basic::kvp(condition, bsoncxx::types::b_date(value)));
                            return NestedBuilder.extract();
                        }()));
                    } else {
                        return ResponseStruct{MongoStatus::FailedParseDataFromString, "Failed to parse date and time from string."};
                    }                  
                    break;
                }
                default:
                    return ResponseStruct{MongoStatus::InvalidMongoType, "Invalid Mongo Type = " + field.type};
            }
        }

    // SHOW_WARNING(bsoncxx::to_json(QueryBuilder, bsoncxx::ExtendedJsonMode::k_relaxed));
    bsoncxx::document::value Filter = QueryBuilder.extract();
    bsoncxx::document::view FilterView = Filter.view();
    
    mongocxx::options::find findopts;
    findopts.limit(FindOption.limit);
    findopts.skip(FindOption.skip);

    mongocxx::cursor cursor = coll.find(FilterView, findopts);
    if(cursor.begin() != cursor.end())
    {
        for (auto doc : cursor) {
            Documents.push_back(bsoncxx::to_json(doc, bsoncxx::ExtendedJsonMode::k_relaxed));
        }
    }
    return ResponseStruct{MongoStatus::FindSuccessful, "Find Successful"};
}
MongoDB::ResponseStruct MongoDB::Update_one(const std::string &DatabaseName, const std::string &CollectionName, const std::vector<Field>& findfields, const std::vector<Field>& updatefields)
{
    std::shared_ptr<mongocxx::pool::entry> client = std::make_shared<mongocxx::pool::entry>(this->mongoPool->acquire());
    std::shared_ptr<mongocxx::database> db = std::make_shared<mongocxx::database>((*client)->database(DatabaseName.c_str()));
    
    // Check if the database is valid
    if (!db) {
        return ResponseStruct{MongoStatus::Database, "Access Database Error"};
    }

    mongocxx::collection coll = (*(db))[CollectionName.c_str()];
    
    // Check if the collection is valid
    if (!coll) {
        return ResponseStruct{MongoStatus::Collection, "Access Collection Error"};
    }

    // Build the query filter
    bsoncxx::builder::basic::document QueryBuilder;
    for (const auto& field : findfields) {
        switch (field.type) {
            case FieldType::ObjectId:
                QueryBuilder.append(bsoncxx::builder::basic::kvp(field.key, bsoncxx::oid(field.value)));
                break;
            case FieldType::String:
                QueryBuilder.append(bsoncxx::builder::basic::kvp(field.key, field.value));
                break;
            case FieldType::Integer:
                QueryBuilder.append(bsoncxx::builder::basic::kvp(field.key, std::stoi(field.value)));
                break;
            case FieldType::Double:
                QueryBuilder.append(bsoncxx::builder::basic::kvp(field.key, std::stod(field.value)));
                break;
            // case Field::FieldType::Int64:
            //     QueryBuilder.append(bsoncxx::builder::basic::kvp(field.key, std::stoll(field.value)));
            //     break;
            case FieldType::Date: {
                std::tm tm{};
                std::istringstream ss(field.value);
                if (ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S")) {
                    time_t UnixSecond = std::mktime(&tm);
                    std::chrono::system_clock::time_point tp = std::chrono::system_clock::from_time_t(UnixSecond);
                    QueryBuilder.append(bsoncxx::builder::basic::kvp(field.key, bsoncxx::types::b_date(tp)));
                } else {
                    return ResponseStruct{MongoStatus::FailedParseDataFromString, "Failed to parse date and time from string."};
                }
                break;
            }
            default:
                return ResponseStruct{MongoStatus::InvalidMongoType, "Invalid Mongo Type = " + field.key};
        }
    }

    // Build the update document
    bsoncxx::builder::basic::document UpdateBuilder;
    for (const auto& field : updatefields) {
        switch (field.type) {
            case FieldType::Int64:
            {
                try {
                    int64_t int64Value = std::stoll(field.value); // Convert string to int64_t

                    // Use $set to modify the field
                    UpdateBuilder.append(bsoncxx::builder::basic::kvp("$set", bsoncxx::builder::basic::make_document(
                        bsoncxx::builder::basic::kvp(field.key, int64Value)
                    )));
                } catch (const std::exception& e) {
                    return ResponseStruct{MongoStatus::FailedParseDataFromString, "Failed to parse int64 from string: " + field.value};
                }
                break;
            }
            case FieldType::String:
                UpdateBuilder.append(bsoncxx::builder::basic::kvp("$set", bsoncxx::builder::basic::make_document(
                    bsoncxx::builder::basic::kvp(field.key, field.value)
                )));
                break;
            case FieldType::Integer:
                UpdateBuilder.append(bsoncxx::builder::basic::kvp("$set", bsoncxx::builder::basic::make_document(
                    bsoncxx::builder::basic::kvp(field.key, std::stoi(field.value))
                )));
                break;
            case FieldType::Double:
                UpdateBuilder.append(bsoncxx::builder::basic::kvp("$set", bsoncxx::builder::basic::make_document(
                    bsoncxx::builder::basic::kvp(field.key, std::stod(field.value))
                )));
                break;
            case FieldType::Date: {
                std::tm tm{};
                std::istringstream ss(field.value);
                if (ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S")) {
                    time_t UnixSecond = std::mktime(&tm);
                    std::chrono::system_clock::time_point tp = std::chrono::system_clock::from_time_t(UnixSecond);
                    UpdateBuilder.append(bsoncxx::builder::basic::kvp("$set", bsoncxx::builder::basic::make_document(
                        bsoncxx::builder::basic::kvp(field.key, bsoncxx::types::b_date(tp))
                    )));
                } else {
                    return ResponseStruct{MongoStatus::FailedParseDataFromString, "Failed to parse date and time from string."};
                }
                break;
            }
            default:
                return ResponseStruct{MongoStatus::InvalidMongoType, "Invalid Mongo Type = " + field.key};
        }
    }

    // Perform the update
    bsoncxx::document::view FilterView = QueryBuilder.view();
    bsoncxx::document::view UpdateView = UpdateBuilder.view();
    auto result = coll.update_one(FilterView, UpdateView);

    if (result && result->modified_count() > 0) {
        return ResponseStruct{MongoStatus::UpdateSuccessful, "Update Successful"};
    } else {
        return ResponseStruct{MongoStatus::UpdateFailed, "No document matched the filter or update failed."};
    }
}


MongoDB::ResponseStruct MongoDB::Update(const std::string &DatabaseName, const std::string &CollectionName, const std::vector<Field>& findfields, const std::vector<Field>& updatefields)
{
    std::shared_ptr<mongocxx::pool::entry> client = std::make_shared<mongocxx::pool::entry>(this->mongoPool->acquire());

    std::shared_ptr<mongocxx::database> db = std::make_shared<mongocxx::database>((*client)->database(DatabaseName.c_str()));
    // Check if the database is valid
    if (!db) {
        return ResponseStruct{MongoStatus::Database, "Access Database Error"};
    }

    mongocxx::collection coll = (*(db))[CollectionName.c_str()];
    // Check if the collection is valid
    if (!coll) {
        return ResponseStruct{MongoStatus::Collection, "Access Collection Error"};
    }

    bsoncxx::builder::basic::document QueryBuilder;

    for (const auto& field : findfields) 
        {
            switch (field.type) {
                case FieldType::ObjectId:
                {
                    QueryBuilder.append(bsoncxx::builder::basic::kvp(field.key, [value = field.value, condition = field.condition]{
                        bsoncxx::builder::basic::document NestedBuilder;
                        NestedBuilder.append(bsoncxx::builder::basic::kvp(condition, bsoncxx::oid(value)));
                        return NestedBuilder.extract();
                    }()));
                    break;
                }
                case FieldType::String:
                {
                    QueryBuilder.append(bsoncxx::builder::basic::kvp(field.key, [value = field.value, condition = field.condition]{
                        bsoncxx::builder::basic::document NestedBuilder;
                        NestedBuilder.append(bsoncxx::builder::basic::kvp(condition, value));
                        return NestedBuilder.extract();
                    }()));
                    break;
                }
                case FieldType::Integer:
                {
                    QueryBuilder.append(bsoncxx::builder::basic::kvp(field.key, [value = field.value, condition = field.condition]{
                        bsoncxx::builder::basic::document NestedBuilder;
                        NestedBuilder.append(bsoncxx::builder::basic::kvp(condition, std::stoi(value)));
                        return NestedBuilder.extract();
                    }()));
                    break;
                }
                case FieldType::Double:
                {
                    QueryBuilder.append(bsoncxx::builder::basic::kvp(field.key, [value = field.value, condition = field.condition]{
                        bsoncxx::builder::basic::document NestedBuilder;
                        NestedBuilder.append(bsoncxx::builder::basic::kvp(condition, std::stod(value)));
                        return NestedBuilder.extract();
                    }()));
                    break;
                }
                case FieldType::Date:               
                {
                    std::tm tm{};
                    std::istringstream ss(field.value);
                    if (ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S")) {
                        time_t UnixSecond = std::mktime(&tm);
                        std::chrono::system_clock::time_point tp = std::chrono::system_clock::from_time_t(UnixSecond);
                        QueryBuilder.append(bsoncxx::builder::basic::kvp(field.key, [value = tp, condition = field.condition]{
                            bsoncxx::builder::basic::document NestedBuilder;
                            NestedBuilder.append(bsoncxx::builder::basic::kvp(condition, bsoncxx::types::b_date(value)));
                            return NestedBuilder.extract();
                        }()));
                    } else {
                        return ResponseStruct{MongoStatus::FailedParseDataFromString, "Failed to parse date and time from string."};
                    }                  
                    break;
                }
                default:
                    return ResponseStruct{MongoStatus::InvalidMongoType, "Invalid Mongo Type = " + field.type};
            }
        }

    bsoncxx::document::value Filter = QueryBuilder.extract();
    bsoncxx::document::view FilterView = Filter.view();

    bsoncxx::builder::basic::document UpdateBuilder;

    for (const auto& field : updatefields) 
        {
            switch (field.type) {
                case FieldType::ObjectId:
                {
                    UpdateBuilder.append(bsoncxx::builder::basic::kvp("$set", [value = field.value, key = field.key]{
                        bsoncxx::builder::basic::document NestedBuilder;
                        NestedBuilder.append(bsoncxx::builder::basic::kvp(key, bsoncxx::oid(value)));
                        return NestedBuilder.extract();
                    }()));
                    break;
                }
                case FieldType::String:
                {
                    UpdateBuilder.append(bsoncxx::builder::basic::kvp("$set", [value = field.value, key = field.key]{
                        bsoncxx::builder::basic::document NestedBuilder;
                        NestedBuilder.append(bsoncxx::builder::basic::kvp(key, value));
                        return NestedBuilder.extract();
                    }()));
                    break;
                }
                case FieldType::Integer:
                {
                    UpdateBuilder.append(bsoncxx::builder::basic::kvp("$set", [value = field.value, key = field.key]{
                        bsoncxx::builder::basic::document NestedBuilder;
                        NestedBuilder.append(bsoncxx::builder::basic::kvp(key, std::stoi(value)));
                        return NestedBuilder.extract();
                    }()));
                    break;
                }
                case FieldType::Double:
                {
                    UpdateBuilder.append(bsoncxx::builder::basic::kvp("$set", [value = field.value, key = field.key]{
                        bsoncxx::builder::basic::document NestedBuilder;
                        NestedBuilder.append(bsoncxx::builder::basic::kvp(key, std::stod(value)));
                        return NestedBuilder.extract();
                    }()));
                    break;
                }
                case FieldType::Date:               
                {
                    std::tm tm{};
                    std::istringstream ss(field.value);
                    if (ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S")) {
                        time_t UnixSecond = std::mktime(&tm);
                        std::chrono::system_clock::time_point tp = std::chrono::system_clock::from_time_t(UnixSecond);
                        UpdateBuilder.append(bsoncxx::builder::basic::kvp("$set", [value = tp, key = field.key]{
                            bsoncxx::builder::basic::document NestedBuilder;
                            NestedBuilder.append(bsoncxx::builder::basic::kvp(key, bsoncxx::types::b_date(value)));
                            return NestedBuilder.extract();
                        }()));
                    } else {
                        return ResponseStruct{MongoStatus::FailedParseDataFromString, "Failed to parse date and time from string."};
                    }                  
                    break;
                }
                default:
                    return ResponseStruct{MongoStatus::InvalidMongoType, "Invalid Mongo Type = " + field.type};
            }
        }

    bsoncxx::document::value Update = UpdateBuilder.extract();
    bsoncxx::document::view UpdateView = Update.view();

    mongocxx::v_noabi::stdx::optional<mongocxx::result::update> update_many_result = coll.update_many(FilterView, UpdateView);

    // SHOW_IMPORTANTLOG("modified count = " << update_many_result->modified_count());

    return ResponseStruct{MongoStatus::UpdateSuccessful, "Update Successful"};
}

MongoDB::ResponseStruct MongoDB::Delete(const std::string &DatabaseName, const std::string &CollectionName, const std::vector<Field>& findfields)
{
    std::shared_ptr<mongocxx::pool::entry> client = std::make_shared<mongocxx::pool::entry>(this->mongoPool->acquire());
        
    std::shared_ptr<mongocxx::database> db = std::make_shared<mongocxx::database>((*client)->database(DatabaseName.c_str()));
    // Check if the database is valid
    if (!db) {
        return ResponseStruct{MongoStatus::Database, "Access Database Error"};
    }

    mongocxx::collection coll = (*(db))[CollectionName.c_str()];
    // Check if the collection is valid
    if (!coll) {
        return ResponseStruct{MongoStatus::Collection, "Access Collection Error"};
    }

    bsoncxx::builder::basic::document QueryBuilder;

    for (const auto& field : findfields) 
        {
            switch (field.type) {
                case FieldType::ObjectId:
                {
                    QueryBuilder.append(bsoncxx::builder::basic::kvp(field.key, [value = field.value, condition = field.condition]{
                        bsoncxx::builder::basic::document NestedBuilder;
                        NestedBuilder.append(bsoncxx::builder::basic::kvp(condition, bsoncxx::oid(value)));
                        return NestedBuilder.extract();
                    }()));
                    break;
                }
                case FieldType::String:
                {
                    QueryBuilder.append(bsoncxx::builder::basic::kvp(field.key, [value = field.value, condition = field.condition]{
                        bsoncxx::builder::basic::document NestedBuilder;
                        NestedBuilder.append(bsoncxx::builder::basic::kvp(condition, value));
                        return NestedBuilder.extract();
                    }()));
                    break;
                }
                case FieldType::Integer:
                {
                    QueryBuilder.append(bsoncxx::builder::basic::kvp(field.key, [value = field.value, condition = field.condition]{
                        bsoncxx::builder::basic::document NestedBuilder;
                        NestedBuilder.append(bsoncxx::builder::basic::kvp(condition, std::stoi(value)));
                        return NestedBuilder.extract();
                    }()));
                    break;
                }
                case FieldType::Double:
                {
                    QueryBuilder.append(bsoncxx::builder::basic::kvp(field.key, [value = field.value, condition = field.condition]{
                        bsoncxx::builder::basic::document NestedBuilder;
                        NestedBuilder.append(bsoncxx::builder::basic::kvp(condition, std::stod(value)));
                        return NestedBuilder.extract();
                    }()));
                    break;
                }
                case FieldType::Date:               
                {
                    std::tm tm{};
                    std::istringstream ss(field.value);
                    if (ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S")) {
                        time_t UnixSecond = std::mktime(&tm);
                        std::chrono::system_clock::time_point tp = std::chrono::system_clock::from_time_t(UnixSecond);
                        QueryBuilder.append(bsoncxx::builder::basic::kvp(field.key, [value = tp, condition = field.condition]{
                            bsoncxx::builder::basic::document NestedBuilder;
                            NestedBuilder.append(bsoncxx::builder::basic::kvp(condition, bsoncxx::types::b_date(value)));
                            return NestedBuilder.extract();
                        }()));
                    } else {
                        return ResponseStruct{MongoStatus::FailedParseDataFromString, "Failed to parse date and time from string."};
                    }                  
                    break;
                }
                default:
                    return ResponseStruct{MongoStatus::InvalidMongoType, "Invalid Mongo Type = " + field.type};
            }
        }

    bsoncxx::document::value Filter = QueryBuilder.extract();
    bsoncxx::document::view FilterView = Filter.view();

    mongocxx::v_noabi::stdx::optional<mongocxx::v_noabi::result::delete_result> delete_many_result = coll.delete_many(FilterView);

    // SHOW_IMPORTANTLOG("Delete count = " << delete_many_result->deleted_count());

    return ResponseStruct{MongoStatus::DeleteSuccessful, "Delete Successful"};
}

// --- پیاده‌سازی متد InsertMany ---
MongoDB::ResponseStruct MongoDB::InsertMany(const std::string &DatabaseName, const std::string &CollectionName, const std::vector<std::vector<Field>>& documents_fields)
{
    std::shared_ptr<mongocxx::pool::entry> client = std::make_shared<mongocxx::pool::entry>(this->mongoPool->acquire());    
    
    std::shared_ptr<mongocxx::database> db = std::make_shared<mongocxx::database>((*client)->database(DatabaseName.c_str()));
    if (!db) {
        return ResponseStruct{MongoStatus::Database, "Access Database Error"};
    }

    mongocxx::collection coll = (*(db))[CollectionName.c_str()];
    if (!coll) {
        return ResponseStruct{MongoStatus::Collection, "Access Collection Error"};
    }
    
    try 
    {
        std::vector<bsoncxx::document::value> documents_to_insert;
        documents_to_insert.reserve(documents_fields.size());

        for (const auto& fields : documents_fields) {
            bsoncxx::builder::basic::document BasicBuilder;
            for (const auto& field : fields) {
                switch (field.type) {
                    case FieldType::ObjectId:
                        BasicBuilder.append(bsoncxx::builder::basic::kvp(field.key, bsoncxx::oid(field.value)));
                        break;
                    case FieldType::String:
                        BasicBuilder.append(bsoncxx::builder::basic::kvp(field.key, field.value));
                        break;
                    case FieldType::Integer:
                        BasicBuilder.append(bsoncxx::builder::basic::kvp(field.key, std::stoi(field.value)));
                        break;
                    case FieldType::Double:
                        BasicBuilder.append(bsoncxx::builder::basic::kvp(field.key, std::stod(field.value)));
                        break;
                    case FieldType::Int64:{
                        int64_t value_int64 = std::stoll(field.value);
                        BasicBuilder.append(bsoncxx::builder::basic::kvp(field.key, value_int64));//std::strtoll(field.value.c_str(),nullptr,10)));
                        break;
                    }
                    case FieldType::Date:
                    {
                        std::tm tm{};
                        std::istringstream ss(field.value);
                        if (ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S")) {
                            time_t UnixSecond = std::mktime(&tm);
                            std::chrono::system_clock::time_point tp = std::chrono::system_clock::from_time_t(UnixSecond);
                            BasicBuilder.append(bsoncxx::builder::basic::kvp(field.key, bsoncxx::types::b_date(tp)));
                        } else {
                            return ResponseStruct{MongoStatus::FailedParseDataFromString, "Failed to parse date and time from string for field " + field.key};
                        }                  
                        break;
                    }
                    default:
                        return ResponseStruct{MongoStatus::InvalidMongoType, "Invalid Mongo Type for field " + field.key + " with type " + std::to_string(static_cast<int>(field.type))};
                }
            }
            documents_to_insert.push_back(BasicBuilder.extract());
        }
        
        if (documents_to_insert.empty()) {
            return ResponseStruct{MongoStatus::InsertSuccessful, "No documents to insert."};
        }

        auto result = coll.insert_many(documents_to_insert);
        if (result) {
            return ResponseStruct{MongoStatus::InsertSuccessful, "Documents inserted successfully. Count: " + std::to_string(result->inserted_count())};
        } else {
            return ResponseStruct{MongoStatus::InsertFailed, "Failed to insert documents."};
        }
    } catch (const mongocxx::exception& e) {
        return ResponseStruct{MongoStatus::InsertError, std::string("MongoDB Exception during bulk insert: ") + e.what()};
    } catch (const std::exception& e) {
        return ResponseStruct{MongoStatus::InsertError, std::string("Standard exception during bulk insert: ") + e.what()};
    } catch (...) {
        return ResponseStruct{MongoStatus::InsertError, "An unknown exception occurred during bulk document insertion."};
    }
}
// --- پایان پیاده‌سازی متد InsertMany ---