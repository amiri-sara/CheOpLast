#include "cipher.h"

cipher::cipher()
{

}


int cipher::initialize()
{
    ERR_load_CRYPTO_strings();
    OpenSSL_add_all_algorithms();
    OpenSSL_add_all_ciphers();
    OpenSSL_add_all_digests();
    OPENSSL_config(NULL);
}
void cipher::FreeRSAKey(RSA *Key)
{
    RSA_free(Key);
}

void  cipher::finalize()
{

    EVP_cleanup();
    ERR_free_strings();
}
std::string  cipher::ReadFile(std::string FileAddress)
{
    std::fstream InputFile;
    InputFile.open(FileAddress.c_str(),std::ios::in | std::ios::binary | std::ios::ate);
    if(!InputFile.is_open())
    {
        std::cout<<"InputFile Error";
        std::cout.flush();

        return "";
    }
    std::streampos size = InputFile.tellg();
    char* memblock = new char [size];
    InputFile.seekg(0, std::ios::beg);
    InputFile.read(memblock, size);
    std::string InputData(memblock, size);
    //    std::string InputData;
    //    std::string line;
    //    while (std::getline (InputFile,line) )
    //    {
    //        InputData += line + "\n";
    //    }
    InputFile.close();
    delete[] memblock;
    return InputData;
}
int  cipher::WriteFile(std::string FileAddress,std::string Data)
{
    std::fstream OutputFile;
    OutputFile.open(FileAddress.c_str(), std::ios::out | std::ios::binary);
    if(!OutputFile.is_open())
    {
        std::cout<<"OutputFile Error \n";
        std::cout.flush();

        return 1;
    }
    OutputFile.write(Data.c_str(), Data.size());
    OutputFile.close();

}
std::string cipher::generateHash(std::string String)
{
    char outputBuffer[65];
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, String.c_str(), String.size());
    SHA256_Final(hash, &sha256);
    int i = 0;
    for(i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
    }
    outputBuffer[64] = 0;
    std::string Result(outputBuffer);
    return Result;
}
std::string cipher::randomBytes(int Size)
{

    unsigned char arr[Size];
    RAND_bytes(arr,Size);

    std::string RandString = std::string(reinterpret_cast<char*>(arr),Size);

    return RandString;
}
std::string cipher::randomString(const int len)
{
    char s[len];
    memset(s, 0, len);
    static const char alphanum[] =
            "0123456789~!@#$%^&*()-_=+|[]{};:,<.>/?"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";

    for (int i = 0; i < len; ++i) {
        s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    std::string str(s);
    return str;
}
RSA*  cipher::GetPublicKey(std::string Data)
{


    const char* PublicKeyStr = Data.data();
    BIO* bio = BIO_new_mem_buf((void *) PublicKeyStr,-1);
    BIO_set_flags(bio,BIO_FLAGS_BASE64_NO_NL);

    RSA* rsaPublicKey = PEM_read_bio_RSA_PUBKEY(bio,NULL,NULL,NULL);
    if(!rsaPublicKey)
    {
        std::cout<<"Could not load the public key ! \n ";
        std::cout.flush();
    }

    BIO_free(bio);
    return rsaPublicKey;
}
RSA*  cipher::GetPrivateKey(std::string Data)
{
    const char* PrivateKeystr = Data.data();
    BIO* bio = BIO_new_mem_buf((void *) PrivateKeystr,-1);
    BIO_set_flags(bio,BIO_FLAGS_BASE64_NO_NL);

    RSA* rsaPrivateKey = PEM_read_bio_RSAPrivateKey(bio,NULL,NULL,NULL);
    if(!rsaPrivateKey)
    {
        std::cout<<"Could not load the public key ! \n ";
    }

    BIO_free(bio);
    return rsaPrivateKey;
}

std::string  cipher::EncryptRSA(RSA* Key, std::string Message, bool PrivateKey)
{

    std::string Buffer;
    int dataSize = Message.size();
    const unsigned char* str= (const unsigned char*) Message.data();
    int rsaLength = RSA_size(Key);

    unsigned char* ed = (unsigned char*)malloc(rsaLength);
    int resultLength;
    if(PrivateKey)
    {
        resultLength = RSA_private_encrypt(dataSize,(const unsigned char*)str, ed,Key,PADDING);

    }
    else
    {
        resultLength = RSA_public_encrypt(dataSize,(const unsigned char*)str, ed,Key,PADDING);

    }

    if(resultLength == -1)
    {
        std::cout<<" Could not Encrypt the message something wrong here ! \n";
        std::cout.flush();
        free(ed);
        return Buffer;
    }

    Buffer = std::string(reinterpret_cast<char*>(ed),resultLength);
    free(ed);
    return Buffer;
}
std::string  cipher::DecryptRSA(RSA* Key, std::string Message, bool PrivateKey)
{
    std::string Buffer;
    const unsigned char * encryptedData = (const unsigned char*) Message.data();
    int rsaLength = RSA_size(Key);

    unsigned char* ed = (unsigned char*)malloc(rsaLength);

    int resultLength ;
    if(PrivateKey)
    {
        resultLength = RSA_private_decrypt(rsaLength,encryptedData, ed,Key,PADDING);
    }
    else
    {
        resultLength = RSA_public_decrypt(rsaLength,encryptedData, ed,Key,PADDING);

    }

    if(resultLength == -1)
    {
        std::cout << "Could not Decrypt the message something wrong here !" << std::endl;
        std::cout.flush();
        free(ed);
        return Buffer;
    }

    Buffer = std::string(reinterpret_cast<char*>(ed),resultLength);
    free(ed);
    return Buffer;
}
std::string  cipher::EncryptAES(std::string Message, std::string EncryptHeader)
{
    EVP_CIPHER_CTX *en;

    if(!(en = EVP_CIPHER_CTX_new()))
    {
        std::cout<<"EncryptAES EVP_CIPHER_CTX_new Error !  "<<ERR_error_string(ERR_get_error(),NULL)<<std::endl;
        return "";
    }

    std::string Key = randomString(KEYSIZE);
    std::string Iv = randomString(IVSIZE);

    if(1 != EVP_EncryptInit_ex(en,EVP_aes_256_cbc(),NULL,(unsigned char*)Key.c_str(),(unsigned char*)Iv.c_str()));
    {
        //std::cout<<"EVP_EncryptInit_ex Error !  "<<ERR_error_string(ERR_get_error(),NULL)<<std::endl;
        //return "";
    }

    int plaintext_len = Message.length();
    int ciphertext_len = plaintext_len + AES_BLOCK_SIZE ;
    int final_len = 0;

    unsigned char * ciphertext = new unsigned char[ciphertext_len];
    bzero(ciphertext,ciphertext_len);

    //May Have to repeat this for large files e.g larger than hundreds of MBs
    if(!EVP_EncryptUpdate(en,ciphertext,&ciphertext_len,(unsigned char*) Message.c_str(),plaintext_len))
    {
        std::cout<<"EVP_EncryptUpdate Error !  "<<ERR_error_string(ERR_get_error(),NULL)<<std::endl;
        return "";
    }

    if(!EVP_EncryptFinal(en,ciphertext + ciphertext_len,&final_len))
    {
        std::cout<<"EVP_EncryptFinal Error !  "<<ERR_error_string(ERR_get_error(),NULL)<<std::endl;
        return "";
    }

    plaintext_len = ciphertext_len + final_len;

    std::string Encrpyted = std::string(reinterpret_cast<char*> (ciphertext),plaintext_len);

    EVP_CIPHER_CTX_cleanup(en);
    delete [] ciphertext;

    std::string ResultEncryptedMessage(EncryptHeader);
    ResultEncryptedMessage.append(base64_encode((unsigned char *)Key.c_str(),Key.length()));
    ResultEncryptedMessage.append(EncryptHeader);
    ResultEncryptedMessage.append(base64_encode((unsigned char *)Iv.c_str(),Iv.length()));
    ResultEncryptedMessage.append(EncryptHeader);
    ResultEncryptedMessage.append(base64_encode((unsigned char *)Encrpyted.data(),Encrpyted.size()));

    return ResultEncryptedMessage;
}

std::string cipher::DecryptAES(std::string Message, std::string Key, std::string Iv)
{
    EVP_CIPHER_CTX *ctx;

    if(!(ctx = EVP_CIPHER_CTX_new()))
    {
        std::cout<<"EncryptAES EVP_CIPHER_CTX_new Error !  "<<ERR_error_string(ERR_get_error(),NULL)<<std::endl;
        return "";
    }

    if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, (unsigned char*)Key.c_str(), (unsigned char*)Iv.c_str()))
    {
        std::cout<<"EVP_DecryptInit_ex Error !  "<<ERR_error_string(ERR_get_error(),NULL)<<std::endl;
    }

    int ciphertext_len = Message.length();
    int len;
    int plaintext_len;
    unsigned char* plaintext = new unsigned char[ciphertext_len];
    bzero(plaintext,ciphertext_len);

    if(1 != EVP_DecryptUpdate(ctx, plaintext, &len, (unsigned char*)Message.c_str(), ciphertext_len))
    {
        std::cout<<"EVP_DecryptUpdate Error !  "<<ERR_error_string(ERR_get_error(),NULL)<<std::endl;
        /* Clean up */
        EVP_CIPHER_CTX_free(ctx);
        delete [] plaintext;
        return "";
    }
    plaintext_len = len;

    if(1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len))
    {
        std::cout<<"4 EVP_EncryptInit_ex Error !  "<<ERR_error_string(ERR_get_error(),NULL)<<std::endl;
        /* Clean up */
        EVP_CIPHER_CTX_free(ctx);
        delete [] plaintext;
        return "";
    }
    plaintext_len += len;

    plaintext[plaintext_len] = 0;

    std::string ret = std::string(reinterpret_cast<char*> (plaintext),plaintext_len);

    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);
    delete [] plaintext;
    return ret;
}

