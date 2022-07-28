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
    ComponentDatabase* getAssetDb(const QString& type);

    // Creates a new component database given the string type
    ComponentDatabase* createAssetDb(const QString& type);

    // Returns all of the component databases that have been created, i.e., that are not null ptrs
    QList<ComponentDatabase*> getAllAssetDatabases();

private:

    static ComponentDatabaseManager *theInstance;

    QMap<QString, ComponentDatabase*> dbMap;
};

#endif // COMPONENTDATABASEMANAGER_H
