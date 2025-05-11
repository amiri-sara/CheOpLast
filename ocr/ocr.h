#pragma once
#include "commondata.hpp"
#include "fstream"
#include<iostream>
#include <iomanip>
#include <string>
#include<map>
#include <vector>
namespace irocr {
#define IR_DIGITS     {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9" , "10" , "11" , "12" , "13" , "14" , "15" , "16" , "17" , "18" , "19" , "20" , "21" , "22" , "23" , "24" , "25" , "26" , "27" , "28" , "29" , "30" , "31" , "32" , "33" , "34"}
}
namespace gocr {

#define ALL_LABELS       {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "f0", "f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8", "f9", "aleph", "be", "pe", "te", "se", "jim", "che", "fHJ", "dal", "re", "ze", "sin", "shin", "sad", "ta", "ayn", "fe", "ghaf", "kaf", "lam", "mim", "nun", "vav","hh", "ye", "SA_flag", "ARQ", "PV", "IR_Trans", "new_DUBAI", "new_RAK",  "new_abudabi", "new_ajman", "new_am", "new_fujairah", "old_DUBAI", "old_RAK", "old_abudabi", "old_ajman", "old_am",  "old_fujira", "old_sharka","Afghanistan","Armenia","Azerbaijan","Commercial","Germany","Iran","Iraq","IranFreeZone","IranTransit","Pakistan","Russia","SaudiArabia","Turkey","Turkmenistan","UAE", "Tajikistan"}

#define ALL_LETTERS      {"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "aleph", "be", "pe", "te", "se", "jim", "che", "fHJ", "dal", "re", "ze", "sin", "shin", "sad", "ta", "ayn", "fe", "ghaf", "kaf", "lam", "mim", "nun", "vav","hh", "ye"}
#define ALL_DIGITS       {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "f0", "f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8", "f9"}

#define LATIN_DIGITS     {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"}
#define PERSIAN_DIGITS   {"f0", "f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8", "f9"}

#define PERSIAN_LETTERS  {"aleph", "be", "pe", "te", "se", "jim", "che", "fHJ","khe", "dal","zal", "re", "ze","zhe", "sin", "shin", "sad","zad", "ta","za", "ayn","ghayn", "fe", "ghaf", "kaf","gaf", "lam", "mim", "nun", "vav","hh", "ye"}
#define LATIN_LETTERS    {"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z"}

#define IRQ_PROVINCE     {"ARQ",  "AR", "BAQ", "SOL", "KAR", "SALAH", "DAH", "ALAN", "AJR", "HML", "BBL", "HKM", "DYL",  "VLT", "PV"}
#define UAE_PROVINCE     {"new_DUBAI", "new_RAK",  "new_abudabi", "new_ajman", "new_am", "new_fujairah", "old_DUBAI", "old_RAK", "old_abudabi", "old_ajman", "old_am",  "old_fujira", "old_sharka"}
#define COUNTRIES     {"Afghanistan","Armenia","Azerbaijan","Commercial","Germany","Iran","Iraq","IranFreeZone","IranTransit","Pakistan","Russia","SaudiArabia","Turkey","Turkmenistan","UAE", "Tajikistan"}



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
    TAJIKISTAN,
    FOREIGN
};
struct Coordinates {
    int x;
    int y;
    int width;
    int height;
};
struct Annotation {
    std::string label;
    Coordinates coordinates;
    double score;
};


struct LicensePlateOCRResult
{
    std::string Country;
    std::string PlateValue;
    int PlateType = 0;
    std::vector<double> ProbabilityVector;
};


struct position_rank
{
    int top;
    int bot;
};



class GlobalPlateRecognition
{

private:
    std::pair<std::string, float> findCountryByDetections(std::vector<Annotation> rotated_annotations);
    std::string PersianRead(std::vector<Annotation> annotations);
    std::string LatinRead(std::vector<Annotation> annotations);
    std::string CompoundRead(std::vector<Annotation> annotations);
    void readLicensePlate(std::map<std::string, std::vector<gocr::Annotation> > top_bot_values, LicensePlateOCRResult &result, std::vector<Annotation> possible_countries);


public:
    LicensePlateOCRResult Run(std::vector<aivision::ObjAttributes> detections , std::string exactCountry);

    GlobalPlateRecognition();

    bool hasUAEprovince(std::vector<Annotation> &annotations);

    std::map<std::string, std::vector<Annotation>> possibleCountryCode(std::vector<Annotation> &annotations);

    //this function is the same as line_format in python
    std::map<std::string, std::vector<Annotation>> top_bot(std::vector<Annotation> &annotations);

    std::vector<Annotation> rotateAnnotations(std::vector<Annotation> &annotations);

    int getPersianDigitCount(const std::vector<Annotation> &annotations);

    int getPersianLetterCount(const std::vector<Annotation> &annotations);

    int getPersianWordCount(const std::vector<Annotation> &annotations);

    int getLatinDigitCount(const std::vector<Annotation> &annotations);

    int getLatinLetterCount(const std::vector<Annotation> &annotations);

    bool hasTRsequence(const std::vector<Annotation> &annotations);


    bool whiteSpace(std::vector<Annotation> &annotations);

    int countSubstring(const std::string& str, const std::string& sub);



    std::string getPersianStringSequence(std::vector<Annotation> &annotations);

    std::string getLatinStringSequence(std::vector<Annotation> &annotations);

    int findSequence(std::string sequnce, std::string query)
    {
        return countSubstring(sequnce, query);
    }


    bool hasIRAQ_Permutations(std::vector<Annotation> &codes);

    bool hasIranTransitFlag(std::vector<Annotation> &singleLine);

    bool hasIRQ_ProvinceFlag(std::vector<Annotation> &annotations);

    double estimateUAEScore(std::map<std::string ,std::vector<Annotation> > &top_bots);

    double estimateArmeniaScore(std::map<std::string ,std::vector<Annotation> > &top_bots);

    double estimateAzerbaijanScore(std::map<std::string ,std::vector<Annotation> > &top_bots);

    double estimateTurkeyScore(std::map<std::string ,std::vector<Annotation> > &top_bots);

    double estimateGermanyScore(std::map<std::string ,std::vector<Annotation> > &top_bots);

    double estimateCommercialScore(std::map<std::string ,std::vector<Annotation> > &top_bots);

    double estimateRussiaScore(std::map<std::string ,std::vector<Annotation> > &top_bots);

    double estimateTurkmenistanScore(std::map<std::string ,std::vector<Annotation> > &top_bots);

    double estimateSaudiArabiaScore(std::map<std::string ,std::vector<Annotation> > &top_bots);

    double estimateIranScore( std::map<std::string ,std::vector<Annotation> > &top_bots);

    double estimateFreezoneScore( std::map<std::string ,std::vector<Annotation> > &top_bots);

    double estimateIranTransitScore( std::map<std::string ,std::vector<Annotation> > &top_bots);

    double estimateIraqScore( std::map<std::string ,std::vector<Annotation> > &top_bots);

    double estimateAfghanistanScore( std::map<std::string ,std::vector<Annotation> > &top_bots);

    std::vector<Annotation>  addWhiteSpace(std::vector<Annotation> annots);


};

}
