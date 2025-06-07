#include "cryptotools.h"

DecryptedOutputStruct decryptFile(std::string FilePath,std::string ServerPublicKey, std::string ClientPublicKey)
{
    DecryptedOutputStruct DecryptedOutput;
    try
    {
        cipher CW;

        std::string Package  = CW.ReadFile(FilePath);
        if(Package.size())
        {
            //std::string ServerKey  = CW.ReadFile(ServerPublicKeyPath);
            //std::string ClientKey  = CW.ReadFile(ClientPublicKeyPath);
            std::string ServerKey  = ServerPublicKey;
            std::string ClientKey  = ClientPublicKey;

            RSA* ServerPubKey  = CW.GetPublicKey(ServerKey);
            RSA* ClientPubKey  = CW.GetPublicKey(ClientKey);

            std::string Base64DecodedMessage = CW.base64_decode(Package);

            std::stringstream InputJsonStructure;
            InputJsonStructure << Base64DecodedMessage;


            auto InputJson = crow::json::load(Base64DecodedMessage);
            std::string EncryptedData = InputJson["Data"].s();
            std::string EncryptedDigitalSignature = InputJson["DigitalSignature"].s();

            DecryptedOutput.DecryptedDigitalSignature = CW.DecryptRSA(ServerPubKey, EncryptedDigitalSignature, false);

            std::string header("Konect__");
            int Indx = EncryptedData.find(header);
            if(Indx < 0)
            {
                std::cout<<"Could not load header from the data!"<<std::endl;
                return DecryptedOutput;
            }

            std::string EncryptedKey     = EncryptedData.substr(0, Indx);
            std::string EncryptedMessage = EncryptedData.substr(Indx);

            std::string Passphrase = CW.DecryptRSA(ClientPubKey, EncryptedKey, false);

            std::string key;
            std::string iv;
            std::string message;
            if(EncryptedMessage.substr(0,8) == "Konect__")
            {
                std::size_t indxKey = EncryptedMessage.find("Konect__", SALTSIZE);
                std::size_t indxIv = EncryptedMessage.find("Konect__", indxKey+SALTSIZE);
                if(indxKey != std::string::npos && indxIv != std::string::npos)
                {
                    key = CW.base64_decode(EncryptedMessage.substr(SALTSIZE, indxKey-SALTSIZE));
                    iv = CW.base64_decode(EncryptedMessage.substr(indxKey+SALTSIZE, indxIv-(indxKey+SALTSIZE)));
                    message = CW.base64_decode(EncryptedMessage.substr(indxIv+SALTSIZE));
                }
                else
                {
                    std::cout<<"Could not load salt from the data!  \n";
                    std::cout.flush();
                    return DecryptedOutput;
                }
            }
            else
            {
                std::cout<<"Could not load salt from the data!  \n";
                std::cout.flush();
                return DecryptedOutput;
            }

            std::string DecodedMessage = CW.DecryptAES(message,key,iv);
            if(DecodedMessage == "")
            {
                std::cout << "DecodedMessage = NULL" << std::endl;
                return DecryptedOutput;
            }
            DecryptedOutput.DecryptedMessage = DecodedMessage;
            std::string MyDigSig = CW.generateHash(DecodedMessage);
            if(MyDigSig != DecryptedOutput.DecryptedDigitalSignature)
            {
                std::cout<<"The DigitalSignatures do not match"<<std::endl;
            }
        }
    }
    catch(std::exception & e)
    {
        std::cout<<"Exception occured! "<<e.what()<<std::endl;
        return DecryptedOutput;
    }

    return DecryptedOutput;
}

