#include "chop.h"

std::string ChOp::getVersion(){return "1.0.2";}

ChOp::ChOp(const ChOp::ConfigStruct& conf)
{
    // try
    // {
    //     if(conf.PDConfig.active)
    //     {
    //         inference::ConfigStruct modelConfig = {conf.PDConfig.model, conf.PDConfig.modelConfig};
    //         this->m_models.PD = std::make_shared<inference::Handler>(modelConfig);
    
    //     }

    //     if(conf.PCConfig.active)
    //     {
    //         inference::ConfigStruct modelConfig = {conf.PCConfig.model, conf.PCConfig.modelConfig};
    //         this->m_models.PC = std::make_shared<inference::Handler>(modelConfig);
    //     }

    //     if(conf.IROCRConfig.active)
    //     {
    //         inference::ConfigStruct modelConfig = {conf.IROCRConfig.model, conf.IROCRConfig.modelConfig};
    //         this->m_models.IROCR = std::make_shared<inference::Handler>(modelConfig);
    //     }

    //     if(conf.MBOCRConfig.active)
    //     {
    //         inference::ConfigStruct modelConfig = {conf.MBOCRConfig.model, conf.MBOCRConfig.modelConfig};
    //         this->m_models.MBOCR = std::make_shared<inference::Handler>(modelConfig);
    //     }

    //     if(conf.TZOCRConfig.active)
    //     {
    //         inference::ConfigStruct modelConfig = {conf.TZOCRConfig.model, conf.TZOCRConfig.modelConfig};
    //         this->m_models.TZOCR = std::make_shared<inference::Handler>(modelConfig);
    //     }
        
    //     if(conf.FZOCRConfig.active)
    //     {
    //         inference::ConfigStruct modelConfig = {conf.FZOCRConfig.model, conf.FZOCRConfig.modelConfig};
    //         this->m_models.FZOCR = std::make_shared<inference::Handler>(modelConfig);
    //     }

    //     if(conf.FROCRConfig.active)
    //     {
    //         inference::ConfigStruct modelConfig = {conf.FROCRConfig.model, conf.FROCRConfig.modelConfig};
    //         this->m_models.FROCR = std::make_shared<inference::Handler>(modelConfig);
    //     }

    //     this->m_ignoreInputPlateType = conf.ignoreInputPlateType;
    // }
    // catch (const std::exception& e)
    // {
    //     throw std::runtime_error(e.what());
    // }
}

std::unique_ptr<NNModel> ChOp::handler(const aivision::nn::BaseNNConfig::ConfigStruct& modelConfig)
{
    aivision::nn::BaseNNConfig config;

    if(config.read(modelConfig.modelConfig)!=0)
    {
        std::cerr << "Failed to read config file\n";
    // return -1;
    }
    aivision::ModelFactory factory;
    // this->m_models.PD =  factory.get(config);
    auto tmodel = factory.get(config);
    // if(this->m_models.PD == nullptr)
    if(tmodel == nullptr)
    {
        std::cerr << "Failed to get model\n";
        // return -2;
    }
    config.modelData = (const char*)modelConfig.model.c_str();
    config.modelByteSize = modelConfig.model.size();
    // auto ret = this->m_models.PD->init(config);
    auto ret = tmodel->init(config);
    if(ret!=0)
    {
        std::cerr << "Fail to initialize model\n";
        // return -1;
    }
    return tmodel;
}

