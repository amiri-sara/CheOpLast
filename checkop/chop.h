#ifndef CHOP_H
#define CHOP_H

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
#include "ocr.h"


using namespace std;
using namespace cv;
using namespace aivision::nn;
using namespace boost::placeholders;
using namespace gocr;





#define SIZE 10  // Size of the circular buffer



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

    struct ModelConfigStruct 
    {
        bool active = true;
        std::string model; // Model value or .onnx file path
        std::string modelConfig;
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

        // classifier
        ModelConfigStruct CDConfig;
        ModelConfigStruct LCCConfig;


        bool ignoreInputPlateType = false;
    };

    struct InputStruct
    {
        cv::Mat plateImage;
        std::string plateImageBase64;
        std::string plateValue;
        int plateType = static_cast<int>(gocr::PlateType::UNKNOWN);
    };

    struct OutputStruct
    {
        std::string newPlateValue = "";
        int newPlateType;
        int codeType = ChOp::CodeTypes::NOT_PROCESSED;
        float probability = 0;
        std::string PlateImageBase64;//TODO remove for test
        cv::Mat croppedPlateImage;
    };
    struct ImageInfo
    {
        std::string ColorImage = "";
        std::string PlateImageBase64 = "";
        std::string TTOInfoId = "";
        cv::Mat PlateImage;
    };
    struct ProcessInputStruct
    {
	    std::string MinID = "";
        std::string CompanyId = "";
        std::string DeviceId = "";
        std::string LineNumber = "";
        std::string PassDateTime = "";
        std::string RecivedDateTime = "";
        std::string SystemD = "";
        std::string TTOInfoId = "";
        std::string VehicleClass = "";
        std::string VehiclePlate = "";
        std::string VehicleSpeed = "";
        std::string WrongDirection = "";
        std::string ColorImage = "";
        std::string PlateImageBase64 = "";
        cv::Mat PlateImage;
    };

    // std::vector<ProcessInputStruct> ProcessInputVec;
    std::vector<std::shared_ptr<DataHandler::DataHandlerStruct>> ProcessInputVec;

    boost::mutex ProcessInputVecMutex;
    boost::mutex ProcessOutputVecMutex;
    boost::condition_variable condVar_;
    std::vector<OutputStruct> OutputStructVector_;
    

    static std::atomic<uint32_t> totalSavingTime; // Total saving time
    static std::atomic<uint32_t> savingCount; // Count of save operations
    static boost::mutex mtx_Saving; // Mutex for synchronizing access to shared data

    static std::atomic<uint32_t> totalChOpTime; // Total processing time
    static std::atomic<uint32_t> ChOpCount; // Count of processing operations
    static boost::mutex mtx_ChOp; // Mutex for synchronizing access to shared data
    ChOp() = delete;
    ChOp(const ChOp::ConfigStruct& conf);
    OutputStruct run(const ChOp::InputStruct& input);
    static std::string getVersion();
    size_t getSize();
    boost::mutex MinId_mutex;



private:


    struct checkOpPlateResult 
    {
        // Rect2d bbox;
        string plateClass;
        float classConfidence;
        // std::string newPlateValue = "";
        // int newPlateType;
        // int codeType = ChOp::CodeTypes::NOT_PROCESSED;
        // float probability = 0;
        string plateNumber;
        int plateType;
    } m_results;


    struct ModelOutputStruct 
    {
        cv::Rect box;
        int label;
        int labelScore; // 0 < Score < 100
        std::string labelType;
    };

    struct OCROutputStruct
    {
        std::string plateValue = "";
        std::string plateTemplate = "";
        int plateType = -1;
    };

    struct oldOutputStruct
    {
        std::vector<ModelOutputStruct> modelOutputs;
        OCROutputStruct OCRResult;
    };

    std::unique_ptr<NNModel> getModel(BaseNNConfig& conf, const string& path);

    struct ModelsStruct
    {

        std::unique_ptr<NNModel> PD = nullptr;
        std::unique_ptr<NNModel> PC = nullptr;
        std::unique_ptr<NNModel> FROCR = nullptr;
        std::unique_ptr<NNModel> CD = nullptr;
        std::unique_ptr<NNModel> LCC = nullptr;
        // std::shared_ptr<inference::Handler> PD      = nullptr;
        // std::shared_ptr<inference::Handler> PC      = nullptr;
        std::shared_ptr<inference::Handler> IROCR   = nullptr;
        std::shared_ptr<inference::Handler> MBOCR   = nullptr;
        std::shared_ptr<inference::Handler> TZOCR   = nullptr;
        std::shared_ptr<inference::Handler> FZOCR   = nullptr;
        // std::shared_ptr<inference::Handler> FROCR   = nullptr;
    } m_models;


    bool m_ignoreInputPlateType = false;
    double totalProcessingTime = 0.0;
    int numRecordsProcessed = 0;
    int desiredNumberOfRecords = 100;
    void fixRectDimension(cv::Rect& candRect,int row, int col);
    int calculateIrCodeType(const std::string& newPlateValue, const std::string& oldPlateValue);
    int calculateProbability(const std::vector<double,std::allocator<double> > probVec);
    int calculateProbabilityStr(const inference::OutputStruct& modelOutput);


};

#endif // !CHOP_H

