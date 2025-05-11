#include "chop.h"


// Define static members
boost::mutex ChOp::mtx_ChOp; // Initialize the mutex
boost::mutex ChOp::mtx_Saving; // Mutex for synchronizing access to shared data


std::atomic<uint32_t> ChOp::totalChOpTime(0); // Initialize totalChOpTime
std::atomic<uint32_t> ChOp::ChOpCount(0); // Initialize ChOpCount
std::atomic<uint32_t> ChOp::totalSavingTime(0); // Initialize totalSavingTime
std::atomic<uint32_t> ChOp::savingCount(0); // Initialize savingCount


std::string ChOp::getVersion(){return "1.0.2";}



std::unique_ptr<NNModel> ChOp::getModel(BaseNNConfig& conf, const std::string& modelData, const string& path) {
    if (conf.read(path) != 0) {
        cerr << "Failed to read config: " << path << endl;
        throw runtime_error("Config read failed");
    }
    aivision::ModelFactory factory;
    auto model = factory.get(conf);
    int read = 0;
    char* pChars = readAllBytes(modelData, &read);//readAllBytes(conf.modelPath.c_str(), &read);
    if (!pChars || read == 0) {
        cerr << "Failed to read model file: " << conf.modelPath << endl;
        throw runtime_error("Model file read failed");
    }
    conf.modelData = pChars;
    conf.modelByteSize = read;
    if (model->init(conf) != 0) {
        cerr << "Failed to init model: " << conf.modelPath << endl;
        delete[] pChars;
        throw runtime_error("Model init failed");
    }
    delete[] pChars;
    return model;
}


