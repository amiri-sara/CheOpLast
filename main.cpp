#include "./ReadConfigurations/configurate.h"

int main()
{
    Configurate* ConfigurateObj = Configurate::getInstance();
    auto DatabaseConfig = ConfigurateObj->getDatabaseConfig();
    SHOW_IMPORTANTLOG2(DatabaseConfig.DatabaseIP);
    SHOW_IMPORTANTLOG2(DatabaseConfig.DatabasePort);
    SHOW_IMPORTANTLOG2(DatabaseConfig.DatabaseUsername);
    SHOW_IMPORTANTLOG2(DatabaseConfig.DatabasePassword);
    SHOW_IMPORTANTLOG2(DatabaseConfig.DETAIL);
    SHOW_IMPORTANTLOG2(DatabaseConfig.Enable);
    SHOW_IMPORTANTLOG2(DatabaseConfig.DatabaseName);
    SHOW_IMPORTANTLOG2(DatabaseConfig.CollectionName);

    SHOW_WARNING("**********");
    auto DatabaseInsert = ConfigurateObj->getDatabaseInsert();
    SHOW_IMPORTANTLOG2(DatabaseInsert.DatabaseIP);
    SHOW_IMPORTANTLOG2(DatabaseInsert.DatabasePort);
    SHOW_IMPORTANTLOG2(DatabaseInsert.DatabaseUsername);
    SHOW_IMPORTANTLOG2(DatabaseInsert.DatabasePassword);
    SHOW_IMPORTANTLOG2(DatabaseInsert.DETAIL);
    SHOW_IMPORTANTLOG2(DatabaseInsert.Enable);
    SHOW_IMPORTANTLOG2(DatabaseInsert.DatabaseName);
    SHOW_IMPORTANTLOG2(DatabaseInsert.CollectionName);

    SHOW_WARNING("**********");
    auto DatabaseFailed = ConfigurateObj->getDatabaseFailed();
    SHOW_IMPORTANTLOG2(DatabaseFailed.DatabaseIP);
    SHOW_IMPORTANTLOG2(DatabaseFailed.DatabasePort);
    SHOW_IMPORTANTLOG2(DatabaseFailed.DatabaseUsername);
    SHOW_IMPORTANTLOG2(DatabaseFailed.DatabasePassword);
    SHOW_IMPORTANTLOG2(DatabaseFailed.DETAIL);
    SHOW_IMPORTANTLOG2(DatabaseFailed.Enable);
    SHOW_IMPORTANTLOG2(DatabaseFailed.DatabaseName);
    SHOW_IMPORTANTLOG2(DatabaseFailed.CollectionName);

    return 0;
}