static inline bool is_base64(unsigned char c) {
    return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string cipher::base64_encode(unsigned char * bytes_to_encode, unsigned int in_len) {
    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    while (in_len--) {
        char_array_3[i++] = *(bytes_to_encode++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for(i = 0; (i <4) ; i++)
                ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i)
    {
        for(j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (j = 0; (j < i + 1); j++)
            ret += base64_chars[char_array_4[j]];

        while((i++ < 3))
            ret += '=';

    }

    return ret;

}

void cipher::EncryptCommandLine()
{
    system("openssl aes-256-cbc -salt -md sha1 -in file.txt -out file.enc -pass pass:password");
}

void cipher::DecryptCommandLine()
{
    system("openssl aes-256-cbc -d -salt -md sha1 -in file.txt -out file.enc -pass pass:password");
}
std::string cipher::base64_decode(std::string const& encoded_string) {
    int in_len = encoded_string.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::string ret;

    while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
        char_array_4[i++] = encoded_string[in_]; in_++;
        if (i ==4) {
            for (i = 0; i <4; i++)
                char_array_4[i] = base64_chars.find(char_array_4[i]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++)
                ret += char_array_3[i];
            i = 0;
        }
    }

    if (i) {
        for (j = i; j <4; j++)
            char_array_4[j] = 0;

        for (j = 0; j <4; j++)
            char_array_4[j] = base64_chars.find(char_array_4[j]);

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
    }

    return ret;
}


