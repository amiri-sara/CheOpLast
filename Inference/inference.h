#ifndef INFERENCE_H
#define INFERENCE_H

#include "log.h"
#include <onnxruntime_cxx_api.h>
#include <boost/filesystem.hpp>

namespace inference
{
    namespace standards
    {
        enum Task
        {
            DETECTION_TASK = 0,
            CLASSIFICATION_TASK,
            OCR_TASK
        };

        enum OcrMethod
        {
            IROCR = 0,
            DETECTED_CHARACTERS
        };

        enum InferenceMethod
        {
            ONNX = 0
        };

        enum MiddleMapSize
        {
            LATIN_DIGITS = 10,
            LATIN_LETTERS = 26,
            PERSIAN_DIGITS = 10,
            PERSIAN_LETTERS = 32,
            PLATE_SPECIFICATIONS = 17,
            PLATE_TYPE = 22
        };

        struct MiddleMapStruct 
        {
            std::vector<int> latinDigits;
            std::vector<int> latinLetters;
            std::vector<int> persianDigits;
            std::vector<int> persianLetters;
            std::vector<int> plateSpecifications;
            std::vector<int> plateType;
        };

        enum PlateSpecifications
        {
            SAUDI_ARABIA_FLAG = 0,
            ARABiC_IRAQ,
            KHOSOSI,
            NEW_DUBAI,
            OLD_DUBAI,
            NEW_RAS_AL_KHAIMAH,
            ODL_RAS_AL_KHAIMAH,
            NEW_ABU_DHABI,
            OLD_ABU_DHABI,
            NEW_AJMAN,
            OLD_AJMAN,
            NEW_UMM_AL_QUWAIN,
            OLD_UMM_AL_QUWAIN,
            NEW_FUJAIRAH,
            OLD_FUJAIRAH,
            OLD_SHARJAH,
            IRAN_TRANSIT_FLAG
        };

        enum PlateType
        {
            UNKNOWN = 0,
            IR,
            MB,
            FREE_ZONE,
            NEW_TRANSIT,
            OLD_TRANSIT,
            SPECIAL,
            HISTORICAL,
            COMMERCE,
            NOT_USE,
            UNKNOWN_FOREIGN,
            IRAQ,
            TURKMENISTAN,
            AFGHANISTAN,
            PAKISTAN,
            AZERBAIJAN,
            TURKIYE,
            ARMENIA,
            GERMANY,
            RUSSIA,
            SAUDI_ARABIA,
            UAE,
            TAJIKISTAN
        };
    }

    namespace algorithm
    {
        namespace ocr
        {            
            struct Character 
            {
                cv::Rect box;
                int label;
                int score;
                int index = -1;
            };
            
            typedef inference::algorithm::ocr::Character Character;
            typedef std::vector<Character> CharacterVector;

            struct OcrOutput
            {
                CharacterVector plateValueCharacters;
                std::string plateValueString = "";
                std::string plateTemplate = "";
                int plateType = -1;
            };

            struct TopBottomStruct
            {
                CharacterVector allCharacters;
                CharacterVector allCharactersWithWhiteSpace;
                CharacterVector plateValueCharacters;
                CharacterVector plateCodeCharacters;

                void clear() {
                    allCharacters.clear();
                    allCharactersWithWhiteSpace.clear();
                    plateValueCharacters.clear();
                    plateCodeCharacters.clear();
                }
            };

            class DetectedCharOCR
            {
            public:
                DetectedCharOCR(const inference::standards::MiddleMapStruct& middleMap) : m_middleMap(middleMap){};
                inference::algorithm::ocr::OcrOutput run(CharacterVector& characters);
            private:
                void getTopBottomCharacters(CharacterVector& characters, CharacterVector& topCharacters, CharacterVector& bottomCharacters);
                void rotateCharacters(CharacterVector& characters);
                CharacterVector addWhiteSpace(CharacterVector& characters);
                void getPlateCodesCharacters(const CharacterVector& characters, CharacterVector& plateValueCharacters, CharacterVector& plateCodesCharacters);
                int findPlateType();
                void getPlateValue(inference::algorithm::ocr::OcrOutput& output);

