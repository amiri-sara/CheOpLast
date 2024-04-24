#ifndef SAVEDATA_H
#define SAVEDATA_H

#include "../Logs/log.h"
#include "../DataHandler/datahandler.h"
#include "../Time/timetools.h"

#ifdef KAFKAOUTPUT
    #include "../Kafka/KafkaProsumer.h"
    #include "../Kafka/Utility.h"
#endif // KAFKAOUTPUT

class savedata
{
public:
    savedata() = default;
    bool run(const std::shared_ptr<DataHandler::DataHandlerStruct> &DH);
#if defined KAFKAOUTPUT
    void setOutputKafkaConnection(std::shared_ptr<KafkaProsumer> OKC){this->OutputKafkaConnection = OKC;}
#endif // KAFKAOUTPUT
private:  
    bool InsertDatabase(const std::shared_ptr<DataHandler::DataHandlerStruct> &DH);
#ifdef KAFKAOUTPUT
    bool InsertKafka(const std::shared_ptr<DataHandler::DataHandlerStruct> &DH);
    std::shared_ptr<KafkaProsumer> OutputKafkaConnection;
#endif // KAFKAOUTPUT
};



#endif //SAVEDATA_H