DecryptedOutputStruct decryptString(std::string InputStr,std::string ServerPrivateKeyPath, std::string ClientPublicKeyPath)
{
    DecryptedOutputStruct DecryptedOutput;
    try
    {
        cipher CW;

        std::string Package  = InputStr;
        std::string ServerKey  = CW.ReadFile(ServerPrivateKeyPath);
        std::string ClientKey  = CW.ReadFile(ClientPublicKeyPath);

        RSA* ServerPriKey  = CW.GetPrivateKey(ServerKey);
        RSA* ClientPubKey  = CW.GetPublicKey(ClientKey);

        std::string Base64DecodedMessage = CW.base64_decode(Package);

        auto InputJson = crow::json::load(Base64DecodedMessage);
        std::string EncryptedData = InputJson["Data"].s();
        std::string EncryptedDigitalSignature = InputJson["DigitalSignature"].s();

        // DecryptedOutput.DecryptedDigitalSignature = CW.DecryptRSA(ClientPubKey, EncryptedDigitalSignature, false);
        std::string header("Konect__");
        int Indx = EncryptedData.find(header);
        if(Indx < 0)
        {
            std::cout << "Could not load header from the data!" << std::endl;
            return DecryptedOutput;
        }

        std::string EncryptedKey     = CW.base64_decode(EncryptedData.substr(0, Indx));
        std::string EncryptedMessage = EncryptedData.substr(Indx);

        // std::string Passphrase = CW.DecryptRSA(ServerPriKey, EncryptedKey, true);

        std::string key;
        std::string iv;
        std::string message;
        if(EncryptedMessage.substr(0,8) == "Konect__")
        {
            std::size_t indxKey = EncryptedMessage.find("Konect__", SALTSIZE);
            std::size_t indxIv = EncryptedMessage.find("Konect__", indxKey+SALTSIZE);
            if(indxKey != std::string::npos && indxIv != std::string::npos)
            {
                key = CW.base64_decode(EncryptedMessage.substr(SALTSIZE, indxKey-SALTSIZE));
                iv = CW.base64_decode(EncryptedMessage.substr(indxKey+SALTSIZE, indxIv-(indxKey+SALTSIZE)));
                message = CW.base64_decode(EncryptedMessage.substr(indxIv+SALTSIZE));
            }
            else
            {
                std::cout << "Could not load salt from the data!" << std::endl;
                return DecryptedOutput;
            }
        }
        else
        {
            std::cout << "Could not load salt from the data!" << std::endl;
            return DecryptedOutput;
        }

        std::string DecodedMessage = CW.DecryptAES(message,key,iv);
        if(DecodedMessage == "")
        {
            std::cout << "DecodedMessage = NULL" << std::endl;
            return DecryptedOutput;
        }
        DecryptedOutput.DecryptedMessage = DecodedMessage;
        // std::string MyDigSig = CW.generateHash(EncryptedData);
        // if(MyDigSig != DecryptedOutput.DecryptedDigitalSignature)
        // {
        //     std::cout<<"The DigitalSignatures do not match"<<std::endl;
        // }
    }
    catch(std::exception & e)
    {
        std::cout<<"Exception occured! "<<e.what()<<std::endl;
        return DecryptedOutput;
    }

    return DecryptedOutput;
}

