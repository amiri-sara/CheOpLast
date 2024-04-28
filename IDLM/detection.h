#ifndef DETECTION_H
#define DETECTION_H

#include "onnx.h"

class Detection : public ONNX
{
public:
    struct DetectedObjectStruct
    {
        cv::Rect Box;
        int Label;
        int LabelScore; //0 < Score < 100
    };

    struct DetectionOutputStruct
    {
        std::vector<Detection::DetectedObjectStruct> Objects;
    };
    
    Detection() = delete;
    Detection(const IDLM::ConfigStruct& conf) : ONNX(conf){}

    // Assume that the model has one input and one output.
    IDLM::ResponseStruct inference(IDLM::InputStruct const &IS) override;

    Detection::DetectionOutputStruct getDetectionOutput();
private:
    Detection::DetectionOutputStruct DetectionOutput;
    
    bool postprocessing(std::vector<Ort::Value>& outputTensors, const cv::Size& MainImageSize, const cv::Size& ModelInputSize);

protected:
};

#endif //DETECTION_H