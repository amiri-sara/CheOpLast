#ifndef CRYPTOTOOLS_H
#define CRYPTOTOOLS_H

//#include <boost/thread.hpp>
//#include <curl/curl.h>
//#include <boost/property_tree/ptree.hpp>
//#include <boost/property_tree/json_parser.hpp>
//#include <boost/foreach.hpp>
//#include <sys/stat.h>
// #include <opencv2/opencv.hpp>
//#include <opencv2/imgcodecs/legacy/constants_c.h>
#include "../crow/json.h"
#include "cipher.h"
// #include "log.h"
#include <sys/time.h>

struct DecryptedOutputStruct
{
    std::string DecryptedMessage = "";
    std::string DecryptedDigitalSignature;
};

DecryptedOutputStruct decryptFile(std::string FilePath,std::string ServerPublicKey, std::string ClientPublicKey);
DecryptedOutputStruct decryptString(std::string InputStr,std::string ServerPrivateKeyPath, std::string ClientPublicKeyPath);
bool isBase64(unsigned char C);
// cv::Mat convertBase64ToMatImage(std::string Base64Str);

#endif // CRYPTOTOOLS_H
