#include "chop.h"


// Define static members
boost::mutex ChOp::mtx_ChOp; // Initialize the mutex
boost::mutex ChOp::mtx_Saving; // Mutex for synchronizing access to shared data


std::atomic<uint32_t> ChOp::totalChOpTime(0); // Initialize totalChOpTime
std::atomic<uint32_t> ChOp::ChOpCount(0); // Initialize ChOpCount
std::atomic<uint32_t> ChOp::totalSavingTime(0); // Initialize totalSavingTime
std::atomic<uint32_t> ChOp::savingCount(0); // Initialize savingCount


std::string ChOp::getVersion(){return "1.0.2";}

ChOp::ChOp(const ChOp::ConfigStruct& conf)
{
    try
    {
        if(conf.PDConfig.active)
        {
            inference::ConfigStruct modelConfig = {conf.PDConfig.model, conf.PDConfig.modelConfig};
            this->m_models.PD = std::make_shared<inference::Handler>(modelConfig);
        }

        if(conf.PCConfig.active)
        {
            inference::ConfigStruct modelConfig = {conf.PCConfig.model, conf.PCConfig.modelConfig};
            this->m_models.PC = std::make_shared<inference::Handler>(modelConfig);
        }

        if(conf.IROCRConfig.active)
        {
            inference::ConfigStruct modelConfig = {conf.IROCRConfig.model, conf.IROCRConfig.modelConfig};
            this->m_models.IROCR = std::make_shared<inference::Handler>(modelConfig);
        }

        if(conf.MBOCRConfig.active)
        {
            inference::ConfigStruct modelConfig = {conf.MBOCRConfig.model, conf.MBOCRConfig.modelConfig};
            this->m_models.MBOCR = std::make_shared<inference::Handler>(modelConfig);
        }

        if(conf.TZOCRConfig.active)
        {
            inference::ConfigStruct modelConfig = {conf.TZOCRConfig.model, conf.TZOCRConfig.modelConfig};
            this->m_models.TZOCR = std::make_shared<inference::Handler>(modelConfig);
        }
        
        if(conf.FZOCRConfig.active)
        {
            inference::ConfigStruct modelConfig = {conf.FZOCRConfig.model, conf.FZOCRConfig.modelConfig};
            this->m_models.FZOCR = std::make_shared<inference::Handler>(modelConfig);
        }

        if(conf.FROCRConfig.active)
        {
            inference::ConfigStruct modelConfig = {conf.FROCRConfig.model, conf.FROCRConfig.modelConfig};
            this->m_models.FROCR = std::make_shared<inference::Handler>(modelConfig);
        }

        this->m_ignoreInputPlateType = conf.ignoreInputPlateType;
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(e.what());
    }
}
size_t ChOp::getSize() {
    ProcessInputVecMutex.lock();
    size_t sizeQueue = ProcessInputVec.size();
    ProcessInputVecMutex.unlock();
    return sizeQueue;
}

