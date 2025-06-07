#ifndef CIPHER_H
#define CIPHER_H
#include <iostream>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/engine.h>
#include <openssl/pem.h>
#include <openssl/conf.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <fstream>
#include <iomanip>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <openssl/sha.h>
class cipher
{
#define PADDING RSA_PKCS1_PADDING
#define KEYSIZE 32
#define IVSIZE 32
#define SALTSIZE 8
#define TOTAL_ROUNDS 1
#define PASSPHRASESIZE 16
public:
    cipher();
    int initialize();
    void finalize();
    std::string ReadFile(std::string FileAddress);
    int WriteFile(std::string FileAddress,std::string Data);
    std::string randomBytes(int Size);
    std::string randomString(const int len);
    RSA* GetPublicKey(std::string Data);
    RSA* GetPrivateKey(std::string Data);
    std::string EncryptRSA(RSA* Key,std::string Message,bool PrivateKey=true);   //Great for storing passwords and ...
    std::string DecryptRSA(RSA* Key,std::string Message,bool PrivateKey=false);
    std::string EncryptAES(std::string Message, std::string EncryptHeader);   //Great for large size files
    std::string DecryptAES(std::string Message, std::string Key, std::string Iv);
    // std::string base64_decode(std::string const& encoded_string);
    std::string base64_decode(std::string const& encoded_string);
    std::string base64_encode(unsigned char * bytes_to_encode, unsigned int in_len);
    std::string generateHash(std::string Input);
    void EncryptCommandLine();
    void DecryptCommandLine();
    void FreeRSAKey(RSA* Key);
    private:
    std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
};

#endif // CIPHER_H
