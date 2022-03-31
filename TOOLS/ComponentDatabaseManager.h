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

    ComponentDatabase* getBuildingComponentDb();
    ComponentDatabase* getPipelineComponentDb();    
    ComponentDatabase* getCPTComponentDb();

    ComponentDatabase* getSiteComponentDb();

private:

    static ComponentDatabaseManager *theInstance;

    std::unique_ptr<ComponentDatabase> theBuildingComponentDb;
    std::unique_ptr<ComponentDatabase> thePipelineComponentDb;
    std::unique_ptr<ComponentDatabase> theSiteComponentDb;
    std::unique_ptr<ComponentDatabase> theCPTComponentDb;
};

#endif // COMPONENTDATABASEMANAGER_H
