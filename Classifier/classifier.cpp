#include "classifier.h"

// std::unique_ptr<NNModel> Classifier::getModel(BaseNNConfig& conf, const std::string& modelData, const string& path) {
//     if (conf.read(path) != 0) {
//         cerr << "Failed to read config: " << path << endl;
//         throw runtime_error("Config read failed");
//     }
//     aivision::ModelFactory factory;
//     auto model = factory.get(conf);
//     int read = 0;
//     char* pChars = readAllBytes(modelData, &read);//readAllBytes(conf.modelPath.c_str(), &read);
//     if (!pChars || read == 0) {
//         cerr << "Failed to read model file: " << conf.modelPath << endl;
//         throw runtime_error("Model file read failed");
//     }
//     conf.modelData = pChars;
//     conf.modelByteSize = read;
//     if (model->init(conf) != 0) {
//         cerr << "Failed to init model: " << conf.modelPath << endl;
//         delete[] pChars;
//         throw runtime_error("Model init failed");
//     }
//     delete[] pChars;
//     return model;
// }

std::unique_ptr<NNModel> Classifier::getModel(BaseNNConfig& conf, const string& path) {
    cout << "Loading config: " << path << endl;
    if (conf.read(path) != 0) {
        cerr << "Failed to read config: " << path << endl;
        throw runtime_error("Config read failed");
    }
    aivision::ModelFactory factory;
    auto model = factory.get(conf);
    if (!model) {
        cerr << "Failed to create model from config: " << path << endl;
        throw runtime_error("Model creation failed");
    }
    int read = 0;
    char* pChars = readAllBytes(conf.modelPath.c_str(), &read);
    if (!pChars || read == 0) {
        cerr << "Failed to read model file: " << conf.modelPath << endl;
        throw runtime_error("Model file read failed");
    }
    conf.modelData = pChars;
    conf.modelByteSize = read;
    cout << "Initializing model: " << conf.modelPath << endl;
    if (model->init(conf) != 0) {
        cerr << "Failed to init model: " << conf.modelPath << endl;
        delete[] pChars;
        throw runtime_error("Model init failed");
    }
    delete[] pChars;
    return model;
}
Classifier::Classifier(const Classifier::ConfigStruct& configs)
{
   try
   {
    if(configs.CDConfig.active)
    {
        BaseNNConfig detConfig;
        std::string detConfigPath = "/home/amiri/projects/c++/models/PlateClassifierPackage/inference_pipeline_v2.2.0_backup/models/carDetection/info.json"; 
        this->m_models.CD = getModel(detConfig,detConfigPath);
    }
    if(configs.LCCConfig.active)
    {
        BaseNNConfig LCConfig;
        std::string LCCConfigPath = "/home/amiri/projects/c++/models/PlateClassifierPackage/inference_pipeline_v2.2.0_backup/models/loadDetection/info.json";
        this->m_models.LCC = getModel(LCConfig, LCCConfigPath);
    }

   }
    catch (const std::exception& e)
    {
        throw std::runtime_error(e.what());
    } 
}


int mapLabelToWebKey(int label) {
    static const std::map<int, int> labelToKeyMap = {
        {0, 0},   // 206
        {1, 1},   // 207
        {2, 2},   // 405
        {3, 4},   // arisan
        {4, 5},   // bereliance
        {5, 6},   // bus
        {6, 7},   // dena
        {7, 8},   // etc -> Other
        {8, 3},   // L90
        {9, 9},   // mazdavanet
        {10, 10}, // minibus
        {11, 11}, // neysan
        {12, 12}, // pars
        {13, 13}, // peykan
        {14, 14}, // peykanvanet
        {15, 15}, // pride
        {16, 16}, // pridevanet
        {17, 17}, // queek -> Quik
        {18, 18}, // rana
        {19, 19}, // rio
        {20, 20}, // saina
        {21, 21}, // samand
        {22, 22}, // sandro -> Sandero
        {23, 23}, // tiba
        {24, 24}, // truck
        {25, 25}, // van
        {26, 26}, // xantia
        {27, 33}, // arisan-loaded -> Arisan Loaded
        {28, 34}, // mazdavanet-loaded -> Mazda Vanet Loaded
        {29, 35}, // neysan-loaded -> Neysan Loaded
        {30, 36}, // peykanvanet-loaded -> Peykan Vanet Loaded
        {31, 37}, // pridevanet-loaded -> Pridevanet Loaded
        {32, 38}  // truck-loaded -> Truck Loaded
    };

    auto it = labelToKeyMap.find(label);
    if (it != labelToKeyMap.end()) {
        return it->second;
    }
    return -1; // Map to Unknown for invalid labels
}


