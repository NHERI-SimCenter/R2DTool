#include "ComponentDatabaseManager.h"



ComponentDatabaseManager *ComponentDatabaseManager::theInstance = nullptr;


ComponentDatabaseManager::ComponentDatabaseManager()
{
    theInstance = this;

    theBuildingComponentDb = nullptr;
    theGasNetworkPipelinesDb = nullptr;
    theGasNetworkWellsCaprockstDb = nullptr;
    theGasNetworkAboveGroundInfrastructureDb = nullptr;
    theWaterNetworkPipeComponentDb = nullptr;
    theWaterNetworkNodesDb = nullptr;
    theSiteSoilColumnDb = nullptr;
    theSiteCPTDataDb = nullptr;
}


ComponentDatabaseManager* ComponentDatabaseManager::getInstance()
{
    if (theInstance == nullptr)
      theInstance = new ComponentDatabaseManager();

    return theInstance;
}


ComponentDatabase* ComponentDatabaseManager::getComponentDb(const QString& type)
{
    if(type.compare("Buildings") == 0)
        return this->getBuildingComponentDb();
    else if(type.compare("GasNetworkPipelines") == 0)
        return this->getGasPipelineComponentDb();
    else if(type.compare("WaterNetworkPipelines") == 0)
        return this->getWaterNetworkPipeComponentDb();
    else if(type.compare("GasNetworkWellsCaprocks") == 0)
        return this->getWellsandCaprocksComponentDb();
    else if(type.compare("GasNetworkAboveGroundInfrastructure") == 0)
        return this->getAboveGroundGasInfrastructureComponentDb();
    else if(type.compare("WaterNetworkNodes") == 0)
        return this->getWaterNetworkNodeComponentDb();
    else if(type.compare("SiteSoilColumn") == 0)
        return this->getSiteComponentDb();
    else if(type.compare("SiteCPTData") == 0)
        return this->getCPTComponentDb();

    return nullptr;
}


ComponentDatabase* ComponentDatabaseManager::getBuildingComponentDb()
{
    if(theBuildingComponentDb == nullptr)
        theBuildingComponentDb = std::make_unique<ComponentDatabase>("Buildings");

    return theBuildingComponentDb.get();
}


ComponentDatabase* ComponentDatabaseManager::getGasPipelineComponentDb()
{
    if(theGasNetworkPipelinesDb == nullptr)
        theGasNetworkPipelinesDb = std::make_unique<ComponentDatabase>("GasNetworkPipelines");

    return theGasNetworkPipelinesDb.get();
}


ComponentDatabase* ComponentDatabaseManager::getWaterNetworkPipeComponentDb()
{
    if(theWaterNetworkPipeComponentDb == nullptr)
        theWaterNetworkPipeComponentDb = std::make_unique<ComponentDatabase>("WaterNetworkPipelines");

    return theWaterNetworkPipeComponentDb.get();
}


ComponentDatabase* ComponentDatabaseManager::getWellsandCaprocksComponentDb()
{
    if(theGasNetworkWellsCaprockstDb == nullptr)
        theGasNetworkWellsCaprockstDb = std::make_unique<ComponentDatabase>("GasNetworkWellsCaprocks");

    return theGasNetworkWellsCaprockstDb.get();
}


ComponentDatabase* ComponentDatabaseManager::getAboveGroundGasInfrastructureComponentDb()
{
    if(theGasNetworkAboveGroundInfrastructureDb == nullptr)
        theGasNetworkAboveGroundInfrastructureDb = std::make_unique<ComponentDatabase>("GasNetworkAboveGroundInfrastructure");

    return theGasNetworkAboveGroundInfrastructureDb.get();
}


ComponentDatabase* ComponentDatabaseManager::getWaterNetworkNodeComponentDb()
{
    if(theWaterNetworkNodesDb == nullptr)
        theWaterNetworkNodesDb = std::make_unique<ComponentDatabase>("WaterNetworkNodes");

    return theWaterNetworkNodesDb.get();
}


// add site component db
ComponentDatabase* ComponentDatabaseManager::getSiteComponentDb()
{
    if(theSiteSoilColumnDb == nullptr)
        theSiteSoilColumnDb = std::make_unique<ComponentDatabase>("SiteSoilColumn");

    return theSiteSoilColumnDb.get();
}


ComponentDatabase* ComponentDatabaseManager::getCPTComponentDb()
{
    if(theSiteCPTDataDb == nullptr)
        theSiteCPTDataDb = std::make_unique<ComponentDatabase>("SiteCPTData");

    return theSiteCPTDataDb.get();
}


