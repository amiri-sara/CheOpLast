#pragma once
#include<string>
#include<iostream>
#include <fstream>
#include <chrono>
#include <cmath>
#include <map>
#include<string>
#include "nn/base.h"
using namespace std;


using namespace aivision::nn;
typedef NNModel* (*ClassFactoryFn)();
class ModelFactoryMapImpl : public std::map<int, ClassFactoryFn>
{
public:
    ModelFactoryMapImpl() {}
    void Register(int ID, ClassFactoryFn fn );
} ;

ModelFactoryMapImpl & ModelFactoryMap();


template<class T> class ModelsFactoryRegistrar
{
public:
    ModelsFactoryRegistrar()
    {
        ModelFactoryMap().
                Register(T::ID, T::Create);
    }
} ;
#define REGISTER_MODEL(name) static  ModelsFactoryRegistrar<name> modelRegisterer##name{}



namespace aivision {
class  ModelFactory
{
public:
     ModelFactory(){}
     std::unique_ptr<NNModel> get(BaseNNConfig conf);
};

}
