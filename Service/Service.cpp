#include "Service.h"

Service::Service()
{
    Configurate* ConfigurateObj = Configurate::getInstance();
    this->InputFields = ConfigurateObj->getInputFields();
    this->InsertDatabaseInfo = ConfigurateObj->getInsertDatabaseInfo();
    this->InsertDatabase = ConfigurateObj->getInsertDatabase();
    this->StoreImageConfig = ConfigurateObj->getStoreImageConfig();
    this->ViolationMap = ConfigurateObj->getViolationMap();
}