void ChOp::process()
{
    while (true)
    {
        if(ProcessInputVec.size() > 0)
        {
            // ProcessInputStruct Input;
            std::shared_ptr<DataHandler::DataHandlerStruct> DH = std::make_shared<DataHandler::DataHandlerStruct>();
            ProcessInputVecMutex.lock();
            if(ProcessInputVec.size() > 0)
            {
                DH = ProcessInputVec[0];
                ProcessInputVec.erase(ProcessInputVec.begin());
                ProcessInputVecMutex.unlock();
            }
            else
            {
                ProcessInputVecMutex.unlock();
                continue;
            }

            crow::json::wvalue Response;
            ChOp::InputStruct inputChOp;

            inputChOp.plateImage = DH->ProcessedInputData.PlateImageMat;
            inputChOp.plateValue = DH->Input.PlateValue;
            inputChOp.plateImageBase64 = DH->Input.PlateImageBase64;
            inputChOp.plateType = DH->hasInputFields.PlateType ? DH->Input.PlateType : static_cast<int>(inference::standards::PlateType::UNKNOWN);//#TODO WHAT IS PLATETYPE?
            OutputStruct OutPutchOp;
            auto CheckOpStartTime = std::chrono::high_resolution_clock::now();
            try
            {
                OutPutchOp = run(inputChOp);
                auto CheckOpFinishTime = std::chrono::high_resolution_clock::now();
                double CheckOpTime =  std::chrono::duration_cast<std::chrono::milliseconds>(CheckOpFinishTime - CheckOpStartTime).count();
                // SHOW_IMPORTANTLOG("CheckOpTime: (ms)" << CheckOpTime << " >>>>>  Record ID : " << DH->Input.PassedVehicleRecordsId << " >>>>> thredID: " <<boost::this_thread::get_id());
                DH->Input.MasterPlate = DH->Input.PlateValue;
                DH->Input.PlateValue = OutPutchOp.newPlateValue;
                DH->Input.CodeType = OutPutchOp.codeType;
                DH->Input.Probability = OutPutchOp.probability;
                DH->Input.PlateType = OutPutchOp.newPlateType;
                DH->ProcessedInputData.croppedPlateImage = OutPutchOp.croppedPlateImage;


                if(DH->DebugMode)
                {
                    mtx_ChOp.lock();
                    // SHOW_IMPORTANTLOG("CheckOpTime: (ms)" << CheckOpTime << " >>>>>  Record ID : " << DH->Input.PassedVehicleRecordsId << " >>>>> thredID: " <<boost::this_thread::get_id());
                    totalChOpTime += CheckOpTime;
                    ChOpCount++;
                    mtx_ChOp.unlock();

                }

            }
            catch(const std::exception& e)
            {
                
                Response["Status"] = CHECKOPERROR;
                Response["Description"] = e.what();
                // if(DH->hasInputFields.DeviceID && DH->hasInputFields.ViolationID && DH->hasInputFields.PassedTime && DH->hasInputFields.PlateValue)
                Response["RecordID"] = DH->Input.PassedVehicleRecordsId; //DH->ProcessedInputData.MongoID;
                Response["CompanyCode"] = DH->Input.CompanyCode;
                if(DH->FailedDatabaseInfo.Enable)
                {
                    std::vector<MongoDB::Field> fields = {
                        {"Status", std::to_string(CHECKOPERROR), MongoDB::FieldType::Integer},
                        {"Description", e.what(), MongoDB::FieldType::String},
                        {"CompanyCode", std::to_string(DH->Input.CompanyCode), MongoDB::FieldType::Integer}
                    };

                    // if(DH->hasInputFields.DeviceID && DH->hasInputFields.ViolationID && DH->hasInputFields.PassedTime && DH->hasInputFields.PlateValue)
                    // {
                    MongoDB::Field RecordIDField = {"RecordID", std::to_string(DH->Input.PassedVehicleRecordsId), MongoDB::FieldType::Int64};
                    fields.push_back(RecordIDField);
                    // }

                    DH->FailedDatabase ->Insert(DH->FailedDatabaseInfo.DatabaseName, DH->FailedDatabaseInfo.CollectionName, fields);
                }
                if(DH->DebugMode)
                    Logger::getInstance().logError(crow::json::dump(Response));
            }

            // 5- Store Image
            std::shared_ptr<storeimage> storeimageobj = std::make_shared<storeimage>();

            auto SaveStartTime = std::chrono::high_resolution_clock::now();

            if(!(storeimageobj->run(DH)))
            {
                Response["Status"] = DH->Response.errorCode;
                Response["Description"] = DH->Response.Description;
                // if(DH->hasInputFields.DeviceID && DH->hasInputFields.ViolationID && DH->hasInputFields.PassedTime && DH->hasInputFields.PlateValue)
                Response["RecordID"] = DH->Input.PassedVehicleRecordsId;//DH->ProcessedInputData.MongoID;
                Response["CompanyCode"] = DH->Input.CompanyCode;
                if(DH->FailedDatabaseInfo.Enable)
                {
                    std::vector<MongoDB::Field> fields = {
                    {"Status", std::to_string(DH->Response.errorCode), MongoDB::FieldType::Integer},
                    {"Description", DH->Response.Description, MongoDB::FieldType::String},
                    {"CompanyCode", std::to_string(DH->Input.CompanyCode), MongoDB::FieldType::Integer}
                };
                //                 if(DH->hasInputFields.DeviceID && DH->hasInputFields.ViolationID && DH->hasInputFields.PassedTime && DH->hasInputFields.PlateValue)
                // {
                MongoDB::Field RecordIDField = {"RecordID", std::to_string(DH->Input.PassedVehicleRecordsId), MongoDB::FieldType::Int64};
                fields.push_back(RecordIDField);
                // }

                DH->FailedDatabase ->Insert(DH->FailedDatabaseInfo.DatabaseName, DH->FailedDatabaseInfo.CollectionName, fields);

                }
                if(DH->DebugMode)
                    Logger::getInstance().logError(crow::json::dump(Response));

            }

            // 6- Save Data
            std::shared_ptr<savedata> savedataobj = std::make_shared<savedata>();   
            if(!(savedataobj->run(DH)))
            {
                Response["Status"] = DH->Response.errorCode;
                Response["Description"] = DH->Response.Description;
                // if(DH->hasInputFields.DeviceID && DH->hasInputFields.ViolationID && DH->hasInputFields.PassedTime && DH->hasInputFields.PlateValue)
                Response["RecordID"] = DH->Input.PassedVehicleRecordsId;
                Response["CompanyCode"] = DH->Input.CompanyCode;
                if(DH->FailedDatabaseInfo.Enable)
                {
                    std::vector<MongoDB::Field> fields = {
                    {"Status", std::to_string(DH->Response.errorCode), MongoDB::FieldType::Integer},
                    {"Description", DH->Response.Description, MongoDB::FieldType::String},
                    {"CompanyCode", std::to_string(DH->Input.CompanyCode), MongoDB::FieldType::Integer}
                };
                //                 if(DH->hasInputFields.DeviceID && DH->hasInputFields.ViolationID && DH->hasInputFields.PassedTime && DH->hasInputFields.PlateValue)
                // {
                MongoDB::Field RecordIDField = {"RecordID", std::to_string(DH->Input.PassedVehicleRecordsId), MongoDB::FieldType::Int64};
                fields.push_back(RecordIDField);
                // }

                DH->FailedDatabase ->Insert(DH->FailedDatabaseInfo.DatabaseName, DH->FailedDatabaseInfo.CollectionName, fields);

                }

                Logger::getInstance().logError(crow::json::dump(Response));

            }

            auto SaveEndTime = std::chrono::high_resolution_clock::now();
            double durationSaveTime = std::chrono::duration_cast<std::chrono::milliseconds>(SaveEndTime - SaveStartTime).count();

            // MinId_mutex.lock();
            std::vector<MongoDB::Field> MetaUpdateFields = { 
                {"PassedVehicleRecordsId", std::to_string(DH->Input.PassedVehicleRecordsId), MongoDB::FieldType::Int64, "set"} //TODO  Think for better 
            };
            DH->ConfigDatabase->Update_one(DH->ConfigDatabaseInfo.DatabaseName,"Meta",MetaFindFields,MetaUpdateFields);

            // MinId_mutex.unlock();


            if(DH->DebugMode)
            {
                mtx_Saving.lock();
                totalSavingTime += durationSaveTime; // Use regular addition
                savingCount++;
                mtx_Saving.unlock();
                
            }
        }
        else
        {
            boost::this_thread::sleep_for(boost::chrono::microseconds(100));
            continue;
        }
        
       
    }
    boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
    

}
int count = 0;

