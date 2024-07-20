#pragma once
#include<string>
#include<iostream>
#include <fstream>
#include <chrono>
#include <cmath>
#include <map>
#include<string>
#include <ctime>
#include <iomanip>
#include <random>
#include<atomic>
#include <mutex>
#include<opencv2/core.hpp>
#include<opencv2/highgui.hpp>
#include<opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include "nn/error_codes.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#ifdef WITH_GPU
#ifdef CV_CUDA
#include <opencv2/cudaimgproc.hpp>
#include<opencv2/cudaarithm.hpp>
#include<opencv2/cudawarping.hpp>
#endif
#include "cuda_utils/basic.h"

#endif
using namespace std;

namespace aivision {
namespace nn{


class ObjectAttributes
{
public:
    ObjectAttributes() = default;
    float               score = 0.0;
    cv::Rect2f          bbox;
    int                 label = -1;
    int                 mapped_label = -1;
    std::string         class_name;
    uint64              num_classes = 100;
    std::vector<float>  embed_vector;
    float               iou = -1.;
    uint64              capture_time = 0;
    uint64              capture_frame = 0;
    std::map<std::string,ObjectAttributes> attributes;

};



enum Det {
    tl_x = 0,
    tl_y = 1,
    br_x = 2,
    br_y = 3,
    score = 4,
    class_idx = 5
};





enum NNType
{
    NONE=0,
    BaseModel,
    Yolov5Custome,
    YOLOv5,
    YOLOv5n,
    YOLOv5s,
    YOLOv5m,
    YOLOv5x,
    YOLOv5l,
    OCRIRNet,
    Clasifier,
    YOLOv8,
    YOLOv8n,
    YOLOv8s,
    YOLOv8m,
    YOLOv8x,
    YOLOv8l,
    YOLOEv8n,
    YOLOEv8s,
    TRT=1000,
    ONNX=2000
};

enum  MLTask{
    NoTask=0,
    ClassificationTask,
    ReggressionTask,
    DetectionTask,
    MatchingTask,
    IROCRTask,
    GLOCRTask,
};


enum ResizeMethod{LetterBox=0, Resize};
enum NormMethod{NoNorm=0, NormalNorm};
enum Device{Cpu, Gpu,Cuda};
enum ActivationFunction {Identity=0, Sigmoid, Softmax};


struct ResizingStruct
{
    ResizeMethod method {LetterBox};
    int          paddingValue {114};
    int64 width = 256;
    int64 height = 256;
};

struct NormalizationStruct
{
    NormMethod  method {NoNorm};
    float       scaleValue {1.0f/255.0f};
    cv::Vec3f   mean{0,0,0};
    cv::Vec3f   std{1,1,1};
};



std::vector<float> LetterboxImage(const cv::Mat& src, cv::Mat& dst, const cv::Size& new_size);
int vectorArgMax(std::vector<float> data);
cv::Rect GetRect(float bbox[4]);




class CpuTimer{
public:
    std::chrono::_V2::steady_clock::time_point start_t;
    CpuTimer();
    void Start();
    double Elapsed();
    std::chrono::_V2::steady_clock::time_point GetTime();

};


static char * readAllBytes(const char * filename, int * read)
{
    ifstream ifs(filename, ios::binary|ios::ate);
    ifstream::pos_type pos = ifs.tellg();

    int length = pos;

    char *pChars = new char[length];
    ifs.seekg(0, ios::beg);
    ifs.read(pChars, length);

    ifs.close();
    *read = length;
    return pChars;
}



class BaseNNConfig
{
public:
    BaseNNConfig();
    const char* inputBlobName = "data";
    const char* outputBlobName = "prob";
    const char* featureBlobName = "features";
    const char* modelData;
    int         modelByteSize {-2};
    int         inputWidth = 480;
    int         inputHeight = 608;
    int         numClasses = 4;
    NNType      netType = NNType::NONE;
    NNType      netSubType = NNType::NONE;
    MLTask      task {MLTask::NoTask};
    bool        useFP16 = false;
    float       confThr = 0.4;
    float       nmsThr = 0.45;
    Device      device= Device::Cpu;
    NNType      backend = NNType::ONNX;
    std::string modelPath = "";
    unsigned int maxBatchSize{1};
    uint16_t    onnx_num_intra_op_threads {2};
    uint16_t    onnx_num_inter_op_threads {2};
    uint64      embedSize = 64;
    ActivationFunction outputActivation {ActivationFunction::Identity};
    ResizingStruct resizingMethod;
    NormalizationStruct normalizingMethod;
    std::map<int,std::string> labelMap;
    std::map<int,int> indexMap;

