#ifndef ONNX_H
#define ONNX_H

#include "idlm.h"

// Header for onnxruntime
#include <onnxruntime_cxx_api.h>

class ONNX : public IDLM
{
public:
    ONNX() = delete;
    ONNX(const IDLM::ConfigStruct& conf);

    virtual IDLM::ResponseStruct inference(IDLM::InputStruct const &IS) = 0;
    
    std::vector<IDLM::ModelInputOutputStruct> getModelInputs();
    std::vector<IDLM::ModelInputOutputStruct> getModelOutputs();
private:

protected:
    bool preprocessing(cv::Mat const &Image, float*& blob);
    size_t vectorProduct(const std::vector<int64_t>& v)
    {
        return std::accumulate(v.begin(), v.end(), 1, std::multiplies<int64_t>());
    }
    void scaleCoords(cv::Rect& coords, const cv::Size& ImageSize, const cv::Size& MainImageSize);

    IDLM::ConfigStruct Config;
    std::shared_ptr<Ort::Session> session;
    std::shared_ptr<Ort::Env> env;
    std::shared_ptr<Ort::SessionOptions> sessionOptions;

    std::vector<IDLM::ModelInputOutputStruct> ModelInputs;
    std::vector<IDLM::ModelInputOutputStruct> ModelOutputs;
};

#endif //ONNX_H