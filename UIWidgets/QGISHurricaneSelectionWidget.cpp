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

#include "QGISHurricaneSelectionWidget.h"
#include "QGISVisualizationWidget.h"
#include "SimCenterMapcanvasWidget.h"
#include "HurricaneParameterWidget.h"

#include "NodeHandle.h"
#include "GridNode.h"
#include "RectangleGrid.h"

#include <QPushButton>
#include <QJsonArray>
#include <QLabel>
#include <QLineEdit>

#include <qgsmapcanvas.h>
#include <qgsvectorlayer.h>

QGISHurricaneSelectionWidget::QGISHurricaneSelectionWidget(VisualizationWidget* visWidget, QWidget *parent) : HurricaneSelectionWidget(visWidget, parent)
{
    theVisualizationWidget = static_cast<QGISVisualizationWidget*>(visWidget);

    gridLayer = nullptr;
    terrainRoughnessLayer = nullptr;
    landfallLayer = nullptr;
    hurricaneTrackLayer = nullptr;
    hurricaneTrackPointsLayer = nullptr;

    connect(truncTrackApplyButton,&QPushButton::clicked,this,&QGISHurricaneSelectionWidget::handleAreaSelected);
    connect(truncTrackSelectButton,&QPushButton::clicked,this,&QGISHurricaneSelectionWidget::handleSelectAreaMap);
    connect(truncTrackClearButton,&QPushButton::clicked,this,&QGISHurricaneSelectionWidget::handleClearSelectAreaMap);
    connect(browseTerrainButton,&QPushButton::clicked,this,&QGISHurricaneSelectionWidget::handleTerrainImport);

    hurricaneImportTool = std::make_unique<QGISHurricanePreprocessor>(progressBar, theVisualizationWidget, this);

    // Test
    //    trackLineEdit->setText("/Users/steve/Desktop/SimCenter/Examples/HurricaneExample/wind_input/Track.csv");
    //    this->handleHurricaneTrackImport();
}


QGISHurricaneSelectionWidget::~QGISHurricaneSelectionWidget()
{

}


QgsVectorLayer* QGISHurricaneSelectionWidget::importHurricaneTrackData(const QString &eventFile, QString &err)
{
    auto allHurricanesLayer = hurricaneImportTool->loadHurricaneDatabaseData(eventFile,err);

    // Set as the current layer so selection of tracks will register
    mapViewSubWidget->setCurrentLayer(allHurricanesLayer);

    return allHurricanesLayer;
}

void QGISHurricaneSelectionWidget::clear(void)
{
    HurricaneSelectionWidget::clear();

    hurricaneImportTool->clear();

    theVisualizationWidget->removeLayer(gridLayer);
    theVisualizationWidget->removeLayer(landfallLayer);
    theVisualizationWidget->removeLayer(hurricaneTrackLayer);
    theVisualizationWidget->removeLayer(hurricaneTrackPointsLayer);
    theVisualizationWidget->removeLayer(terrainRoughnessLayer);

    terrainRoughnessLayer = nullptr;
    gridLayer = nullptr;
    landfallLayer = nullptr;
    hurricaneTrackLayer = nullptr;
    hurricaneTrackPointsLayer = nullptr;
}


