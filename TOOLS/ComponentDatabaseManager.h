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

    ComponentDatabase *getBuildingComponentDb();

    ComponentDatabase* getPipelineComponentDb();

private:

    static ComponentDatabaseManager *theInstance;

    std::unique_ptr<ComponentDatabase> theBuildingComponentDb;
    std::unique_ptr<ComponentDatabase> thePipelineComponentDb;

};

#endif // COMPONENTDATABASEMANAGER_H
