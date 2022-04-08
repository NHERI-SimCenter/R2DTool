#ifndef HousingUnitAllocationWidget_H
#define HousingUnitAllocationWidget_H
/* *****************************************************************************
Copyright (c) 2016-2021, The Regents of the University of California (Regents).
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the FreeBSD Project.

REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS
PROVIDED "AS IS". REGENTS HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT,
UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

*************************************************************************** */

// Written by: Stevan Gavrilovic

#include "SimCenterAppWidget.h"

#include <qgsfeature.h>
#include <QMap>

class QgsVectorLayer;
class QGISVisualizationWidget;
class VisualizationWidget;

struct Building;
struct CensusBlockGroup;

struct Parcel
{
    Parcel() {}

    QgsFeature parcelFeat;

    QgsFeatureId getFeatureId(){return parcelFeat.id();}

    std::vector<std::shared_ptr<Building>> associatedBuildings;
};


struct Building
{
    Building() {}

    QgsFeature buildingFeat;
    QgsPointXY buildingCentroidXY;

    QgsFeatureId getFeatureId(){return buildingFeat.id();}

    std::shared_ptr<Parcel> associatedParcel;
};


// Struct to store block information
struct CensusBlock
{
    CensusBlock() {}

    QgsFeature blockFeat;

    QgsFeatureId getFeatureId(){return blockFeat.id();}

    std::shared_ptr<CensusBlockGroup> associatedCensusBlockGroup;
    std::vector<std::shared_ptr<Parcel>> associatedParcels;
};


// Struct to store block group information
struct CensusBlockGroup
{
    CensusBlockGroup() {}

    QgsFeature blockGroupFeat;

    QgsFeatureId getFeatureId(){return blockGroupFeat.id();}

    std::vector<std::shared_ptr<CensusBlock>> associatedBlocks;
};


struct HousingUnit
{
    HousingUnit() {}

    QgsFeature housingUnitFeat;

    QgsFeatureId getFeatureId(){return housingUnitFeat.id();}

    std::shared_ptr<Building> associatedBuilding;
};

class HousingUnitAllocationWidget : public  SimCenterAppWidget
{
    Q_OBJECT

public:
    explicit HousingUnitAllocationWidget(QWidget *parent, VisualizationWidget* visWidget);
    virtual ~HousingUnitAllocationWidget();

    bool outputAppDataToJSON(QJsonObject &jsonObject) override;
    bool inputAppDataFromJSON(QJsonObject &jsonObject) override;

    int createInputFiles();

    int importJoinAssets();

    void clear() override;

signals:

    void emitErrorMsg(QString);
    void emitStatusMsg(QString);
    void emitInfoMsg(QString);

private slots:

    void handleErrorMsg(const QString msg);
    void handleStatusMsg(const QString msg);
    void handleInfoMsg(const QString msg);

private:

    std::unique_ptr<std::thread> thread;

    QgsVectorLayer* parcelsLayer = nullptr;

    // Mutex to sample the building layer
    std::mutex buildingLayerSemaphore;
    QgsVectorLayer* buildingsLayer = nullptr;

    QgsVectorLayer* addressPointLayer = nullptr;

    QgsVectorLayer* blockLayer = nullptr;
    QgsVectorLayer* blockGroupLayer = nullptr;

    QGISVisualizationWidget* theVisualizationWidget = nullptr;

//    QVector<std::shared_ptr<Building>> buildingsMap;
//    QVector<std::shared_ptr<Parcel>> parcelsMap;

    QMap<QgsFeatureId,std::shared_ptr<Building>> buildingsMap;
    QMap<QgsFeatureId,std::shared_ptr<Parcel>> parcelsMap;
    QMap<QgsFeatureId,std::shared_ptr<HousingUnit>> housingUnitMap;

    int getBuildingFeatures(void);
    int getParcelFeatures(void);

    int linkBuildingsAndParcels(void);

    int extractCensusData(void);

};

#endif // HousingUnitAllocationWidget_H
