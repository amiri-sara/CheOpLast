#ifndef CHECKOP_H
#define CHECKOP_H

#include "../IDLM/detection.h"
#include "../IDLM/classifier.h"
#include <opencv2/imgproc/types_c.h>

#define CHECK_OP_VERSION "0.0.0"

class CheckOP
{
public:
    enum CodeTypes
    {
        CodeTypeNotPlateNumber = -5,
        CodeTypeInvalidPacket,
        CodeTypeCorruptedImg,
        CodeTypeNullImg,
        CodeTypeNotPlate,

        CodeTypeOther,
        CodeTypeMotorBike,
        CodeTypeRepairePlate,
        CodeTypeTwoSmallNum = 4,
        CodeTypeThreeBigNum,
        CodeTypeTwoBigNum,
        CodeTypeDifferentAlph,
        CodeTypeNotProcessed = 9,
        CodeTypeSureNightTime,
        CodeTypeSure = 20
    };
    enum PlateTypes
    {
        PLATE_OTHER,
        PLATE_PERSIAN,
        PLATE_MOTORBIKE,
        PLATE_FREEZONE,
        PLATE_IRAN_TRANSIT_NEW,
        PLATE_IRAN_TRANSIT_OLD,
        PLATE_FOREIGN,
    };
    struct CheckOPConfigStruct
    {
        std::string PDModelValue;
        cv::Size PDInputSize; // It is needed when the input of the model is dynamic
        int PDPrimaryThreshold;
        int PDSecondaryThreshold;

        std::string PCModelValue;
        cv::Size PCInputSize; // It is needed when the input of the model is dynamic
        int PCPrimaryThreshold;
        int PCSecondaryThreshold;

        std::string PROCRModelValue;
        cv::Size PROCRInputSize; // It is needed when the input of the model is dynamic
        int PROCRPrimaryThreshold;
        int PROCRSecondaryThreshold;

        std::string MBOCRModelValue;
        cv::Size MBOCRInputSize; // It is needed when the input of the model is dynamic
        int MBOCRPrimaryThreshold;
        int MBOCRSecondaryThreshold;

        std::string TZOCRModelValue;
        cv::Size TZOCRInputSize; // It is needed when the input of the model is dynamic
        int TZOCRPrimaryThreshold;
        int TZOCRSecondaryThreshold;

        bool log = false;
    };
    struct CheckOPOutputStruct
    {
        std::string NewPlateValue = "";
        int CodeType = CodeTypeNotProcessed;
        float Probability = 0;
        cv::Rect PlateRect = cv::Rect(0,0,0,0);
        cv::Mat PlateImage;
    };
    CheckOP(CheckOPConfigStruct CheckOPConfig);
    CheckOPOutputStruct run(cv::Mat PlateImage, std::string PlateValue);

private:
    CheckOPConfigStruct Config;
    //std::shared_ptr<Detection> PDObj;
    std::map<IDLM::ModelType, std::shared_ptr<Detection>> DetectionObjMap;
    std::map<IDLM::ModelType, std::shared_ptr<Classifier>> ClassifierObjMap;
    //std::shared_ptr<Classifier> PCObj;
    //std::shared_ptr<Classifier> PROCRObj;
    //std::shared_ptr<Classifier> MBOCRObj;

    void fixRectDimension(cv::Rect *Cand_Rect, int Col, int Row)
    {
        Cand_Rect->x = Cand_Rect->x > 0 ? Cand_Rect->x:0;
        Cand_Rect->y = Cand_Rect->y > 0 ? Cand_Rect->y:0;
        Cand_Rect->height = (Cand_Rect->y + Cand_Rect->height) > Row ?  Row-Cand_Rect->y:Cand_Rect->height;
        Cand_Rect->width  = (Cand_Rect->x + Cand_Rect->width ) > Col ?  Col-Cand_Rect->x:Cand_Rect->width;

        if(Cand_Rect->width<0 || Cand_Rect->height<0)
        {
            *Cand_Rect = cv::Rect(0,0,0,0);
        }
    }

    std::string convertPlateValue(std::vector<int> PlateValue, int PlateType)
    {
        std::string PlateValueString = "";

        if(PlateType == PlateTypes::PLATE_PERSIAN || PlateType == PlateTypes::PLATE_IRAN_TRANSIT_OLD)
        {
            for(int i = 0; i < PlateValue.size(); i++)
            {
                if(i == 2)
                {
                    if(PlateValue[i] < 10)
                    {
                        PlateValueString.append(std::to_string(0));

                    }
                    PlateValueString.append(std::to_string(PlateValue[i]));
                }
                else
                {
                    PlateValueString.append(std::to_string(PlateValue[i]));
                }
            }
        }
        else if(PlateType == PlateTypes::PLATE_IRAN_TRANSIT_NEW)
        {
            for(int i = 0; i < PlateValue.size(); i++)
            {
                if(i == 2 || i == 3)
                {
                    if(PlateValue[i] < 10)
                    {
                        PlateValueString.append(std::to_string(0));

                    }
                    PlateValueString.append(std::to_string(PlateValue[i]));
                }
                else
                {
                    PlateValueString.append(std::to_string(PlateValue[i]));
                }
            }
        }
        else
        {
            for(int i = 0; i < PlateValue.size(); i++)
            {
                PlateValueString.append(std::to_string(PlateValue[i]));
            }
        }

        return PlateValueString;
    }

    int calculateCodeType(std::string OutputPlateName, std::string InputPlateName, int PlateType)
    {
        int CodeType = CodeTypes::CodeTypeNotPlate;
        if(PlateType == PlateTypes::PLATE_MOTORBIKE)
        {
            return CodeTypes::CodeTypeMotorBike;
        }

        std::string OutputPlateExtension         =  OutputPlateName.substr(2,2);
        std::string OutputPlateTwoBigNumbers     =  OutputPlateName.substr(0,2);
        std::string OutputPlateThreeBigNumbers   =  OutputPlateName.substr(4,3);
        std::string OutputPlateTwoSmallNumbers   =  OutputPlateName.substr(7,2);

        std::string InputPlateExtension         =  InputPlateName.substr(2,2);
        std::string InputPlateTwoBigNumbers     =  InputPlateName.substr(0,2);
        std::string InputPlateThreeBigNumbers   =  InputPlateName.substr(4,3);
        std::string InputPlateTwoSmallNumbers   =  InputPlateName.substr(7,2);


        if(OutputPlateName == InputPlateName)
        {
            CodeType = CodeTypeSure;
        }
        else if(OutputPlateExtension != InputPlateExtension)
        {
            CodeType =  CodeTypeDifferentAlph;
        }
        else if(OutputPlateTwoSmallNumbers!= InputPlateTwoSmallNumbers)
        {
            CodeType =  CodeTypeTwoSmallNum;
        }
        else if(OutputPlateTwoBigNumbers != InputPlateTwoBigNumbers)
        {
            CodeType =  CodeTypeTwoBigNum;
        }
        else if(OutputPlateThreeBigNumbers != InputPlateThreeBigNumbers)
        {
            CodeType =  CodeTypeThreeBigNum;
        }
        else
        {
            std::cout<<"Somethings Wrong ! OutputPlateName "<<OutputPlateName<<" InputPlate Name is "<<InputPlateName;
            CodeType =  CodeTypeOther;
        }

        return CodeType;
    }

};

#endif // CHECKOP_H
