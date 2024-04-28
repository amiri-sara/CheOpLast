#ifndef IDLM_H
#define IDLM_H

#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <memory>
#include <numeric>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

#include <opencv2/dnn/dnn.hpp>
#include <opencv2/opencv.hpp>

#define SHOW_ERROR(X)             std::cout<<"\033[1;31m"<<X<<"\033[0m"<<std::endl
#define SHOW_LOG(X)               std::cout<<"\033[1;32m"<<X<<"\033[0m"<<std::endl
#define SHOW_WARNING(X)           std::cout<<"\033[1;33m"<<X<<"\033[0m"<<std::endl
#define SHOW_IMPORTANTLOG(X)      std::cout<<"\033[1;34m"<<X<<"\033[0m"<<std::endl
#define SHOW_IMPORTANTLOG2(X)     std::cout<<"\033[1;35m"<<X<<"\033[0m"<<std::endl
#define SHOW_IMPORTANTLOG3(X)     std::cout<<"\033[1;36m"<<X<<"\033[0m"<<std::endl
#define SHOW_IMPORTANTLOG4(X)     std::cout<<"\033[1;37m"<<X<<"\033[0m"<<std::endl

#define IDLM_VERSION "0.1.0"
#define ONNX_RUNTIME_VERSION std::to_string(ORT_API_VERSION)

class IDLM
{
public:    
    enum class ModelType {
        DLM_YOLOCDM,
        DLM_YOLOCCDM,
        DLM_YOLOPDM,
        DLM_YOLOPCM,
        DLM_YOLOPROCRM,
        DLM_YOLOFZOCRM,
        DLM_YOLOMBOCRM,
        DLM_YOLOCRM
    };
    
    struct ModelInputOutputStruct
    {
        std::string Name;
        std::vector<int64_t> Shape;
        bool isDynamicInputShape = false;
    };
    
    struct ExecutionConfigStruct
    {
        bool ExecutionProviderGPU   = false;
        bool ParallelMode           = false;
        int IntraOpNumThreads       = 2;
        int InterOpNumThreads       = 1;
    };
    
    struct ConfigStruct
    {
        std::string Model;
        cv::Size InputSize; // It is needed when the input of the model is dynamic
        int PrimaryThreshold;
        int SecondaryThreshold;
        IDLM::ExecutionConfigStruct ExecutionConfig;
        bool log = false; 
    };

    struct InputStruct
    {
        cv::Mat Image;
        IDLM::ModelType Modeltype;
    };

    struct ResponseStruct
    {
        int Code;
        std::string Description;
    };

    virtual ~IDLM() = default;
    virtual IDLM::ResponseStruct inference(IDLM::InputStruct const &IS) = 0;

private:

protected:
};


#endif //IDLM_H