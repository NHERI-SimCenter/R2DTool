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

    // Returns the component database given the string type
    ComponentDatabase* getComponentDb(const QString& type);

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
    std::unique_ptr<ComponentDatabase> theGasNetworkPipelinesDb;
    std::unique_ptr<ComponentDatabase> theGasNetworkWellsCaprockstDb;
    std::unique_ptr<ComponentDatabase> theGasNetworkAboveGroundInfrastructureDb;
    std::unique_ptr<ComponentDatabase> theWaterNetworkPipeComponentDb;
    std::unique_ptr<ComponentDatabase> theWaterNetworkNodesDb;
    std::unique_ptr<ComponentDatabase> theSiteSoilColumnDb;
    std::unique_ptr<ComponentDatabase> theSiteCPTDataDb;
};

#endif // COMPONENTDATABASEMANAGER_H
