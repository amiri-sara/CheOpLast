#ifndef VALIDATOR_H
#define VALIDATOR_H

#include "../Logs/log.h"
#include "../DataHandler/datahandler.h"
#include "../Cryptography/cryptotools.h"
#include "../Time/timetools.h"

class Validator
{
public:
    Validator() = default;
    bool run(const std::shared_ptr<DataHandler::DataHandlerStruct> &DH);
private:  
    bool CheckRequstFormatJSON(const std::shared_ptr<DataHandler::DataHandlerStruct> &DH);
    bool checkDataExistOrNo(const std::shared_ptr<DataHandler::DataHandlerStruct> &DH);
    bool CheckNumberOfJSONFields(const std::shared_ptr<DataHandler::DataHandlerStruct> &DH);
    bool CheckRequestValues(const std::shared_ptr<DataHandler::DataHandlerStruct> &DH);
};



#endif //VALIDATOR_H