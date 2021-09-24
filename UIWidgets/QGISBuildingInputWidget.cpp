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

#include "QGISBuildingInputWidget.h"
#include "QGISVisualizationWidget.h"
#include "ComponentDatabaseManager.h"
#include "ComponentTableView.h"

#include <qgsfield.h>
#include <qgsfillsymbol.h>
#include <qgsvectorlayer.h>
#include <qgsmarkersymbol.h>

QGISBuildingInputWidget::QGISBuildingInputWidget(QWidget *parent, VisualizationWidget* visWidget, QString componentType, QString appType) : ComponentInputWidget(parent, visWidget, componentType, appType)
{
    theComponentDb = ComponentDatabaseManager::getInstance()->getBuildingComponentDb();
}


int QGISBuildingInputWidget::loadComponentVisualization()
{

    // Create the building attributes that are fixed
    QList<QgsField> attrib;
    attrib.append(QgsField("LossRatio", QVariant::Double));
    attrib.append(QgsField("ID", QVariant::Int));
    attrib.append(QgsField("AssetType", QVariant::String));
    attrib.append(QgsField("UID", QVariant::String));

    // Set the table headers as fields in the table
    for(int i = 1; i<componentTableWidget->columnCount(); ++i)
    {
        auto fieldText = componentTableWidget->horizontalHeaderItemVariant(i);

        attrib.append(QgsField(fieldText.toString(),fieldText.type()));
    }

    // Create the buildings group layer that will hold the sublayers
    auto buildingLayer = theVisualizationWidget->addVectorLayer("polygon","Buildings");

    if(buildingLayer == nullptr)
    {
        this->errorMessage("Error adding a vector layer");
        return -1;
    }

    auto nRows = componentTableWidget->rowCount();

    auto pr = buildingLayer->dataProvider();

    auto res = pr->addAttributes(attrib);

    if(!res)
        this->errorMessage("Error adding attributes to the layer");

    buildingLayer->updateFields(); // tell the vector layer to fetch changes from the provider

    auto headers = this->getTableHorizontalHeadings();

    // First check if a footprint was provided
    auto indexFootprint = headers.indexOf("Footprint");
    auto indexLatitude = headers.indexOf("Latitude");
    auto indexLongitude = headers.indexOf("Longitude");

    if(indexLongitude == -1 || indexLatitude == -1)
    {
        this->errorMessage("Could not find latitude and longitude in the header columns");
        return -1;
    }

    auto numAtrb = attrib.size();

    QgsFeatureList featList;

    featList.reserve(nRows);

    for(int i = 0; i<nRows; ++i)
    {
        // create the feature attributes
        QgsAttributes featureAttributes(numAtrb);
        QMap<QString, QVariant> buildingAttributeMap;

        // Create a new building
        Component building;

        QString buildingIDStr = componentTableWidget->item(i,0).toString();

        int buildingID = buildingIDStr.toInt();

        building.ID = buildingID;

        // Create a unique ID for the building
        auto uid = theVisualizationWidget->createUniqueID();

        // "LossRatio"
        // "ID"
        // "AssetType"
        // "UID"

        featureAttributes[0] = QVariant(0.0);
        featureAttributes[1] = QVariant(buildingIDStr);
        featureAttributes[2] = QVariant("BUILDINGS");
        featureAttributes[3] = QVariant(uid);

        // The feature attributes are the columns from the table
        for(int j = 1; j<componentTableWidget->columnCount(); ++j)
        {
            auto attrbText = componentTableWidget->horizontalHeaderItem(j);
            auto attrbVal = componentTableWidget->item(i,j);

            buildingAttributeMap.insert(attrbText,attrbVal.toString());

            featureAttributes[3+j] = attrbVal;
        }

        building.ComponentAttributes = buildingAttributeMap;

        auto latitude = componentTableWidget->item(i,indexLatitude).toDouble();
        auto longitude = componentTableWidget->item(i,indexLongitude).toDouble();

        QgsFeature feature;

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

        building.UID = uid;
        building.ComponentFeature = &feature;

        theComponentDb->addComponent(buildingID, building);

        featList.push_back(feature);
    }

    pr->addFeatures(featList);

    buildingLayer->updateExtents();

    auto attrName = "OccupancyClass";

    auto indexOcc = headers.indexOf(attrName);

    if(indexOcc != -1)
    {
        QgsSymbol* markerSymbol = nullptr;

        if(indexFootprint != -1)
            markerSymbol = new QgsFillSymbol();
        else
            markerSymbol = new QgsMarkerSymbol();

        theVisualizationWidget->createCategoryRenderer(attrName, buildingLayer, markerSymbol);

    }

    theVisualizationWidget->zoomToLayer(buildingLayer);

    // Create the selected building layer
    selectedBuildingsLayer = new QgsVectorLayer("polygon","Selected Buildings","memory");

    QgsFillSymbol* fillSymbol = new QgsFillSymbol();
    fillSymbol->setColor(Qt::yellow);
    theVisualizationWidget->createSimpleRenderer(fillSymbol,selectedBuildingsLayer);

    if(selectedBuildingsLayer == nullptr)
    {
        this->errorMessage("Error adding the selected assets vector layer");
        return -1;
    }

    auto pr2 = selectedBuildingsLayer->dataProvider();

    auto res2 = pr2->addAttributes(attrib);

    if(!res2)
        this->errorMessage("Error adding attributes to the layer");

    buildingLayer->updateFields(); // tell the vector layer to fetch changes from the provider

    return 0;
}


bool QGISBuildingInputWidget::addFeatureToSelectedLayer(QgsFeature& feature)
{
    auto res = selectedBuildingsLayer->addFeature(feature);

    return res;
}


int QGISBuildingInputWidget::removeFeatureFromSelectedLayer(QgsFeature& feat)
{
    //    selectedBuildingsTable->deleteFeature(feat);

    return 0;
}


//SimpleRenderer* QGISBuildingInputWidget::createBuildingRenderer(void)
//{
//    SimpleFillSymbol* fillSymbol = new SimpleFillSymbol(SimpleFillSymbolStyle::Solid, QColor(0, 0, 255, 125), this);

//    SimpleRenderer* lineRenderer = new SimpleRenderer(fillSymbol, this);

//    lineRenderer->setLabel("Building footprint");

//    return lineRenderer;
//}


QgsVectorLayer* QGISBuildingInputWidget::getSelectedFeatureLayer(void)
{
    return selectedBuildingsLayer;
}


void QGISBuildingInputWidget::clear()
{    
    delete selectedBuildingsLayer;

    selectedBuildingsLayer = nullptr;

    ComponentInputWidget::clear();
}