// این تابع کمکی is_base64 را که قبلاً در کد شما بود، نیاز داریم.
// (اگر قبلاً در فایل cipher.cpp تعریف نشده، آن را اضافه کنید)
static inline bool is_base64(unsigned char c) {
    return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string DecodeBase64(std::string const& encoded_string) {
    std::string Base64Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                              "abcdefghijklmnopqrstuvwxyz"
                              "0123456789+/";

    int in_len = encoded_string.size();
    int i = 0;
    int j = 0;
    int in_ = 0; // این متغیر به عنوان ایندکس خواندن از encoded_string استفاده می‌شود
    unsigned char char_array_4[4], char_array_3[3];
    std::string ret;

    // حلقه‌ای برای پیمایش کل رشته ورودی
    while (in_ < in_len) {
        char c = encoded_string[in_];
        in_++; // پیشروی ایندکس خواندن

        if (c == '=') break; // اگر به کاراکتر Padding '=' رسیدیم، پایان داده Base64 است.

        // --- مرحله کلیدی: نادیده گرفتن کاراکترهای فضای خالی (از جمله newline) ---
        // دیکودرهای استاندارد Base64 معمولاً کاراکترهای whitespace را نادیده می‌گیرند.
        // این کارآمدترین و حرفه‌ای‌ترین روش برای مدیریت این کاراکترهاست.
        if (c == '\n' || c == '\r' || c == ' ' || c == '\t') {
            continue; // کاراکتر فضای خالی را نادیده بگیر و به کاراکتر بعدی برو
        }
        // --- پایان مرحله نادیده گرفتن ---

        // بررسی اینکه آیا کاراکتر یک کاراکتر معتبر Base64 (بعد از نادیده گرفتن فضاهای خالی) است.
        if (!is_base64(c)) { 
             // اختیاری: می‌توانید در اینجا یک لاگ خطا برای کاراکترهای نامعتبر اضافه کنید
             // Logger::getInstance().logError("Invalid Base64 character encountered: " + std::string(1, c));
             continue; // کاراکتر نامعتبر را نادیده بگیر و به کاراکتر بعدی برو
        }

        char_array_4[i++] = c; // کاراکتر معتبر Base64 را به بافر موقت اضافه کن

        // وقتی 4 کاراکتر Base64 جمع شد، آن‌ها را به 3 بایت داده اصلی دیکود کن.
        if (i == 4) {
            for (i = 0; i < 4; i++)
                char_array_4[i] = Base64Chars.find(char_array_4[i]); // تبدیل کاراکتر Base64 به مقدار عددی آن

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++)
                ret += char_array_3[i]; // اضافه کردن 3 بایت دیکود شده به رشته نهایی
            i = 0; // ریست کردن شمارنده بافر موقت
        }
    }

    // مدیریت کاراکترهای باقی‌مانده در بافر موقت (اگر Padding وجود داشته باشد)
    if (i) {
        for (j = i; j < 4; j++)
            char_array_4[j] = 0;

        for (j = 0; j < 4; j++)
            char_array_4[j] = Base64Chars.find(char_array_4[j]);

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2); // Error in original line
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3]; // Corrected line based on context.

        for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
    }

    return ret;
}
void FI2MAT(FIBITMAP* src, cv::Mat& dst)
{
    using namespace cv;

    //FIT_BITMAP    //standard image : 1 - , 4 - , 8 - , 16 - , 24 - , 32 - bit
    //FIT_UINT16    //array of unsigned short : unsigned 16 - bit
    //FIT_INT16     //array of short : signed 16 - bit
    //FIT_UINT32    //array of unsigned long : unsigned 32 - bit
    //FIT_INT32     //array of long : signed 32 - bit
    //FIT_FLOAT     //array of float : 32 - bit IEEE floating point
    //FIT_DOUBLE    //array of double : 64 - bit IEEE floating point
    //FIT_COMPLEX   //array of FICOMPLEX : 2 x 64 - bit IEEE floating point
    //FIT_RGB16     //48 - bit RGB image : 3 x 16 - bit
    //FIT_RGBA16    //64 - bit RGBA image : 4 x 16 - bit
    //FIT_RGBF      //96 - bit RGB float image : 3 x 32 - bit IEEE floating point
    //FIT_RGBAF     //128 - bit RGBA float image : 4 x 32 - bit IEEE floating point

    int bpp = FreeImage_GetBPP(src);
    FREE_IMAGE_TYPE fit = FreeImage_GetImageType(src);

    int cv_type = -1;
    int cv_cvt = -1;

    switch (fit)
    {
    case FIT_UINT16: cv_type = cv::DataType<ushort>::type; break;
    case FIT_INT16: cv_type = DataType<short>::type; break;
    case FIT_INT32: cv_type = DataType<int>::type; break;
    case FIT_FLOAT: cv_type = DataType<float>::type; break;
    case FIT_DOUBLE: cv_type = DataType<double>::type; break;
    case FIT_RGB16: cv_type = CV_16UC3; cv_cvt = COLOR_RGB2BGR; break;
    case FIT_RGBA16: cv_type = CV_16UC4; cv_cvt = COLOR_RGBA2BGRA; break;
    case FIT_RGBF: cv_type = CV_32FC3; cv_cvt = COLOR_RGB2BGR; break;
    case FIT_RGBAF: cv_type = CV_32FC4; cv_cvt = COLOR_RGBA2BGRA; break;
    case FIT_BITMAP:
        switch (bpp) {
        case 8: cv_type = CV_8UC1; break;
        case 16: cv_type = CV_8UC2; break;
        case 24: cv_type = CV_8UC3; break;
        case 32: cv_type = CV_8UC4; break;
        default:
            // 1, 4 // Unsupported natively
            cv_type = -1;
        }
        break;
    default:
        // FIT_UNKNOWN // unknown type
        dst = Mat(); // return empty Mat
        return;
    }

    int width = FreeImage_GetWidth(src);
    int height = FreeImage_GetHeight(src);
    int step = FreeImage_GetPitch(src);

    if (cv_type >= 0) {
        dst = Mat(height, width, cv_type, FreeImage_GetBits(src), step);
        if (cv_cvt > 0)
        {
            cvtColor(dst, dst, cv_cvt);
        }
    }
    else {

        std::vector<uchar> lut;
        int n = pow(2, bpp);
        for (int i = 0; i < n; ++i)
        {
            lut.push_back(static_cast<uchar>((255 / (n - 1))*i));
        }

        FIBITMAP* palletized = FreeImage_ConvertTo8Bits(src);
        BYTE* data = FreeImage_GetBits(src);
        for (int r = 0; r < height; ++r) {
            for (int c = 0; c < width; ++c) {
                dst.at<uchar>(r, c) = saturate_cast<uchar>(lut[data[r*step + c]]);
            }
        }

        lut.clear();
        delete palletized;
        free(data);
    }

    flip(dst, dst, 0);
}