std::vector<Classifier::OutputStruct> Classifier::run(const Classifier::InputStruct& input)
{
    std::vector<Classifier::OutputStruct> outputVec;
    cv::Mat ImageMat = createMatImage(input.ImageBase64); 
    if(ImageMat.total() == 0)
    {
        // output.codeType      = ChOp::CodeTypes::NULL_IMAGE;
        // output.newPlateValue = input.plateValue;
        // output.probability   = 0;
        // output.newPlateType  = static_cast<int>(gocr::PlateType::UNKNOWN);
        return outputVec;
    }
    // std::vector<aivision::nn::ObjectAttributes> dets;

    // Detection
    NNModel* detectorModel = this->m_models.CD.get();
    cout << "Running detection..." << endl;
    auto detStream = make_shared<DataModel>();
    detStream->addView({Point2f(0,0), Point2f(1,0), Point2f(1,1), Point2f(0,1)}, "offline");
    detStream->SetFrame(ImageMat);
    detStream->SetActivePreset(1);
    vector<shared_ptr<DataModel>> detStreams = {detStream};
    detectorModel->forward(detStreams);
    auto dets = detStream->getResults();
    if (dets.empty()) {
        cerr << "No boxes found" << endl;
    }

    for (const auto& det : dets) {
            Classifier::OutputStruct output;

       Rect2d bbox = det.bbox;
//        cout << "Detected bbox: x=" << bbox.x << ", y=" << bbox.y
//             << ", width=" << bbox.width << ", height=" << bbox.height
//             << ", score=" << det.score << endl;

//        // Validate bbox
//        if (bbox.x < 0 || bbox.y < 0 || bbox.width <= 0 || bbox.height <= 0 ||
//            bbox.x + bbox.width > img.cols || bbox.y + bbox.height > img.rows) {
//            cerr << "Invalid bbox detected, skipping..." << endl;
//            continue;
//        }

        cout << "Extracting ROI..." << endl;
        Mat carROI = ImageMat(bbox).clone();
        if (carROI.empty()) {
            cerr << "Failed to extract ROI for bbox" << endl;
            continue;
        }
        cout << "ROI extracted, size: " << carROI.cols << "x" << carROI.rows << endl;

        // Plate Classification
        NNModel* loadClassifierModel = this->m_models.LCC.get();
        cout << "Running classification..." << endl;
        auto plateStream = make_shared<DataModel>();
        plateStream->addView({Point2f(0,0), Point2f(1,0), Point2f(1,1), Point2f(0,1)}, "offline");
        plateStream->SetFrame(carROI);
        plateStream->SetActivePreset(1);
        vector<shared_ptr<DataModel>> plateStreams = {plateStream};
        loadClassifierModel->forward(plateStreams);
        auto platePreds = plateStream->getResults();

        string plateClass = "Unknown";
        float classConfidence = 0.0f;
        int lable = 0;
        if (!platePreds.empty()) {
            plateClass = platePreds[0].class_name.empty() ? to_string(platePreds[0].label) : platePreds[0].class_name;
            classConfidence = platePreds[0].score;
            lable = platePreds[0].label;

        }
        cout << "Classification result: " << plateClass << " (" << classConfidence << ")" << endl;
        output.vehicleModel = mapLabelToWebKey(lable);
        output.probability = classConfidence;
        output.box = bbox;
        outputVec.push_back(output);


    }

return outputVec;
}





















    // if(this->m_models.CD)
    // {

        // NNModel* detectorModel = this->m_models.CD.get();
    //     auto detStream = make_shared<DataModel>();
    //     detStream->addView({Point2f(0,0), Point2f(1,0), Point2f(1,1), Point2f(0,1)}, "offline");
    //     detStream->SetFrame(ImageMat);
    //     detStream->SetActivePreset(1);
    //     vector<shared_ptr<DataModel>> detStreams = {detStream};
    //     detectorModel->forward(detStreams);
    //     dets = detStream->getResults();
    //     if(dets.empty()){
    //         std::cerr << "no boxes found" << std::endl;
    //     }

    //     if(dets.size() == 0)
    //     { //TODO
    //         // output.codeType = ChOp::CodeTypes::NOT_PLATE;
    //         // output.newPlateValue = input.plateValue;
    //         // output.probability = 0;
    //         // output.croppedPlateImage = PlateImageMat;
    //         return output;
    //     }
        
        

    // // }
    // cv::Mat LoadImage;

    // if(dets.size() > 0)
    // {
    //     std::vector<int> scores;
    //     for (const auto& det : dets)
    //         scores.push_back(det.score * 100);
        
    //     int maxIndex = std::max_element(scores.begin(), scores.end()) - scores.begin();
    //     Rect2d carRect = dets[maxIndex].bbox;
    //     cv::imshow("t",ImageMat(carRect));
    //     cv::waitKey(0);
        

    //     //Validate the ROI Parameters:
    //     if (carRect.x >= 0 && carRect.y >= 0 && 
    //         carRect.width > 0 && carRect.height > 0 && 
    //         carRect.x + carRect.width <= ImageMat.cols && 
    //         carRect.y + carRect.height <= ImageMat.rows) {
    //         LoadImage = ImageMat(carRect).clone(); // Safe to use ROI
    //     }
    //     else{
    //         LoadImage = ImageMat.clone();
    //     }
    // }
    // else{
    //     LoadImage = ImageMat.clone();
    // }
    // int Type = -1;


    // if(this->m_models.LCC)
    // {
    //     NNModel* loadClassifierModel = this->m_models.LCC.get();
    //     auto frameStream = make_shared<DataModel>();
    //     frameStream->addView({Point2f(0,0), Point2f(1,0), Point2f(1,1), Point2f(0,1)}, "offline");
    //     frameStream->SetFrame(LoadImage);
    //     frameStream->SetActivePreset(1);
    //     vector<shared_ptr<DataModel>> frameStreams = {frameStream};
    //     loadClassifierModel->forward(frameStreams);
    //     auto Preds = frameStream->getResults();


    //     string plateClass = "Unknown";
    //     float classConfidence = 0.0f;
    //     if (!Preds.empty()) {
    //         // plateClass = platePreds[0].class_name.empty() ? to_string(platePreds[0].label) : platePreds[0].class_name;
    //         // classConfidence = platePreds[0].score;

    //         std::vector<int> scores;
    //         for(auto& object : Preds)
    //             scores.push_back(object.score * 100);
            
    //         int maxIndex = std::max_element(scores.begin(), scores.end()) - scores.begin();
    //         Type = Preds[maxIndex].label;
    //     }
    // }


// }