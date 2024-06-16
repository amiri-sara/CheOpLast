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
    SHOW_IMPORTANTLOG3("Runinng Aggregation(Kafka Service) : " << this->InputKafkaConfig.BootstrapServers << " - " << this->InputKafkaConfig.GroupID);
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
                DH->DecryptedData = true;
                DH->WebServiceAuthentication = false;
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
                DH->FailedDatabase = ConfigurateObj->getFailedDatabase();
                DH->FailedDatabaseInfo = ConfigurateObj->getFailedDatabaseInfo();
                DH->Modules = ConfigurateObj->getModules();
                DH->DebugMode = this->InputKafkaConfig.DebugMode;

                // 1- Validation Input data
                auto validationStartTime = std::chrono::high_resolution_clock::now();
                std::shared_ptr<Validator> Validatorobj = std::make_shared<Validator>();
                if(!(Validatorobj->run(DH)))
                {
                    Response["Status"] = DH->Response.errorCode;
                    Response["Description"] = DH->Response.Description;
                    if(DH->hasInputFields.DeviceID && DH->hasInputFields.ViolationID && DH->hasInputFields.PassedTime && DH->hasInputFields.PlateValue)
                        Response["RecordID"] = DH->ProcessedInputData.MongoID;
                    Response["Partition"] = InputData.partition;
                    Response["Offset"] = InputData.offset;
                    if(DH->FailedDatabaseInfo.Enable)
                    {
                        std::vector<MongoDB::Field> fields = {
                            {"Status", std::to_string(DH->Response.errorCode), MongoDB::FieldType::Integer},
                            {"Description", DH->Response.Description, MongoDB::FieldType::String},
                            {"Partition", std::to_string(InputData.partition), MongoDB::FieldType::Integer},
                            {"Offset", std::to_string(InputData.offset), MongoDB::FieldType::Integer}
                        };

                        if(DH->hasInputFields.DeviceID && DH->hasInputFields.ViolationID && DH->hasInputFields.PassedTime && DH->hasInputFields.PlateValue)
                        {
                            MongoDB::Field RecordIDField = {"RecordID", DH->ProcessedInputData.MongoID, MongoDB::FieldType::ObjectId};
                            fields.push_back(RecordIDField);
                        }

                        DH->FailedDatabase ->Insert(DH->FailedDatabaseInfo.DatabaseName, DH->FailedDatabaseInfo.CollectionName, fields);
                    }
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
                        if(DH->hasInputFields.DeviceID && DH->hasInputFields.ViolationID && DH->hasInputFields.PassedTime && DH->hasInputFields.PlateValue)
                            Response["RecordID"] = DH->ProcessedInputData.MongoID;
                        Response["Partition"] = InputData.partition;
                        Response["Offset"] = InputData.offset;
                        if(DH->FailedDatabaseInfo.Enable)
                        {
                            std::vector<MongoDB::Field> fields = {
                                {"Status", std::to_string(DUPLICATERECORD), MongoDB::FieldType::Integer},
                                {"Description", "Duplicate Record.", MongoDB::FieldType::String},
                                {"Partition", std::to_string(InputData.partition), MongoDB::FieldType::Integer},
                                {"Offset", std::to_string(InputData.offset), MongoDB::FieldType::Integer}
                            };

                            if(DH->hasInputFields.DeviceID && DH->hasInputFields.ViolationID && DH->hasInputFields.PassedTime && DH->hasInputFields.PlateValue)
                            {
                                MongoDB::Field RecordIDField = {"RecordID", DH->ProcessedInputData.MongoID, MongoDB::FieldType::ObjectId};
                                fields.push_back(RecordIDField);
                            }

                            DH->FailedDatabase ->Insert(DH->FailedDatabaseInfo.DatabaseName, DH->FailedDatabaseInfo.CollectionName, fields);
                        }
                        SHOW_ERROR(crow::json::dump(Response));
                        continue;
                    }
                }else
                {
                    Response["Status"] = DATABASEERROR;
                    Response["Description"] = "Network Internal Service Error.";
                    if(DH->hasInputFields.DeviceID && DH->hasInputFields.ViolationID && DH->hasInputFields.PassedTime && DH->hasInputFields.PlateValue)
                        Response["RecordID"] = DH->ProcessedInputData.MongoID;
                    Response["Partition"] = InputData.partition;
                    Response["Offset"] = InputData.offset;
                    if(DH->FailedDatabaseInfo.Enable)
                    {
                        std::vector<MongoDB::Field> fields = {
                            {"Status", std::to_string(DATABASEERROR), MongoDB::FieldType::Integer},
                            {"Description", "Network Internal Service Error.", MongoDB::FieldType::String},
                            {"Partition", std::to_string(InputData.partition), MongoDB::FieldType::Integer},
                            {"Offset", std::to_string(InputData.offset), MongoDB::FieldType::Integer}
                        };

                        if(DH->hasInputFields.DeviceID && DH->hasInputFields.ViolationID && DH->hasInputFields.PassedTime && DH->hasInputFields.PlateValue)
                        {
                            MongoDB::Field RecordIDField = {"RecordID", DH->ProcessedInputData.MongoID, MongoDB::FieldType::ObjectId};
                            fields.push_back(RecordIDField);
                        }

                        DH->FailedDatabase ->Insert(DH->FailedDatabaseInfo.DatabaseName, DH->FailedDatabaseInfo.CollectionName, fields);
                    }
                    SHOW_ERROR(crow::json::dump(Response));
                    continue;
                }
