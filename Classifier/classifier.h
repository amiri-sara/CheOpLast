#ifndef CLASSIFIER_H
#define CLASSIFIER_H

#include "inference.h"

class Classifier
{
public:

    using ConfigStruct = inference::ConfigStruct;
    
    struct InputStruct : public inference::InputStruct
    {
        cv::Rect desiredRect;
        bool useRect;
    };

    struct OutputStruct
    {
        std::unordered_map<std::string,int> keyLabels;
    };

    Classifier() = delete;
    Classifier(const std::vector<Classifier::ConfigStruct>& configs);
    Classifier::OutputStruct run(const std::vector<Classifier::InputStruct>& inputs);
    static std::string getVersion(){return "1.0.0";}
private:
    std::vector<std::shared_ptr<inference::Handler>> m_pIHs;
};


#endif //!CLASSIFIER_H