ChOp::OutputStruct ChOp::run(const ChOp::InputStruct& input)
{
    ChOp::OutputStruct output;
    output.PlateImageBase64 = input.plateImageBase64;//TODO must remove only for test
    if(input.plateImage.empty())
    {
        output.codeType      = ChOp::CodeTypes::NULL_IMAGE;
        output.newPlateValue = input.plateValue;
        output.probability   = 0;
        output.newPlateType  = static_cast<int>(inference::standards::PlateType::UNKNOWN);
        return output;
    }
    
    try
    {
        cv::Mat PDImage = input.plateImage;
        if(PDImage.channels() < 3)
            cv::cvtColor(PDImage, PDImage, cv::COLOR_GRAY2BGR);
        if(PDImage.channels() == 4)
            cv::cvtColor(PDImage, PDImage, cv::COLOR_BGRA2BGR); 
        
        // Step 1 : Plate Detection 
        inference::OutputStruct PDOutput;
        if(this->m_models.PD)
        {
            float AspectRatio = (float)PDImage.cols / (float)PDImage.rows;
            if(AspectRatio < 2.5)
            {
                int topbot = std::round(PDImage.rows / 2);
                int rileft = std::round(PDImage.cols / 2);
                cv::copyMakeBorder(PDImage, PDImage, topbot, topbot, rileft, rileft, 0);

                inference::InputStruct PDInput;
                PDInput.Image = PDImage;

                this->m_models.PD->run(PDInput);
                PDOutput = this->m_models.PD->getOutput();

                if(PDOutput.modelOutputs.size() == 0)
                {
                    output.codeType = ChOp::CodeTypes::NOT_PLATE;
                    output.newPlateValue = input.plateValue;
                    output.probability = 0;
                    output.croppedPlateImage = PDInput.Image;
                    return output;
                }
            }
        }

        cv::Mat ocrImage;
        if(PDOutput.modelOutputs.size() > 0)
        {
            std::vector<int> scores;
            for(auto& object : PDOutput.modelOutputs)
                scores.push_back(object.labelScore);

            int maxIndex = std::max_element(scores.begin(), scores.end()) - scores.begin();
            cv::Rect plateRect = PDOutput.modelOutputs[maxIndex].box;
            /////////////////////////////////////////////////////////////////


            // cv::imwrite(std::to_string(count)+".jpg", PDImage(plateRect));

                //////////////////////////////////////////////////////////////////
            // plateRect.x       -= (0.1 * plateRect.width);
            // plateRect.width   *= 1.15;
            // plateRect.height  *= 1.15;
            // this->fixRectDimension(plateRect, PDImage.rows, PDImage.cols);
            ocrImage = PDImage(plateRect);
            // cv::imwrite("ocr"+std::to_string(count)+".jpg", ocrImage);

        } else 
        {
            ocrImage = PDImage;
        }

        // Step 2 : Plate Classifier
        int plateType;
        if(this->m_models.PC)
        {            
            if(this->m_ignoreInputPlateType || (input.plateType == static_cast<int>(inference::standards::PlateType::UNKNOWN)))
            {
                inference::InputStruct PCInput;
                PCInput.Image = ocrImage;
                this->m_models.PC->run(PCInput);
                auto PCOutput = this->m_models.PC->getOutput();

                std::vector<int> scores;
                for(auto& object : PCOutput.modelOutputs)
                    scores.push_back(object.labelScore);
                
                int maxIndex = std::max_element(scores.begin(), scores.end()) - scores.begin();
                if(scores[maxIndex] < 70)//TODO PrimaryThreshold
                    plateType = 0 ;
                else
                    plateType = PCOutput.modelOutputs[maxIndex].label;
            } else
            {
                plateType = input.plateType;
            }

        } else 
        {
            plateType = input.plateType;
        }
        output.newPlateType = plateType;

        // cv::imwrite("ocr"+std::to_string(count)+"newPlateType_ "+std::to_string(output.newPlateType)+".jpg", ocrImage);
        // std::string cmd = "echo \"newPlateType: " + std::to_string(output.newPlateType) + "_ plateValue: " + input.plateValue + "_ image: " + output.PlateImageBase64 + "\" >> out1.txt";        
        // system(cmd.c_str());


    //     count++;

        // Step 3 : OCR
        inference::InputStruct ocrInput;
        ocrInput.Image = ocrImage;
        switch(plateType) 
        {
            case static_cast<int>(inference::standards::PlateType::UNKNOWN) :
            {
                output.codeType = ChOp::CodeTypes::NOT_PROCESSED;
                output.newPlateValue = input.plateValue;
                output.probability = 0;
                output.croppedPlateImage = ocrInput.Image;
                break;
            }
            case static_cast<int>(inference::standards::PlateType::IR) :
            {
                if(this->m_models.IROCR)
                {
                    this->m_models.IROCR->run(ocrInput);
                    auto IROCROutput = this->m_models.IROCR->getOutput();

                    if(IROCROutput.modelOutputs.size() != 0)
                    {
                        output.codeType = this->calculateIrCodeType(IROCROutput.OCRResult.plateValue, input.plateValue);
                        output.newPlateValue = IROCROutput.OCRResult.plateValue;
                        output.probability = this->calculateProbability(IROCROutput);
                        output.croppedPlateImage = ocrInput.Image;
                    } else 
                    {
                        output.codeType = ChOp::CodeTypes::NOT_PROCESSED;
                        output.newPlateValue = input.plateValue;
                        output.probability = 0;
                        output.croppedPlateImage = ocrInput.Image;
                    }
                } else
                {
                    output.codeType = ChOp::CodeTypes::NOT_PROCESSED;
                    output.newPlateValue = input.plateValue;
                    output.probability = 0;
                    output.croppedPlateImage = ocrInput.Image;
                }
                
                break;
            }
            case static_cast<int>(inference::standards::PlateType::MB) :
            {
                if(this->m_models.MBOCR)
                {
                    this->m_models.MBOCR->run(ocrInput);
                    auto MBOCROutput = this->m_models.MBOCR->getOutput();

                    output.codeType = ChOp::CodeTypes::MOTOR_BIKE;
                    if(MBOCROutput.modelOutputs.size() != 0)
                    {
                        output.newPlateValue = MBOCROutput.OCRResult.plateValue;
                        output.probability = this->calculateProbability(MBOCROutput);
                        output.croppedPlateImage = ocrInput.Image;
                    } else 
                    {
                        output.newPlateValue = input.plateValue;
                        output.probability = 0;
                        output.croppedPlateImage = ocrInput.Image;
                    }
                } else
                {
                    output.codeType = ChOp::CodeTypes::NOT_PROCESSED;
                    output.newPlateValue = input.plateValue;
                    output.probability = 0;
                    output.croppedPlateImage = ocrInput.Image;
                }
                
                break;
            }
            case static_cast<int>(inference::standards::PlateType::FREE_ZONE) :
            {
                if(this->m_models.FZOCR)
                {
                    this->m_models.FZOCR->run(ocrInput);
                    auto FZOCROutput = this->m_models.FZOCR->getOutput();

                    output.codeType = ChOp::CodeTypes::FREE_ZONE;
                    if(FZOCROutput.modelOutputs.size() != 0)
                    {
                        output.newPlateValue = FZOCROutput.OCRResult.plateValue;
                        output.probability = this->calculateProbability(FZOCROutput);
                        output.croppedPlateImage = ocrInput.Image;
                    } else 
                    {
                        output.newPlateValue = input.plateValue;
                        output.probability = 0;
                        output.croppedPlateImage = ocrInput.Image;
                    }
                } else
                {
                    output.codeType = ChOp::CodeTypes::NOT_PROCESSED;
                    output.newPlateValue = input.plateValue;
                    output.probability = 0;
                    output.croppedPlateImage = ocrInput.Image;
                }
                
                break;
            }
            case static_cast<int>(inference::standards::PlateType::NEW_TRANSIT) :
            case static_cast<int>(inference::standards::PlateType::OLD_TRANSIT) :
            {
                if(this->m_models.TZOCR)
                {
                    this->m_models.TZOCR->run(ocrInput);
                    auto TZOCROutput = this->m_models.TZOCR->getOutput();

                    output.codeType = ChOp::CodeTypes::TRANSIT;
                    if(TZOCROutput.modelOutputs.size() != 0)
                    {
                        output.newPlateValue = TZOCROutput.OCRResult.plateValue;
                        output.probability = this->calculateProbability(TZOCROutput);
                        output.croppedPlateImage = ocrInput.Image;
                    } else 
                    {
                        output.newPlateValue = input.plateValue;
                        output.probability = 0;
                        output.croppedPlateImage = ocrInput.Image;
                    }
                } else
                {
                    output.codeType = ChOp::CodeTypes::NOT_PROCESSED;
                    output.newPlateValue = input.plateValue;
                    output.probability = 0;
                    output.croppedPlateImage = ocrInput.Image;
                }
                
                break;
            }
            default :
            {
                if(this->m_models.FROCR)
                {
                    this->m_models.FROCR->run(ocrInput);
                    auto FROCROutput = this->m_models.FROCR->getOutput();

                    output.codeType = ChOp::CodeTypes::FOREIGN;
                    if(FROCROutput.modelOutputs.size() != 0)
                    {
                        output.newPlateValue = FROCROutput.OCRResult.plateValue;
                        output.probability = this->calculateProbability(FROCROutput);
                        output.newPlateType = FROCROutput.OCRResult.plateType;
                        output.croppedPlateImage = ocrInput.Image;
                    } else 
                    {
                        output.newPlateValue = input.plateValue;
                        output.probability = 0;
                        output.croppedPlateImage = ocrInput.Image;
                    }
                    
                } else
                {
                    output.codeType = ChOp::CodeTypes::NOT_PROCESSED;
                    output.newPlateValue = input.plateValue;
                    output.probability = 0;
                    output.croppedPlateImage = ocrInput.Image;
                }

                break;
            }
        }

    }
    catch (const std::exception& e)
    {
        std::string ErrorLog = "ChOp Error : " + std::string(e.what());
        throw std::runtime_error(ErrorLog);
    }

    return output;
}

