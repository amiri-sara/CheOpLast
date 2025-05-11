#include "ocr.h"

using namespace gocr;
using namespace std;
bool sortAnnotationsByX(Annotation &a, Annotation &b)
{
    return a.coordinates.x < b.coordinates.x;
}

LicensePlateOCRResult GlobalPlateRecognition::Run(std::vector<aivision::ObjAttributes> detections , std::string exactCountry)
{
    if(exactCountry==""){

        LicensePlateOCRResult output;
        output.Country = "None";
        std::vector<Annotation> annotations;
        std::vector<Annotation> rotated_annotations;
        std::vector<std::string> classes = ALL_LABELS;
        std::vector<Annotation> possible_countries;
        for(auto &r : detections)
        {
            Annotation a;
            a.label = classes[r.label];
            a.coordinates.x = r.bbox.x + r.bbox.width/2;
            a.coordinates.y = r.bbox.y + r.bbox.height/2;
            a.coordinates.width  = r.bbox.width;
            a.coordinates.height = r.bbox.height;
            a.score = r.score;
            if(r.label>87)
                possible_countries.push_back(a);
            else
                annotations.push_back(a);
        }

        rotated_annotations = rotateAnnotations(annotations);
        auto top_bots  = top_bot(rotated_annotations);

        if(possible_countries.size()<1)
        {
            auto res = findCountryByDetections(rotated_annotations);
            output.Country  = res.first;
        }else
        {

            auto res = findCountryByDetections(rotated_annotations);
            output.Country  = possible_countries[0].label;
        }
        if(rotated_annotations.size()<3 && possible_countries.size()<1)
            output.Country = "None";
        readLicensePlate(top_bots,output,possible_countries);

        return output;
    }

    if(exactCountry=="Iran"){

        LicensePlateOCRResult output;
        output.Country = "Iran";
        std::vector<Annotation> annotations;
        std::vector<Annotation> rotated_annotations;
        std::vector<std::string> classes = ALL_LABELS;
        std::vector<Annotation> possible_countries;
        for(auto &r : detections)
        {
            Annotation a;
            a.label = classes[r.label];
            a.coordinates.x = r.bbox.x + r.bbox.width/2;
            a.coordinates.y = r.bbox.y + r.bbox.height/2;
            a.coordinates.width  = r.bbox.width;
            a.coordinates.height = r.bbox.height;
            a.score = r.score;
            if(r.label>87)
                possible_countries.push_back(a);
            else{
                possible_countries.push_back(a);
                annotations.push_back(a);
            }
        }

        rotated_annotations = rotateAnnotations(annotations);
        auto top_bots  = top_bot(rotated_annotations);

        if(rotated_annotations.size()<3)
            output.Country = "None";
        readLicensePlate(top_bots,output,possible_countries);

        return output;

    }

    if(exactCountry=="Freezone"){

        LicensePlateOCRResult output;
        output.Country = "IranFreeZone";
        std::vector<Annotation> annotations;
        std::vector<Annotation> rotated_annotations;
        std::vector<std::string> classes = ALL_LABELS;
        std::vector<Annotation> possible_countries;
        for(auto &r : detections)
        {
            Annotation a;
            a.label = classes[r.label];
            a.coordinates.x = r.bbox.x + r.bbox.width/2;
            a.coordinates.y = r.bbox.y + r.bbox.height/2;
            a.coordinates.width  = r.bbox.width;
            a.coordinates.height = r.bbox.height;
            a.score = r.score;
            if(r.label>87)
                possible_countries.push_back(a);
            else
                annotations.push_back(a);
        }

        rotated_annotations = rotateAnnotations(annotations);
        auto top_bots  = top_bot(rotated_annotations);

        if(rotated_annotations.size()<3)
            output.Country = "None";
        readLicensePlate(top_bots,output,possible_countries);

        return output;

    }

    if(exactCountry=="IranTransit"){

        LicensePlateOCRResult output;
        output.Country = "IranTransit";
        std::vector<Annotation> annotations;
        std::vector<Annotation> rotated_annotations;
        std::vector<std::string> classes = ALL_LABELS;
        std::vector<Annotation> possible_countries;
        for(auto &r : detections)
        {
            Annotation a;
            a.label = classes[r.label];
            a.coordinates.x = r.bbox.x + r.bbox.width/2;
            a.coordinates.y = r.bbox.y + r.bbox.height/2;
            a.coordinates.width  = r.bbox.width;
            a.coordinates.height = r.bbox.height;
            a.score = r.score;
            if(r.label>87)
                possible_countries.push_back(a);
            else
                annotations.push_back(a);
        }

        rotated_annotations = rotateAnnotations(annotations);
        auto top_bots  = top_bot(rotated_annotations);

        if(rotated_annotations.size()<3)
            output.Country = "None";
        readLicensePlate(top_bots,output,possible_countries);

        return output;

    }

    if(exactCountry=="Motorcycle"){

        LicensePlateOCRResult output;
        output.Country = "None";
        std::vector<Annotation> annotations;
        std::vector<Annotation> rotated_annotations;
        std::vector<std::string> classes = ALL_LABELS;
        std::vector<Annotation> possible_countries;
        for(auto &r : detections)
        {
            Annotation a;
            a.label = classes[r.label];
            a.coordinates.x = r.bbox.x + r.bbox.width/2;
            a.coordinates.y = r.bbox.y + r.bbox.height/2;
            a.coordinates.width  = r.bbox.width;
            a.coordinates.height = r.bbox.height;
            a.score = r.score;
            if(r.label>87)
                possible_countries.push_back(a);
            else
                annotations.push_back(a);
        }

        rotated_annotations = rotateAnnotations(annotations);
        auto top_bots  = top_bot(rotated_annotations);

        if(rotated_annotations.size()<3)
            output.Country = "None";
        readLicensePlate(top_bots,output,possible_countries);

        return output;

    }

}