ChOp::ChOp(const ChOp::ConfigStruct& conf)
{
    try
    {
        if(conf.PDConfig.active)
        {
            BaseNNConfig detConfig;
            this->m_models.PD = getModel(detConfig, conf.PDConfig.model,conf.PDConfig.modelConfig);

        }

        if(conf.PCConfig.active)
        {
            BaseNNConfig PCConfig;
            // string PCConfigPath = "/home/amiri/projects/c++/models/PlateClassifierPackage/inference_pipeline_v2.1.0/models/plateClassification/info.json";
            this->m_models.PC = getModel(PCConfig, conf.PCConfig.model , conf.PCConfig.modelConfig);

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
            BaseNNConfig FROCRConfig;
            this->m_models.FROCR = getModel(FROCRConfig, conf.FROCRConfig.model,conf.FROCRConfig.modelConfig);

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


ChOp::OutputStruct ChOp::run(const ChOp::InputStruct& input)
{
    ChOp::OutputStruct output;

    cv::Mat PlateImageMat = createMatImage(input.plateImageBase64); 
    // std::cout<<input.plateImageBase64<<std::endl;
    if(PlateImageMat.total() == 0)
    {
        output.codeType      = ChOp::CodeTypes::NULL_IMAGE;
        output.newPlateValue = input.plateValue;
        output.probability   = 0;
        output.newPlateType  = static_cast<int>(gocr::PlateType::UNKNOWN);
        return output;
    }

    
    try
    {
        if(PlateImageMat.channels() < 3)
            cv::cvtColor(PlateImageMat, PlateImageMat, cv::COLOR_GRAY2BGR);
        if(PlateImageMat.channels() == 4)
            cv::cvtColor(PlateImageMat, PlateImageMat, cv::COLOR_BGRA2BGR); 
        
        std::vector<aivision::nn::ObjectAttributes> dets;
        // Step 1 : Plate Detection 
        if(this->m_models.PD)
        {
            float AspectRatio = (float)PlateImageMat.cols / (float)PlateImageMat.rows;
            if(AspectRatio < 2.5)
            {
                NNModel* detectorModel = this->m_models.PD.get();
                auto detStream = make_shared<DataModel>();
                detStream->addView({Point2f(0,0), Point2f(1,0), Point2f(1,1), Point2f(0,1)}, "offline");
                detStream->SetFrame(PlateImageMat);
                detStream->SetActivePreset(1);
                vector<shared_ptr<DataModel>> detStreams = {detStream};
                detectorModel->forward(detStreams);
                dets = detStream->getResults();
                if(dets.empty()){
                    std::cerr << "no boxes found" << std::endl;
                }

                if(dets.size() == 0)
                {
                    output.codeType = ChOp::CodeTypes::NOT_PLATE;
                    output.newPlateValue = input.plateValue;
                    output.probability = 0;
                    output.croppedPlateImage = PlateImageMat;
                    return output;
                }
            }
            

        }

        cv::Mat ocrImage;
        if(dets.size() > 0)
        {
            std::vector<int> scores;
            for (const auto& det : dets)
                scores.push_back(det.score * 100);
            
            int maxIndex = std::max_element(scores.begin(), scores.end()) - scores.begin();
            Rect2d plateRect = dets[maxIndex].bbox;

            //Validate the ROI Parameters:
            if (plateRect.x >= 0 && plateRect.y >= 0 && 
                plateRect.width > 0 && plateRect.height > 0 && 
                plateRect.x + plateRect.width <= PlateImageMat.cols && 
                plateRect.y + plateRect.height <= PlateImageMat.rows) {
                ocrImage = PlateImageMat(plateRect).clone(); // Safe to use ROI
            }
            else{
                ocrImage = PlateImageMat.clone();
            }
        }
        else{
            ocrImage = PlateImageMat.clone();
        }

        // Step 2 : Plate Classifier

        int Type = -1;
        int CPlateType;
        if(this->m_models.PC)
        {
            NNModel* plateClassifierModel = this->m_models.PC.get();
            auto plateStream = make_shared<DataModel>();
            plateStream->addView({Point2f(0,0), Point2f(1,0), Point2f(1,1), Point2f(0,1)}, "offline");
            plateStream->SetFrame(ocrImage);
            plateStream->SetActivePreset(1);
            vector<shared_ptr<DataModel>> plateStreams = {plateStream};
            plateClassifierModel->forward(plateStreams);
            auto platePreds = plateStream->getResults();


            string plateClass = "Unknown";
            float classConfidence = 0.0f;
            if (!platePreds.empty()) {
                // plateClass = platePreds[0].class_name.empty() ? to_string(platePreds[0].label) : platePreds[0].class_name;
                // classConfidence = platePreds[0].score;

                std::vector<int> scores;
                for(auto& object : platePreds)
                    scores.push_back(object.score * 100);
                
                int maxIndex = std::max_element(scores.begin(), scores.end()) - scores.begin();
                CPlateType = platePreds[maxIndex].label;
            }
    //match lables
            switch (CPlateType)
            {
            case 1:
            {
                Type = 3;
                break;
            }

            case 2:
            {
                Type = 1;
                break;
            }

            case 3:
            {
                Type = 4;//TODO OLD OR NEW
                break;
            }

            case 4 :
            {
                Type = 2;
                break;
            }

            case 5:
            {
                Type = 0;
                break;
            }

            default:
                Type = 5;
                break;
            }

        }

        output.newPlateType = Type;
        
// Step 3 : OCR
        inference::InputStruct ocrInput;
        ocrInput.Image = ocrImage;
        switch(Type) 
        {   
            case static_cast<int>(gocr::PlateType::UNKNOWN) :
            {
                output.codeType = ChOp::CodeTypes::NOT_PROCESSED;
                output.newPlateValue = input.plateValue;
                output.probability = 0;
                output.croppedPlateImage = ocrImage;
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
                        output.probability = this->calculateProbabilityStr(IROCROutput);
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
                        output.probability = this->calculateProbabilityStr(MBOCROutput);
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
                        output.probability = this->calculateProbabilityStr(FZOCROutput);
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
                        output.probability = this->calculateProbabilityStr(TZOCROutput);
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
                    string plateNumber = "";
                    int plateType = static_cast<int>(gocr::PlateType::UNKNOWN);

                    // Run OCR model
                    NNModel* ocrModel = this->m_models.FROCR.get();
                    auto ocrStream = make_shared<DataModel>();
                    ocrStream->addView({Point2f(0,0), Point2f(1,0), Point2f(1,1), Point2f(0,1)}, "offline");
                    ocrStream->SetFrame(ocrImage);
                    ocrStream->SetActivePreset(1);
                    vector<shared_ptr<DataModel>> ocrStreams = {ocrStream};
                    ocrModel->forward(ocrStreams);

                    // Convert OCR model output to ObjAttributes for GlobalPlateRecognition
                    auto ocrPreds = ocrStream->getResults();
                    vector<aivision::ObjAttributes> ocrAttributes;
                    for (const auto& pred : ocrPreds) {
                        aivision::ObjAttributes attr;
                        attr.label = pred.label;
                        attr.score = pred.score;
                        attr.bbox = pred.bbox;
                        ocrAttributes.push_back(attr);
                        // cout << "OCR char: label=" << attr.label << ", score=" << attr.score
                        //      << ", box=(" << attr.bbox.x << "," << attr.bbox.y << "," << attr.bbox.width << "," << attr.bbox.height << ")" << endl;
                    }



                    // Use GlobalPlateRecognition to process OCR results
                    shared_ptr<GlobalPlateRecognition> plateRecognitionEngine{new GlobalPlateRecognition()};
                    auto ocrResult = plateRecognitionEngine->Run(ocrAttributes , "");


                    // Map country to plateType (you may need to adjust this mapping based on your requirements)
                    if (ocrResult.Country == "None") {
                        plateType = static_cast<int>(gocr::PlateType::UNKNOWN);
                    } else if (ocrResult.Country == "IranFreeZone") {
                        plateType = static_cast<int>(gocr::PlateType::FREE_ZONE);
                    } else if (ocrResult.Country == "Iran") {
                        plateType = static_cast<int>(gocr::PlateType::IR);
                    } else if (ocrResult.Country == "IranTransit") { //TODO
                        plateType = static_cast<int>(PlateType::NEW_TRANSIT);
                    }
                    else{
                        plateType = static_cast<int>(PlateType::FOREIGN);
                    }

                    if(ocrResult.PlateValue.empty())
                    {
                        output.codeType = ChOp::CodeTypes::FOREIGN;
                        output.newPlateValue = input.plateValue;
                        output.probability = 0;
                        output.croppedPlateImage = ocrImage;
                        output.newPlateType = plateType;
                        return output;
                    }
                    else
                        plateNumber = ocrResult.PlateValue;


                output.codeType = this->calculateIrCodeType(plateNumber, input.plateValue);
                output.newPlateValue = plateNumber;
                output.probability = this->calculateProbability(ocrResult.ProbabilityVector);
                output.newPlateType = plateType;
                output.croppedPlateImage = ocrImage;
                }

                else
                {
                    output.codeType = ChOp::CodeTypes::NOT_PROCESSED;
                    output.newPlateValue = input.plateValue;
                    output.probability = 0;
                    output.croppedPlateImage = ocrImage;
                }
                break;
            }

        }


        // int maxIndex = -1;
        // float maxConfidence = -1.0f; // Assuming classConfidence is a float; adjust type if needed
        // for (size_t i = 0; i < results.size(); ++i) {
        //     if (results[i].classConfidence > maxConfidence) {
        //         maxConfidence = results[i].classConfidence;
        //         maxIndex = i;
        //     }
        // }
        //     // Now maxIndex holds the index with the highest classConfidence
        // if (maxIndex != -1) {
        //     output.codeType = calculateIrCodeType(results[maxIndex].plateNumber, input.plateValue);
        //     output.newPlateValue = results[maxIndex].plateNumber;
        //     output.probability = results[maxIndex].classConfidence;
        //     output.newPlateType = results[maxIndex].plateType;
        // }

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
    }
    const size_t minLength = 9; // Adjust based on your actual requirements
    if (newPlateValue.size() < minLength || oldPlateValue.size() < minLength)
    {
        return ChOp::CodeTypes::OTHER; // Or define a new CodeType for invalid input
    }
    
    else if(newPlateValue.substr(2,2) != oldPlateValue.substr(2,2))
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

int ChOp::calculateProbability(std::vector<double,std::allocator<double> > probVec)
{
    std::vector<int> scores;
    for(auto& object : probVec)
        scores.push_back(static_cast<int>(object* 100));
    
    int sum = std::accumulate(scores.begin(), scores.end(), 0);
    int mean = sum / scores.size();
    return mean;
}

int ChOp::calculateProbabilityStr(const inference::OutputStruct& modelOutput)
{
    std::vector<int> scores;
    for(auto& object : modelOutput.modelOutputs)
        scores.push_back(object.labelScore);
    
    int sum = std::accumulate(scores.begin(), scores.end(), 0);
    int mean = sum / scores.size();
    return mean;
}