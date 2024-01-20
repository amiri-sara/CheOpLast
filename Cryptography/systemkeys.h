#ifndef SYSTEMKEYS_H
#define SYSTEMKEYS_H
#include <iostream>
#include <string>
#include <vector>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/seq/for_each_i.hpp>
#include <boost/preprocessor/seq/enum.hpp>

#define LocalDatabase
class SystemKeys
{
public:
    struct DatabaseOutputStruct
    {
        std::string UserName;
        std::string Password;
        std::string Server;
    };
    struct KeyOutputStruct
    {
        std::string Key;
    };

    SystemKeys();
    DatabaseOutputStruct GetDatabaseKey();
    KeyOutputStruct GetKey1();
    KeyOutputStruct GetKey2();
    KeyOutputStruct GetKey3();
    KeyOutputStruct GetKey4();
    KeyOutputStruct GetKey5();
    KeyOutputStruct GetKey6();
    KeyOutputStruct GetKey7();
    KeyOutputStruct GetKey8();
    KeyOutputStruct GetKey9();
    KeyOutputStruct GetKey10();
    KeyOutputStruct GetKey11();
    KeyOutputStruct GetKey12();
    KeyOutputStruct GetKey13();
    KeyOutputStruct GetKey14();
    KeyOutputStruct GetKey15();
    KeyOutputStruct GetKey16();
    KeyOutputStruct GetKey17();
    KeyOutputStruct GetKey18();
    KeyOutputStruct GetKey19();
    KeyOutputStruct GetKey20();
    KeyOutputStruct GetKey21();
    KeyOutputStruct GetKey24();
    KeyOutputStruct GetKey25();
    KeyOutputStruct GetKey26();
    KeyOutputStruct GetKeySerPriCounter();
    KeyOutputStruct GetKeySerPubCounter();
    KeyOutputStruct GetKeyCliPriCounter();
    KeyOutputStruct GetKeyCliPubCounter();
    KeyOutputStruct GetKeyConfigServerPublic();
    KeyOutputStruct GetKeyConfigClientPublic();
    KeyOutputStruct GetBinaryPath();

};

#endif // SYSTEMKEYS_H
