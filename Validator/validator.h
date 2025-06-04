#ifndef VALIDATOR_H
#define VALIDATOR_H

#include "../Logs/log.h"
#include "../DataHandler/datahandler.h"
#include "../Cryptography/cryptotools.h"
#include "../Time/timetools.h"
#include "key_Mapper.h"
class Validator
{
public:

// struct FieldMapping {
//     std::string internalName;
//     std::vector<std::string> externalKeys;
//     std::string type;
//     bool required;
//     // rapidjson::Document validation; // Updated to RapidJSON
//     };

        // Constructor: Initialize with JSON config file path
    explicit Validator(const std::string& config_path) {
        std::ifstream ifs(config_path);
        if (!ifs.is_open()) {
            throw std::runtime_error("Failed to open field_mappings.json file: " + config_path);
        }

        std::stringstream buffer;
        buffer << ifs.rdbuf();
        std::string json_str = buffer.str();

        rapidjson::Document config;
        if (config.Parse(json_str.c_str()).HasParseError()) {
            throw std::runtime_error("JSON parsing error in field_mappings.json file");
        }

        key_mapper_ = std::make_unique<KeyMapper>(config);
    }
    Validator() = default;
        // Set configuration dynamically
    void setConfig(const std::string& config_path) {
        Validator tmp(config_path); // Reuse constructor logic
        key_mapper_ = std::move(tmp.key_mapper_);
    }



    bool run(const std::shared_ptr<DataHandler::DataHandlerStruct> &DH);
private:  

    bool CheckRequstFormatJSON(const std::shared_ptr<DataHandler::DataHandlerStruct> &DH);
    bool checkDataExistOrNo(const std::shared_ptr<DataHandler::DataHandlerStruct> &DH);
    bool checkEncryptedDataExistOrNo(const std::shared_ptr<DataHandler::DataHandlerStruct> &DH);
    bool checkTokenDataExistOrNo(const std::shared_ptr<DataHandler::DataHandlerStruct> &DH);
    bool CheckNumberOfJSONFields(const std::shared_ptr<DataHandler::DataHandlerStruct> &DH);
    bool CheckRequestValues(const std::shared_ptr<DataHandler::DataHandlerStruct> &DH);
    bool CheckEncryptedRequestValues(const std::shared_ptr<DataHandler::DataHandlerStruct> &DH);
    bool CheckTokenRequestValues(const std::shared_ptr<DataHandler::DataHandlerStruct> &DH);

    std::string GeneratMongoIDHash(const std::tm &PassedTime, const std::string &PlateValue, const int &ViolationID, const int &DeviceID);


    ////////////////////////////

    // std::vector<FieldMapping> fieldMappings_;
    // // void loadConfig(const std::string& configPath);
    // std::optional<rapidjson::Value> findFieldValue(const rapidjson::Document& input, const FieldMapping& field);
    // bool CheckRequestValuesConfigDriven(const std::shared_ptr<DataHandler::DataHandlerStruct>& DH);
//////////////////////////////////////////////////////////////

std::unique_ptr<KeyMapper> key_mapper_; // KeyMapper instance





};



#endif //VALIDATOR_H