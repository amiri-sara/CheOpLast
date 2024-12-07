#ifndef CRYPTOTOOLS_H
#define CRYPTOTOOLS_H

#include "../crow/json.h"
#include "cipher.h"
#include "../Logs/log.h"
#include <FreeImage.h>


struct DecryptedOutputStruct
{
    std::string DecryptedMessage = "";
    std::string DecryptedDigitalSignature;
};

DecryptedOutputStruct decryptFile(std::string FilePath,std::string ServerPublicKey, std::string ClientPublicKey);
DecryptedOutputStruct decryptString(std::string InputStr,std::string ServerPrivateKeyPath, std::string ClientPublicKeyPath);
bool isBase64(unsigned char C);
cv::Mat convertBase64ToMatImage(std::string Base64Str);
cv::Mat createMatImage(std::string &Image, std::string tto);
void FI2MAT(FIBITMAP* src, cv::Mat& dst);
std::string DecodeBase64(std::string const& EncodedString);
std::string GetHex(const int &N);

#endif // CRYPTOTOOLS_H