                std::string getPersianStringSequence(CharacterVector& characters);
                std::string getLatinStringSequence(CharacterVector& characters);
                void persianRead(CharacterVector& characters, std::string& plateValue, std::string& plateTemplate);
                void latinRead(CharacterVector& characters, std::string& plateValue, std::string& plateTemplate);
                void compoundRead(CharacterVector& characters, std::string& plateValue, std::string& plateTemplate);
                bool hasIranTransitFlag(CharacterVector &characters);
                double estimateFreezoneScore();
                double estimateIranTransitScore();

                static bool contains(const std::vector<int>& vec, int value);
                int countSubstring(const std::string &str, const std::string &sub);
                static bool remove(const Character& character, const std::vector<int>& a, const std::vector<int>& b);

                inference::standards::MiddleMapStruct m_middleMap;

                inference::algorithm::ocr::TopBottomStruct m_topCharacters;
                inference::algorithm::ocr::TopBottomStruct m_bottomCharacters;
                
                // void readLicensePlate(std::vector<Inference::Algorithm::Annotation> rotated_annotations, Inference::Algorithm::AlgOytput& Output, const Inference::Algorithm::MiddleMapStruct& MiddleMap);
            };
        }
    }
    
    namespace engine
    {
        enum ModelType 
        {
            DLM = 0,        // Deep Learning Model
            IROCR           // Iran Plate OCR
        };

        enum NormalizingMethod 
        {
            VANILLA_NORMALIZE = 0,    // 1/255
            IMAGENET_NORMALIZE = 1
        };
        
        enum ResizingMethod 
        {
            VANILLA_RESIZE = 0,
            LETTER_BOX
        };
        
        enum ActivationMethod 
        {
            NONE = 0,
            SOFTMAX,
            SIGMOID,
            RELU,
            LEAKY_RELU
        };
        
        struct ModelInputOutputStruct
        {
            std::string name;
            std::vector<int64_t> shape;
            bool isDynamicInputShape = false;
        };
        
        struct ExecutionConfigStruct
        {
            bool executionProviderGPU   = false;
            bool parallelMode           = false;
            int intraOpNumThreads       = 2;
            int interOpNumThreads       = 1;
        };
        
        struct ConfigStruct
        {
            std::string model;
            cv::Size inputSize;     // It is needed when the input of the model is dynamic
            int primaryThreshold;
            int secondaryThreshold;
            inference::engine::ExecutionConfigStruct executionConfig;
            inference::engine::ActivationMethod activationMethod = inference::engine::ActivationMethod::NONE;
            inference::engine::ResizingMethod resizingMethod = inference::engine::ResizingMethod::VANILLA_RESIZE;
            inference::engine::NormalizingMethod normalizingMethod = inference::engine::NormalizingMethod::VANILLA_NORMALIZE;
            bool debugMode = false;
            std::vector<float> normalizingMethodmean;
            std::vector<float> normalizingMethodstd;
        };

        struct InputStruct
        {
            cv::Mat image;
            inference::engine::ModelType modelType;
        };

        struct ResponseStruct
        {
            int code;
            std::string description;
        };

        struct OutputStruct
        {
            cv::Rect box;
            int label;
            int labelScore; // 0 < Score < 100
        };

        class IEngine
        {
        public:
            virtual ~IEngine() = default;
            virtual inference::engine::ResponseStruct inference(inference::engine::InputStruct const &IS) = 0;
            virtual std::vector<inference::engine::OutputStruct> getOutput() = 0;
        private:

        protected:
        };

        namespace onnx
        {
            class ONNX : public inference::engine::IEngine
            {
            public:
                ONNX() = delete;
                ONNX(const inference::engine::ConfigStruct& conf);

                virtual inference::engine::ResponseStruct inference(inference::engine::InputStruct const &IS) = 0;
                virtual std::vector<inference::engine::OutputStruct> getOutput() = 0;
            private:

            protected:
                void letterbox(const cv::Mat &src, cv::Mat &dst);    
                bool preprocessing(cv::Mat const &image, float*& blob);
                inference::engine::ResponseStruct runSession(float *blob, std::vector<Ort::Value>& inputTensors);
                std::vector<float> tensot2Vector(std::vector<Ort::Value>& outputTensors);
                std::vector<int> softMax(std::vector<float> input);

                size_t vectorProduct(const std::vector<int64_t>& v) {
                    return std::accumulate(v.begin(), v.end(), 1, std::multiplies<int64_t>());
                }

