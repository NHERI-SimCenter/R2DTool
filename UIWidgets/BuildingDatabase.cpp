#include "BuildingDatabase.h"

BuildingDatabase::BuildingDatabase()
{

}


int BuildingDatabase::getNumberOfBuildings()
{
    return buildingsDB.size();
}


void BuildingDatabase::addBuilding(int ID, Building& asset)
{
    buildingsDB.insert(ID, asset);
}


Building& BuildingDatabase::getBuilding(const int ID)
{
  return buildingsDB[ID];
}
