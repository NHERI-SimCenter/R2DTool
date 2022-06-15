#ifndef COMPONENTDATABASEMANAGER_H
#define COMPONENTDATABASEMANAGER_H

#include "ComponentDatabase.h"

#include <memory>

class ComponentDatabaseManager
{
public:
    explicit ComponentDatabaseManager();
    ~ComponentDatabaseManager();

    static ComponentDatabaseManager *getInstance(void);

    // Buildings
    ComponentDatabase* getBuildingComponentDb();

    // Natural gas distribution network
    ComponentDatabase* getGasPipelineComponentDb();
    ComponentDatabase* getWellsandCaprocksComponentDb();
    ComponentDatabase* getAboveGroundGasInfrastructureComponentDb();

    // Water distribution network
    ComponentDatabase* getWaterNetworkPipeComponentDb();
    ComponentDatabase* getWaterNetworkNodeComponentDb();

    // Site-specific information
    ComponentDatabase* getCPTComponentDb();
    ComponentDatabase* getSiteComponentDb();

private:

    static ComponentDatabaseManager *theInstance;

    std::unique_ptr<ComponentDatabase> theBuildingComponentDb;
    std::unique_ptr<ComponentDatabase> theGasPipelineComponentDb;
    std::unique_ptr<ComponentDatabase> theWellsandCaptrocksComponentDb;
    std::unique_ptr<ComponentDatabase> theAboveGroundInfrastructureComponentDb;
    std::unique_ptr<ComponentDatabase> theWaterNetworkPipeComponentDb;
    std::unique_ptr<ComponentDatabase> theWaterNetworkNodeComponentDb;
    std::unique_ptr<ComponentDatabase> theSiteComponentDb;
    std::unique_ptr<ComponentDatabase> theCPTComponentDb;
};

#endif // COMPONENTDATABASEMANAGER_H
