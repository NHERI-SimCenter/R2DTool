#include "ComponentDatabaseManager.h"



ComponentDatabaseManager *ComponentDatabaseManager::theInstance = nullptr;


ComponentDatabaseManager::ComponentDatabaseManager()
{
    theInstance = this;
    theBuildingComponentDb = nullptr;
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


ComponentDatabase* ComponentDatabaseManager::getPipelineComponentDb()
{
    if(thePipelineComponentDb == nullptr)
        thePipelineComponentDb = std::make_unique<ComponentDatabase>();

    return thePipelineComponentDb.get();
}
