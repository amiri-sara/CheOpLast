#ifndef CLASSIFIER_H
#define CLASSIFIER_H

#include "inference.h"
#include "../DataHandler/datahandler.h"
#include "../StoreImage/storeimage.h"
#include "../SaveData/savedata.h"
#include "../crow.h"

#include <opencv2/opencv.hpp>
#include <boost/filesystem.hpp>
#include <boost/bind/bind.hpp> 
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "base.h"
#include "common.h"
#include "error_codes.h"
#include "nn/onnx_inference/onnxinference.h"

using namespace std;
using namespace cv;
using namespace aivision::nn;
using namespace boost::placeholders;
class Classifier
{
public:

    enum CodeTypes
    {
        NOT_PROCESSED = -5,
        // INVALID_PACKET,
        // CORRUPTED_IMAGE,
        // NULL_IMAGE,
        // NOT_PLATE,
        // OTHER,
        // MOTOR_BIKE,
        // REPAIRE_PLATE,
        // Two_SMALL_NUM = 4,
        // THREE_BIG_NUM,
        // TWO_BIG_NUM,
        // DIFFERENT_ALPH,
        // NOT_PROCESSED = 9,
        // SURE_NIGHT_TIME,
        // FREE_ZONE,
        // TRANSIT,
        // FOREIGN,
        // SURE = 20
    };


    struct InputStruct
    {
        cv::Mat Image;
        std::string ImageBase64;
    };
    struct ModelConfigStruct 
    {
        bool active = true;
        std::string model; // Model value or .onnx file path
        std::string modelConfig;
    };
    struct ConfigStruct 
    {
        // classifier
        ModelConfigStruct CDConfig;
        ModelConfigStruct LCCConfig;
    };
    struct OutputStruct
    {
        cv::Rect box;
        int label;
        int labelScore; // 0 < Score < 100
        int vehicleModel;
        std::unordered_map<std::string,int> keyLabels;
        float probability = 0;
        int codeType = CodeTypes::NOT_PROCESSED; //TODO load detection code type 
        std::string ImageBase64;
    };


    Classifier() = delete;
    Classifier(const ConfigStruct& configs);
    std::vector<OutputStruct> run(const InputStruct& input);

private:
    struct ModelsStruct
    {

        std::unique_ptr<NNModel> CD = nullptr;
        std::unique_ptr<NNModel> LCC = nullptr;
    } m_models;

std::unique_ptr<NNModel> getModel(BaseNNConfig& conf, const string& path);
//     std::vector<std::shared_ptr<inference::Handler>> m_pIHs;
};




















// #include "inference.h"

// class Classifier
// {
// public:

//     using ConfigStruct = inference::ConfigStruct;
    
//     struct InputStruct : public inference::InputStruct
//     {
//         cv::Rect desiredRect;
//         bool useRect;
//     };

//     struct OutputStruct
//     {
//         std::unordered_map<std::string,int> keyLabels;
//     };

//     Classifier() = delete;
//     Classifier(const std::vector<Classifier::ConfigStruct>& configs){}
//     Classifier::OutputStruct run(const std::vector<Classifier::InputStruct>& inputs);
//     static std::string getVersion();
// private:
//     std::vector<std::shared_ptr<inference::Handler>> m_pIHs;
// };


#endif //!CLASSIFIER_H