#endif // INSERTDATABASE
                auto ChecRecordIDFinishTime = std::chrono::high_resolution_clock::now();
                auto ChecRecordIDTime =  std::chrono::duration_cast<std::chrono::nanoseconds>(ChecRecordIDFinishTime - ChecRecordIDStartTime);

                auto CheckOpStartTime = std::chrono::high_resolution_clock::now();
                // 3- Run Check Operator Module
                if(DH->Modules.CheckOperator.active && DH->hasInputFields.PlateImage)
                {
                    ChOp::InputStruct inputChOp;
                    inputChOp.plateImage = DH->ProcessedInputData.PlateImageMat;
                    inputChOp.plateValue = DH->hasInputFields.PlateValue ? DH->Input.PlateValue : "";
                    inputChOp.plateType = DH->hasInputFields.PlateType ? DH->Input.PlateType : static_cast<int>(inference::standards::PlateType::UNKNOWN);

                    int CheckOpObjectIndex = this->getCheckOpIndex();
                    ChOp::OutputStruct ChOpOutput;
                    try
                    {
                        ChOpOutput = this->m_pChOpObjects[CheckOpObjectIndex]->run(inputChOp);
                        this->releaseCheckOpIndex(CheckOpObjectIndex);
                        DH->Input.MasterPlate = DH->Input.PlateValue;
                        DH->Input.PlateValue = ChOpOutput.newPlateValue;
                        DH->Input.CodeType = ChOpOutput.codeType;
                        DH->Input.Probability = ChOpOutput.probability;
                    } catch (const std::exception& e)
                    {
                        Response["Status"] = CHECKOPERROR;
                        Response["Description"] = e.what();
                        if(DH->hasInputFields.DeviceID && DH->hasInputFields.ViolationID && DH->hasInputFields.PassedTime && DH->hasInputFields.PlateValue)
                            Response["RecordID"] = DH->ProcessedInputData.MongoID;
                        Response["Partition"] = InputData.partition;
                        Response["Offset"] = InputData.offset;
                        if(DH->FailedDatabaseInfo.Enable)
                        {
                            std::vector<MongoDB::Field> fields = {
                                {"Status", std::to_string(CHECKOPERROR), MongoDB::FieldType::Integer},
                                {"Description", e.what(), MongoDB::FieldType::String},
                                {"Partition", std::to_string(InputData.partition), MongoDB::FieldType::Integer},
                                {"Offset", std::to_string(InputData.offset), MongoDB::FieldType::Integer}
                            };

                            if(DH->hasInputFields.DeviceID && DH->hasInputFields.ViolationID && DH->hasInputFields.PassedTime && DH->hasInputFields.PlateValue)
                            {
                                MongoDB::Field RecordIDField = {"RecordID", DH->ProcessedInputData.MongoID, MongoDB::FieldType::ObjectId};
                                fields.push_back(RecordIDField);
                            }

                            DH->FailedDatabase->Insert(DH->FailedDatabaseInfo.DatabaseName, DH->FailedDatabaseInfo.CollectionName, fields);
                        }
                        if(DH->DebugMode)
                            SHOW_ERROR(crow::json::dump(Response));
                        this->releaseCheckOpIndex(CheckOpObjectIndex);
                        continue;
                    }
                }
                auto CheckOpFinishTime = std::chrono::high_resolution_clock::now();
                auto CheckOpTime =  std::chrono::duration_cast<std::chrono::nanoseconds>(CheckOpFinishTime - CheckOpStartTime);

                auto ClassifierStartTime = std::chrono::high_resolution_clock::now();
                // 4- Run Classifier Module
                if(DH->Modules.Classifier.active)
                {   
                    std::vector<Classifier::InputStruct> classifierModelsInput;
                    for(int i = 0; i < DH->Modules.Classifier.Models.size(); i++)
                    {
                        Classifier::InputStruct input;
                        input.useRect = DH->Modules.Classifier.Models[i].UseRect;

                        switch(DH->Modules.Classifier.Models[i].InputImageType) 
                        {
                            case 0:
                            {
                                input.Image = DH->hasInputFields.ColorImage ? DH->ProcessedInputData.ColorImageMat : cv::Mat(0, 0, CV_8UC3);
                                break;
                            }
                            case 1:
                            {
                                input.Image = DH->hasInputFields.PlateImage ? DH->ProcessedInputData.PlateImageMat : cv::Mat(0, 0, CV_8UC3);
                                break;
                            }
                        }

                        switch(DH->Modules.Classifier.Models[i].InputRectField) 
                        {
                            case 0:
                            {
                                input.desiredRect = cv::Rect(0,0,0,0);
                                break;
                            }
                            case 1:
                            {
                                input.desiredRect = DH->hasInputFields.CarRect ? DH->ProcessedInputData.CarRect : cv::Rect(0,0,0,0);
                                break;
                            }
                            case 2:
                            {
                                input.desiredRect = DH->hasInputFields.PlateRect ? DH->ProcessedInputData.PlateRect : cv::Rect(0,0,0,0);
                                break;
                            }
                        }

                        classifierModelsInput.push_back(input);
                    }

                    int ClassifierObjectIndex = this->getClassifierIndex();
                    Classifier::OutputStruct ClassifierOutput;

                    try
                    {
                        ClassifierOutput = this->m_pClassifierObjects[ClassifierObjectIndex]->run(classifierModelsInput);
                        this->releaseClassifierIndex(ClassifierObjectIndex);
                        DH->ProcessedInputData.ClassifierModuleOutput = ClassifierOutput.keyLabels;
                        // for(const auto& keyLabel : ClassifierOutput.keyLabels)
                        //     SHOW_IMPORTANTLOG2(keyLabel.first << " = " << keyLabel.second);
                    } 
                    catch (const std::exception& e)
                    {
                        Response["Status"] = CLASSIFIERERROR;
                        Response["Description"] = e.what();
                        if(DH->hasInputFields.DeviceID && DH->hasInputFields.ViolationID && DH->hasInputFields.PassedTime && DH->hasInputFields.PlateValue)
                            Response["RecordID"] = DH->ProcessedInputData.MongoID;
                        Response["Partition"] = InputData.partition;
                        Response["Offset"] = InputData.offset;
                        if(DH->FailedDatabaseInfo.Enable)
                        {
                            std::vector<MongoDB::Field> fields = {
                                {"Status", std::to_string(CLASSIFIERERROR), MongoDB::FieldType::Integer},
                                {"Description", e.what(), MongoDB::FieldType::String},
                                {"Partition", std::to_string(InputData.partition), MongoDB::FieldType::Integer},
                                {"Offset", std::to_string(InputData.offset), MongoDB::FieldType::Integer}
                            };

                            if(DH->hasInputFields.DeviceID && DH->hasInputFields.ViolationID && DH->hasInputFields.PassedTime && DH->hasInputFields.PlateValue)
                            {
                                MongoDB::Field RecordIDField = {"RecordID", DH->ProcessedInputData.MongoID, MongoDB::FieldType::ObjectId};
                                fields.push_back(RecordIDField);
                            }

                            DH->FailedDatabase ->Insert(DH->FailedDatabaseInfo.DatabaseName, DH->FailedDatabaseInfo.CollectionName, fields);
                        }
                        if(DH->DebugMode)
                            SHOW_ERROR(crow::json::dump(Response));
                        this->releaseClassifierIndex(ClassifierObjectIndex);
                        continue;
                    }
                }

                auto ClassifierFinishTime = std::chrono::high_resolution_clock::now();
                auto ClassifierTime =  std::chrono::duration_cast<std::chrono::nanoseconds>(ClassifierFinishTime - ClassifierStartTime);


                auto storeImageStartTime = std::chrono::high_resolution_clock::now();
