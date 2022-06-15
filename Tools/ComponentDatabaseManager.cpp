#include "ComponentDatabaseManager.h"



ComponentDatabaseManager *ComponentDatabaseManager::theInstance = nullptr;


ComponentDatabaseManager::ComponentDatabaseManager()
{
    theInstance = this;
    theBuildingComponentDb = nullptr;
    theSiteComponentDb = nullptr;
    theGasPipelineComponentDb = nullptr;
    theCPTComponentDb = nullptr;

}


ComponentDatabaseManager* ComponentDatabaseManager::getInstance()
{
    if (theInstance == nullptr)
      theInstance = new ComponentDatabaseManager();

    return theInstance;
}


ComponentDatabase* ComponentDatabaseManager::getBuildingComponentDb()
{
    if(theBuildingComponentDb == nullptr)
        theBuildingComponentDb = std::make_unique<ComponentDatabase>();

    return theBuildingComponentDb.get();
}


ComponentDatabase* ComponentDatabaseManager::getGasPipelineComponentDb()
{
    if(theGasPipelineComponentDb == nullptr)
        theGasPipelineComponentDb = std::make_unique<ComponentDatabase>();

    return theGasPipelineComponentDb.get();
}


ComponentDatabase* ComponentDatabaseManager::getWaterNetworkPipeComponentDb()
{
    if(theWaterNetworkPipeComponentDb == nullptr)
        theWaterNetworkPipeComponentDb = std::make_unique<ComponentDatabase>();

    return theWaterNetworkPipeComponentDb.get();
}


ComponentDatabase* ComponentDatabaseManager::getWellsandCaprocksComponentDb()
{
    if(theWellsandCaptrocksComponentDb == nullptr)
        theWellsandCaptrocksComponentDb = std::make_unique<ComponentDatabase>();

    return theWellsandCaptrocksComponentDb.get();
}


ComponentDatabase* ComponentDatabaseManager::getAboveGroundGasInfrastructureComponentDb()
{
    if(theAboveGroundInfrastructureComponentDb == nullptr)
        theAboveGroundInfrastructureComponentDb = std::make_unique<ComponentDatabase>();

    return theAboveGroundInfrastructureComponentDb.get();
}


ComponentDatabase* ComponentDatabaseManager::getWaterNetworkNodeComponentDb()
{
    if(theWaterNetworkNodeComponentDb == nullptr)
        theWaterNetworkNodeComponentDb = std::make_unique<ComponentDatabase>();

    return theWaterNetworkNodeComponentDb.get();
}


// add site component db
ComponentDatabase* ComponentDatabaseManager::getSiteComponentDb()
{
    if(theSiteComponentDb == nullptr)
        theSiteComponentDb = std::make_unique<ComponentDatabase>();

    return theSiteComponentDb.get();
}


ComponentDatabase* ComponentDatabaseManager::getCPTComponentDb()
{
    if(theCPTComponentDb == nullptr)
        theCPTComponentDb = std::make_unique<ComponentDatabase>();

    return theCPTComponentDb.get();
}


