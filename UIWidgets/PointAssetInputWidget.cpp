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
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
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

#include "PointAssetInputWidget.h"
#include "QGISVisualizationWidget.h"
#include "ComponentTableView.h"
#include "ComponentDatabaseManager.h"
#include "AssetFilterDelegate.h"

#include <qgsfield.h>
#include <qgsfillsymbol.h>
#include <qgsvectorlayer.h>
#include <qgsmarkersymbol.h>

PointAssetInputWidget::PointAssetInputWidget(QWidget *parent, VisualizationWidget* visWidget, QString componentType, QString appType) : AssetInputWidget(parent, visWidget, componentType, appType)
{
    theComponentDb = ComponentDatabaseManager::getInstance()->getBuildingComponentDb();
}


int PointAssetInputWidget::loadAssetVisualization()
{
    // Create the building attributes that are fixed
    QgsFields featFields;
    featFields.append(QgsField("ID", QVariant::Int));
    featFields.append(QgsField("AssetType", QVariant::String));
    featFields.append(QgsField("TabName", QVariant::String));

    // Set the table headers as fields in the table
    for(int i = 1; i<componentTableWidget->columnCount(); ++i)
    {
        auto fieldText = componentTableWidget->horizontalHeaderItemVariant(i);
        featFields.append(QgsField(fieldText.toString(),fieldText.type()));
    }

    QList<QgsField> attribFields;
    for(int i = 0; i<featFields.size(); ++i)
        attribFields.push_back(featFields[i]);

    auto headers = this->getTableHorizontalHeadings();

    // First check if a footprint was provided
    auto indexFootprint = headers.indexOf("Footprint");
    auto indexLatitude = theVisualizationWidget->getIndexOfVal(headers, "latitude");
    auto indexLongitude = theVisualizationWidget->getIndexOfVal(headers, "longitude");

    if(indexLongitude == -1 || indexLatitude == -1)
    {
        this->errorMessage("Could not find latitude and longitude in the header columns");
        return -1;
    }

    // Get the number of rows
    auto nRows = componentTableWidget->rowCount();

    QString layerType;

    if(indexFootprint != -1)
        layerType = "polygon";
    else
        layerType = "point";

    // Create the buildings layer
    mainLayer = theVisualizationWidget->addVectorLayer(layerType,"All Buildings");

    if(mainLayer == nullptr)
    {
        this->errorMessage("Error adding a vector layer");
        return -1;
    }

    auto pr = mainLayer->dataProvider();

    mainLayer->startEditing();

    auto res = pr->addAttributes(attribFields);

    if(!res)
        this->errorMessage("Error adding attributes to the layer" + mainLayer->name());

    mainLayer->updateFields(); // tell the vector layer to fetch changes from the provider

    theComponentDb->setMainLayer(mainLayer);

    filterDelegateWidget  = new AssetFilterDelegate(mainLayer);

    auto numAtrb = attribFields.size();

    for(int i = 0; i<nRows; ++i)
    {
        // create the feature attributes
        QgsAttributes featureAttributes(numAtrb);

        // Create a new building
        QString buildingIDStr = componentTableWidget->item(i,0).toString();

        int buildingID = buildingIDStr.toInt();

        // Create a unique ID for the building
//        auto uid = theVisualizationWidget->createUniqueID();

        //  "ID"
        //  "AssetType"
        //  "TabName"

        featureAttributes[0] = QVariant(buildingID);
        featureAttributes[1] = QVariant("BUILDINGS");
        featureAttributes[2] = QVariant(buildingID);

        // The feature attributes are the columns from the table
        for(int j = 1; j<componentTableWidget->columnCount(); ++j)
        {
            auto attrbVal = componentTableWidget->item(i,j);
            featureAttributes[2+j] = attrbVal;
        }

        auto latitude = componentTableWidget->item(i,indexLatitude).toDouble();
        auto longitude = componentTableWidget->item(i,indexLongitude).toDouble();

        QgsFeature feature;
        feature.setFields(featFields);

        // If a footprint is given use that
        if(indexFootprint != -1)
        {
            QString footprint = componentTableWidget->item(i,indexFootprint).toString();

            if(footprint.compare("NA") == 0)
            {
                feature.setGeometry(QgsGeometry::fromPointXY(QgsPointXY(longitude,latitude)));
            }
            else
            {
                auto geom = theVisualizationWidget->getPolygonGeometryFromJson(footprint);
                if(geom.isEmpty())
                {
                    this->errorMessage("Error getting the building footprint geometry");
                    return -1;
                }

                feature.setGeometry(geom);
            }
        }
        else
        {
            QgsPointXY(longitude,latitude);
            auto geom = QgsGeometry::fromPointXY(QgsPointXY(longitude,latitude));
            if(geom.isEmpty())
            {
                this->errorMessage("Error getting the building footprint geometry");
                return -1;
            }

            feature.setGeometry(geom);
        }

        feature.setAttributes(featureAttributes);

        if(!feature.isValid())
            return -1;


        auto res = pr->addFeature(feature, QgsFeatureSink::FastInsert);
        if(!res)
        {
            this->errorMessage("Error adding the feature to the layer");
            return -1;
        }

        //        auto id = feature.id();

        //        qDebug()<<id;
    }

    mainLayer->commitChanges(true);
    mainLayer->updateExtents();

    QgsSymbol* markerSymbol = nullptr;

    if(indexFootprint != -1)
        markerSymbol = new QgsFillSymbol();
    else
        markerSymbol = new QgsMarkerSymbol();

    // Try to categorize the layer by occupancy class
    auto attrName = "OccupancyClass";
    auto indexOcc = headers.indexOf(attrName);
    if(indexOcc != -1)
    {
        theVisualizationWidget->createCategoryRenderer(attrName, mainLayer, markerSymbol);
    }
    else // Else default to a single color fill for all features
    {
        markerSymbol->setColor(Qt::gray);
        theVisualizationWidget->createSimpleRenderer(markerSymbol,mainLayer);
    }

//    auto numFeat = mainLayer->featureCount();

    theVisualizationWidget->zoomToLayer(mainLayer);

    auto layerId = mainLayer->id();

    theVisualizationWidget->registerLayerForSelection(layerId,this);

    selectedFeaturesLayer = theVisualizationWidget->addVectorLayer(layerType,"Selected Buildings");

    if(selectedFeaturesLayer == nullptr)
    {
        this->errorMessage("Error adding the selected assets vector layer");
        return -1;
    }

    QgsSymbol* selectedLayerMarkerSymbol = nullptr;

    if(indexFootprint != -1)
        selectedLayerMarkerSymbol = new QgsFillSymbol();
    else
        selectedLayerMarkerSymbol = new QgsMarkerSymbol();

    selectedLayerMarkerSymbol->setColor(Qt::yellow);
    theVisualizationWidget->createSimpleRenderer(selectedLayerMarkerSymbol,selectedFeaturesLayer);

    auto pr2 = selectedFeaturesLayer->dataProvider();

    auto res2 = pr2->addAttributes(attribFields);

    if(!res2)
        this->errorMessage("Error adding attributes to the layer");

    selectedFeaturesLayer->updateFields(); // tell the vector layer to fetch changes from the provider

    theComponentDb->setSelectedLayer(selectedFeaturesLayer);

    QVector<QgsMapLayer*> mapLayers;
    mapLayers.push_back(selectedFeaturesLayer);
    mapLayers.push_back(mainLayer);

    theVisualizationWidget->createLayerGroup(mapLayers,"Buildings");

    return 0;
}



void PointAssetInputWidget::clear()
{    
//    if(selectedFeaturesLayer != nullptr)
//    {
//        theVisualizationWidget->removeLayer(selectedFeaturesLayer);
//        theVisualizationWidget->deregisterLayerForSelection(selectedFeaturesLayer->id());
//    }
//    if(mainLayer != nullptr)
//        theVisualizationWidget->removeLayer(mainLayer);

    AssetInputWidget::clear();
}