void QGISHurricaneSelectionWidget::handleHurricaneSelect(void)
{
    auto selectedHurricanes = mapViewSubWidget->getSelectedIds();

    if(selectedHurricanes.empty())
        return;

    auto allHurricanesLayer = hurricaneImportTool->getAllHurricanesLayer();
    auto selectedFeatIt = allHurricanesLayer->getFeatures(selectedHurricanes);

    QString hurricaneSID;

    QgsFeature feat;
    while (selectedFeatIt.nextFeature(feat))
    {

        auto assetType = feat.attribute("AssetType").toString();

        if(assetType != "HURRICANE")
            continue;

        if(selectedHurricaneFeature == feat)
            return;
        else
            selectedHurricaneFeature = feat;

        auto hurricaneName = feat.attribute("NAME").toString();
        hurricaneSID = feat.attribute("SID").toString();
        auto hurricaneSeason = feat.attribute("SEASON").toString();

        selectedHurricaneName->setText(hurricaneName);
        selectedHurricaneSID->setText(hurricaneSID);
        selectedHurricaneSeason->setText(hurricaneSeason);

        break;
    }


    // Get the selected hurricane from the preprocessor
    auto importedHurricane = hurricaneImportTool->getHurricane(hurricaneSID);

    if(importedHurricane == nullptr)
    {
        QString err = "Could not find the hurricane with the SID " + hurricaneSID;
        qDebug()<<err;
        return;
    }

    selectedHurricaneObj = *importedHurricane;

    // Populate the landfall parameters
    auto pressure = selectedHurricaneObj.getPressureAtLandfall();
    auto lat = selectedHurricaneObj.getLatitudeAtLandfall();
    auto lon = selectedHurricaneObj.getLongitudeAtLandfall();
    auto stormDir = selectedHurricaneObj.getLandingAngle();
    auto stormSpeed = selectedHurricaneObj.getStormSpeedAtLandfall();
    auto radius = selectedHurricaneObj.getRadiusAtLandfall();

    if(lat == 0.0 || lon == 0.0)
    {
        QString err = "The latitude and/or longitude should not be 0";
        qDebug()<<err;
    }

    if(radius == 0.0)
    {
        QString warn = "Warning: The storm radius is 0.0 from the database, assuming a radius of 50 nmile";
        qDebug()<<warn;
        radius = 50.0;
    }

    hurricaneParamsWidget->setLandfallLat(lat);
    hurricaneParamsWidget->setLandfallLon(lon);
    hurricaneParamsWidget->setLandfallPress(pressure);
    hurricaneParamsWidget->setLandingAngle(stormDir);
    hurricaneParamsWidget->setLandfallSpeed(stormSpeed);
    hurricaneParamsWidget->setLandfallRadius(radius);

    this->createHurricaneVisuals(&selectedHurricaneObj);

    // Set the all hurricanes layer to off
    theVisualizationWidget->setLayerVisibility(allHurricanesLayer, false);
    theVisualizationWidget->clearSelection();
    mapViewSubWidget->setCurrentLayer(nullptr);
}


int QGISHurricaneSelectionWidget::createHurricaneVisuals(HurricaneObject* hurricane)
{
    // Get the hurricane description
    auto name = hurricane->name;

    auto layerGroupName = "Hurricane " + name;

    QVector<QgsMapLayer*> layerGroup;

    // Track
    QString err;
    hurricaneTrackLayer = hurricaneImportTool->createTrackVisualization(hurricane, err);
    if(hurricaneTrackLayer == nullptr)
    {
        this->errorMessage(err);
        return -1;
    }

    layerGroup.push_back(hurricaneTrackLayer);

    // Track points
    hurricaneTrackPointsLayer = hurricaneImportTool->createTrackPointsVisualization(hurricane, err);
    if(hurricaneTrackPointsLayer == nullptr)
    {
        this->errorMessage(err);
        return -1;
    }

    layerGroup.push_back(hurricaneTrackPointsLayer);

    auto landfallData = hurricane->landfallData;

    auto lat = hurricane->getLatitudeAtLandfall();
    auto lon = hurricane->getLongitudeAtLandfall();

    // Landfall
    if(!landfallData.empty())
    {
        auto lfParams = hurricane->parameterLabels;

        QMap<QString, QVariant> featureAttributes;
        for(int i = 0; i < landfallData.size(); ++i)
        {
            auto dataVal = landfallData.at(i);

            if(!dataVal.isEmpty())
                featureAttributes.insert(lfParams.at(i), landfallData.at(i));
        }

        featureAttributes.insert("Station Name", "Landfall");
        featureAttributes.insert("AssetType", "HURRICANE_LANDFALL");
        featureAttributes.insert("TabName", "Landfall Point");
        featureAttributes.insert("Latitude", lat);
        featureAttributes.insert("Longitude", lon);

        landfallLayer = hurricaneImportTool->createLandfallVisualization(lat, lon, featureAttributes);

        if(landfallLayer == nullptr)
        {
            err = "Error creating the landfall layer";
            this->errorMessage(err);
            return -1;
        }

        layerGroup.push_back(landfallLayer);
    }

    theVisualizationWidget->createLayerGroup(layerGroup,layerGroupName);

    theVisualizationWidget->zoomToLayer(hurricaneTrackLayer);

    return 0;
}