void ChOp::fixRectDimension(cv::Rect& candRect,int row, int col)
{
    candRect.x      = (candRect.x > 0) ? candRect.x : 0;
    candRect.y      = (candRect.y > 0) ? candRect.y : 0;
    candRect.width  = ((candRect.x + candRect.width) > col) ? (col - candRect.x) : candRect.width;
    candRect.height = ((candRect.y + candRect.height) > row) ? (row - candRect.y) : candRect.height;

    if((candRect.width < 0) || (candRect.height < 0))
        candRect = cv::Rect(0,0,0,0); 
}

int ChOp::calculateIrCodeType(const std::string& newPlateValue, const std::string& oldPlateValue)
{
    if(newPlateValue == oldPlateValue)
    {
        return ChOp::CodeTypes::SURE;
    } else if(newPlateValue.substr(2,2) != oldPlateValue.substr(2,2))
    {
        return ChOp::CodeTypes::DIFFERENT_ALPH;
    } else if(newPlateValue.substr(7,2) != oldPlateValue.substr(7,2))
    {
        return ChOp::CodeTypes::Two_SMALL_NUM;
    } else if(newPlateValue.substr(0,2) != oldPlateValue.substr(0,2))
    {
        return ChOp::CodeTypes::TWO_BIG_NUM;
    } else if(newPlateValue.substr(4,3) != oldPlateValue.substr(4,3))
    {
        return ChOp::CodeTypes::THREE_BIG_NUM;
    } else
    {
        return ChOp::CodeTypes::OTHER;
    }
}

int ChOp::calculateProbability(const inference::OutputStruct& modelOutput)
{
    std::vector<int> scores;
    for(auto& object : modelOutput.modelOutputs)
        scores.push_back(object.labelScore);
    
    int sum = std::accumulate(scores.begin(), scores.end(), 0);
    int mean = sum / scores.size();
    return mean;
}