cv::Mat convertBase64ToMatImage(std::string Base64Str)
{
    cv::Mat Img;
    cipher cipherobj;
    std::string DecodedImage = cipherobj.base64_decode(Base64Str);
    std::vector<uchar> bufferToCompress(DecodedImage.begin(), DecodedImage.end());
    if(DecodedImage == "")
    {
        return Img;
    }
    Img = cv::imdecode(bufferToCompress, cv::IMREAD_UNCHANGED);
    return Img;
}
cv::Mat createMatImage(const std::string &Image)
{

    //cv::Mat test = cv::imdecode(Image, 0);

    cv::Mat Img;
    std::string DecodedImage = DecodeBase64(Image);
    //    std::vector<uchar> BufferToCompress(DecodedImage.begin(), DecodedImage.end());
    FIBITMAP* fi_image;

    if(DecodedImage != "")
    {
        try
        {

            FIMEMORY * memImage = FreeImage_OpenMemory((BYTE*)DecodedImage.c_str(), DecodedImage.size());
            FREE_IMAGE_FORMAT format = FreeImage_GetFileTypeFromMemory(memImage, 0);
            fi_image = FreeImage_LoadFromMemory(format, memImage);
            FreeImage_CloseMemory(memImage);
            FI2MAT(fi_image, Img);




            //            Img = cv::imdecode(BufferToCompress, -1);
        }
        catch(...)
        {
            Logger::getInstance().logWarning("Couldn't Convert Base64 to cv::Mat ");
        }
    }

    cv::Mat out = Img.clone();

    FreeImage_Unload(fi_image);
    Img.release();

    return out;
}

std::string GetHex(const int &N)
{
    char hex_string[20];
    for(int i = 0 ; i < 20 ; i ++)
        hex_string[i] = ' ' ;
    sprintf(hex_string, "%X", N); //convert number to hex

    std::string Hex = "";
    for(int i = 0 ; i  < 20 ; i ++)
    {
        if(hex_string[i]!= ' ')
        {
            Hex+=hex_string[i];
        }
        else
            break;
    }
    Hex.pop_back();
    return Hex;
}