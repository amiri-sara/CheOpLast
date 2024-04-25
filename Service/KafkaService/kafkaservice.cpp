#include "kafkaservice.h"
KafkaService::KafkaService(Configurate::KafkaConfigStruct ServiceConfig)
{
    this->configuration = std::shared_ptr<RdKafka::Conf>(RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL));
    this->InputKafkaConfig = ServiceConfig;

    configuration->set("bootstrap.servers", this->InputKafkaConfig.BootstrapServers, this->error);
    configuration->set("group.id", this->InputKafkaConfig.GroupID, this->error);
    configuration->set("auto.offset.reset", "earliest", this->error);
    configuration->set("enable.auto.commit", "true", this->error);
    configuration->set("auto.commit.interval.ms", "500", this->error);
}

void KafkaService::run()
{
    std::shared_ptr<KafkaProsumer> kp = std::make_shared<KafkaProsumer>(this->configuration.get(), this->InputKafkaConfig.Topic);
    
    while(true)
    {
        try 
        {   
            DummyData InputData;
            bool kafkard = kp->read(&InputData, this->error);
            if(kafkard)
            {
                //! "startTime" for Computing process time for this request
                auto requstStartTime = std::chrono::high_resolution_clock::now();
                SHOW_IMPORTANTLOG("Partition: " << InputData.partition << " - Offset: " << InputData.offset);
                
                std::shared_ptr<DataHandler::DataHandlerStruct> DH = std::make_shared<DataHandler::DataHandlerStruct>();
                DH->InsertRoute = true;
                DH->Request.body = InputData.text;
                crow::json::wvalue Response;

                Configurate* ConfigurateObj = Configurate::getInstance();

                DH->hasInputFields = ConfigurateObj->getInputFields();
                DH->hasOutputFields = ConfigurateObj->getOutputFields();
                DH->StoreImageConfig = ConfigurateObj->getStoreImageConfig();
                DH->ViolationMap = ConfigurateObj->getViolationMap();
                DH->Cameras = ConfigurateObj->getCameras();
                DH->DaysforPassedTimeAcceptable = this->InputKafkaConfig.DaysforPassedTimeAcceptable;
                DH->InsertDatabase = ConfigurateObj->getInsertDatabase();
                DH->InsertDatabaseInfo = ConfigurateObj->getInsertDatabaseInfo();
                DH->DebugMode = this->InputKafkaConfig.DebugMode;

                // 1- Validation Input data
                auto validationStartTime = std::chrono::high_resolution_clock::now();
                std::shared_ptr<Validator> Validatorobj = std::make_shared<Validator>();
                if(!(Validatorobj->run(DH)))
                {
                    Response["Status"] = DH->Response.errorCode;
                    Response["Description"] = DH->Response.Description;
                    if(DH->DebugMode)
                        SHOW_ERROR(crow::json::dump(Response));
                    continue;
                }
                auto validationFinishTime = std::chrono::high_resolution_clock::now();
                auto ValidationTime =  std::chrono::duration_cast<std::chrono::nanoseconds>(validationFinishTime - validationStartTime);

                auto ChecRecordIDStartTime = std::chrono::high_resolution_clock::now();
#ifdef INSERTDATABASE
                // 2- Check RecordID exist in database or not
                std::vector<MongoDB::Field> filter = {
                    // equal
                    {"RecordID", DH->ProcessedInputData.MongoID, MongoDB::FieldType::ObjectId, "$gte"},
                    {"RecordID", DH->ProcessedInputData.MongoID, MongoDB::FieldType::ObjectId, "$lte"}

                };
                MongoDB::FindOptionStruct Option;
                std::vector<std::string> ResultDoc;
                auto FindReturn = DH->InsertDatabase->Find(DH->InsertDatabaseInfo.DatabaseName, DH->InsertDatabaseInfo.CollectionName, filter, Option, ResultDoc);
                if(FindReturn.Code == MongoDB::MongoStatus::FindSuccessful)
                {
                    if(!(ResultDoc.empty()))
                    {
                        Response["Status"] = DUPLICATERECORD;
                        Response["Description"] = "Duplicate Record.";
                        if(DH->DebugMode)
                            SHOW_ERROR(crow::json::dump(Response));
                        continue;
                    }
                }else
                {
                    Response["Status"] = DATABASEERROR;
                    Response["Description"] = "Network Internal Service Error.";
                    if(DH->DebugMode)
                        SHOW_ERROR(crow::json::dump(Response));
                    continue;
                }
#endif // INSERTDATABASE
                auto ChecRecordIDFinishTime = std::chrono::high_resolution_clock::now();
                auto ChecRecordIDTime =  std::chrono::duration_cast<std::chrono::nanoseconds>(ChecRecordIDFinishTime - ChecRecordIDStartTime);

                auto storeImageStartTime = std::chrono::high_resolution_clock::now();
#ifdef STOREIMAGE
                // 3- Store Image
                std::shared_ptr<storeimage> storeimageobj = std::make_shared<storeimage>();
                if(!(storeimageobj->run(DH)))
                {
                    Response["Status"] = DH->Response.errorCode;
                    Response["Description"] = DH->Response.Description;
                    if(DH->DebugMode)
                        SHOW_ERROR(crow::json::dump(Response));
                    continue;
                }
#endif // STOREIMAGE        
                auto storeImageFinishTime = std::chrono::high_resolution_clock::now();
                auto storeImaheTime =  std::chrono::duration_cast<std::chrono::nanoseconds>(storeImageFinishTime - storeImageStartTime);        

                auto saveDataStartTime = std::chrono::high_resolution_clock::now();
#if defined KAFKAOUTPUT || defined INSERTDATABASE
                // 4- Save Data
                std::shared_ptr<savedata> savedataobj = std::make_shared<savedata>();
#ifdef KAFKAOUTPUT
                int OutputKafkaConnectionIndex = this->getKafkaConnectionIndex();
                savedataobj->setOutputKafkaConnection(this->OutputKafkaConnections[OutputKafkaConnectionIndex]);
#endif // KAFKAOUTPUT
                if(!(savedataobj->run(DH)))
                {
                    Response["Status"] = DH->Response.errorCode;
                    Response["Description"] = DH->Response.Description;
                    if(DH->DebugMode)
                        SHOW_ERROR(crow::json::dump(Response));
                    continue;
                }
#ifdef KAFKAOUTPUT
                this->releaseIndex(OutputKafkaConnectionIndex);
#endif // KAFKAOUTPUT
#endif // KAFKAOUTPUT || INSERTDATABASE
                auto saveDataFinishTime = std::chrono::high_resolution_clock::now();
                auto saveDataTime =  std::chrono::duration_cast<std::chrono::nanoseconds>(saveDataFinishTime - saveDataStartTime);     

                auto requestFinishTime = std::chrono::high_resolution_clock::now();
                auto requestTime =  std::chrono::duration_cast<std::chrono::nanoseconds>(requestFinishTime - requstStartTime);
                
                if(DH->DebugMode)
                    SHOW_IMPORTANTLOG3("ProccessTime(ns) = " << std::to_string(requestTime.count()) << std::endl << "1- Validation ProccessTime(ns) = " << std::to_string(ValidationTime.count())
                                << std::endl << "2- Check RecordID ProccessTime(ns) = " << std::to_string(ChecRecordIDTime.count()) << std::endl << "3- Store image ProccessTime(ns) = " << std::to_string(storeImaheTime.count())
                                << std::endl << "4- Save data ProccessTime(ns) = " << std::to_string(saveDataTime.count()));

                Response["Status"] = SUCCESSFUL;
                Response["Description"] = "Successful";
                SHOW_LOG(crow::json::dump(Response));
            }

        }
        catch (std::exception e)
        {

            SHOW_ERROR("Error Code 0x" + std::to_string(__LINE__) + " KAFKASERVICEFILE : " + e.what());
        }
    }
}