void GlobalPlateRecognition::readLicensePlate(std::map<std::string, std::vector<gocr::Annotation> > top_bot_values, LicensePlateOCRResult &result, std::vector<Annotation> possible_countries)
{
    if(result.Country=="None")
    {
        std::vector<Annotation> single_line;
        single_line.insert(single_line.end(), top_bot_values["top"].begin(),top_bot_values["top"].end());
        single_line.insert(single_line.end(), top_bot_values["bot"].begin(),top_bot_values["bot"].end());
        result.PlateValue = CompoundRead(single_line);
        for(const Annotation &annotation:single_line)
        {
            result.ProbabilityVector.push_back(annotation.score);
        }

        return;
    }else
    {
        std::string Country = result.Country;
        if(Country=="Iran")
        {
            std::vector<Annotation> single_line;
            single_line.insert(single_line.end(), top_bot_values["top"].begin(),top_bot_values["top"].end());
            single_line.insert(single_line.end(), top_bot_values["bot"].begin(),top_bot_values["bot"].end());
            result.PlateValue = PersianRead(single_line);
            for(const Annotation &annotation:single_line)
            {
                result.ProbabilityVector.push_back(annotation.score);
            }
        }else if(Country == "Armenia")
            //#################################################################
            /*Armenia types:
          * type1 : NN  LL NNN
          * type2 : NNN LL NN
          */
        {
            std::vector<Annotation> single_line;
            std::vector<Annotation> edited_single_line;
            single_line.insert(single_line.end(), top_bot_values["top"].begin(),top_bot_values["top"].end());
            single_line.insert(single_line.end(), top_bot_values["bot"].begin(),top_bot_values["bot"].end());

            int ySum = 0;
            int hSum = 0;
            int wSum = 0;

            for (const Annotation &annotation : single_line) {
                ySum += annotation.coordinates.y;
                hSum += annotation.coordinates.height;
                wSum += annotation.coordinates.width;
            }

            double meanY = static_cast<double>(ySum) / single_line.size();
            double meanH = static_cast<double>(hSum) / single_line.size();

            std::string annotation_str = "";
            std::vector<std::string> digits = LATIN_DIGITS;
            std::vector<std::string> letters = LATIN_LETTERS;
            for(Annotation &annotation : single_line)
            {
                if((annotation.label == "A" | annotation.label == "M") && annotation.coordinates.height < 0.85*meanH)
                {
                    continue;
                }else
                {
                    edited_single_line.push_back(annotation);
                    if(std::find(digits.begin(),digits.end(),annotation.label) != digits.end())
                    {
                        annotation_str += "D";
                    }else if(std::find(letters.begin(),letters.end(),annotation.label) != letters.end())
                    {
                        annotation_str += "A";
                    }
                }
            }

            if(annotation_str == "DDAADDD")
            {
                ////result.PlateType = Type1;
                result.PlateValue = LatinRead(edited_single_line);
                for(const Annotation &annotation:edited_single_line)
                {
                    result.ProbabilityVector.push_back(annotation.score);
                }
            }else if(annotation_str == "DDDAADD")
            {
                //result.PlateType = Type2;
                result.PlateValue = LatinRead(edited_single_line);
                for(const Annotation &annotation:edited_single_line)
                {
                    result.ProbabilityVector.push_back(annotation.score);
                }
            }else ////result.PlateType = Unk;
            {
                edited_single_line = addWhiteSpace(edited_single_line);
                annotation_str = "";
                for(Annotation &annotation:edited_single_line)
                {
                    if(std::find(digits.begin(),digits.end(),annotation.label) != digits.end())
                    {
                        annotation_str += "D";
                    }else if(std::find(letters.begin(),letters.end(),annotation.label) != letters.end())
                    {
                        annotation_str += "A";
                    }else if(annotation.label == "blank") annotation_str += "_";
                }
                if(annotation_str.find("DDDA"))
                {
                    //result.PlateType = Type1;
                    result.PlateValue = LatinRead(edited_single_line);
                    for(const Annotation &annotation:edited_single_line)
                    {
                        result.ProbabilityVector.push_back(annotation.score);
                    }
                }else if(annotation_str.find("DDA"))
                {
                    //result.PlateType = Type2;
                    result.PlateValue = LatinRead(edited_single_line);
                    for(const Annotation &annotation:edited_single_line)
                    {
                        result.ProbabilityVector.push_back(annotation.score);
                    }
                }else
                {
                    //result.PlateType = Unk;
                    result.PlateValue = LatinRead(edited_single_line);
                    for(const Annotation &annotation:edited_single_line)
                    {
                        result.ProbabilityVector.push_back(annotation.score);
                    }
                }
            }

        }else if(Country == "Azerbaijan")
            //#################################################################
            /*Azerbaijan types:
             * type1 : NN LL NNN
             * format is always the same
             */
        {
            std::vector<Annotation> single_line;
            std::vector<Annotation> edited_single_line;
            single_line.insert(single_line.end(), top_bot_values["top"].begin(),top_bot_values["top"].end());
            single_line.insert(single_line.end(), top_bot_values["bot"].begin(),top_bot_values["bot"].end());

            int ySum = 0;
            int hSum = 0;
            int wSum = 0;

            for (const Annotation &annotation : single_line) {
                ySum += annotation.coordinates.y;
                hSum += annotation.coordinates.height;
                wSum += annotation.coordinates.width;
            }

            double meanY = static_cast<double>(ySum) / single_line.size();
            double meanH = static_cast<double>(hSum) / single_line.size();

            std::string annotation_str = "";
            std::vector<std::string> digits = LATIN_DIGITS;
            std::vector<std::string> letters = LATIN_LETTERS;
            for(Annotation &annotation : single_line)
            {
                if((annotation.label == "A" | annotation.label == "Z") && annotation.coordinates.height < 0.85*meanH)
                {
                    continue;
                }else
                {
                    edited_single_line.push_back(annotation);
                    if(std::find(digits.begin(),digits.end(),annotation.label) != digits.end())
                    {
                        annotation_str += "D";
                    }else if(std::find(letters.begin(),letters.end(),annotation.label) != letters.end())
                    {
                        annotation_str += "A";
                    }
                }
            }

            if(annotation_str == "DDAADDD")
            {
                //result.PlateType = Type1;
                result.PlateValue = LatinRead(edited_single_line);
                for(const Annotation &annotation:edited_single_line)
                {
                    result.ProbabilityVector.push_back(annotation.score);
                }
            }else ////result.PlateType = Unk;
            {
                edited_single_line = addWhiteSpace(edited_single_line);
                annotation_str = "";
                for(Annotation &annotation:edited_single_line)
                {
                    if(std::find(digits.begin(),digits.end(),annotation.label) != digits.end())
                    {
                        annotation_str += "D";
                    }else if(std::find(letters.begin(),letters.end(),annotation.label) != letters.end())
                    {
                        annotation_str += "A";
                    }else if(annotation.label == "blank") annotation_str += "_";
                }

                result.PlateValue = LatinRead(edited_single_line);
                for(const Annotation &annotation:edited_single_line)
                {
                    result.ProbabilityVector.push_back(annotation.score);
                }

            }
        }else if(Country == "Commercial")
            //#################################################################
            /*Commercial types:
          * We dont care about Commercial. It only helps us to improve character recognition
          *
          */
        {
            std::vector<Annotation> single_line;
            single_line.insert(single_line.end(), top_bot_values["top"].begin(),top_bot_values["top"].end());
            single_line.insert(single_line.end(), top_bot_values["bot"].begin(),top_bot_values["bot"].end());
            result.PlateValue = LatinRead(single_line);
            //result.PlateType = Type1;
            for(const Annotation &annotation:single_line)
            {
                result.ProbabilityVector.push_back(annotation.score);
            }
        }else if(Country == "Germany")
            //#################################################################
            /*Germany types:
          * Germany has uncountable types. We ignore its type variety for now and we will consider it later.
          */
        {
            std::vector<Annotation> single_line;
            single_line.insert(single_line.end(), top_bot_values["top"].begin(),top_bot_values["top"].end());
            single_line.insert(single_line.end(), top_bot_values["bot"].begin(),top_bot_values["bot"].end());
            result.PlateValue = LatinRead(single_line);
            //result.PlateType = Type1;
            for(const Annotation &annotation:single_line)
            {
                result.ProbabilityVector.push_back(annotation.score);
            }
        }else if(Country == "IranTransit")
            //#################################################################
            /*IranTransit types:
          * type1: new plates: NNLLNNN
          * type2: old plates: NNLNNN10
          * type3: ola plates: IR/THR/TEH-NNNNN
          * type4: decorative plates: RT-NNNNNNN
          */
        {
            std::vector<Annotation> single_line;
            std::string annotation_str = "";
            std::vector<std::string> digits = LATIN_DIGITS;
            std::vector<std::string> letters = LATIN_LETTERS;
            single_line.insert(single_line.end(), top_bot_values["top"].begin(),top_bot_values["top"].end());
            single_line.insert(single_line.end(), top_bot_values["bot"].begin(),top_bot_values["bot"].end());
            single_line = addWhiteSpace(single_line);
            for(Annotation &annotation:single_line)
            {
                if(std::find(digits.begin(),digits.end(),annotation.label) != digits.end())
                {
                    annotation_str += "D";
                }else if(std::find(letters.begin(),letters.end(),annotation.label) != letters.end())
                {
                    annotation_str += "A";
                }else if(annotation.label == "blank") annotation_str += "_";
            }
            if(annotation_str.find("DDDDDDD") || annotation_str.find("DDDDDD"))
            {
                //result.PlateType = Type4;
            }else if(annotation_str.find("AAA") || annotation_str.find("AADDDD"))
            {
                //result.PlateType = Type3;
            }else if(annotation_str.find("DAD") || (annotation_str.size()==8 && single_line.back().label=="0"))
            {
                //result.PlateType = Type2;
            }else if(annotation_str.find("DDAADD") || annotation_str.find("DAADDD")|| annotation_str.find("AADDD"))
            {
                //result.PlateType = Type1;
            }else //result.PlateType = Unk;

            result.PlateValue = LatinRead(single_line);
            for(const Annotation &annotation:single_line)
            {
                result.ProbabilityVector.push_back(annotation.score);
            }
        }else if(Country == "Russia")
            //#################################################################
            /*Russia types:
          * type1: L NNN LL NN(N)
          */
        {
            std::vector<Annotation> single_line;
            single_line.insert(single_line.end(), top_bot_values["top"].begin(),top_bot_values["top"].end());
            single_line.insert(single_line.end(), top_bot_values["bot"].begin(),top_bot_values["bot"].end());
            result.PlateValue = LatinRead(single_line);
            //result.PlateType = Type1;
            for(const Annotation &annotation:single_line)
            {
                result.ProbabilityVector.push_back(annotation.score);
            }
        }else if(Country == "Turkey")
            //#################################################################
            /*Turkey types:
          * type1: NN L   NNNN(N)
          * type2: NN LL  NNN(N)
          * type3: NN LLL NN(N)
          */
        {
            std::vector<Annotation> single_line;
            std::vector<Annotation> edited_single_line;
            std::string annotation_str = "";
            std::vector<std::string> digits = LATIN_DIGITS;
            std::vector<std::string> letters = LATIN_LETTERS;
            single_line.insert(single_line.end(), top_bot_values["top"].begin(),top_bot_values["top"].end());
            single_line.insert(single_line.end(), top_bot_values["bot"].begin(),top_bot_values["bot"].end());

            int ySum = 0;
            int hSum = 0;
            int wSum = 0;

            for (const Annotation &annotation : single_line) {
                ySum += annotation.coordinates.y;
                hSum += annotation.coordinates.height;
                wSum += annotation.coordinates.width;
            }

            double meanY = static_cast<double>(ySum) / single_line.size();
            double meanH = static_cast<double>(hSum) / single_line.size();

            for(Annotation &annotation : single_line)
            {
                if((annotation.label == "T" | annotation.label == "R") && annotation.coordinates.height < 0.85*meanH)
                {
                    continue;
                }else
                {
                    edited_single_line.push_back(annotation);
                    if(std::find(digits.begin(),digits.end(),annotation.label) != digits.end())
                    {
                        annotation_str += "D";
                    }else if(std::find(letters.begin(),letters.end(),annotation.label) != letters.end())
                    {
                        annotation_str += "A";
                    }
                }
            }
            if(annotation_str.find("DDADD") || annotation_str.find("DADDD"))
            {
                //result.PlateType = Type1;
            }else if(annotation_str.find("DDAADD") || annotation_str.find("DAADDD"))
            {
                //result.PlateType = Type2;
            }else if(annotation_str.find("DAAAD") || annotation_str.find("DDAAAD"))
            {
                //result.PlateType = Type3;
            }else //result.PlateType = Unk;
            result.PlateValue = LatinRead(edited_single_line);
            for(const Annotation &annotation:edited_single_line)
            {
                result.ProbabilityVector.push_back(annotation.score);
            }
        }else if(Country == "Turkmenistan")
            //#################################################################
            /*Turkmenistan types:
          * type1: usual plates: LL NNNN LL
          * type2: diplomatic: NN (CD) NNN
          * type3: export: (TDG) NNNN
          * foreign national, foreign trailer, govermental, ceremony and military are not covered.
          */
        {
            std::vector<Annotation> single_line;
            std::vector<Annotation> edited_single_line;
            std::string annotation_str = "";
            std::string label_str = "";
            std::vector<std::string> digits = LATIN_DIGITS;
            std::vector<std::string> letters = LATIN_LETTERS;
            single_line.insert(single_line.end(), top_bot_values["top"].begin(),top_bot_values["top"].end());
            single_line.insert(single_line.end(), top_bot_values["bot"].begin(),top_bot_values["bot"].end());
            single_line = addWhiteSpace(single_line);

            int ySum = 0;
            int hSum = 0;
            int wSum = 0;

            for (const Annotation &annotation : single_line) {
                ySum += annotation.coordinates.y;
                hSum += annotation.coordinates.height;
                wSum += annotation.coordinates.width;
            }

            double meanY = static_cast<double>(ySum) / single_line.size();
            double meanH = static_cast<double>(hSum) / single_line.size();

            for(Annotation &annotation : single_line)
            {
                if((annotation.label == "T" || annotation.label == "M") && annotation.coordinates.height < 0.85*meanH)
                {
                    continue;
                }else
                {
                    edited_single_line.push_back(annotation);
                    if(std::find(digits.begin(),digits.end(),annotation.label) != digits.end())
                    {
                        annotation_str += "D";
                    }else if(std::find(letters.begin(),letters.end(),annotation.label) != letters.end())
                    {
                        annotation_str += "A";
                    }else if(annotation.label == "blank") annotation_str += "_";
                }
            }

            for(Annotation &annotation:edited_single_line)
            {
                label_str += annotation.label;
            }

            if(label_str.find("TDG") || annotation_str.find("A_A")|| annotation_str.find("AA_DDDD"))
            {
                //result.PlateType = Type3;
            }else if(label_str.find("CD") || annotation_str.find("DAAD"))
            {
                //result.PlateType = Type2;
            }else if(annotation_str.find("DDDDAA") || annotation_str.find("DDDDA_")|| annotation_str.find("DDDD_A"))
            {
                //result.PlateType = Type1;
            }else //result.PlateType = Unk;
            result.PlateValue = LatinRead(edited_single_line);
            for(const Annotation &annotation:edited_single_line)
            {
                result.ProbabilityVector.push_back(annotation.score);
            }
        }else if(Country == "Afghanistan")
            //#################################################################
            /*Afghanistan types:
          * type1:
          *
          */
        {
            std::vector<std::string> persian_letters = PERSIAN_LETTERS;
            Annotation last_top_char = top_bot_values["top"].back();
            int ySum = 0;
            int hSum = 0;
            int wSum = 0;

            for (const Annotation &annotation : top_bot_values["top"]) {
                ySum += annotation.coordinates.y;
                hSum += annotation.coordinates.height;
                wSum += annotation.coordinates.width;
            }

            double meanY = static_cast<double>(ySum) / top_bot_values["top"].size();
            double meanH = static_cast<double>(hSum) / top_bot_values["top"].size();

            if ((std::find(persian_letters.begin(), persian_letters.end(), last_top_char.label) != persian_letters.end())&&(last_top_char.coordinates.height < 0.85*meanH))
            {
                top_bot_values["top"].pop_back();
            }

            std::vector<std::string> letters = LATIN_LETTERS;
            std::vector<std::string> digits = LATIN_DIGITS;
            std::vector<Annotation> latin_code;
            std::vector<Annotation> common_chars;

            for (const Annotation &annotation : top_bot_values["bot"])
            {
                if ((annotation.coordinates.height < 0.85 * meanH))
                {
                    latin_code.push_back(annotation);

                }else
                {
                    common_chars.push_back(annotation);
                }

            }
            std::vector<Annotation> plate_value_top;
            std::vector<Annotation> plate_value_bot;
            std::string persian_prefix = "f";
            Annotation blank;
            int min_common_chars = min(common_chars.size(),top_bot_values["top"].size());
            for(int i=0;i<min_common_chars;i++)
            {
                if(top_bot_values["top"][i].label == (persian_prefix + common_chars[i].label))
                {
                    plate_value_top.push_back(top_bot_values["top"][i]);
                    plate_value_bot.push_back(common_chars[i]);
                }else if(common_chars[i].coordinates.x > (top_bot_values["top"][i].coordinates.x +0.25*top_bot_values["top"][i].coordinates.width))
                {
                    blank.coordinates.x = top_bot_values["top"][i].coordinates.x;
                    blank.coordinates.y = top_bot_values["top"][i].coordinates.y;
                    blank.coordinates.width = top_bot_values["top"][i].coordinates.width;
                    blank.coordinates.height = top_bot_values["top"][i].coordinates.height;
                    blank.label = "blank";
                    blank.score = 0;
                    plate_value_top.push_back(top_bot_values["top"][i]);
                    plate_value_bot.push_back(blank);
                }else if(top_bot_values["top"][i].coordinates.x > (common_chars[i].coordinates.x +0.25*common_chars[i].coordinates.width))
                {
                    blank.coordinates.x = common_chars[i].coordinates.x;
                    blank.coordinates.y = common_chars[i].coordinates.y;
                    blank.coordinates.width = common_chars[i].coordinates.width;
                    blank.coordinates.height = common_chars[i].coordinates.height;
                    blank.label = "blank";
                    blank.score = 0;
                    plate_value_top.push_back(blank);
                    plate_value_bot.push_back(common_chars[i]);
                }else if(top_bot_values["top"][i].score > common_chars[i].score)
                {
                    plate_value_top.push_back(top_bot_values["top"][i]);
                    top_bot_values["top"][i].label = top_bot_values["top"][i].label.erase(0,1);
                    plate_value_bot.push_back(top_bot_values["top"][i]);
                }else if(top_bot_values["top"][i].score < common_chars[i].score)
                {
                    plate_value_bot.push_back(common_chars[i]);
                    common_chars[i].label = persian_prefix + common_chars[i].label;
                    plate_value_top.push_back(common_chars[i]);
                }
            }

            result.PlateValue = PersianRead(plate_value_top);
            for(const Annotation &annotation:plate_value_top)
            {
                result.ProbabilityVector.push_back(annotation.score);
            }
            result.PlateValue += "\n";
            result.PlateValue += LatinRead(plate_value_bot);
            for(const Annotation &annotation:plate_value_bot)
            {
                result.ProbabilityVector.push_back(annotation.score);
            }
            //result.PlateType = Type1;
        }else if(Country == "IranFreeZone")
            //#################################################################
            /*Freezone types:
             * type1 : NNNNN
             * type2 : NNNNNNN
             */
        {
            std::vector<std::string> persian_digits = PERSIAN_DIGITS;
            std::vector<std::string> latin_digits = LATIN_DIGITS;
            std::vector<Annotation> plate_value_top;
            std::vector<Annotation> plate_value_bot;
            std::string persian_prefix = "f";
            Annotation blank;
            int min_common_chars = min(top_bot_values["top"].size(),top_bot_values["bot"].size());
            for(int i=0;i<min_common_chars;i++)
            {
                if(top_bot_values["top"][i].label == (persian_prefix + top_bot_values["bot"][i].label))
                {
                    plate_value_top.push_back(top_bot_values["top"][i]);
                    plate_value_bot.push_back(top_bot_values["bot"][i]);
                }else if(top_bot_values["bot"][i].coordinates.x > (top_bot_values["top"][i].coordinates.x +0.25*top_bot_values["top"][i].coordinates.width))
                {
                    blank.coordinates.x = top_bot_values["top"][i].coordinates.x;
                    blank.coordinates.y = top_bot_values["top"][i].coordinates.y;
                    blank.coordinates.width = top_bot_values["top"][i].coordinates.width;
                    blank.coordinates.height = top_bot_values["top"][i].coordinates.height;
                    if(top_bot_values["top"][i].score > 0.8)
                    {
                        blank.label = top_bot_values["top"][i].label;
                        blank.score = top_bot_values["top"][i].score;
                    }else
                    {
                        blank.label = "blank";
                        blank.score = 0;
                    }
                    plate_value_top.push_back(top_bot_values["top"][i]);
                    plate_value_bot.push_back(blank);
                }else if(top_bot_values["top"][i].coordinates.x > (top_bot_values["bot"][i].coordinates.x +0.25*top_bot_values["bot"][i].coordinates.width))
                {
                    blank.coordinates.x = top_bot_values["bot"][i].coordinates.x;
                    blank.coordinates.y = top_bot_values["bot"][i].coordinates.y;
                    blank.coordinates.width = top_bot_values["bot"][i].coordinates.width;
                    blank.coordinates.height = top_bot_values["bot"][i].coordinates.height;
                    if(top_bot_values["bot"][i].score > 0.8)
                    {
                        blank.label = top_bot_values["bot"][i].label;
                        blank.score = top_bot_values["bot"][i].score;
                    }else
                    {
                        blank.label = "blank";
                        blank.score = 0;
                    }
                    plate_value_top.push_back(blank);
                    plate_value_bot.push_back(top_bot_values["bot"][i]);
                }else if(top_bot_values["top"][i].score > top_bot_values["bot"][i].score)
                {
                    plate_value_top.push_back(top_bot_values["top"][i]);
                    top_bot_values["top"][i].label = top_bot_values["top"][i].label.erase(0,1);
                    plate_value_bot.push_back(top_bot_values["top"][i]);
                }else if(top_bot_values["top"][i].score < top_bot_values["bot"][i].score)
                {
                    plate_value_bot.push_back(top_bot_values["bot"][i]);
                    top_bot_values["bot"][i].label = persian_prefix + top_bot_values["bot"][i].label;
                    plate_value_top.push_back(top_bot_values["bot"][i]);
                }
            }

            int max_common_chars = max(plate_value_top.size(), plate_value_bot.size());
            if(max_common_chars == 5)
            {
                //result.PlateType = Type1;
            }else if(max_common_chars == 7)
            {
                //result.PlateType = Type2;
            }else //result.PlateType = Unk;

            result.PlateValue = PersianRead(plate_value_top);
            for(const Annotation &annotation:plate_value_top)
            {
                result.ProbabilityVector.push_back(annotation.score);
            }
            result.PlateValue += "\n";
            result.PlateValue += LatinRead(plate_value_bot);
            for(const Annotation &annotation:plate_value_bot)
            {
                result.ProbabilityVector.push_back(annotation.score);
            }

        }else if(Country == "Iraq")
            //#################################################################
            /*Iraq types:
             * type1 : only Persian characters
             * type2 : only Persian characters with colourful columnwise IRAQ
             *  ------------
             * |I  1 2 3 4  |
             * |R           |
             * |A           |
             * |Q           |
             *  ------------
             * type3 : only Latin characters
             */
        {
            std::vector<Annotation> single_line;
            std::vector<Annotation> edited_single_line;
            std::string annotation_str = "";
            std::string label_str = "";
            std::vector<std::string> digits = ALL_DIGITS;
            std::vector<std::string> letters = ALL_LETTERS;
            std::vector<std::string> latin_digits = LATIN_DIGITS;
            std::vector<std::string> latin_letters = LATIN_LETTERS;
            std::vector<std::string> persian_digits = PERSIAN_DIGITS;
            std::vector<std::string> persian_letters = PERSIAN_LETTERS;
            single_line.insert(single_line.end(), top_bot_values["top"].begin(),top_bot_values["top"].end());
            single_line.insert(single_line.end(), top_bot_values["bot"].begin(),top_bot_values["bot"].end());
            int persian_digit_count = getPersianDigitCount(single_line);
            int persian_letter_count = getPersianLetterCount(single_line);
            int persian_word_count = getPersianWordCount(single_line);
            int latin_digit_count = getLatinDigitCount(single_line);
            int latin_letter_count = getLatinLetterCount(single_line);
            bool IRAQ_colum_flag = hasIRAQ_Permutations(single_line);


            int ySum = 0;
            int hSum = 0;
            int wSum = 0;

            for (const Annotation &annotation : single_line)
            {
                ySum += annotation.coordinates.y;
                hSum += annotation.coordinates.height;
                wSum += annotation.coordinates.width;
            }

            double meanY = static_cast<double>(ySum) / single_line.size();
            double meanH = static_cast<double>(hSum) / single_line.size();

            for(Annotation &annotation:single_line)
            {
                if(((annotation.label == "I" || annotation.label == "R"|| annotation.label == "A"|| annotation.label == "Q") && annotation.coordinates.height < 0.9*meanH)||(annotation.coordinates.height < 0.75*meanH))
                {
                    continue;
                }else
                {
                    edited_single_line.push_back(annotation);
                }
            }

            if((IRAQ_colum_flag && persian_digit_count)||(IRAQ_colum_flag && persian_letter_count)||(IRAQ_colum_flag && persian_word_count))
            {
                //result.PlateType = Type2;
                result.PlateValue = PersianRead(edited_single_line);
                for(const Annotation &annotation:edited_single_line)
                {
                    result.ProbabilityVector.push_back(annotation.score);
                }
            }else if((persian_digit_count>0) && (latin_digit_count==0) && (latin_letter_count==0))
            {
                //result.PlateType = Type1;
                result.PlateValue = PersianRead(edited_single_line);
                for(const Annotation &annotation:edited_single_line)
                {
                    result.ProbabilityVector.push_back(annotation.score);
                }
            }else if((persian_digit_count==0) && (persian_letter_count==0) && (persian_word_count==0) && (latin_digit_count || latin_letter_count))
            {
                //result.PlateType = Type3;
                result.PlateValue = LatinRead(edited_single_line);
                for(const Annotation &annotation:edited_single_line)
                {
                    result.ProbabilityVector.push_back(annotation.score);
                }
            }else
            {
                //result.PlateType = Unk;
                result.PlateValue = PersianRead(edited_single_line);
                result.PlateValue += LatinRead(edited_single_line);
                for(const Annotation &annotation:edited_single_line)
                {
                    result.ProbabilityVector.push_back(annotation.score);
                }
            }

        }else if(Country == "Pakistan")
            //#################################################################
            /*Pakistan types:
             * too many varieties, we ignore this issue at the moment
             * and simplify reading Pakistan's plates by reading from
             * top left to bottom right
             */
        {
            std::vector<Annotation> single_line;
            single_line.insert(single_line.end(), top_bot_values["top"].begin(),top_bot_values["top"].end());
            single_line.insert(single_line.end(), top_bot_values["bot"].begin(),top_bot_values["bot"].end());
            result.PlateValue = LatinRead(single_line);
            //result.PlateType = Type1;
            for(const Annotation &annotation:single_line)
            {
                result.ProbabilityVector.push_back(annotation.score);
            }

        }else if(Country == "SaudiArabia")
            //#################################################################
            /*SaudiArabia types:
             * type1: both types have the same format :
             *   pd pd pd pd  pl pl pl
             *   ld ld ld ld  ll ll ll
             */
        {
            std::vector<Annotation> single_line;
            std::vector<Annotation> edited_single_line;
            std::string annotation_str = "";
            std::string label_str = "";
            std::vector<std::string> digits = ALL_DIGITS;
            std::vector<std::string> letters = ALL_LETTERS;
            std::vector<std::string> latin_digits = LATIN_DIGITS;
            std::vector<std::string> latin_letters = LATIN_LETTERS;
            std::vector<std::string> persian_digits = PERSIAN_DIGITS;
            std::vector<std::string> persian_letters = PERSIAN_LETTERS;
            single_line.insert(single_line.end(), top_bot_values["top"].begin(),top_bot_values["top"].end());
            single_line.insert(single_line.end(), top_bot_values["bot"].begin(),top_bot_values["bot"].end());
            //            single_line = addWhiteSpace(single_line);

            int ySum = 0;
            int hSum = 0;
            int wSum = 0;

            for (const Annotation &annotation : single_line) {
                ySum += annotation.coordinates.y;
                hSum += annotation.coordinates.height;
                wSum += annotation.coordinates.width;
            }

            double meanY = static_cast<double>(ySum) / single_line.size();
            double meanH = static_cast<double>(hSum) / single_line.size();

            for(Annotation &annotation : single_line)
            {
                if((annotation.label == "K" || annotation.label == "S"|| annotation.label == "A") && annotation.coordinates.height < 0.85*meanH)
                {
                    continue;
                }else
                {
                    edited_single_line.push_back(annotation);
                    if(std::find(digits.begin(),digits.end(),annotation.label) != digits.end())
                    {
                        annotation_str += "D";
                    }else if(std::find(letters.begin(),letters.end(),annotation.label) != letters.end())
                    {
                        annotation_str += "A";
                    }else if(annotation.label == "blank") annotation_str += "_";
                }
            }

            for(Annotation &annotation:edited_single_line)
            {
                label_str += annotation.label;
            }

            if(annotation_str.find("DDDDAAADDDDAAA"))
            {
                //result.PlateType = Type1;
            }else //result.PlateType = Unk;
            result.PlateValue = CompoundRead(edited_single_line);
            for(const Annotation &annotation:edited_single_line)
            {
                result.ProbabilityVector.push_back(annotation.score);
            }

        }else if(Country == "UAE")
            //#################################################################
            /*UAE types:
             * type1 : digits, province, digits : Abu Dhabi, Sharjah
             * type2 : letter, province, digits : Ajman, Dubai, Fujairah, Ras Al Khaimah, Umm Al Quwain
             */
        {
            std::vector<Annotation> single_line;
            std::vector<Annotation> letter_fraction;
            std::vector<Annotation> first_digits_fraction;
            std::vector<Annotation> second_digits_fraction;
            std::vector<Annotation> province_fraction;
            std::string annotation_str = "";
            std::string label_str = "";
            std::vector<std::string> digits = LATIN_DIGITS;
            std::vector<std::string> letters = LATIN_LETTERS;
            std::vector<std::string> province = UAE_PROVINCE;
            single_line.insert(single_line.end(), top_bot_values["top"].begin(),top_bot_values["top"].end());
            single_line.insert(single_line.end(), top_bot_values["bot"].begin(),top_bot_values["bot"].end());

            for(int global_i=0 ; global_i<single_line.size(); global_i++)
            {

                if((std::find(digits.begin(),digits.end(),single_line[global_i].label) != digits.end()) && province_fraction.size()<1)
                {
                    first_digits_fraction.push_back(single_line[global_i]);
                }else if(std::find(letters.begin(),letters.end(),single_line[global_i].label) != letters.end())
                {
                    letter_fraction.push_back(single_line[global_i]);
                }else if(std::find(province.begin(),province.end(),single_line[global_i].label) != province.end())
                {
                    province_fraction.push_back(single_line[global_i]);
                }else if((std::find(digits.begin(),digits.end(),single_line[global_i].label) != digits.end()) && province_fraction.size()>0)
                {
                    second_digits_fraction.push_back(single_line[global_i]);
                }
            }

            for(Annotation &annotation:single_line)
            {
                label_str += annotation.label;
            }

            if((first_digits_fraction.size()>0) && (second_digits_fraction.size()>0))
            {
                //result.PlateType = Type1;
            }else if((letter_fraction.size()>0) && (second_digits_fraction.size()>0 || first_digits_fraction.size()>0))
            {
                //result.PlateType = Type2;
            }else //result.PlateType = Unk;

            result.PlateValue = LatinRead(first_digits_fraction);
            for(const Annotation &annotation:first_digits_fraction)
            {
                result.ProbabilityVector.push_back(annotation.score);
            }
            result.PlateValue += "\n";
            result.PlateValue += LatinRead(letter_fraction);
            for(const Annotation &annotation:letter_fraction)
            {
                result.ProbabilityVector.push_back(annotation.score);
            }
            result.PlateValue += "\n";
            (province_fraction.size() > 0) ? (result.PlateValue += province_fraction[0].label) : (result.PlateValue += "");
//            result.PlateValue += province_fraction[0].label;
            for(const Annotation &annotation:province_fraction)
            {
                result.ProbabilityVector.push_back(annotation.score);
            }
            result.PlateValue += "\n";
            result.PlateValue += LatinRead(second_digits_fraction);
            for(const Annotation &annotation:second_digits_fraction)
            {
                result.ProbabilityVector.push_back(annotation.score);
            }
            // erasing useless std::endl
            if(result.PlateValue[0] == '\n') result.PlateValue.erase(0,1);
            for(int i=1; i<result.PlateValue.length();i++)
            {
                if(result.PlateValue[i] == '\n')
                {
                    if(result.PlateValue[i-1] == '\n')
                    {
                        result.PlateValue.erase(i,1);
                    }
                }
            }


        }else if(Country == "Tajikistan")
            //#################################################################
            /*Tajikistan types:
             * type1 : NNN(N) LL NN
             * type2 : L(L) NNNN NN   1996 series
             */
        {
            std::vector<Annotation> single_line;
            std::vector<Annotation> edited_single_line;
            single_line.insert(single_line.end(), top_bot_values["top"].begin(),top_bot_values["top"].end());
            single_line.insert(single_line.end(), top_bot_values["bot"].begin(),top_bot_values["bot"].end());

            int ySum = 0;
            int hSum = 0;
            int wSum = 0;

            for (const Annotation &annotation : single_line) {
                ySum += annotation.coordinates.y;
                hSum += annotation.coordinates.height;
                wSum += annotation.coordinates.width;
            }

            double meanY = static_cast<double>(ySum) / single_line.size();
            double meanH = static_cast<double>(hSum) / single_line.size();

            std::string annotation_str = "";
            std::vector<std::string> digits = LATIN_DIGITS;
            std::vector<std::string> letters = LATIN_LETTERS;
            for(Annotation &annotation : single_line)
            {
                if((annotation.label == "T" | annotation.label == "J") && annotation.coordinates.height < 0.7*meanH)
                {
                    continue;
                }else
                {
                    edited_single_line.push_back(annotation);
                    if(std::find(digits.begin(),digits.end(),annotation.label) != digits.end())
                    {
                        annotation_str += "D";
                    }else if(std::find(letters.begin(),letters.end(),annotation.label) != letters.end())
                    {
                        annotation_str += "A";
                    }
                }
            }

            if((annotation_str == "DDDAADD") || (annotation_str == "DDDDAADD"))
            {
                //result.PlateType = Type1;
                result.PlateValue = LatinRead(edited_single_line);
                for(const Annotation &annotation:edited_single_line)
                {
                    result.ProbabilityVector.push_back(annotation.score);
                }
            }else if((annotation_str == "AADDDDDD") || (annotation_str == "ADDDDDD"))
            {
                //result.PlateType = Type2;
                result.PlateValue = LatinRead(edited_single_line);
                for(const Annotation &annotation:edited_single_line)
                {
                    result.ProbabilityVector.push_back(annotation.score);
                }
            }else ////result.PlateType = Unk;
            {
                //result.PlateType = Unk;
                result.PlateValue = LatinRead(edited_single_line);
                for(const Annotation &annotation:edited_single_line)
                {
                    result.ProbabilityVector.push_back(annotation.score);
                }
            }
        }

    }
}