#ifdef STOREIMAGE
                // 5- Store Image
                std::shared_ptr<storeimage> storeimageobj = std::make_shared<storeimage>();
                if(!(storeimageobj->run(DH)))
                {
                    Response["Status"] = DH->Response.errorCode;
                    Response["Description"] = DH->Response.Description;
                    if(DH->hasInputFields.DeviceID && DH->hasInputFields.ViolationID && DH->hasInputFields.PassedTime && DH->hasInputFields.PlateValue)
                        Response["RecordID"] = DH->ProcessedInputData.MongoID;
                    Response["Partition"] = InputData.partition;
                    Response["Offset"] = InputData.offset;
                    if(DH->FailedDatabaseInfo.Enable)
                    {
                        std::vector<MongoDB::Field> fields = {
                            {"Status", std::to_string(DH->Response.errorCode), MongoDB::FieldType::Integer},
                            {"Description", DH->Response.Description, MongoDB::FieldType::String},
                            {"Partition", std::to_string(InputData.partition), MongoDB::FieldType::Integer},
                            {"Offset", std::to_string(InputData.offset), MongoDB::FieldType::Integer}
                        };

                        if(DH->hasInputFields.DeviceID && DH->hasInputFields.ViolationID && DH->hasInputFields.PassedTime && DH->hasInputFields.PlateValue)
                        {
                            MongoDB::Field RecordIDField = {"RecordID", DH->ProcessedInputData.MongoID, MongoDB::FieldType::ObjectId};
                            fields.push_back(RecordIDField);
                        }

                        DH->FailedDatabase ->Insert(DH->FailedDatabaseInfo.DatabaseName, DH->FailedDatabaseInfo.CollectionName, fields);
                    }
                    SHOW_ERROR(crow::json::dump(Response));
                    continue;
                }
