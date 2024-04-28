#ifndef CLASSIFIER_H
#define CLASSIFIER_H

#include "onnx.h"

class Classifier : public ONNX
{
public:
    struct ClassifierOutputStruct
    {
        std::vector<int> Label;
        std::vector<int> Score; //0 < Score < 100
    };
    
    Classifier() = delete;
    Classifier(const IDLM::ConfigStruct& conf) : ONNX(conf)
    {
        // for Persian plate OCR
        this->PersianAlphMapping[0] =  01;
        this->PersianAlphMapping[1] =  02;
        this->PersianAlphMapping[2] =  03;
        this->PersianAlphMapping[3] =  04;
        this->PersianAlphMapping[4] =  05;
        this->PersianAlphMapping[5] =  06;
        this->PersianAlphMapping[6] =  07;
        this->PersianAlphMapping[7] =  10;
        this->PersianAlphMapping[8] =  13;
        this->PersianAlphMapping[9] =  15;
        this->PersianAlphMapping[10] =  16;
        this->PersianAlphMapping[11] =  17;
        this->PersianAlphMapping[12] =  19;
        this->PersianAlphMapping[13] =  21;
        this->PersianAlphMapping[14] =  23;
        this->PersianAlphMapping[15] =  24;
        this->PersianAlphMapping[16] =  27;
        this->PersianAlphMapping[17] =  28;
        this->PersianAlphMapping[18] =  29;
        this->PersianAlphMapping[19] =  30;
        this->PersianAlphMapping[20] =  31;
        this->PersianAlphMapping[21] =  32;
        this->PersianAlphMapping[22] =  33;
        this->PersianAlphMapping[23] =  34;
    }

    IDLM::ResponseStruct inference(IDLM::InputStruct const &IS) override;

    Classifier::ClassifierOutputStruct getClassifierOutput();
private:
    bool postprocessing(std::vector<Ort::Value>& outputTensors, IDLM::ModelType Modeltype);
    bool CarClassificationpreprocessing(cv::Mat const &Image, float*& blob);
    std::vector<int> SoftMax(std::vector<float> input);

    Classifier::ClassifierOutputStruct ClassifierOutput;
    std::map<int, int> PersianAlphMapping;
protected:
};

#endif //CLASSIFIER_H