    int read(std::string const& configFile);

    template<class T>
    std::vector<T> getListValues(boost::property_tree::basic_ptree<std::__cxx11::basic_string<char>,std::__cxx11::basic_string<char>> &root){
        std::vector<T> d;
        for(auto row:root){
            d.push_back(row.second.get_value<T>());
        }
        return d;

    }

    int read(boost::property_tree::basic_ptree<std::__cxx11::basic_string<char>, std::__cxx11::basic_string<char> > &root);
    struct ConfigStruct
    {
        std::string model; // Model value or .onnx file path
        std::string modelConfig;
    };

};
bool xcmp(const ObjectAttributes& a, const ObjectAttributes& b);

class DataModel{
protected:
    static uint64 StreamID;
    uint64 sID;
    std::string viewName;
    std::vector<cv::Point2f> viewROI;
    cv::Rect2f viewRect;
    cv::Mat frame;
    cv::Size frameSize;
    unsigned char* gpuPtr = nullptr;
    unsigned char* gpuMaskPtr = nullptr;
    int activePreset {-1};
    std::mutex dIStreamMutex;
    std::vector<ObjectAttributes> outputResults;

public:
    DataModel(){
        this->sID = DataModel::StreamID++;
    }
    ~DataModel(){
#ifdef WITH_GPU
        if(this->gpuPtr)
            HANDLE_ERROR(cudaFree(this->gpuPtr));
        if(this->gpuMaskPtr)
            HANDLE_ERROR(cudaFree(this->gpuMaskPtr));
#endif
    }
    bool HasGpu()
    {
        return this->gpuPtr!=nullptr;
    }
    std::vector<ObjectAttributes>& getResults(){
        return this->outputResults;
    }
    void setResults(std::vector<ObjectAttributes> results){
        this->outputResults = results;
    }
    void clearResults(){
        this->outputResults.clear();
    }
    uint64 GetID(){return this->sID;}
    bool addView(const std::vector<cv::Point2f> roi_points, const std::string name); // if adding was successfull it returns true
    bool AllocateGpuData(cv::Mat &frame);
    std::string GetName() ;
    cv::Rect2f GetRect() ;
    cv::Rect2d GetActiveRect() ;
    std::vector<cv::Point2f> GetRoi() ;
    virtual bool SetFrame(cv::Mat &frame);
#ifdef WITH_GPU
    virtual bool SetFrameGpu(cv::Mat frame, cudaStream_t &cuda_stream);
    bool DeAllocateGpuData()
    {
        if(this->gpuPtr)
        {
            HANDLE_ERROR(cudaFree(this->gpuPtr));
            this->gpuPtr = nullptr;
        }
        if(this->gpuMaskPtr)
        {
            HANDLE_ERROR(cudaFree(this->gpuMaskPtr));
            this->gpuMaskPtr = nullptr;
        }
        return true;

    }
#endif

    cv::Mat & GetFrame();
    unsigned char* GetGpuPtr(){
        return this->gpuPtr;
    }
    void PopFrame();
    void SetActivePreset(int num){
        std::lock_guard<std::mutex > lc(this->dIStreamMutex);
        this->activePreset = num;
        return;
    }
    bool IsActive();
    unsigned char *GetGpuMaskPtr();
};

class NNModel{
protected:
    BaseNNConfig Config;
#ifdef WITH_GPU
    cudaStream_t stream_;
#endif

public:
    NNModel() 
    {
    }
    virtual ~NNModel(){
    }
#ifdef WITH_GPU
    virtual cudaStream_t &GetCudaStream(){
        return stream_;
    }
#endif

    virtual int init(BaseNNConfig conf) = 0;
    virtual void outputPostProcess(std::vector<std::shared_ptr<DataModel> > &input_streams);
    virtual bool forwardOnce(std::vector<std::shared_ptr<DataModel> > &input_streams) = 0;
    virtual bool forward(std::vector<std::shared_ptr<DataModel> > &input_streams)
    {
        auto ret = forwardOnce(input_streams);
        if(ret)
        {
            outputPostProcess(input_streams);
        }
        return ret;
    }
    BaseNNConfig& getConfig()
    {
        return this->Config;
    }
    virtual const std::vector<std::vector<int64_t> > &getOutputDims() const;
    virtual const std::vector<std::vector<int64_t> > &getInputDims() const;
};

}
}

