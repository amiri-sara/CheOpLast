#ifndef CHOP_H
#define CHOP_H

#include "inference.h"

class ChOp
{
public:
    enum CodeTypes
    {
        NOT_PLATE_NUMBER = -5,
        INVALID_PACKET,
        CORRUPTED_IMAGE,
        NULL_IMAGE,
        NOT_PLATE,
        OTHER,
        MOTOR_BIKE,
        REPAIRE_PLATE,
        Two_SMALL_NUM = 4,
        THREE_BIG_NUM,
        TWO_BIG_NUM,
        DIFFERENT_ALPH,
        NOT_PROCESSED = 9,
        SURE_NIGHT_TIME,
        FREE_ZONE,
        TRANSIT,
        FOREIGN,
        SURE = 20
    };

    struct ModelConfigStruct : public inference::ConfigStruct
    {
        bool active = true;
    };

    struct ConfigStruct 
    {
        ModelConfigStruct PDConfig;
        ModelConfigStruct PCConfig;
        ModelConfigStruct IROCRConfig;
        ModelConfigStruct MBOCRConfig;
        ModelConfigStruct TZOCRConfig;
        ModelConfigStruct FZOCRConfig;
        ModelConfigStruct FROCRConfig;
        bool ignoreInputPlateType = false;
    };

    struct InputStruct
    {
        cv::Mat plateImage;
        std::string plateValue;
        int plateType = static_cast<int>(inference::standards::PlateType::UNKNOWN);
    };

    struct OutputStruct
    {
        std::string newPlateValue = "";
        int newPlateType;
        int codeType = ChOp::CodeTypes::NOT_PROCESSED;
        float probability = 0;
    };

    ChOp() = delete;
    ChOp(const ChOp::ConfigStruct& conf);
    int init(const ChOp::ConfigStruct& conf);
    ChOp::OutputStruct run(const ChOp::InputStruct& input);
    static std::string getVersion();
private:

    struct ModelsStruct
    {
        std::unique_ptr< aivision::nn::NNModel> PD      = nullptr;
        std::unique_ptr< aivision::nn::NNModel> PC      = nullptr;
        std::unique_ptr< aivision::nn::NNModel> IROCR   = nullptr;
        std::unique_ptr< aivision::nn::NNModel> MBOCR   = nullptr;
        std::unique_ptr< aivision::nn::NNModel> TZOCR   = nullptr;
        std::unique_ptr< aivision::nn::NNModel> FZOCR   = nullptr;
        std::unique_ptr< aivision::nn::NNModel> FROCR   = nullptr;
        // std::shared_ptr<inference::Handler> PD      = nullptr;
        // std::shared_ptr<inference::Handler> PC      = nullptr;
        // std::shared_ptr<inference::Handler> IROCR   = nullptr;
        // std::shared_ptr<inference::Handler> MBOCR   = nullptr;
        // std::shared_ptr<inference::Handler> TZOCR   = nullptr;
        // std::shared_ptr<inference::Handler> FZOCR   = nullptr;
        // std::shared_ptr<inference::Handler> FROCR   = nullptr;
    } m_models;

    bool m_ignoreInputPlateType = false;

    void fixRectDimension(cv::Rect& candRect,int row, int col);
    int calculateIrCodeType(const std::string& newPlateValue, const std::string& oldPlateValue);
    int calculateProbability(const inference::OutputStruct& modelOutput);
    std::unique_ptr<NNModel> handler(const aivision::nn::BaseNNConfig::ConfigStruct& modelConfig);

};

#endif // !CHOP_H
