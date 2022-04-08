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

#include "HousingUnitAllocationWidget.h"

#include "AssetInputDelegate.h"
#include "ComponentTableView.h"
#include "ComponentTableModel.h"
#include "ComponentDatabaseManager.h"
#include <QDir>
#include <QApplication>
#include <QLineEdit>
#include <QLabel>
#include <QFileInfo>
#include <QHeaderView>

#include "QGISVisualizationWidget.h"

#include <qgsfillsymbol.h>
#include <qgsmarkersymbol.h>
#include <qgslinesymbol.h>
#include <qgsgeometryengine.h>
#include <qgsproject.h>

// Test to remove start
#include <chrono>
#include <thread>
#include <future>
using namespace std::chrono;
// Test to remove end


HousingUnitAllocationWidget::HousingUnitAllocationWidget(QWidget *parent, VisualizationWidget* visWidget) : SimCenterAppWidget(parent)
{
    theVisualizationWidget=static_cast<QGISVisualizationWidget*>(visWidget);

    this->setContentsMargins(0,0,0,0);

    connect(this,&HousingUnitAllocationWidget::emitStatusMsg,this,&HousingUnitAllocationWidget::handleStatusMsg);
    connect(this,&HousingUnitAllocationWidget::emitErrorMsg,this,&HousingUnitAllocationWidget::handleErrorMsg);
    connect(this,&HousingUnitAllocationWidget::emitInfoMsg,this,&HousingUnitAllocationWidget::handleInfoMsg);

    //this->createInputFiles();
}


HousingUnitAllocationWidget::~HousingUnitAllocationWidget()
{

}


int HousingUnitAllocationWidget::createInputFiles()
{
    auto addressPointGISFile =  "/Users/steve/Desktop/AnchorageTestbed/Addresses_Hosted/Address_Point.shp";

    addressPointLayer = theVisualizationWidget->addVectorLayer(addressPointGISFile, "Address Points", "ogr");

    auto censusBlocksGISFile = "/Users/steve/Desktop/AnchorageTestbed/AlaskaCensus/AkBlocks2020/Blocks2020.gpkg";

    blockLayer = theVisualizationWidget->addVectorLayer(censusBlocksGISFile, "Census Blocks", "ogr");
    blockLayer->setCrs(QgsCoordinateReferenceSystem("EPSG:3857"));
    blockLayer->setOpacity(0.50);

    auto censusBlockGroupsGISFile = "/Users/steve/Desktop/AnchorageTestbed/AlaskaCensus/ALASKA_INCOME_BG/AKBGIncome2019.gpkg";

    blockGroupLayer = theVisualizationWidget->addVectorLayer(censusBlockGroupsGISFile, "Census Block Groups", "ogr");
    blockGroupLayer->setCrs(QgsCoordinateReferenceSystem("EPSG:3857"));
    blockGroupLayer->setOpacity(0.50);

    auto parcelGISFile =  "/Users/steve/Desktop/AnchorageTestbed/AnchorageTaxParcels/AnchorageTaxParcels.gpkg";
    parcelsLayer = theVisualizationWidget->addVectorLayer(parcelGISFile, "Tax Parcels", "ogr");
    parcelsLayer->setOpacity(0.50);
    parcelsLayer->setCrs(QgsCoordinateReferenceSystem("EPSG:3857"));

    auto buildingsGISFile =  "/Users/steve/Desktop/AnchorageTestbed/AnchorageBuildings/Buildings.gpkg";

    auto origBuildingLayer = theVisualizationWidget->addVectorLayer(buildingsGISFile, "Buildings", "ogr");

    if(origBuildingLayer == nullptr)
    {
        this->errorMessage("Error importing the building layer");
        return -1;
    }

    origBuildingLayer->setCrs(QgsCoordinateReferenceSystem("EPSG:3857"));

    // Create a duplicate layer since we will be modifying it
    buildingsLayer = theVisualizationWidget->duplicateExistingLayer(origBuildingLayer);

    if(buildingsLayer == nullptr)
    {
        this->errorMessage("Error copying the building layer");
        return -1;
    }

    this->statusMessage("Starting extraction of census data,  linking buildings to parcels,  and addresses to parcels.  Process running in the background and it may take a while.");
    QApplication::processEvents();

    thread.release();
    thread = std::make_unique<std::thread>(&HousingUnitAllocationWidget::importJoinAssets, this);

    return 0;
}