#endif // STOREIMAGE        
                auto storeImageFinishTime = std::chrono::high_resolution_clock::now();
                auto storeImaheTime =  std::chrono::duration_cast<std::chrono::nanoseconds>(storeImageFinishTime - storeImageStartTime);        

                auto saveDataStartTime = std::chrono::high_resolution_clock::now();
#if defined KAFKAOUTPUT || defined INSERTDATABASE
                // 6- Save Data
                std::shared_ptr<savedata> savedataobj = std::make_shared<savedata>();
#ifdef KAFKAOUTPUT
                int OutputKafkaConnectionIndex = this->getKafkaConnectionIndex();
                savedataobj->setOutputKafkaConnection(this->OutputKafkaConnections[OutputKafkaConnectionIndex]);
#endif // KAFKAOUTPUT
                if(!(savedataobj->run(DH)))
                {
                    Response["Status"] = DH->Response.errorCode;
                    Response["Description"] = DH->Response.Description;
                    if(DH->hasInputFields.DeviceID && DH->hasInputFields.ViolationID && DH->hasInputFields.PassedTime && DH->hasInputFields.PlateValue)
                        Response["RecordID"] = DH->ProcessedInputData.MongoID;
                    Response["Partition"] = InputData.partition;
                    Response["Offset"] = InputData.offset;
                    if(DH->FailedDatabaseInfo.Enable)
                    {
                        std::vector<MongoDB::Field> fields = {
                            {"Status", std::to_string(DH->Response.errorCode), MongoDB::FieldType::Integer},
                            {"Description", DH->Response.Description, MongoDB::FieldType::String},
                            {"Partition", std::to_string(InputData.partition), MongoDB::FieldType::Integer},
                            {"Offset", std::to_string(InputData.offset), MongoDB::FieldType::Integer}
                        };

                        if(DH->hasInputFields.DeviceID && DH->hasInputFields.ViolationID && DH->hasInputFields.PassedTime && DH->hasInputFields.PlateValue)
                        {
                            MongoDB::Field RecordIDField = {"RecordID", DH->ProcessedInputData.MongoID, MongoDB::FieldType::ObjectId};
                            fields.push_back(RecordIDField);
                        }

                        DH->FailedDatabase ->Insert(DH->FailedDatabaseInfo.DatabaseName, DH->FailedDatabaseInfo.CollectionName, fields);
                    }
                    SHOW_ERROR(crow::json::dump(Response));
                    continue;
                }
