#ifndef BUILDINGDATABASE_H
#define BUILDINGDATABASE_H

#include <QMap>
#include <QVariant>

namespace Esri
{
namespace ArcGISRuntime
{
class Feature;
}
}

struct Building
{
public:

    int ID = -1;

    // The building feature in the GIS widget
    Esri::ArcGISRuntime::Feature* buildingFeature = nullptr;

    // Map to store the building attributes
    QMap<QString, QVariant> buildingAttributes;

    // Map to store the results - the QString (key) is the header text while the double is the value for that building
    QMap<QString, double> ResultsValues;
};


class BuildingDatabase
{
public:
    BuildingDatabase();

    // Gets the building as a modifiable reference
    Building& getBuilding(const int ID);

    int getNumberOfBuildings();

    void addBuilding(int ID, Building& asset);

private:

    QMap<int,Building> buildingsDB;

};

#endif // BUILDINGDATABASE_H