int ChOp::init(const ChOp::ConfigStruct& conf)
{
    if(conf.PDConfig.active) //#TODO optimize it
    {
        // aivision::nn::BaseNNConfig::ConfigStruct modelConfig = {conf.PDConfig.model, conf.PDConfig.modelConfig}; 
        aivision::nn::BaseNNConfig config;
        std::cout<<conf.PDConfig.modelConfig<<std::endl;
        if(config.read(conf.PDConfig.modelConfig)!=0)
        {
            std::cerr << "Failed to read config file\n";
            return -1;
        }
        aivision::ModelFactory factory;
        this->m_models.PD =  factory.get(config);
        if(this->m_models.PD == nullptr)
        {
            std::cerr << "Failed to get model\n";
            return -2;
        }
        config.modelData = (const char*)conf.PDConfig.model.c_str();
        config.modelByteSize = conf.PDConfig.model.size();
        auto ret = this->m_models.PD->init(config);
        if(ret!=0)
        {
            std::cerr << "Fail to initialize model\n";
            return -1;
        }
    }

    if(conf.PCConfig.active)
    {
        aivision::nn::BaseNNConfig config;
        std::cout<<conf.PCConfig.modelConfig<<std::endl;
        if(config.read(conf.PCConfig.modelConfig)!=0)
        {
            std::cerr << "Failed to read config file\n";
            return -1;
        }
        aivision::ModelFactory factory;
        this->m_models.PC =  factory.get(config);
        if(this->m_models.PC == nullptr)
        {
            std::cerr << "Failed to get model\n";
            return -2;
        }
        config.modelData = (const char*)conf.PCConfig.model.c_str();
        config.modelByteSize = conf.PCConfig.model.size();
        auto ret = this->m_models.PC->init(config);
        if(ret!=0)
        {
            std::cerr << "Fail to initialize model\n";
            return -1;
        }
    }

    if(conf.IROCRConfig.active)
    {
        aivision::nn::BaseNNConfig config;

        if(config.read(conf.IROCRConfig.modelConfig)!=0)
        {
            std::cerr << "Failed to read config file\n";
            return -1;
        }
        aivision::ModelFactory factory;
        this->m_models.IROCR =  factory.get(config);
        if(this->m_models.IROCR == nullptr)
        {
            std::cerr << "Failed to get model\n";
            return -2;
        }
        config.modelData = (const char*)conf.IROCRConfig.model.c_str();
        config.modelByteSize = conf.IROCRConfig.model.size();
        auto ret = this->m_models.IROCR->init(config);
        if(ret!=0)
        {
            std::cerr << "Fail to initialize model\n";
            return -1;
        }
    }

    if(conf.MBOCRConfig.active)
    {
        aivision::nn::BaseNNConfig config;

        if(config.read(conf.MBOCRConfig.modelConfig)!=0)
        {
            std::cerr << "Failed to read config file\n";
            return -1;
        }
        aivision::ModelFactory factory;
        this->m_models.MBOCR =  factory.get(config);
        if(this->m_models.MBOCR == nullptr)
        {
            std::cerr << "Failed to get model\n";
            return -2;
        }
        config.modelData = (const char*)conf.MBOCRConfig.model.c_str();
        config.modelByteSize = conf.MBOCRConfig.model.size();
        auto ret = this->m_models.MBOCR->init(config);
        if(ret!=0)
        {
            std::cerr << "Fail to initialize model\n";
            return -1;
        }
    }

    if(conf.TZOCRConfig.active)
    {
        aivision::nn::BaseNNConfig config;

        if(config.read(conf.TZOCRConfig.modelConfig)!=0)
        {
            std::cerr << "Failed to read config file\n";
            return -1;
        }
        aivision::ModelFactory factory;
        this->m_models.TZOCR =  factory.get(config);
        if(this->m_models.TZOCR == nullptr)
        {
            std::cerr << "Failed to get model\n";
            return -2;
        }
        config.modelData = (const char*)conf.TZOCRConfig.model.c_str();
        config.modelByteSize = conf.TZOCRConfig.model.size();
        auto ret = this->m_models.TZOCR->init(config);
        if(ret!=0)
        {
            std::cerr << "Fail to initialize model\n";
            return -1;
        }
    }
    
    if(conf.FZOCRConfig.active)
    {
        aivision::nn::BaseNNConfig config;

        if(config.read(conf.FZOCRConfig.modelConfig)!=0)
        {
            std::cerr << "Failed to read config file\n";
            return -1;
        }
        aivision::ModelFactory factory;
        this->m_models.FZOCR =  factory.get(config);
        if(this->m_models.FZOCR == nullptr)
        {
            std::cerr << "Failed to get model\n";
            return -2;
        }
        config.modelData = (const char*)conf.FZOCRConfig.model.c_str();
        config.modelByteSize = conf.FZOCRConfig.model.size();
        auto ret = this->m_models.FZOCR->init(config);
        if(ret!=0)
        {
            std::cerr << "Fail to initialize model\n";
            return -1;
        }
    }

    if(conf.FROCRConfig.active)
    {
        aivision::nn::BaseNNConfig config;

        if(config.read(conf.FROCRConfig.modelConfig)!=0)
        {
            std::cerr << "Failed to read config file\n";
            return -1;
        }
        aivision::ModelFactory factory;
        this->m_models.FROCR =  factory.get(config);
        if(this->m_models.FROCR == nullptr)
        {
            std::cerr << "Failed to get model\n";
            return -2;
        }
        config.modelData = (const char*)conf.FROCRConfig.model.c_str();
        config.modelByteSize = conf.FROCRConfig.model.size();
        auto ret = this->m_models.FROCR->init(config);
        if(ret!=0)
        {
            std::cerr << "Fail to initialize model\n";
            return -1;
        }
    }

    this->m_ignoreInputPlateType = conf.ignoreInputPlateType;

return 0;
    
}

