#include "ComponentDatabaseManager.h"



ComponentDatabaseManager *ComponentDatabaseManager::theInstance = nullptr;


ComponentDatabaseManager::ComponentDatabaseManager()
{
    theInstance = this;
}


ComponentDatabaseManager::~ComponentDatabaseManager()
{
    qDeleteAll(dbMap);
}


ComponentDatabaseManager* ComponentDatabaseManager::getInstance()
{
    if (theInstance == nullptr)
      theInstance = new ComponentDatabaseManager();

    return theInstance;
}


ComponentDatabase* ComponentDatabaseManager::getAssetDb(const QString& type)
{
//    if(type.compare("Buildings") == 0)
//        return this->getBuildingComponentDb();
//    else if(type.compare("GasNetworkPipelines") == 0)
//        return this->getGasPipelineComponentDb();
//    else if(type.compare("WaterNetworkPipelines") == 0)
//        return this->getWaterNetworkPipeComponentDb();
//    else if(type.compare("GasNetworkWellsCaprocks") == 0)
//        return this->getWellsandCaprocksComponentDb();
//    else if(type.compare("GasNetworkAboveGroundInfrastructure") == 0)
//        return this->getAboveGroundGasInfrastructureComponentDb();
//    else if(type.compare("WaterNetworkNodes") == 0)
//        return this->getWaterNetworkNodeComponentDb();
//    else if(type.compare("SiteSoilColumn") == 0)
//        return this->getSiteComponentDb();
//    else if(type.compare("SiteCPTData") == 0)
//        return this->getCPTComponentDb();

    return dbMap.value(type);
}


ComponentDatabase* ComponentDatabaseManager::createAssetDb(const QString& type)
{
    if(dbMap.contains(type))
        return dbMap.value(type);

    auto assetDB = new ComponentDatabase(type);

    dbMap.insert(type,assetDB);

    return assetDB;
}


QList<ComponentDatabase*> ComponentDatabaseManager::getAllAssetDatabases()
{    
    return dbMap.values();
}