int HousingUnitAllocationWidget::importJoinAssets()
{
    auto future1 = std::async(&HousingUnitAllocationWidget::extractCensusData, this);

    if(future1.get() != 0)
    {
        emit emitErrorMsg("Error extracting census data");
        return -1;
    }

    auto future2 = std::async(&HousingUnitAllocationWidget::getBuildingFeatures, this);

    if(future2.get() != 0)
    {
        emit emitErrorMsg("Error getting the building features");
        return -1;
    }

    auto future3 = std::async(&HousingUnitAllocationWidget::getParcelFeatures, this);

    if(future3.get() != 0)
    {
        emit emitErrorMsg("Error getting the parcel features");
        return -1;
    }

    auto future4 = std::async(&HousingUnitAllocationWidget::linkBuildingsAndParcels, this);

    if(future4.get() != 0)
    {
        emit emitErrorMsg("Error getting the parcel features");
        return -1;
    }

    emit emitStatusMsg("Done import...");
    QApplication::processEvents();

    return 0;
}

bool HousingUnitAllocationWidget::outputAppDataToJSON(QJsonObject &jsonObject)
{


    return true;
}


bool HousingUnitAllocationWidget::inputAppDataFromJSON(QJsonObject &jsonObject)
{



    return true;
}


int HousingUnitAllocationWidget::getBuildingFeatures(void)
{
    emit emitStatusMsg("Parsing buildings.");

    buildingLayerSemaphore.lock();

    if(buildingsLayer == nullptr)
    {
        emit emitErrorMsg("Error: no buildings layer");
        buildingLayerSemaphore.unlock();

        return -1;
    }

    auto features = buildingsLayer->getFeatures();

    //buildingsMap.reserve(buildingsLayer->featureCount());

    QgsFeature feat;
    while (features.nextFeature(feat))
    {
        auto newBuilding = std::make_shared<Building>();

        // Create a deep copy
        newBuilding->buildingFeat = QgsFeature(feat);

        auto buildGeom = feat.geometry().get()->clone();

        auto buildCentroid = buildGeom->centroid();

        newBuilding->buildingCentroidXY = buildCentroid;

        auto featId = feat.id();
        buildingsMap.insert(featId,newBuilding);

       // buildingsMap.push_back(newBuilding);
    }

    emit emitStatusMsg("Loaded "+QString::number(buildingsLayer->featureCount())+" buildings");

    buildingLayerSemaphore.unlock();

    return 0;
}


int HousingUnitAllocationWidget::getParcelFeatures(void)
{
    emit emitStatusMsg("Parsing parcels.");

    if(parcelsLayer == nullptr)
    {
        emit emitErrorMsg("Error: no parcels layer");
        return -1;
    }

    // Coordinate transform to transform parcel layer into the building layers coordinate system
    buildingLayerSemaphore.lock();
    QgsCoordinateTransform ct(parcelsLayer->crs(), buildingsLayer->crs(), QgsProject::instance());
    buildingLayerSemaphore.unlock();

    auto features = parcelsLayer->getFeatures();

//    parcelsMap.reserve(parcelsLayer->featureCount());

    QgsFeature feat;
    while (features.nextFeature(feat))
    {
        auto newParcel = std::make_shared<Parcel>();

        auto geom = feat.geometry();
        geom.get()->transform(ct);
        feat.setGeometry(QgsGeometry(geom.get()->clone()));

        newParcel->parcelFeat = QgsFeature(feat);

        auto featId = feat.id();
        parcelsMap.insert(featId,newParcel);

        // parcelsMap.push_back(newParcel);
    }

    emit emitStatusMsg("Loaded "+QString::number(parcelsLayer->featureCount())+" parcels");


    return 0;
}