                std::string printShape(const std::vector<std::int64_t>& v) {
                    std::stringstream ss("");
                    for (std::size_t i = 0; i < v.size() - 1; i++) ss << v[i] << "x";
                    ss << v[v.size() - 1];
                    return ss.str();
                }

                inference::engine::ConfigStruct m_config;
                std::shared_ptr<Ort::Session> m_pSession;
                std::shared_ptr<Ort::Env> m_pEnv;
                std::shared_ptr<Ort::SessionOptions> m_pSessionOptions;

                std::vector<inference::engine::ModelInputOutputStruct> m_modelInputs;
                std::vector<inference::engine::ModelInputOutputStruct> m_modelOutputs;

                std::vector<inference::engine::OutputStruct> m_output;
            };

            class Detection final : public inference::engine::onnx::ONNX
            {
            public:
                Detection() = delete;
                Detection(const inference::engine::ConfigStruct& conf) : ONNX(conf){}

                // Assume that the model has one input and one output.
                inference::engine::ResponseStruct inference(inference::engine::InputStruct const &IS) override;
                std::vector<inference::engine::OutputStruct> getOutput() override;
            private:
                void letterBoxScaleCoords(cv::Rect& coords, const cv::Size& imageSize, const cv::Size& mainImageSize);
                void vanillaResizeScaleCoords(cv::Rect& coords, const cv::Size& imageSize, const cv::Size& mainImageSize);
                bool postprocessing(std::vector<Ort::Value>& outputTensors, const cv::Size& mainImageSize, const cv::Size& modelInputSize);

            protected:
            };

            class Classification : public inference::engine::onnx::ONNX
            {
            public:
                Classification() = delete;
                Classification(const inference::engine::ConfigStruct& conf) : ONNX(conf){}

                // Assume that the model has one input and one output.
                inference::engine::ResponseStruct inference(inference::engine::InputStruct const &IS) override;
                std::vector<inference::engine::OutputStruct> getOutput() override;
            private:
                bool postprocessing(std::vector<Ort::Value>& outputTensors, inference::engine::ModelType modelType);
            protected:
            };   
        }
    }

    struct ConfigStruct
    {
        std::string model; // Model value or .onnx file path
        std::string modelConfig;
    };

    struct InputStruct
    {
        cv::Mat Image;
    };
    struct ModelOutputStruct : public inference::engine::OutputStruct
    {
        std::string labelType;
    };
    
    struct OCROutputStruct
    {
        std::string plateValue = "";
        std::string plateTemplate = "";
        int plateType = -1;
    };

    struct OutputStruct
    {
        std::vector<inference::ModelOutputStruct> modelOutputs;
        inference::OCROutputStruct OCRResult;
    };

    struct ModelConfigStruct
    {
        int inputHeight;
        int inputWidth;
        int numClasses;
        inference::standards::Task task;
        inference::standards::OcrMethod ocrMethod;
        int outputActivation;
        int resizingMethod;
        int resizingMethodpaddingValue;
        int normalizingMethod;
        int normalizingMethodscaleValue;
        std::vector<float> normalizingMethodmean;
        std::vector<float> normalizingMethodstd;
        int primaryThreshold;
        int secondaryThreshold;
        inference::standards::InferenceMethod inferenceMethod;
        int executionProviderGPU;
        int executionParallelMode;
        int intraOpNumThreads;
        int interOpNumThreads;
        inference::standards::MiddleMapStruct middleMap;
        std::vector<std::string> keys;
        std::unordered_map<std::string, std::unordered_map<int,int>> labelMap;
    };

    class Handler
    {
    public:
        Handler() = delete;
        Handler(const inference::ConfigStruct& conf);
        void run(const inference::InputStruct& input);
        inference::OutputStruct getOutput();
        std::vector<std::string> getOutputKeys();
    private:
        std::shared_ptr<inference::engine::IEngine> m_pEngine;
        std::shared_ptr<inference::algorithm::ocr::DetectedCharOCR> m_pDCOcr;
        inference::ConfigStruct m_conf;
        inference::ModelConfigStruct m_modelConfig;
        inference::OutputStruct m_output;

        size_t readFile(const std::string& filePath, std::string& output);
        void parseConfigJSON(const std::string& config);
        void initModel();
    };

    std::string getVersion();
}

#endif // !INFERENCE_H