void QGISHurricaneSelectionWidget::handleGridSelected(void)
{

    if(!userGrid->isVisible())
        return;

    // Get the vector of grid nodes
    auto gridNodeVec = userGrid->getGridNodeVec();

    if(gridNodeVec.isEmpty())
        return;

    auto mapCanvas = mapViewSubWidget->mapCanvas();

    // Create the fields
    QgsFields featFields;
    featFields.append(QgsField("AssetType", QVariant::String));
    featFields.append(QgsField("TabName", QVariant::String));
    featFields.append(QgsField("Station Name", QVariant::String));
    featFields.append(QgsField("Latitude", QVariant::Double));
    featFields.append(QgsField("Longitude", QVariant::Double));
    featFields.append(QgsField("Peak Wind Speeds", QVariant::String));

    QList<QgsField> attribFields;
    for(int i = 0; i<featFields.size(); ++i)
        attribFields.push_back(featFields[i]);

    QgsFeatureList featureList;

    QStringList headerRow = {"GP_file", "Latitude", "Longitude"};
    gridData.push_back(headerRow);

    for(int i = 0; i<gridNodeVec.size(); ++i)
    {
        auto gridNode = gridNodeVec.at(i);

        // The station id
        auto stationName = QString::number(i+1);

        auto screenPoint = gridNode->getPoint();

        // The latitude and longitude
        auto longitude = theVisualizationWidget->getLongFromScreenPoint(screenPoint,mapCanvas);
        auto latitude = theVisualizationWidget->getLatFromScreenPoint(screenPoint,mapCanvas);

        WindFieldStation station(stationName,latitude,longitude);

        // create the feature attributes
        QgsAttributes featAttributes(attribFields.size());

        featAttributes[0] = "HurricaneGridPoint"; // AssetType
        featAttributes[1] = "Hurricane Grid Point"; // TabName
        featAttributes[2] = stationName; // Station Name
        featAttributes[3] = latitude; // Latitude
        featAttributes[4] = longitude; // Longitude
        featAttributes[5] = "N/A"; // Peak Wind Speeds

        // Create the point and add it to the feature table
        // Create the point and add it to the feature table
        QgsFeature feature;
        feature.setFields(featFields);
        feature.setGeometry(QgsGeometry::fromPointXY(QgsPointXY(longitude,latitude)));
        feature.setAttributes(featAttributes);
        featureList.append(feature);

        station.setStationFeature(feature);

        QStringList stationRow;
        stationRow.push_back(stationName);
        stationRow.push_back(QString::number(latitude));
        stationRow.push_back(QString::number(longitude));

        stationMap.insert(stationName,station);

        gridData.push_back(stationRow);
    }


    gridLayer = theVisualizationWidget->addVectorLayer("Point", "Hurricane Grid");

    if(gridLayer == nullptr)
    {
        this->errorMessage("Error creating a layer");
        return;
    }


    auto dProvider = gridLayer->dataProvider();
    auto res = dProvider->addAttributes(attribFields);

    if(!res)
    {
        this->errorMessage("Error adding attribute fields to layer");
        theVisualizationWidget->removeLayer(gridLayer);
        return;
    }

    gridLayer->updateFields(); // tell the vector layer to fetch changes from the provider

    dProvider->addFeatures(featureList);
    gridLayer->updateExtents();

    theVisualizationWidget->createSymbolRenderer(Qgis::MarkerShape::Cross,Qt::black,2.0,gridLayer);

    progressLabel->setVisible(false);

    userGrid->hide();

    // Enable selection
    mapViewSubWidget->enableSelectionTool();
}


void QGISHurricaneSelectionWidget::handleLandfallPointSelected(void)
{
    // Create the objects needed to visualize the grid in the GIS
    if(!userPoint->isVisible())
        return;

    // Get the vector of grid nodes
    auto posNodeVec = userPoint->pos();

    if(posNodeVec.isNull())
        return;

    // The latitude and longitude
    auto longitude = theVisualizationWidget->getLongFromScreenPoint(posNodeVec,mapViewSubWidget->mapCanvas());
    auto latitude = theVisualizationWidget->getLatFromScreenPoint(posNodeVec,mapViewSubWidget->mapCanvas());

    hurricaneParamsWidget->setLandfallLat(latitude);
    hurricaneParamsWidget->setLandfallLon(longitude);

    // create the feature attributes
    QMap<QString, QVariant> featureAttributes;
    featureAttributes.insert("Station Name", "Landfall");
    featureAttributes.insert("AssetType", "HURRICANE_LANDFALL");
    featureAttributes.insert("TabName", "Landfall Point");
    featureAttributes.insert("Latitude", latitude);
    featureAttributes.insert("Longitude", longitude);

    landfallLayer = hurricaneImportTool->createLandfallVisualization(latitude,longitude,featureAttributes);

    userPoint->hide();
}