int HousingUnitAllocationWidget::linkBuildingsAndParcels(void)
{

    // Test to remove start
    auto start = high_resolution_clock::now();
    // Test to remove end


    //auto buildingsMapCpy = buildingsMap;
    //auto pacrcelsMapCpy = buildingsMap;

    emit emitStatusMsg("Linking buildings to parcels.");

    if(buildingsMap.isEmpty() || parcelsMap.isEmpty())
    {
        emit emitErrorMsg("Error: empty buildings or parcels vector");
        return -1;
    }

    auto countFound = 0;
    auto countNotFound = 0;

    //    QMapIterator<QgsFeatureId,std::shared_ptr<Building>> buildingIt(buildingsMap);
    //    while (buildingIt.hasNext())

    for(auto&& buildObj : buildingsMap)
    {
        //        if(count == 1000)
        //            break;

        // buildingIt.next();
        //        std::shared_ptr<Building> buildObj = buildingIt.value();

        auto buildCentroidXY = buildObj->buildingCentroidXY;

        // auto buildGeom = buildObj->buildingFeat.geometry();
        //        std::unique_ptr< QgsGeometryEngine > polygonGeometryEngine(QgsGeometry::createGeometryEngine(buildGeom.constGet()));

        //        polygonGeometryEngine->prepareGeometry();

        //        if(!polygonGeometryEngine->isValid())
        //        {
        //            emit emitStatusMsg("Error: the geometry engine is not valid for feature "+QString::number(buildingIt.key()));
        //            return -1;
        //        }

        bool found = false;

        for(auto&& parcel : parcelsMap)
        {
            auto parcelGeom = parcel->parcelFeat.geometry();

            // Do initial bounding box check which is very fast to exclude points that are far away
            auto bb = parcelGeom.boundingBox();

            if(bb.contains(buildCentroidXY))
            {
                //                if(polygonGeometryEngine->intersects(parcelGeom.constGet()))
                //                {
                // Associate the parcel with the building and vice versa
                parcel->associatedBuildings.push_back(buildObj);
                buildObj->associatedParcel = parcel;
                found = true;
                ++countFound;
                break;
                //                }
            }
        }

        if(!found)
        {
            emit emitInfoMsg("Warning: could not find a parcel for building "+QString::number(buildObj->buildingFeat.id()));
            ++countNotFound;
            // return -1;
        }
    }

    emit emitStatusMsg("Done linking buildings to parcels.");

    // Test to remove start
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
    emit emitStatusMsg("Duration linking buildings to parcels: " + QString::number(duration.count()/1000.0) + " seconds");
    // Test to remove end


    return 0;
}


