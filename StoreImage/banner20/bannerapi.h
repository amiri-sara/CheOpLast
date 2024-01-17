#ifndef BANNERAPI_H
#define BANNERAPI_H


#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <opencv2/core.hpp>
#include <opencv2/freetype.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/cvstd.hpp>

#define BA_VERSION_MAJOR    0
#define BA_VERSION_MINOR    2
#define BA_VERSION_REVISION 0
std::string getVersion();
extern "C" class BannerAPI
{
public:
    BannerAPI();
    std::string Out;
    struct Config{
        std::string FontFamily="";
        std::string FontColor="";
        int BannerHeightPercent=20;
        int fontHeight = 150;
        int thickness = -1;
        int linestyle = 16;
        int baseline = 0;
        int padding=1;
        int BannerHeight=400;
        int BannerWidth=1200;
        int BannerMargin=10;
    };
    void init(Config& config);
    struct Data{
        std::string Date;
        std::string Time;
        std::string LocationPrefix="محل تردد:";
        std::string Location;
        std::string WeekDayName[8]={"",
                                    "شنبه",
                                    "یکشنبه",
                                    "دوشنبه",
                                    "سه شنبه",
                                    "چهارشنبه",
                                    "پنج شنبه",
                                    "جمعه"};
        char WeekDay;
        std::string PlatePrefix="پلاک:";
        long int Plate;

    };

    Config ConfigStruct;
    Data DataStruct;
    struct LineNum_{
        int RTL=0;
        int LTR=0;
    };
    enum Alligns{
        RTL,
        LTR
    };

    struct LineStruct{
        std::string Text;
        Alligns LineAllignment;
        cv::Size LineSize;
        bool newLine=true;
        int FontHeight;
    };
    std::vector<LineStruct> Lines;
    LineNum_ LineNum;
    const cv::Mat Run(std::vector<LineStruct> Lines);
    const cv::Mat Run(std::vector<LineStruct> Lines,int width, int height);
    const std::string WeekDayName[8]={"",
                                "شنبه",
                                "یکشنبه",
                                "دوشنبه",
                                "سه شنبه",
                                "چهارشنبه",
                                "پنج شنبه",
                                "جمعه"};
    const std::string TypePlate(long int plate);
private:

    std::string Text;
    cv::Ptr<cv::freetype::FreeType2> ft2;
    void FontInit(std::string& fontFamily);
    cv::Size GetTextSize(std::string &text,int fontHeight);
    inline void Write(std::string &text,cv::Mat &img,cv::Point &WritePoint,int &fontHeight){ft2->putText( img, text, WritePoint, fontHeight,
                                                           cv::Scalar::all(0), ConfigStruct.thickness, ConfigStruct.linestyle, true );}
    class PersianType;
    PersianType *PT;//new PersianType();
    cv::Mat img;//

    std::string PlateAlphabet[36]={
        "",
        "الف",
        "ب",
        "پ",
        "ت",
        "ث",
        "ج",
        "چ", //Wheelchair♿
        "ح",
        "خ",
        "د",
        "ذ",
        "ر",
        "ز",
        "ژ",
        "س",
        "ش",
        "ص",
        "ض",
        "ط",
        "ظ",
        "ع",
        "غ",
        "ف",
        "ق",
        "ک",
        "گ",
        "ل",
        "م",
        "ن",
        "و",
        "هـ",
        "ی",
        "D",
        "S",
        "-"
    };
    inline int getNumByPos(long int num,int size,int from, int to){return static_cast<int>(num/(pow(10,size-to)))%static_cast<int>(pow(10,to-from+1));};
    void CreateBG();
};
class BannerAPI::PersianType
{
public:
    PersianType();
    void run(std::string& InputText,std::string& OutputText);

private:

            unsigned char GetCharPlace(const std::string &fa_character, const std::string &prevFAChar, const std::string &nextFAChar);

            bool PersianIsBeginner(const std::string &fa_character);
            unsigned char GetCharIndex(const std::string &fa_character);
            bool IsPersian(const std::string &fa_character);
    std::vector<std::string> ReverseText(const std::string &str);
    std::string GetFACharGlyph(const std::string &fa_character, const std::string &prevFAChar, const std::string &nextFAChar);
    std::string ConvertToFAGlyphs(const std::string &text);

    std::string NumericalConvert( std::string &text);


};


#endif // BANNERAPI_H