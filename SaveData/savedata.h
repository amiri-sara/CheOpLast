#ifndef SAVEDATA_H
#define SAVEDATA_H

#include "../Logs/log.h"
#include "../DataHandler/datahandler.h"
#include "../Time/timetools.h"

class savedata
{
public:
    savedata() = default;
    bool run(const std::shared_ptr<DataHandler::DataHandlerStruct> &DH);
private:  
    bool InsertDatabase(const std::shared_ptr<DataHandler::DataHandlerStruct> &DH);
    bool InsertKafka(const std::shared_ptr<DataHandler::DataHandlerStruct> &DH);
};



#endif //SAVEDATA_H