ChOp::OutputStruct ChOp::run(const ChOp::InputStruct& input)
{
    ChOp::OutputStruct output;

    if(input.plateImage.empty())
    {
        output.codeType = ChOp::CodeTypes::NULL_IMAGE;
        output.newPlateValue = input.plateValue;
        output.probability = 0;
        return output;
    }
    
    try
    {
        cv::Mat PDImage = input.plateImage;
        if(PDImage.channels() < 3)
            cv::cvtColor(PDImage, PDImage, cv::COLOR_GRAY2BGR);
        
        // Step 1 : Plate Detection 
        inference::OutputStruct PDOutput;
        if(this->m_models.PD)
        {
            float AspectRatio = (float)PDImage.cols / (float)PDImage.rows;
            if(AspectRatio < 2.5)
            {
                int topbot = std::round(PDImage.rows / 2); //#TODO Must remove??
                int rileft = std::round(PDImage.cols / 2);
                cv::copyMakeBorder(PDImage, PDImage, topbot, topbot, rileft, rileft, 0);

                inference::InputStruct PDInput;
                PDInput.Image = PDImage;
                /////////////////////////////////    
                std::vector<std::shared_ptr< aivision::nn::DataModel> > input_streams;
                int batch_size = 1;
                for(int b=0;b<batch_size;b++)
                {
                    std::shared_ptr< aivision::nn::DataModel> stream{new  aivision::nn::DataModel()};
                    stream->addView({cv::Point2f(0,0),cv::Point2f(0,1),cv::Point2f(1,1),cv::Point2f(1,0)},"offline");
                    stream->SetFrame(PDImage);
                    stream->SetActivePreset(1);
                    input_streams.push_back(stream);
                }
                this->m_models.PD->forward(input_streams);
                auto preds = input_streams[0]->getResults();
                //////////////////////////
                // this->m_models.PD->run(PDInput);#TODO
                // PDOutput = this->m_models.PD->getOutput();
                if(PDOutput.modelOutputs.size() == 0)
                {
                    output.codeType = ChOp::CodeTypes::NOT_PLATE;
                    output.newPlateValue = input.plateValue;
                    output.probability = 0;
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
            plateRect.x       -= (0.1 * plateRect.width);
            plateRect.width   *= 1.15;
            plateRect.height  *= 1.15;

            this->fixRectDimension(plateRect, PDImage.rows, PDImage.cols);
            ocrImage = PDImage(plateRect);
        } else 
        {
            ocrImage = PDImage;
        }

        // // Step 2 : Plate Classifier
        // int plateType;
        // if(this->m_models.PC)
        // {
        //     if(this->m_ignoreInputPlateType || (input.plateType == static_cast<int>(inference::standards::PlateType::UNKNOWN)))
        //     {
        //         inference::InputStruct PCInput;
        //         PCInput.Image = ocrImage;
        //         this->m_models.PC->run(PCInput);
        //         auto PCOutput = this->m_models.PC->getOutput();

        //         std::vector<int> scores;
        //         for(auto& object : PCOutput.modelOutputs)
        //             scores.push_back(object.labelScore);
                
        //         int maxIndex = std::max_element(scores.begin(), scores.end()) - scores.begin();
        //         plateType = PCOutput.modelOutputs[maxIndex].label;
        //     } else
        //     {
        //         plateType = input.plateType;
        //     }

        // } else 
        // {
        //     plateType = input.plateType;
        // }
        // output.newPlateType = plateType;

        // // Step 3 : OCR
        // inference::InputStruct ocrInput;
        // ocrInput.Image = ocrImage;
        // switch(plateType) 
        // {
        //     case static_cast<int>(inference::standards::PlateType::UNKNOWN) :
        //     {
        //         output.codeType = ChOp::CodeTypes::NOT_PROCESSED;
        //         output.newPlateValue = input.plateValue;
        //         output.probability = 0;
        //         break;
        //     }
        //     case static_cast<int>(inference::standards::PlateType::IR) :
        //     {
        //         if(this->m_models.IROCR)
        //         {
        //             this->m_models.IROCR->run(ocrInput);
        //             auto IROCROutput = this->m_models.IROCR->getOutput();

        //             if(IROCROutput.modelOutputs.size() != 0)
        //             {
        //                 output.codeType = this->calculateIrCodeType(IROCROutput.OCRResult.plateValue, input.plateValue);
        //                 output.newPlateValue = IROCROutput.OCRResult.plateValue;
        //                 output.probability = this->calculateProbability(IROCROutput);
        //             } else 
        //             {
        //                 output.codeType = ChOp::CodeTypes::NOT_PROCESSED;
        //                 output.newPlateValue = input.plateValue;
        //                 output.probability = 0;
        //             }
        //         } else
        //         {
        //             output.codeType = ChOp::CodeTypes::NOT_PROCESSED;
        //             output.newPlateValue = input.plateValue;
        //             output.probability = 0;
        //         }
                
        //         break;
        //     }
        //     case static_cast<int>(inference::standards::PlateType::MB) :
        //     {
        //         if(this->m_models.MBOCR)
        //         {
        //             this->m_models.MBOCR->run(ocrInput);
        //             auto MBOCROutput = this->m_models.MBOCR->getOutput();

        //             output.codeType = ChOp::CodeTypes::MOTOR_BIKE;
        //             if(MBOCROutput.modelOutputs.size() != 0)
        //             {
        //                 output.newPlateValue = MBOCROutput.OCRResult.plateValue;
        //                 output.probability = this->calculateProbability(MBOCROutput);
        //             } else 
        //             {
        //                 output.newPlateValue = input.plateValue;
        //                 output.probability = 0;
        //             }
        //         } else
        //         {
        //             output.codeType = ChOp::CodeTypes::NOT_PROCESSED;
        //             output.newPlateValue = input.plateValue;
        //             output.probability = 0;
        //         }
                
        //         break;
        //     }
        //     case static_cast<int>(inference::standards::PlateType::FREE_ZONE) :
        //     {
        //         if(this->m_models.FZOCR)
        //         {
        //             this->m_models.FZOCR->run(ocrInput);
        //             auto FZOCROutput = this->m_models.FZOCR->getOutput();

        //             output.codeType = ChOp::CodeTypes::FREE_ZONE;
        //             if(FZOCROutput.modelOutputs.size() != 0)
        //             {
        //                 output.newPlateValue = FZOCROutput.OCRResult.plateValue;
        //                 output.probability = this->calculateProbability(FZOCROutput);
        //             } else 
        //             {
        //                 output.newPlateValue = input.plateValue;
        //                 output.probability = 0;
        //             }
        //         } else
        //         {
        //             output.codeType = ChOp::CodeTypes::NOT_PROCESSED;
        //             output.newPlateValue = input.plateValue;
        //             output.probability = 0;
        //         }
                
        //         break;
        //     }
        //     case static_cast<int>(inference::standards::PlateType::NEW_TRANSIT) :
        //     case static_cast<int>(inference::standards::PlateType::OLD_TRANSIT) :
        //     {
        //         if(this->m_models.TZOCR)
        //         {
        //             this->m_models.TZOCR->run(ocrInput);
        //             auto TZOCROutput = this->m_models.TZOCR->getOutput();

        //             output.codeType = ChOp::CodeTypes::TRANSIT;
        //             if(TZOCROutput.modelOutputs.size() != 0)
        //             {
        //                 output.newPlateValue = TZOCROutput.OCRResult.plateValue;
        //                 output.probability = this->calculateProbability(TZOCROutput);
        //             } else 
        //             {
        //                 output.newPlateValue = input.plateValue;
        //                 output.probability = 0;
        //             }
        //         } else
        //         {
        //             output.codeType = ChOp::CodeTypes::NOT_PROCESSED;
        //             output.newPlateValue = input.plateValue;
        //             output.probability = 0;
        //         }
                
        //         break;
        //     }
        //     default :
        //     {
        //         if(this->m_models.FROCR)
        //         {
        //             this->m_models.FROCR->run(ocrInput);
        //             auto FROCROutput = this->m_models.FROCR->getOutput();

        //             output.codeType = ChOp::CodeTypes::FOREIGN;
        //             if(FROCROutput.modelOutputs.size() != 0)
        //             {
        //                 output.newPlateValue = FROCROutput.OCRResult.plateValue;
        //                 output.probability = this->calculateProbability(FROCROutput);
        //             } else 
        //             {
        //                 output.newPlateValue = input.plateValue;
        //                 output.probability = 0;
        //             }
                    
        //         } else
        //         {
        //             output.codeType = ChOp::CodeTypes::NOT_PROCESSED;
        //             output.newPlateValue = input.plateValue;
        //             output.probability = 0;
        //         }

        //         break;
        //     }
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

