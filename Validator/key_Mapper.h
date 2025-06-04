#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <rapidjson/document.h>
#include "datahandler.h"
class KeyMapper {
public:

    // Mapping structure
    struct FieldMapping {
        std::string internal_name;
        std::string type;
    };

    // Constructor: Initialize the mapper from JSON configuration
    explicit KeyMapper(const rapidjson::Document& config) {
        if (config.IsObject() && config.HasMember("fields") && config["fields"].IsArray()) {
            for (const auto& field : config["fields"].GetArray()) {
                if (field.IsObject() && 
                    field.HasMember("internal_name") && field["internal_name"].IsString() &&
                    field.HasMember("external_keys") && field["external_keys"].IsArray()) {
                    std::string internal_name = field["internal_name"].GetString();
                    std::string type = field["type"].GetString();
                    for (const auto& ext_key : field["external_keys"].GetArray()) {
                        if (ext_key.IsString()) {
                            key_map[ext_key.GetString()] = {internal_name,type};
                        }
                    }
                }
            }
            //TODO
        }
    }

            // Helper function to set value based on type
        void set_value(crow::json::wvalue& output, const std::string& internal_name, 
                       const crow::json::rvalue& value) const {
            if (internal_name == "DeviceID") {
                output[internal_name] = static_cast<int32_t>(value.i());
            } else if (internal_name == "RecordID") {
                output[internal_name] = static_cast<uint64_t>(value.u());
            } else if (internal_name == "UserID") {
                output[internal_name] = static_cast<int32_t>(value.i());
            } else if (internal_name == "StreetID") {
                output[internal_name] = static_cast<int32_t>(value.i());
            } else if (internal_name == "ViolationID") {
                output[internal_name] = static_cast<int32_t>(value.i());
            } else if (internal_name == "Direction") {
                output[internal_name] = static_cast<int32_t>(value.i());
            } else if (internal_name == "PlateType") {
                output[internal_name] = static_cast<int32_t>(value.i());
            } else if (internal_name == "PlateValue") {
                output[internal_name] = value.s();
            } else if (internal_name == "Suspicious") {
                output[internal_name] = value.s();
            } else if (internal_name == "Speed") {
                output[internal_name] = static_cast<int32_t>(value.i());
            } else if (internal_name == "VehicleType") {
                output[internal_name] = static_cast<int32_t>(value.i());
            } else if (internal_name == "VehicleColor") {
                output[internal_name] = static_cast<int32_t>(value.i());
            } else if (internal_name == "VehicleModel") {
                output[internal_name] = static_cast<int32_t>(value.i());
            } else if (internal_name == "Lane") {
                output[internal_name] = static_cast<int32_t>(value.i());
            } else if (internal_name == "PassedTime") {
                output[internal_name] = value.s();
            } else if (internal_name == "ColorImage") {
                output[internal_name] = value.s();
            } else if (internal_name == "ImageAddress") {
                output[internal_name] = value.s();
            } else if (internal_name == "PlateImage") {
                output[internal_name] = value.s();
            } else if (internal_name == "Latitude") {
                output[internal_name] = value.d();
            } else if (internal_name == "Longitude") {
                output[internal_name] = value.d();
            } else if (internal_name == "Accuracy") {
                output[internal_name] = static_cast<int32_t>(value.i());
            } else if (internal_name == "PlateRect") {
                output[internal_name] = value.s();
            } else if (internal_name == "CarRect") {
                output[internal_name] = value.s();
            } else if (internal_name == "CodeType") {
                output[internal_name] = static_cast<int32_t>(value.i());
            } else if (internal_name == "Probability") {
                output[internal_name] = static_cast<int32_t>(value.i());
            } else if (internal_name == "MasterPlate") {
                output[internal_name] = value.i();
            } else if (internal_name == "ReceivedTime") {
                output[internal_name] = value.s();
            } else if (internal_name == "CompanyCode"){
                output[internal_name] = value.i();
            } else if (internal_name == "SystemCode"){
                output[internal_name] = value.i();
            }
            
        }

    // Map an input key to its internal_name
    crow::json::rvalue mapKey(const crow::json::rvalue& input) const {
        crow::json::wvalue result;
        for (const auto& item : input) {
            std::string key = item.key();
            auto it = key_map.find(key);
            if (it != key_map.end()) {
                set_value(result, it->second.internal_name, item);
                // std::string test = crow::json::dump(result);
                // std::cout<<test<<std::endl;
            }
        }

        std::string json_str = crow::json::dump(result);
        // std::cout<<json_str<<std::endl;

        auto rvalue_result = crow::json::load(json_str);
        if (!rvalue_result) {
        throw std::runtime_error("Failed to parse JSON output");
        }
        return rvalue_result;
    }

private:
    std::unordered_map<std::string, FieldMapping> key_map;
};