int HousingUnitAllocationWidget::extractCensusData(void)
{
    emit emitStatusMsg("Extracting census data.");

    // Lock access to the building layer
    std::unique_lock<std::mutex> lck (buildingLayerSemaphore);

    if(blockLayer == nullptr || buildingsLayer == nullptr)
    {
        emit emitErrorMsg("Error in extracting census data. nullptr received.");
        return -1;
    }

    auto blockLayerFields = blockLayer->fields();

    QStringList fieldNames;
    for(int i = 0; i<blockLayerFields.count(); ++i)
    {
        auto field = blockLayerFields.at(i);
        auto fieldName = field.name();

        if(fieldName.compare("fid") == 0)
            continue;

        fieldNames.append("BLOCKLAYER_"+fieldName);
    }

    std::vector<QgsFeature> blockFeatVec;
    blockFeatVec.reserve(blockLayer->featureCount());

    auto blockFeatures = blockLayer->getFeatures();
    QgsFeature blockFeat;

    // Coordinate transformation to transform from the block layer crs to the building layer crs - in the case where they are different
    if(buildingsLayer->crs() != blockLayer->crs())
    {
        QgsCoordinateTransform coordTrans(blockLayer->crs(), buildingsLayer->crs(), QgsProject::instance());
        while (blockFeatures.nextFeature(blockFeat))
        {
            auto geom = blockFeat.geometry();
            geom.get()->transform(coordTrans);
            blockFeat.setGeometry(geom);
            blockFeatVec.push_back(blockFeat);
        }
    }
    else
    {
        while (blockFeatures.nextFeature(blockFeat))
            blockFeatVec.push_back(blockFeat);
    }

    auto numFeat = buildingsLayer->featureCount();

    if(numFeat == 0)
    {
        emit emitErrorMsg("Error, number of features is zero in the buildings layer "+buildingsLayer->name());
        return -1;
    }

    QVector< QgsAttributes > fieldAttributes(numFeat, QgsAttributes(fieldNames.size()));

    // Need to return the features with ascending ids so that when we set the updated features to the layer, things will be in order
    QgsFeatureRequest featRequest;
    QgsFeatureRequest::OrderByClause orderByClause(QString("id"),true);
    QList<QgsFeatureRequest::OrderByClause> obcList = {orderByClause};
    QgsFeatureRequest::OrderBy orderBy(obcList);
    featRequest.setOrderBy(orderBy);

    auto buildingFeatures = buildingsLayer->getFeatures(featRequest);
    QgsFeature buildingFeat;
    int count = 0;
    while (buildingFeatures.nextFeature(buildingFeat))
    {
        // Get the building centroid
        auto buildCentroidPoint = buildingFeat.geometry().centroid().asPoint();

        // Did we find the information that we are looking for
        bool found = false;

        //Iterate through the blockLayer features
        for(auto&& it : blockFeatVec)
        {
            auto blockGeom = it.geometry();

            // Do bounding box check which is very fast to check if building is within a block group
            auto bb = blockGeom.boundingBox();
            if(bb.contains(buildCentroidPoint))
            {
                QgsAttributes featAtrbs = it.attributes();

                auto fidIndx = it.fieldNameIndex("fid");

                if(fidIndx != 0)
                {
                    emit emitErrorMsg("Error getting the fid index for  "+QString::number(buildingFeat.id()));
                    return -1;
                }

                for(int i = 0; i<featAtrbs.size()-1; ++i)
                {
                    auto atrb = featAtrbs.at(i+1);
                    fieldAttributes[count][i] = atrb;
                }

                found = true;
                break;
            }
        }

        if(!found)
        {
            emit emitErrorMsg("Error: could not find a census block for the feature  "+QString::number(buildingFeat.id()));
            return -1;
        }

        ++count;
    }

    if(fieldNames.size() != fieldAttributes.front().size())
    {
        emit emitErrorMsg("Error: inconsistency between the field names and number of attributes  "+QString::number(buildingFeat.id()));
        return -1;
    }

    QString errMsg;
    auto res = theVisualizationWidget->addNewFeatureAttributesToLayer(buildingsLayer,fieldNames,fieldAttributes,errMsg);
    if(res != 0)
    {
        emit emitErrorMsg(errMsg);
        return -1;
    }

    emit emitStatusMsg("Done extracting census data.");

    return 0;
}


void HousingUnitAllocationWidget::clear()
{

}

void HousingUnitAllocationWidget::handleErrorMsg(const QString msg)
{
    this->errorMessage(msg);
}


void HousingUnitAllocationWidget::handleStatusMsg(const QString msg)
{
    this->statusMessage(msg);
}


void HousingUnitAllocationWidget::handleInfoMsg(const QString msg)
{
    this->infoMessage(msg);
}
