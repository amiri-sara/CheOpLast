#ifndef CRYPTOTOOLS_H
#define CRYPTOTOOLS_H

#include "../crow/json.h"
#include "cipher.h"
#include "../Logs/log.h"

struct DecryptedOutputStruct
{
    std::string DecryptedMessage = "";
    std::string DecryptedDigitalSignature;
};

DecryptedOutputStruct decryptFile(std::string FilePath,std::string ServerPublicKey, std::string ClientPublicKey);
DecryptedOutputStruct decryptString(std::string InputStr,std::string ServerPrivateKeyPath, std::string ClientPublicKeyPath);
bool isBase64(unsigned char C);
cv::Mat convertBase64ToMatImage(std::string Base64Str);

#endif // CRYPTOTOOLS_H
