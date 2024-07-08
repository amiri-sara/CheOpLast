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

bool isBase64(unsigned char C)
{
    return (isalnum(C) || (C == '+') || (C == '/'));
}

cv::Mat convertBase64ToMatImage(std::string Base64Str)
{
    cv::Mat Img;
    cipher cipherobj;
    std::string DecodedImage = cipherobj.base64_decode(Base64Str);
    std::vector<uchar> bufferToCompress(DecodedImage.begin(), DecodedImage.end());
    Img = cv::imdecode(bufferToCompress, cv::IMREAD_UNCHANGED);
    return Img;
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