void QGISHurricaneSelectionWidget::clearGridFromMap(void)
{
    if(gridLayer)
    {
        theVisualizationWidget->removeLayer(gridLayer);

        gridLayer = nullptr;
    }

    stationMap.clear();
    gridData.clear();
    userGrid->hide();
    mapViewSubWidget->enableSelectionTool();

}


void QGISHurricaneSelectionWidget::clearLandfallFromMap(void)
{
    if(landfallLayer)
    {
        theVisualizationWidget->removeLayer(landfallLayer);

        landfallLayer = nullptr;

        hurricaneParamsWidget->setLandfallLat(0.0);
        hurricaneParamsWidget->setLandfallLon(0.0);
    }

    userPoint->hide();
    mapViewSubWidget->enableSelectionTool();
}


void QGISHurricaneSelectionWidget::handleTerrainImport(void)
{
    // Extract the features
    auto pathToFile = HurricaneSelectionWidget::getTerrainGeojsonPath();

    if(pathToFile.isEmpty())
        return;

    terrainRoughnessLayer = theVisualizationWidget->addVectorLayer(pathToFile,"Terrain Roughness", "ogr");

    terrainRoughnessLayer->setOpacity(0.4);

    if(terrainRoughnessLayer == nullptr)
        this->errorMessage("Failed to load terrain roughness layer");

    terrainRoughnessLayer->setOpacity(0.4);

}


void QGISHurricaneSelectionWidget::handleClearSelectAreaMap(void)
{
    mapViewSubWidget->enableSelectionTool();

    if(hurricaneTrackPointsLayer)
        hurricaneTrackPointsLayer->removeSelection();

    mapViewSubWidget->mapCanvas()->setCurrentLayer(nullptr);
}


void QGISHurricaneSelectionWidget::handleSelectAreaMap(void)
{
    mapViewSubWidget->enablePolygonSelectionTool();

    // Set as the current layer so selection of tracks will register
    mapViewSubWidget->mapCanvas()->setCurrentLayer(hurricaneTrackPointsLayer);

}


void QGISHurricaneSelectionWidget::handleAreaSelected(void)
{

    if(selectedHurricaneObj.empty() || hurricaneTrackPointsLayer == nullptr)
        return;

    HurricaneObject newHurricaneObj = selectedHurricaneObj;

    QVector<QStringList>& hurricaneData = newHurricaneObj.getHurricaneData();

    hurricaneData.clear();

    // Save only the features that are track points
    QgsFeatureIterator it = hurricaneTrackPointsLayer->getSelectedFeatures();
    QgsFeature feat;
    QList<QgsFeature> featureList;
    while (it.nextFeature(feat))
    {
        featureList.append(feat);
    }

    if(featureList.empty())
        return;


    std::sort(featureList.begin(),
              featureList.end(),
              [](QgsFeature a, QgsFeature b) {return a.id() > b.id(); });

    for(auto&& it : featureList)
    {
        auto lat = it.attribute("LAT").toDouble();
        auto lon = it.attribute("LON").toDouble();

        QStringList trackPoint = selectedHurricaneObj.trackPointAtLatLon(lat,lon);

        if(trackPoint.isEmpty())
        {
            this->errorMessage("Could not get the track point");
            return;
        }

        hurricaneData.push_back(trackPoint);
    }

    // Delete the old hurricane layer(s)
    if(hurricaneTrackLayer != nullptr)
    {
        theVisualizationWidget->removeLayer(hurricaneTrackLayer);
        hurricaneTrackLayer = nullptr;
    }

    if(hurricaneTrackPointsLayer != nullptr)
    {
        theVisualizationWidget->removeLayer(hurricaneTrackPointsLayer);
        hurricaneTrackPointsLayer = nullptr;
    }

    // Create the new hurricane layer
    this->createHurricaneVisuals(&newHurricaneObj);
}


int QGISHurricaneSelectionWidget::updateGridLayerFeatures(QgsFeatureList& featList)
{
    auto res = gridLayer->dataProvider()->truncate();

    if(!res)
        return -1;

    res = gridLayer->dataProvider()->addFeatures(featList);

    if(!res)
        return -1;

    gridLayer->updateExtents();

    return 0;
}


bool QGISHurricaneSelectionWidget::outputToJSON(QJsonObject &jsonObject)
{
    QJsonObject unitsObj;

    unitsObj["PWS"] = "mph";

    jsonObject["Units"] = unitsObj;

    return true;
}