#ifdef KAFKAOUTPUT
                this->releaseKafkaIndex(OutputKafkaConnectionIndex);
#endif // KAFKAOUTPUT
#endif // KAFKAOUTPUT || INSERTDATABASE
                auto saveDataFinishTime = std::chrono::high_resolution_clock::now();
                auto saveDataTime =  std::chrono::duration_cast<std::chrono::nanoseconds>(saveDataFinishTime - saveDataStartTime);     

                auto requestFinishTime = std::chrono::high_resolution_clock::now();
                auto requestTime =  std::chrono::duration_cast<std::chrono::nanoseconds>(requestFinishTime - requstStartTime);
                
                if(DH->DebugMode)
                    SHOW_IMPORTANTLOG3("ProccessTime(ns) = " << std::to_string(requestTime.count()) << std::endl << "1- Validation ProccessTime(ns) = " << std::to_string(ValidationTime.count())
                           << std::endl << "2- Check RecordID ProccessTime(ns) = " << std::to_string(ChecRecordIDTime.count()) << std::endl << "3- CheckOp ProccessTime(ns) = " << std::to_string(CheckOpTime.count())
                           << std::endl << "4- Classifier ProccessTime(ns) = " << std::to_string(ClassifierTime.count())
                           << std::endl << "5- Store image ProccessTime(ns) = " << std::to_string(storeImaheTime.count())
                           << std::endl << "6- Save data ProccessTime(ns) = " << std::to_string(saveDataTime.count()));

                Response["Status"] = SUCCESSFUL;
                if(DH->hasInputFields.DeviceID && DH->hasInputFields.ViolationID && DH->hasInputFields.PassedTime && DH->hasInputFields.PlateValue)
                    Response["RecordID"] = DH->ProcessedInputData.MongoID;
                SHOW_LOG(crow::json::dump(Response));
            }

        }
        catch (std::exception e)
        {

            SHOW_ERROR("Error Code 0x" + std::to_string(__LINE__) + " KAFKASERVICEFILE : " + e.what());
        }
    }
}