GlobalPlateRecognition::GlobalPlateRecognition()
{

}



std::pair<std::string, float> GlobalPlateRecognition::findCountryByDetections(std::vector<Annotation> rotated_annotations)
{
    std::string country {"None"};
    float score = 0;

    auto top_bots  = top_bot(rotated_annotations);
    top_bots["top"] = addWhiteSpace(top_bots["top"]);
    top_bots["bot"] = addWhiteSpace(top_bots["bot"]);
    double Iran_score = estimateIranScore(top_bots);
    double Freezone_score = estimateFreezoneScore(top_bots);
    double IranTransit_score = estimateIranTransitScore(top_bots);
    double UAE_score = estimateUAEScore(top_bots);
    double Armenia_score = estimateArmeniaScore(top_bots);
    double Azerbaijan_score = estimateAzerbaijanScore(top_bots);
    double Turkey_score = estimateTurkeyScore(top_bots);
    double Germany_score = estimateGermanyScore(top_bots);
    double Commercial_score = estimateCommercialScore(top_bots);
    double Turkmenistan_score = estimateTurkmenistanScore(top_bots);
    double Saudi_score = estimateSaudiArabiaScore(top_bots);
    double Iraq_score = estimateIraqScore(top_bots);
    double Afghanistan_score = estimateAfghanistanScore(top_bots);
    double Russia_score = estimateRussiaScore(top_bots);
    auto country_codes = possibleCountryCode(rotated_annotations);
    bool has_a = false;
    bool has_m = false;
    bool has_z = false;
    bool has_t = false;
    bool has_r = false;
    bool has_u = false;
    bool has_s = false;
    for(auto &m:country_codes["codes"])
    {
        if(m.label=="A")
            has_a = true;
        if(m.label=="M")
            has_m = true;
        if(m.label=="Z")
            has_z = true;
        if(m.label=="T")
            has_t = true;
        if(m.label=="R")
            has_r = true;
        if(m.label=="U")
            has_u = true;
        if(m.label=="S")
            has_s = true;
    }
    if(has_a & has_m)
    {
        IranTransit_score = 0;
        Azerbaijan_score = 0;
        Turkey_score = 0;
        Germany_score = 0;
    }

    if(has_t & has_r)
    {
        IranTransit_score = 0;
        Azerbaijan_score = 0;
        Armenia_score = 0;
        Germany_score = 0;
    }
    if(has_a & has_z)
    {
        IranTransit_score = 0;
        Armenia_score = 0;
        Turkey_score = 0;
        Germany_score = 0;
    }

    if(has_t & has_m)
    {
        IranTransit_score = 0;
        Armenia_score = 0;
        Turkey_score = 0;
        Azerbaijan_score = 0;
        Germany_score = 0;
    }
    if(has_a | has_m | has_r | has_t | has_z | has_u | has_s)
    {
        IranTransit_score = 0;

    }


    if(Iran_score>0.5)
    {

        country = "Iran";
        score = Iran_score;

    }else
    {
        if(Afghanistan_score>0.5)
        {
            country = "Afghanistan";
            score = Afghanistan_score;
        }else
        {
            if(Freezone_score>0.5)
            {
                country = "IranFreeZone";
                score = Freezone_score;
            }
            else
            {
                if(Russia_score>0.5)
                {
                    country = "Russia";
                    score = Russia_score;
                }
                else
                {
                    if(IranTransit_score>0.5)
                    {
                        country = "IranTransit";
                        score = IranTransit_score;
                    }else
                    {
                        if(Germany_score>0.5)
                        {
                            if(Turkmenistan_score>0.5)
                            {
                                country = "Turkmenistan";
                                score = Turkmenistan_score;
                            }else
                            {
                                country = "Germany";
                                score = Germany_score;
                            }
                        }else
                        {

                            if(Turkey_score>0.5)
                            {
                                country = "Turkey";
                                score = Turkey_score;
                            }else
                            {
                                if(Armenia_score>0.5)
                                {
                                    country = "Armenia";
                                    score = Armenia_score;
                                }else
                                {
                                    if(Azerbaijan_score>0.5)
                                    {
                                        country = "Azerbaijan";
                                        score = Azerbaijan_score;
                                    }else
                                    {
                                        if(Turkmenistan_score>0.5)
                                        {
                                            country = "Turkmenistan";
                                            score = Turkmenistan_score;
                                        }else
                                        {
                                            if(Iraq_score>0.5)
                                            {
                                                country = "Iraq";
                                                score = Iraq_score;
                                            }else
                                            {
                                                if(UAE_score>0.5)
                                                {
                                                    country = "UAE";
                                                    score = UAE_score;
                                                }else
                                                {
                                                    if(Saudi_score>0.5)
                                                    {
                                                        country = "SaudiArabia";
                                                        score = UAE_score;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }

                    }
                }
            }
        }

    }
    return std::make_pair(country, score);
}


int findIndex(std::string query,std::vector<std::string> values)
{
    int index = -1;
    for(int i=0;i<values.size();i++)
    {
        if(values[i]==query)
        {
            index = i;
            break;
        }
    }
    return index;
}

string GlobalPlateRecognition::PersianRead(std::vector<Annotation> annotations)
{
    std::string result{""};
    std::vector<std::string> letters = PERSIAN_LETTERS;
    std::vector<std::string> digits = PERSIAN_DIGITS;
    for (const Annotation &annotation : annotations) {
        if (std::find(digits.begin(), digits.end(), annotation.label) != digits.end())
        {
            auto idx = findIndex(annotation.label,PERSIAN_DIGITS);
            if(idx>-1)
                result += std::to_string(idx);
            else
                result += "D";

        } else
        {
            if (std::find(letters.begin(), letters.end(), annotation.label) != letters.end())
            {
                auto idx = findIndex(annotation.label,PERSIAN_LETTERS);
                if(idx>-1)
                {
                    stringstream ss;
                    ss << std::setfill('0') << std::setw(2) << idx+1;
                    result += ss.str();
                }
                else
                    result += "AA";
            }else
            {
                if(annotation.label == "blank")
                {
                    result += "_";
                }
            }
        }
    }
    return result;

}

string GlobalPlateRecognition::LatinRead(std::vector<Annotation> annotations)
{
    std::string result{""};
    std::vector<std::string> letters = LATIN_LETTERS;
    std::vector<std::string> digits = LATIN_DIGITS;
    for (const Annotation &annotation : annotations) {
        if (std::find(digits.begin(), digits.end(), annotation.label) != digits.end())
        {
            result += annotation.label;

        } else
        {
            if (std::find(letters.begin(), letters.end(), annotation.label) != letters.end())
            {

                result += annotation.label;
            } else
            {
                if(annotation.label == "blank")
                {
                    result += "_";
                }
            }
        }
    }
    return result;
}

string GlobalPlateRecognition::CompoundRead(std::vector<Annotation> annotations)
{
    std::string result{""};
    std::vector<std::string> letters = ALL_LETTERS;
    std::vector<std::string> digits = ALL_DIGITS;
    for (const Annotation &annotation : annotations) {
        if (std::find(digits.begin(), digits.end(), annotation.label) != digits.end())
        {
            result += annotation.label;

        } else
        {
            if (std::find(letters.begin(), letters.end(), annotation.label) != letters.end())
            {

                result += annotation.label;
            } else
            {
                if(annotation.label == "blank")
                {
                    result += "_";
                }
            }
        }
    }
    return result;
}

bool gocr::GlobalPlateRecognition::hasUAEprovince(std::vector<Annotation> &annotations)
{
    bool UAEprovinceFlag = false;

    std::vector<std::string> provinces = UAE_PROVINCE;

    for (const Annotation &annotation : annotations)
    {
        if (std::find(provinces.begin(), provinces.end(), annotation.label) != provinces.end())
        {
            UAEprovinceFlag = true;
        }
    }

    return UAEprovinceFlag;

}

std::map<std::string, std::vector<gocr::Annotation> > gocr::GlobalPlateRecognition::possibleCountryCode(std::vector<Annotation> &annotations)
{
    std::vector<Annotation> possibleCountryCode;
    std::vector<Annotation> chars;
    std::vector<Annotation> botChars;
    std::vector<Annotation> topChars;
    bool UAEprovinceFlag = false;
    std::vector<Annotation> newAnnotations;

    std::sort(annotations.begin(), annotations.end(), sortAnnotationsByX);

    std::vector<std::string> provinces = UAE_PROVINCE;

    for (const Annotation &annotation : annotations) {
        if (std::find(provinces.begin(), provinces.end(), annotation.label) != provinces.end())
        {
            UAEprovinceFlag = true;
            newAnnotations.push_back(annotation);
        } else
        {
            newAnnotations.push_back(annotation);
        }
    }

    int ySum = 0;
    int hSum = 0;
    int wSum = 0;

    for (const Annotation &annotation : newAnnotations) {
        ySum += annotation.coordinates.y;
        hSum += annotation.coordinates.height;
        wSum += annotation.coordinates.width;
    }

    double meanY = static_cast<double>(ySum) / newAnnotations.size();
    double meanH = static_cast<double>(hSum) / newAnnotations.size();

    for (const Annotation &annotation : newAnnotations)
    {
        if ((annotation.coordinates.height < 0.7 * meanH)&&(annotation.label != "f0"))
        {
            possibleCountryCode.push_back(annotation);

        }else
        {
            chars.push_back(annotation);
        }

    }

    std::map<std::string, std::vector<Annotation>> result;
    result["chars"] = chars;
    result["codes"] = possibleCountryCode;

    return result;
}

std::map<std::string, std::vector<gocr::Annotation> > gocr::GlobalPlateRecognition::top_bot(std::vector<Annotation> &annotations)
{
    bool doubleLineFlag = false;
    std::vector<Annotation> bot_chars;
    std::vector<Annotation> top_chars;
    std::vector<Annotation> possibleCountryCode;
    std::sort(annotations.begin(), annotations.end(), sortAnnotationsByX);

    float representetive_h = 0;
    float representetive_w = 0;
    float representetive_y = 0;
    float representetive_x = 0;

    std::vector<position_rank> votes(annotations.size());
    double h_thr = 0.7;

    int first_for_index = 0;
    for (const Annotation &annotation : annotations)
    {
        //        Annotation first_for_annot = annotation;
        std::vector<Annotation> window_0;
        std::vector<Annotation> window_1;
        std::vector<Annotation> window_2;
        representetive_x = annotation.coordinates.x;
        representetive_y = annotation.coordinates.y;
        representetive_w = annotation.coordinates.width;
        representetive_h = annotation.coordinates.height;
        for (const Annotation &annot : annotations)
        {
            Annotation second_for_annot = annot;
            if ((annot.coordinates.y < (representetive_y + h_thr *(std::max( (representetive_h/annot.coordinates.height), (annot.coordinates.height/representetive_h) ))* representetive_h*0.5 )) && (annot.coordinates.y > (representetive_y - h_thr *(std::max( (representetive_h/annot.coordinates.height), (annot.coordinates.height/representetive_h) ))* representetive_h*0.5))){
                window_1.push_back(annot);
            }
            else if ((annot.coordinates.y > (representetive_y + h_thr *(std::max( (representetive_h/annot.coordinates.height), (annot.coordinates.height/representetive_h) ))* representetive_h*0.5)))
            {
                window_2.push_back(annot);
            }
            else
            {
                window_0.push_back(annot);
            }

        }

        if(window_0.empty())
        {
            //            for(const Annotation &annot: window_1)
            for(const Annotation &annot: window_2)
            {
                votes[first_for_index].top += 1;
            }
            //            for(const Annotation &annot: window_2)
            //            {
            //                votes[first_for_index].bot += 1;
            //            }
        }
        else{
            //            for(const Annotation &annot: window_0)
            //            {
            //                votes[first_for_index].top += 1;
            //            }
            //            for(const Annotation &annot: window_1)
            for(const Annotation &annot: window_0)
            {
                votes[first_for_index].bot += 1;
            }
        }

        first_for_index += 1;
    }

    for(int i=0 ; i < votes.size(); i++)
    {
        if(votes[i].top >= votes[i].bot)
        {
            top_chars.push_back(annotations[i]);
        }else
        {
            bot_chars.push_back(annotations[i]);
        }
    }

    if ((bot_chars.size()) && (top_chars.size()))
    {
        doubleLineFlag = true;
    }
    else
    {
        doubleLineFlag = false;
    }
    //    topChars.insert(topChars.end(), botChars.begin(), botChars.end());
    std::map<std::string, std::vector<Annotation>> result;
    result["top"] = top_chars;
    result["bot"] = bot_chars;

    return result;
}

std::vector<gocr::Annotation> gocr::GlobalPlateRecognition::rotateAnnotations(std::vector<Annotation> &annotations)
{
    int annotations_len = annotations.size();
    std::vector<Annotation> rotated_annotations(annotations_len);
    std::vector<Annotation> single_line;
    auto top_bots = top_bot(annotations);
    auto top_chars  = top_bots["top"];
    auto bot_chars  = top_bots["bot"];
    double theta = 0;
    double top_theta = 0;
    double bot_theta = 0;
    double top_estimated = 0;
    double bot_estimated = 0;
    double num_estimated = 0;
    float hbar = 0;
    for(auto &annot:annotations)
    {
        hbar += annot.coordinates.height;
    }
    hbar = hbar/(annotations.size()+1e-3);
    for(int i=0; i < top_chars.size(); i++)
    {
        for(int j=i+1;j<top_chars.size();j++)
        {
            double numerator = top_chars[i].coordinates.y - top_chars[j].coordinates.y;
            if(abs(numerator)>hbar)
                continue;
            double denominator = top_chars[j].coordinates.x - top_chars[i].coordinates.x;
            denominator += 1e-10;
            top_theta += atan(numerator/denominator) * 180/(M_PI);
            top_estimated ++;
        }
    }
    top_theta /= top_estimated;


    for(int i=0; i < bot_chars.size(); i++)
    {
        for(int j=i+1;j<bot_chars.size();j++)
        {
            double numerator = bot_chars[i].coordinates.y - bot_chars[j].coordinates.y;
            if(abs(numerator)>hbar)
                continue;
            double denominator = bot_chars[j].coordinates.x - bot_chars[i].coordinates.x;
            denominator += 1e-10;
            bot_theta += atan(numerator/denominator) * 180/(M_PI);
            bot_estimated ++;
        }
    }
    bot_theta /= bot_estimated;
    num_estimated = top_estimated + bot_estimated;
    theta = (bot_theta*bot_estimated+top_theta*top_estimated)/(num_estimated);

    if (theta <=1 && theta >=-1)
    {
        single_line.insert(single_line.end(), top_chars.begin(),top_chars.end());
        single_line.insert(single_line.end(), bot_chars.begin(),bot_chars.end());
        return single_line;
    }
    else
    {
        double a1 = cos(theta * (M_PI/180));
        double a2 = -sin(theta * (M_PI/180));
        double b1 = sin(theta * (M_PI/180));
        double b2 = cos(theta * (M_PI/180));
        int index = 0;
        for (const Annotation &annotation : annotations)
        {
            rotated_annotations[index].coordinates.x = a1 * annotation.coordinates.x + a2 * annotation.coordinates.y;
            rotated_annotations[index].coordinates.y = b1 * annotation.coordinates.x + b2 * annotation.coordinates.y;
            rotated_annotations[index].coordinates.width = annotation.coordinates.width;
            rotated_annotations[index].coordinates.height = annotation.coordinates.height;
            rotated_annotations[index].label = annotation.label;
            rotated_annotations[index].score = annotation.score;
            index += 1;
        }
    }
    return rotated_annotations;
}

int gocr::GlobalPlateRecognition::getPersianDigitCount(const std::vector<Annotation> &annotations)
{
    int digitCount = 0;
    std::vector<std::string> digits = PERSIAN_DIGITS;

    for (const Annotation &annotation : annotations) {
        if (std::find(digits.begin(), digits.end(), annotation.label) != digits.end()) {
            digitCount++;
        }
    }
    return digitCount;
}

int gocr::GlobalPlateRecognition::getPersianLetterCount(const std::vector<Annotation> &annotations) {

    int letterCount = 0;
    std::vector<std::string> words = PERSIAN_LETTERS;

    for (const Annotation &annotation : annotations) {
        if (std::find(words.begin(), words.end(), annotation.label) != words.end()) {
            letterCount++;
        }
    }

    return letterCount;
}

int gocr::GlobalPlateRecognition::getPersianWordCount(const std::vector<Annotation> &annotations) {
    int wordCount = 0;
    std::vector<std::string> words = IRQ_PROVINCE;

    for (const Annotation &annotation : annotations) {
        if (std::find(words.begin(), words.end(), annotation.label) != words.end()) {
            wordCount++;
        }
    }

    return wordCount;
}

int gocr::GlobalPlateRecognition::getLatinDigitCount(const std::vector<Annotation> &annotations) {
    int digitCount = 0;
    std::vector<std::string> digits = LATIN_DIGITS;

    for (const Annotation &annotation : annotations) {
        if (std::find(digits.begin(), digits.end(), annotation.label) != digits.end()) {
            digitCount++;
        }
    }

    return digitCount;
}

int gocr::GlobalPlateRecognition::getLatinLetterCount(const std::vector<Annotation> &annotations) {
    int letterCount = 0;
    std::vector<std::string> letters = LATIN_LETTERS;

    for (const Annotation &annotation : annotations) {
        if (std::find(letters.begin(), letters.end(), annotation.label) != letters.end()) {
            letterCount++;
        }
    }

    return letterCount;
}

bool gocr::GlobalPlateRecognition::hasTRsequence(const std::vector<Annotation> &annotations)
{
    double plate_width = 0;
    for (const Annotation &annotation : annotations)
    {
        plate_width += annotation.coordinates.x;
    }
    plate_width *= 7;

    for (const Annotation &annotation : annotations)
    {
        if ((annotation.label == "T" || annotation.label == "R") && annotation.coordinates.x < (plate_width/3))
        {
            return true;
        }
    }
    return false;
}

bool gocr::GlobalPlateRecognition::whiteSpace(std::vector<Annotation> &annotations)
{
    bool whiteSpaceFlag = false;
    if(annotations.size()<2)
        return whiteSpaceFlag;

    std::sort(annotations.begin(), annotations.end(), sortAnnotationsByX);

    int ySum = 0;
    int hSum = 0;
    int wSum = 0;

    for (const Annotation &annotation : annotations) {
        ySum += annotation.coordinates.y;
        hSum += annotation.coordinates.height;
        wSum += annotation.coordinates.width;
    }

    double meanY = static_cast<double>(ySum) / annotations.size();
    double meanH = static_cast<double>(hSum) / annotations.size();
    double meanW = static_cast<double>(wSum) / annotations.size();

    for (int i=0; i< annotations.size() -1 ; i++) {
        if (annotations[i+1].coordinates.x - annotations[i].coordinates.x > 1.1*meanW) {
            whiteSpaceFlag = true;
        } else {
            continue;
        }
    }

    return whiteSpaceFlag;
}

int gocr::GlobalPlateRecognition::countSubstring(const std::string &str, const std::string &sub)
{
    int count = 0;
    std::string::size_type pos = 0;
    while ((pos = str.find(sub, pos)) != std::string::npos) {
        ++count;
        pos += sub.length();
    }
    return count;
}

std::string gocr::GlobalPlateRecognition::getPersianStringSequence(std::vector<Annotation> &annotations)
{

    std::string ann_str = "";
    std::string _A = "A";
    std::string _D = "D";
    std::vector<std::string> letters = PERSIAN_LETTERS;
    std::vector<std::string> digits = PERSIAN_DIGITS;
    for (const Annotation &annotation : annotations) {
        if (std::find(digits.begin(), digits.end(), annotation.label) != digits.end())
        {
            ann_str += "D";
        } else
        {
            if (std::find(letters.begin(), letters.end(), annotation.label) != letters.end())
            {
                ann_str += "A";
            }else
            {
                if(annotation.label == "blank") ann_str += "B";
            }
        }
    }
    return ann_str;
}

std::string gocr::GlobalPlateRecognition::getLatinStringSequence(std::vector<Annotation> &annotations)
{
    std::string ann_str = "";
    std::string _A = "A";
    std::string _D = "D";
    std::vector<std::string> letters = LATIN_LETTERS;
    std::vector<std::string> digits = LATIN_DIGITS;
    for (const Annotation &annotation : annotations) {
        if (std::find(digits.begin(), digits.end(), annotation.label) != digits.end())
        {
            ann_str += "D";
        } else
        {
            if (std::find(letters.begin(), letters.end(), annotation.label) != letters.end())
            {
                ann_str += "A";
            }else
            {
                if(annotation.label == "blank") ann_str += "B";
            }
        }
    }

    return ann_str;
}

bool gocr::GlobalPlateRecognition::hasIRAQ_Permutations(std::vector<Annotation> &codes)
{
    bool has_iraq = false;
    if(codes.size()<2)
        return has_iraq;
    int first_c = -1;
    for(int i=0;i<codes.size();i++)
    {
        if(codes[i].label=="I" | codes[i].label=="R" | codes[i].label=="A" | codes[i].label=="Q" )
        {
            first_c = i;
            break;
        }
    }
    if(first_c<0)
        return has_iraq;
    for(int i=1;i<codes.size();i++)
    {
        float d = codes[i].coordinates.x - codes[first_c].coordinates.x;
        if(abs(d) < codes[first_c].coordinates.width)
        {
            has_iraq = true;
            break;
        }
    }
    return has_iraq;
}

bool gocr::GlobalPlateRecognition::hasIranTransitFlag(std::vector<Annotation> &singleLine){
    for (const Annotation &annotation : singleLine)
    {
        if ((annotation.label) == "IR_Trans")
        {
            return true;
        } else continue;
    }
    return false;
}

bool gocr::GlobalPlateRecognition::hasIRQ_ProvinceFlag(std::vector<Annotation> &annotations){
    std::vector<std::string> IRQ = IRQ_PROVINCE;
    for (const Annotation &annotation : annotations)
    {
        if (std::find(IRQ.begin(), IRQ.end(), annotation.label) != IRQ.end())
        {
            return true;
        }
    }
    return false;
}

double gocr::GlobalPlateRecognition::estimateUAEScore(std::map<std::string, std::vector<Annotation> > &top_bots) {


    auto bot_chars = top_bots["bot"];
    auto top_chars = top_bots["top"];
    std::vector<Annotation> single_line;
    single_line.insert(single_line.end(), top_chars.begin(), top_chars.end());
    single_line.insert(single_line.end(), bot_chars.begin(), bot_chars.end());

    int uae_flag = hasUAEprovince(single_line);
    auto latin_sequence = getLatinStringSequence(single_line);
    int _DDD_latin = findSequence(latin_sequence,"DDD");
    int _DDDD_latin = findSequence(latin_sequence,"DDDD");


    double score = 0;
    if (_DDD_latin>0)
    {
        score+= 1.0;
    }
    if (uae_flag>0)
    {
        score+= 2.0;
    }
    if (_DDDD_latin>0)
    {
        score+= 1.0;
    }

    score /= 4;
    return score;
}

double gocr::GlobalPlateRecognition::estimateArmeniaScore(std::map<std::string, std::vector<Annotation> > &top_bots)
{


    auto bot_chars = top_bots["bot"];
    auto top_chars = top_bots["top"];
    std::vector<Annotation> single_line;
    single_line.insert(single_line.end(), top_chars.begin(), top_chars.end());
    single_line.insert(single_line.end(), bot_chars.begin(), bot_chars.end());
    auto latin_sequence = getLatinStringSequence(possibleCountryCode(top_chars)["chars"]);
    int latin_digit_count = getLatinDigitCount(single_line);
    auto country_codes = possibleCountryCode(single_line);
    bool has_a = false;
    bool has_m = false;
    bool has_z = false;
    for(auto &m:country_codes["codes"])
    {
        if(m.label=="A")
            has_a = true;
        if(m.label=="M")
            has_m = true;
        if(m.label=="Z")
            has_z = true;
    }


    double score = 0;
    if (has_a)
    {
        score += 1.0;
    }
    if (has_m)
    {
        score+= 1.0;
    }
    if (has_a & has_m)
    {
        score+= 1.0;
    }
    if(has_z)
        score -= 2.0;

    if(findSequence(latin_sequence,"DDAADD"))
        score += 1;
    if(findSequence(latin_sequence,"DDDAD"))
        score += 1;


    score /= 2;
    return score;
}

double gocr::GlobalPlateRecognition::estimateAzerbaijanScore(std::map<std::string, std::vector<Annotation> > &top_bots)
{


    auto bot_chars = top_bots["bot"];
    auto top_chars = top_bots["top"];
    std::vector<Annotation> single_line;
    single_line.insert(single_line.end(), top_chars.begin(), top_chars.end());
    single_line.insert(single_line.end(), bot_chars.begin(), bot_chars.end());
    auto latin_sequence = getLatinStringSequence(possibleCountryCode(top_chars)["chars"]);
    int latin_digit_count = getLatinDigitCount(single_line);
    auto country_codes = possibleCountryCode(single_line);
    bool has_a = false;
    bool has_m = false;
    bool has_z = false;
    for(auto &m:country_codes["codes"])
    {
        if(m.label=="A")
            has_a = true;
        if(m.label=="M")
            has_m = true;
        if(m.label=="Z")
            has_z = true;
    }



    double score = 0;
    if (has_a)
    {
        score += 1.0;
    }
    if (has_z)
    {
        score+= 1.0;
    }
    if(has_a & has_z)
    {
        score+= 1.0;
    }
    if(findSequence(latin_sequence,"DDAADDD"))
        score += 1;
    if(findSequence(latin_sequence,"DDDADDD"))
        score += 1;

    score /= 2;
    return score;
}

double gocr::GlobalPlateRecognition::estimateTurkeyScore(std::map<std::string, std::vector<Annotation> > &top_bots)
{


    auto bot_chars = top_bots["bot"];
    auto top_chars = top_bots["top"];
    std::vector<Annotation> single_line;

    single_line.insert(single_line.end(), top_chars.begin(), top_chars.end());
    single_line.insert(single_line.end(), bot_chars.begin(), bot_chars.end());
    auto country_codes = possibleCountryCode(single_line);
    auto latin_sequence = getLatinStringSequence(possibleCountryCode(top_chars)["chars"]);

    int latin_digit_count = getLatinDigitCount(single_line);
    bool has_a = false;
    bool has_m = false;
    bool has_z = false;
    bool has_t = false;
    bool has_r = false;
    for(auto &m:country_codes["codes"])
    {
        if(m.label=="A")
            has_a = true;
        if(m.label=="M")
            has_m = true;
        if(m.label=="Z")
            has_z = true;
        if(m.label=="T")
            has_t = true;
        if(m.label=="R")
            has_r = true;
    }



    double score = 0;
    if (has_t)
    {
        score += 1.0;
    }
    if (has_r)
    {
        score+= 1.0;
    }
    if (has_t & has_r)
    {
        score+= 1.0;
    }
    if(has_a | has_m)
        score -= 2;

    if(findSequence(latin_sequence,"DDAADDD"))
        score += 1.5;
    if(findSequence(latin_sequence,"DDAAADDD"))
        score += 1.3;
    if(findSequence(latin_sequence,"DDAAADD"))
        score += 1.2;
    if(findSequence(latin_sequence,"DDAADDDD"))
        score += 1.2;
    if(findSequence(latin_sequence,"DDAAADD"))
        score += 1;
    if(findSequence(latin_sequence,"DDADDDD"))
        score += 1;
    if(findSequence(latin_sequence,"DDDAAADDD"))
        score += 1;
    if(findSequence(latin_sequence,"DDDAADDD"))
        score += 1;


    score /= 2;
    return score;
}

double gocr::GlobalPlateRecognition::estimateGermanyScore(std::map<std::string, std::vector<Annotation> > &top_bots)
{

    auto bot_chars = top_bots["bot"];
    auto top_chars = top_bots["top"];
    std::vector<Annotation> single_line;

    single_line.insert(single_line.end(), top_chars.begin(), top_chars.end());
    single_line.insert(single_line.end(), bot_chars.begin(), bot_chars.end());
    auto country_codes = possibleCountryCode(single_line);
    auto latin_sequence = getLatinStringSequence(possibleCountryCode(top_chars)["chars"]);
    int latin_digit_count = getLatinDigitCount(single_line);

    double score = 0;

    if(findSequence(latin_sequence,"AAAADDD"))
        score += 1;
    if(findSequence(latin_sequence,"AAADDDD"))
        score += 1;
    if(findSequence(latin_sequence,"AAAAADDD"))
        score += 1;

    if(findSequence(latin_sequence,"AAADD"))
        score += 1;
    if(findSequence(latin_sequence,"AADD") && latin_sequence[0]=='A' && latin_sequence[1]=='A')
        score += 2;

    score /= 3;
    return score;
}

double gocr::GlobalPlateRecognition::estimateCommercialScore(std::map<std::string, std::vector<Annotation> > &top_bots)
{
    auto bot_chars = top_bots["bot"];
    auto top_chars = top_bots["top"];
    std::vector<Annotation> single_line;

    single_line.insert(single_line.end(), top_chars.begin(), top_chars.end());
    single_line.insert(single_line.end(), bot_chars.begin(), bot_chars.end());
    auto country_codes = possibleCountryCode(single_line);
    auto latin_sequence = getLatinStringSequence(country_codes["chars"]);
    int latin_digit_count = getLatinDigitCount(single_line);




    double score = 0;


    if(findSequence(latin_sequence,"ADDDDAA"))
        score += 1;

    if(findSequence(latin_sequence,"ADDDDAAA"))
        score += 1;
    if(findSequence(latin_sequence,"ADDDDDA") && latin_sequence[0]=='A')
        score += 1;

    return score;
}

double gocr::GlobalPlateRecognition::estimateRussiaScore(std::map<std::string, std::vector<Annotation> > &top_bots)
{
    auto bot_chars = top_bots["bot"];
    auto top_chars = top_bots["top"];
    std::vector<Annotation> single_line;

    single_line.insert(single_line.end(), top_chars.begin(), top_chars.end());
    single_line.insert(single_line.end(), bot_chars.begin(), bot_chars.end());
    auto country_codes = possibleCountryCode(single_line);
    auto latin_sequence = getLatinStringSequence(country_codes["chars"]);

    bool has_s = false;
    bool has_u = false;
    bool has_r = false;
    bool has_a = false;
    bool has_m = false;
    bool has_z = false;
    bool has_t = false;

    for(auto &m:country_codes["codes"])
    {
        if(m.label=="R")
            has_r = true;
        if(m.label=="U")
            has_u = true;
        if(m.label=="S")
            has_s = true;
        if(m.label=="A")
            has_a = true;
        if(m.label=="M")
            has_m = true;
        if(m.label=="Z")
            has_z = true;
        if(m.label=="T")
            has_t = true;
        if(m.label=="R")
            has_r = true;
    }



    double score = 0;
    if(has_u)
        score += 1;
    if(has_s)
        score += 1;
    if(has_r & (has_u|has_s))
        score += 1;
    if(has_t | has_a | has_m | has_z)
        score -= 2;
    if(findSequence(latin_sequence,"ADDDAADD"))
        score += 1.1;
    if(findSequence(latin_sequence,"ADDDAADDD"))
        score += 1.1;


    score /= 2;
    return score;
}

double gocr::GlobalPlateRecognition::estimateTurkmenistanScore(std::map<std::string, std::vector<Annotation> > &top_bots)
{
    auto bot_chars = top_bots["bot"];
    auto top_chars = top_bots["top"];
    std::vector<Annotation> single_line;

    single_line.insert(single_line.end(), top_chars.begin(), top_chars.end());
    single_line.insert(single_line.end(), bot_chars.begin(), bot_chars.end());
    auto country_codes = possibleCountryCode(single_line);
    auto latin_sequence = getLatinStringSequence(country_codes["chars"]);
    int latin_digit_count = getLatinDigitCount(single_line);
    bool has_a = false;
    bool has_m = false;
    bool has_z = false;
    bool has_t = false;
    bool has_r = false;
    for(auto &m:country_codes["codes"])
    {
        if(m.label=="A")
            has_a = true;
        if(m.label=="M")
            has_m = true;
        if(m.label=="Z")
            has_z = true;
        if(m.label=="T")
            has_t = true;
        if(m.label=="R")
            has_r = true;
    }



    double score = 0;

    if(has_t)
        score += 1;
    if(has_m)
        score += 1;
    if(has_r)
        score -= 1;
    if(has_t & has_r)
        score -= 1;

    if(findSequence(latin_sequence,"AADDDDAA"))
        score += 1;

    //    if(findSequence(latin_sequence,"AAADD"))
    //        score += 1;
    //    if(findSequence(latin_sequence,"AADD") & latin_sequence[0]=='A')
    //        score += 2;

    score /= 1;
    return score;
}

double gocr::GlobalPlateRecognition::estimateSaudiArabiaScore(std::map<std::string, std::vector<Annotation> > &top_bots)
{
    auto bot_chars = top_bots["bot"];
    auto top_chars = top_bots["top"];
    std::vector<Annotation> single_line;

    single_line.insert(single_line.end(), top_chars.begin(), top_chars.end());
    single_line.insert(single_line.end(), bot_chars.begin(), bot_chars.end());
    auto country_codes = possibleCountryCode(single_line);
    auto latin_sequence = getLatinStringSequence(possibleCountryCode(bot_chars)["chars"]);
    int latin_digit_count = getLatinDigitCount(single_line);
    bool has_sa_flag = false;
    for(auto &m:country_codes["codes"])
    {
        if(m.label=="SA_flag")
            has_sa_flag = true;
    }



    double score = 0;
    if(has_sa_flag)
        score += 1;
    if(findSequence(latin_sequence,"DDDAAA"))
        score += 1;

    if(findSequence(latin_sequence,"DDDBAAA"))
        score += 1;
    if(findSequence(latin_sequence,"DDDDAA"))
        score += 1;

    //    if(findSequence(latin_sequence,"AAADD"))
    //        score += 1;
    //    if(findSequence(latin_sequence,"AADD") & latin_sequence[0]=='A')
    //        score += 2;

    score /= 1;
    return score;
}

double gocr::GlobalPlateRecognition::estimateIranScore(std::map<std::string, std::vector<Annotation> > &top_bots)
{

    auto bot_chars = top_bots["bot"];
    auto top_chars = top_bots["top"];
    std::vector<Annotation> single_line;

    single_line.insert(single_line.end(), top_chars.begin(), top_chars.end());
    single_line.insert(single_line.end(), bot_chars.begin(), bot_chars.end());
    auto country_codes = possibleCountryCode(single_line);
    auto persian_sequence = getPersianStringSequence(possibleCountryCode(top_chars)["chars"]);


    double score = 0;
    if(findSequence(persian_sequence,"DDADDDDD"))
        score += 1;
    if(findSequence(persian_sequence,"DADDDDD"))
        score += 1;
    if(findSequence(persian_sequence,"DDADDDD"))
        score += 1;
    if(findSequence(persian_sequence,"DDADDD"))
        score += 1;
    if(findSequence(persian_sequence,"DDBDDDDDD"))
        score += 1;

    if(findSequence(persian_sequence,"DDBDDDD"))
        score += 1;

    score /= 1;
    return score;
}

double gocr::GlobalPlateRecognition::estimateFreezoneScore(std::map<std::string, std::vector<Annotation> > &top_bots)
{

    auto bot_chars = top_bots["bot"];
    auto top_chars = top_bots["top"];
    std::vector<Annotation> single_line;

    single_line.insert(single_line.end(), top_chars.begin(), top_chars.end());
    single_line.insert(single_line.end(), bot_chars.begin(), bot_chars.end());
    auto country_codes = possibleCountryCode(single_line);
    auto persian_sequence = getPersianStringSequence(possibleCountryCode(top_chars)["chars"]);
    auto latin_sequence = getLatinStringSequence(possibleCountryCode(bot_chars)["chars"]);


    double score = 0;
    if(findSequence(persian_sequence,"DDDD") && findSequence(latin_sequence,"DDDD"))
        score += 1;
    score /= 1;
    return score;
}

double gocr::GlobalPlateRecognition::estimateIranTransitScore(std::map<std::string, std::vector<Annotation> > &top_bots) {

    auto bot_chars = top_bots["bot"];
    auto top_chars = top_bots["top"];
    std::vector<Annotation> single_line;
    single_line.insert(single_line.end(), top_chars.begin(), top_chars.end());
    single_line.insert(single_line.end(), bot_chars.begin(), bot_chars.end());
    auto latin_sequence = getLatinStringSequence(possibleCountryCode(top_chars)["chars"]);
    bool IranTransitFlag;
    IranTransitFlag = hasIranTransitFlag(single_line);
    bool has_a_b_d_e = false;
    auto tops = possibleCountryCode(top_chars)["chars"];
    for(auto m:tops)
    {
        if(m.label=="A" | m.label=="B" | m.label=="D" | m.label=="E")
            has_a_b_d_e = true;
    }
    double score = 0;


    if (IranTransitFlag)
    {
        score+= 1.0;
    }
    if(!has_a_b_d_e)
        score -= 1;
    else
        score += 1;
    if(findSequence(latin_sequence,"DDADDDDD"))
        score += 1;
    if(findSequence(latin_sequence,"DDAADDD"))
        score += 1;
    if(findSequence(latin_sequence,"DDBDDDDD"))
        score += 1;
    score /= 2;

    return score;
}

double gocr::GlobalPlateRecognition::estimateIraqScore(std::map<std::string, std::vector<Annotation> > &top_bots) {

    auto bot_chars = top_bots["bot"];
    auto top_chars = top_bots["top"];
    std::vector<Annotation> single_line;
    single_line.insert(single_line.end(), top_chars.begin(), top_chars.end());
    single_line.insert(single_line.end(), bot_chars.begin(), bot_chars.end());

    auto all_chars  = possibleCountryCode(single_line);
    auto chars = all_chars["chars"];
    auto codes = all_chars["codes"];

    bool IRQ_PROVINCE_flag = false;
    bool IRAQ_permutations = false;
    auto persian_sequence = getPersianStringSequence(possibleCountryCode(top_chars)["chars"]);
    auto latin_sequence = getLatinStringSequence(possibleCountryCode(bot_chars)["chars"]);


    IRQ_PROVINCE_flag = hasIRQ_ProvinceFlag(single_line);
    IRAQ_permutations = hasIRAQ_Permutations(codes);
    auto persianLetterCount = getPersianLetterCount(single_line);

    double score = 0;
    if (IRQ_PROVINCE_flag)
    {
        score+= 2.5;
    }
    if (IRAQ_permutations)
    {
        score+= 2.0;
    }

    if (persianLetterCount>2)
    {
        score+= -0.5;
    }else if(persianLetterCount==2){
        score+= 0.5;
    }else if(persianLetterCount==1){
        score+= 1.0;
    }else if(persianLetterCount==0){
        score+= 0.0;
    }

    if (findSequence(persian_sequence,"DDDDDD"))
    {
        score+= 1;
    }
    if (findSequence(persian_sequence,"DDDDD"))
    {
        score+= 1;
    }
    if (findSequence(persian_sequence,"DDD"))
    {
        score+= 1;
    }
    if (findSequence(persian_sequence,"DD"))
    {
        score+= 0.5;
    }


    if (findSequence(persian_sequence,"D"))
    {
        score+= 0.25;
    }

    if (findSequence(persian_sequence,"ADDDDD"))
    {
        score+= 1;
    }
    if (findSequence(persian_sequence,"ADDD"))
    {
        score+= 1;
    }

    if (findSequence(latin_sequence,"ABA"))
    {
        score+= 1;
    }
    if (findSequence(persian_sequence,"DDADDD"))
    {
        score-= 3.0;
    }
    if (findSequence(persian_sequence,"DDBDDD"))
    {
        score-= 2.0;
    }

    score /= 5;

    return score;
}

double gocr::GlobalPlateRecognition::estimateAfghanistanScore(std::map<std::string, std::vector<Annotation> > &top_bots) {

    auto bot_chars = top_bots["bot"];
    auto top_chars = top_bots["top"];
    std::vector<Annotation> single_line;
    single_line.insert(single_line.end(), top_chars.begin(), top_chars.end());
    single_line.insert(single_line.end(), bot_chars.begin(), bot_chars.end());
    auto country_codes = possibleCountryCode(single_line);
    auto latin_sequence = getLatinStringSequence(possibleCountryCode(bot_chars)["chars"]);
    auto persian_sequence = getPersianStringSequence(possibleCountryCode(top_chars)["chars"]);
    bool has_h = false;
    bool has_r = false;
    bool has_t = false;
    bool has_k = false;
    bool has_b = false;
    bool has_l = false;
    bool has_z = false;
    bool has_a = false;
    bool has_lam = false;
    for(auto &m:country_codes["codes"])
    {
        if(m.label=="H")
            has_h = true;
        if(m.label=="R")
            has_r = true;
        if(m.label=="T")
            has_t = true;
        if(m.label=="K")
            has_k = true;
        if(m.label=="B")
            has_b = true;
        if(m.label=="L")
            has_l = true;
        if(m.label=="Z")
            has_z = true;
        if(m.label=="A")
            has_a = true;
        if(m.label=="lam")
            has_lam = true;
    }
    for(int i=0;i<bot_chars.size() & i<3;i++)
    {
        auto m = bot_chars[i];
        if(m.label=="H")
            has_h = true;
        if(m.label=="R")
            has_r = true;
        if(m.label=="T")
            has_t = true;
        if(m.label=="K")
            has_k = true;
        if(m.label=="B")
            has_b = true;
        if(m.label=="L")
            has_l = true;
        if(m.label=="Z")
            has_z = true;
        if(m.label=="A")
            has_a = true;

    }
    if(top_chars.size())
        if(top_chars.back().label=="lam")
            has_lam = true;


    double score = 0;
    if(has_lam)
        score += 1.0;
    if(has_h | has_r | has_t)
        score += 1;
    if(has_k | has_b | has_l)
        score += 1;
    if(has_z | has_lam)
        score += 1;
    if(has_t | has_a | has_k)
        score += 1;


    if(findSequence(persian_sequence,"DDDDDA"))
        score += 2;
    if(findSequence(persian_sequence,"DDDDD"))
        score += 1;
    if(findSequence(persian_sequence,"DDDD"))
        score += 0.5;

    if(findSequence(latin_sequence,"DDDDDA"))
        score += 2;
    if(findSequence(latin_sequence,"DDDDBA"))
        score += 1;
    if(findSequence(latin_sequence,"AAADDDDDA"))
        score += 1;
    if(findSequence(latin_sequence,"AADDDDDA"))
        score += 1;
    if(latin_sequence[latin_sequence.size()-1]=='A' & persian_sequence[persian_sequence.size()-1]=='A')
        score += 1;
    score /= 6;
    return score;
}

std::vector<gocr::Annotation> gocr::GlobalPlateRecognition::addWhiteSpace(std::vector<Annotation> annots)
{
    float xbar = 0;
    if(annots.size()<2)
    {
        return annots;
    }
    float wbar = 0;
    for(size_t i=0;i<annots.size()-1;i++)
    {
        xbar += annots[i+1].coordinates.x - annots[i].coordinates.x;
        wbar += annots[i].coordinates.width;
    }
    wbar = wbar/float(annots.size()-1);
    xbar = xbar/float(annots.size()-1);
    std::vector<Annotation> new_annots;
    new_annots.push_back(annots[0]);
    for(size_t i=1;i<annots.size();i++)
    {
        float s = annots[i].coordinates.x - annots[i-1].coordinates.x;
        int nw = s/(0.9*xbar) - 1;
        for(int j=0;j<nw;j++)
        {
            Annotation blank;
            blank.label = "blank";
            blank.coordinates.x = annots[i-1].coordinates.x + xbar;
            blank.coordinates.y = annots[i-1].coordinates.y;
            blank.coordinates.width = annots[i-1].coordinates.width;
            blank.coordinates.height = annots[i-1].coordinates.height;
            blank.score = 0.5;
            new_annots.push_back(blank);
        }
        new_annots.push_back(annots[i]);
    }
    return new_annots;

}
