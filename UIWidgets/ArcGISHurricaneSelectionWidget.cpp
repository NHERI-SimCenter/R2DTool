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

#include "LayerTreeView.h"
#include "ArcGISHurricaneSelectionWidget.h"

#include "ArcGISVisualizationWidget.h"
#include "VisualizationWidget.h"

#include "WorkflowAppR2D.h"
#include "HurricaneParameterWidget.h"
#include "SimCenterPreferences.h"
#include "SiteConfig.h"
#include "GridNode.h"
#include "NodeHandle.h"
#include "LayerTreeItem.h"
#include "PolygonBoundary.h"
#include "CSVReaderWriter.h"
#include "Utils/PythonProgressDialog.h"

#include "GroupLayer.h"
#include "Feature.h"
#include "FeatureCollection.h"
#include "FeatureCollectionLayer.h"
#include "SimpleRenderer.h"
#include "SimpleFillSymbol.h"
#include "SimpleMarkerSymbol.h"

#include <QApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QComboBox>
#include <QDialog>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QSpinBox>
#include <QSpacerItem>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QDir>

using namespace Esri::ArcGISRuntime;

ArcGISHurricaneSelectionWidget::ArcGISHurricaneSelectionWidget(VisualizationWidget* visWidget, QWidget *parent) : HurricaneSelectionWidget(visWidget, parent)
{
    theVisualizationWidget = static_cast<ArcGISVisualizationWidget*>(visWidget);

    gridLayer = nullptr;
    landfallItem = nullptr;
    hurricaneTrackItem = nullptr;
    hurricaneTrackPointsItem = nullptr;

    hurricaneImportTool = std::make_unique<ArcGISHurricanePreprocessor>(progressBar, theVisualizationWidget, this);

    auto thePolygonBoundaryTool = theVisualizationWidget->getThePolygonBoundaryTool();
    connect(truncTrackApplyButton,SIGNAL(clicked()),thePolygonBoundaryTool,SLOT(getItemsInPolygonBoundary())); // Asynchronous task
    connect(truncTrackSelectButton,&QPushButton::clicked,this,&ArcGISHurricaneSelectionWidget::handleSelectAreaMap);
    connect(truncTrackClearButton,&QPushButton::clicked,this,&ArcGISHurricaneSelectionWidget::handleClearSelectAreaMap);
    connect(browseTerrainButton,&QPushButton::clicked,this,&ArcGISHurricaneSelectionWidget::handleTerrainImport);

    selectedHurricaneFeature = nullptr;
    selectedHurricaneLayer = nullptr;
    selectedHurricaneItem = nullptr;
}


ArcGISHurricaneSelectionWidget::~ArcGISHurricaneSelectionWidget()
{

}


int ArcGISHurricaneSelectionWidget::importHurricaneTrackData(const QString &eventFile, QString &err)
{
    return hurricaneImportTool->loadHurricaneTrackData(eventFile,err);
}


void ArcGISHurricaneSelectionWidget::clear(void)
{

    HurricaneSelectionWidget::clear();

    hurricaneImportTool->clear();

    delete selectedHurricaneLayer;
    selectedHurricaneLayer = nullptr;
    selectedHurricaneItem = nullptr;
    hurricaneTrackPointsItem = nullptr;
    hurricaneTrackItem = nullptr;

}


void ArcGISHurricaneSelectionWidget::handleHurricaneSelect(void)
{
    auto selectedFeatures = theVisualizationWidget->getSelectedFeaturesList();

    if(selectedFeatures.empty())
        return;

    QString hurricaneSID;

    // Only select the first hurricane, if there is more than one selected
    for(auto&& it : selectedFeatures)
    {
        auto attrbList = it->attributes();

        auto featType = attrbList->attributeValue("AssetType");

        if(featType.toString() != "HURRICANE")
            continue;

        if(selectedHurricaneFeature == it)
            return;
        else
            selectedHurricaneFeature = it;

        auto hurricaneName = attrbList->attributeValue("NAME").toString();
        hurricaneSID = attrbList->attributeValue("SID").toString();
        auto hurricaneSeason = attrbList->attributeValue("SEASON").toString();

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
    auto allHurricanesLayer = hurricaneImportTool->getAllHurricanesLayer();
    theVisualizationWidget->setLayerVisibility(allHurricanesLayer->layerId(), false);
    theVisualizationWidget->clearSelection();
}


void ArcGISHurricaneSelectionWidget::createHurricaneVisuals(HurricaneObject* hurricane)
{
    // Get the hurricane description
    auto name = hurricane->name;
    auto landfallData = hurricane->landfallData;

    if(selectedHurricaneLayer == nullptr)
    {
        selectedHurricaneLayer = new GroupLayer(QList<Layer*>{},this);
        selectedHurricaneLayer->setName("Hurricane " + name);
        selectedHurricaneLayer->setAutoFetchLegendInfos(true);

        selectedHurricaneItem = theVisualizationWidget->addSelectedFeatureLayerToMap(selectedHurricaneLayer);
    }

    // Track
    QString err;
    hurricaneTrackItem = hurricaneImportTool->createTrackVisualization(hurricane,selectedHurricaneItem,selectedHurricaneLayer, err);
    if(hurricaneTrackItem == nullptr)
    {
        this->errorMessage(err);
        return;
    }

    // Track points
    hurricaneTrackPointsItem = hurricaneImportTool->createTrackPointsVisualization(hurricane,selectedHurricaneItem, selectedHurricaneLayer,err);
    if(hurricaneTrackPointsItem == nullptr)
    {
        this->errorMessage(err);
        return;
    }

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

        landfallItem = hurricaneImportTool->createLandfallVisualization(lat,lon,featureAttributes, selectedHurricaneItem, selectedHurricaneLayer);
    }

    selectedHurricaneLayer->load();

    theVisualizationWidget->zoomToLayer(selectedHurricaneLayer->layerId());
}


void ArcGISHurricaneSelectionWidget::handleGridSelected(void)
{

    // Create the objects needed to visualize the grid in the GIS
    auto siteGrid = mapViewSubWidget->getGrid();

    if(!siteGrid->isVisible())
        return;

    // Get the vector of grid nodes
    auto gridNodeVec = siteGrid->getGridNodeVec();

    if(gridNodeVec.isEmpty())
        return;

    // Create the table to store the fields
    QList<Field> tableFields;
    tableFields.append(Field::createText("AssetType", "NULL",4));
    tableFields.append(Field::createText("TabName", "NULL",4));
    tableFields.append(Field::createText("Station Name", "NULL",4));
    tableFields.append(Field::createText("Latitude", "NULL",8));
    tableFields.append(Field::createText("Longitude", "NULL",9));
    tableFields.append(Field::createText("Peak Wind Speeds", "NULL",9));

    auto gridFeatureCollection = new FeatureCollection(this);

    // Create the feature collection table/layers
    auto gridFeatureCollectionTable = new FeatureCollectionTable(tableFields, GeometryType::Point, SpatialReference::wgs84(), this);
    gridFeatureCollection->tables()->append(gridFeatureCollectionTable);

    gridLayer = new FeatureCollectionLayer(gridFeatureCollection,this);
    gridLayer->setAutoFetchLegendInfos(true);
    gridLayer->setName("Wind Field Grid");

    // Create red cross SimpleMarkerSymbol
    SimpleMarkerSymbol* crossSymbol = new SimpleMarkerSymbol(SimpleMarkerSymbolStyle::Cross, QColor("black"), 6, this);

    // Create renderer and set symbol to crossSymbol
    SimpleRenderer* renderer = new SimpleRenderer(crossSymbol, this);
    renderer->setLabel("Windfield Grid Point");

    // Set the renderer for the feature layer
    gridFeatureCollectionTable->setRenderer(renderer);

    QStringList headerRow = {"GP_file", "Latitude", "Longitude"};
    gridData.push_back(headerRow);

    for(int i = 0; i<gridNodeVec.size(); ++i)
    {
        auto gridNode = gridNodeVec.at(i);

        // The station id
        auto stationName = QString::number(i+1);

        auto screenPoint = gridNode->getPoint();

        // The latitude and longitude
        auto longitude = theVisualizationWidget->getLongFromScreenPoint(screenPoint);
        auto latitude = theVisualizationWidget->getLatFromScreenPoint(screenPoint);

        WindFieldStation station(stationName,latitude,longitude);

        // create the feature attributes
        QMap<QString, QVariant> featureAttributes;
        featureAttributes.insert("Station Name", stationName);
        featureAttributes.insert("AssetType", "WindfieldGridPoint");
        featureAttributes.insert("TabName", "Wind Field Grid Point");
        featureAttributes.insert("Latitude", latitude);
        featureAttributes.insert("Longitude", longitude);
        featureAttributes.insert("Peak Wind Speeds", "N/A");

        // Create the point and add it to the feature table
        Point point(longitude,latitude);
        Feature* feature = gridFeatureCollectionTable->createFeature(featureAttributes, point, this);

        station.setStationFeature(feature);

        gridFeatureCollectionTable->addFeature(feature);

        QStringList stationRow;
        stationRow.push_back(stationName);
        stationRow.push_back(QString::number(latitude));
        stationRow.push_back(QString::number(longitude));

        stationMap.insert(stationName,station);

        gridData.push_back(stationRow);
    }


    // Create a new layer
    LayerTreeView *layersTreeView = theVisualizationWidget->getLayersTree();

    // Check if there is a 'User Ground Motions' root item in the tree
    auto hurricaneMainItem = layersTreeView->getTreeItem("Hurricanes", nullptr);

    // If there is no item, create one
    if(hurricaneMainItem == nullptr)
    {
        auto gridID = theVisualizationWidget->createUniqueID();
        hurricaneMainItem = layersTreeView->addItemToTree("Hurricanes", gridID);
    }

    // Add the event layer to the layer tree
    // auto eventID = theVisualizationWidget->createUniqueID();
    // auto eventItem = layersTreeView->addItemToTree("Windfield Grid", eventID, hurricaneGridItem);

    // Add the event layer to the map
    theVisualizationWidget->addLayerToMap(gridLayer,hurricaneMainItem);

    mapViewSubWidget->removeGridFromScene();
}


void ArcGISHurricaneSelectionWidget::handleLandfallPointSelected(void)
{

    // Create the objects needed to visualize the grid in the GIS
    auto landfallPoint = mapViewSubWidget->getPoint();

    if(!landfallPoint->isVisible())
        return;

    // Get the vector of grid nodes
    auto posNodeVec = landfallPoint->pos();

    if(posNodeVec.isNull())
        return;

    // The latitude and longitude
    auto longitude = theVisualizationWidget->getLongFromScreenPoint(posNodeVec);
    auto latitude = theVisualizationWidget->getLatFromScreenPoint(posNodeVec);

    hurricaneParamsWidget->setLandfallLat(latitude);
    hurricaneParamsWidget->setLandfallLon(longitude);

    // create the feature attributes
    QMap<QString, QVariant> featureAttributes;
    featureAttributes.insert("Station Name", "Landfall");
    featureAttributes.insert("AssetType", "HURRICANE_LANDFALL");
    featureAttributes.insert("TabName", "Landfall Point");
    featureAttributes.insert("Latitude", latitude);
    featureAttributes.insert("Longitude", longitude);


    // Create a new hurricane layer
    if(selectedHurricaneLayer == nullptr)
    {
        auto name =selectedHurricaneObj.name;

        if(name.isEmpty())
            name = "USER SPECIFIED";

        selectedHurricaneLayer = new GroupLayer(QList<Layer*>{},this);
        selectedHurricaneLayer->setName("Hurricane " + name);
        selectedHurricaneLayer->setAutoFetchLegendInfos(true);

        selectedHurricaneItem = theVisualizationWidget->addSelectedFeatureLayerToMap(selectedHurricaneLayer);
    }


    landfallItem = hurricaneImportTool->createLandfallVisualization(latitude,longitude,featureAttributes, selectedHurricaneItem,selectedHurricaneLayer);

    mapViewSubWidget->removePointFromScene();

    theVisualizationWidget->hideLegend();
}


void ArcGISHurricaneSelectionWidget::clearGridFromMap(void)
{
    if(gridLayer)
    {
        LayerTreeView *layersTreeView = theVisualizationWidget->getLayersTree();
        layersTreeView->removeItemFromTree(gridLayer->layerId());

        delete gridLayer;
        gridLayer = nullptr;
    }

    stationMap.clear();
    gridData.clear();
    mapViewSubWidget->removeGridFromScene();
}


void ArcGISHurricaneSelectionWidget::clearLandfallFromMap(void)
{
    if(landfallItem)
    {
        LayerTreeView *layersTreeView = theVisualizationWidget->getLayersTree();
        auto res = layersTreeView->removeItemFromTree(landfallItem->getItemID());

        if(res == false)
            qDebug()<<"Error removing landfall item from tree";

        landfallItem = nullptr;

        hurricaneParamsWidget->setLandfallLat(0.0);
        hurricaneParamsWidget->setLandfallLon(0.0);

        // Check if the selected hurricane item is still there...
        if(selectedHurricaneLayer)
            selectedHurricaneItem = theVisualizationWidget->getLayersTree()->getTreeItem(selectedHurricaneLayer->layerId());

        if(selectedHurricaneItem == nullptr)
        {
            delete selectedHurricaneLayer;
            selectedHurricaneLayer = nullptr;
        }
    }

    mapViewSubWidget->removePointFromScene();
}


void ArcGISHurricaneSelectionWidget::handleTerrainImport(void)
{
    // Extract the features
    auto featureArray = HurricaneSelectionWidget::getTerrainData();

    auto numFeat = featureArray.size();

    if(numFeat == 0)
        return;

    std::vector<Esri::ArcGISRuntime::Geometry> geomVec;
    std::vector<QVariantMap> propertiesMapVec;

    for(auto&& it : featureArray)
    {
        auto featObj = it.toObject();

        auto geom = featObj["geometry"].toObject();

        auto coordArray = geom["coordinates"].toArray();

        auto featGeom = theVisualizationWidget->getPolygonGeometryFromJson(coordArray);

        if(featGeom.isEmpty())
        {
            QString msg ="Error getting the hurricane geometry in terrain import";
            this->errorMessage(msg);
            return;
        }

        geomVec.push_back(featGeom);

        auto properties = featObj["properties"].toObject();
        propertiesMapVec.push_back(properties.toVariantMap());
    }

    QList<Field> tableFields;
    tableFields.append(Field::createText("AssetType", "NULL",4));
    tableFields.append(Field::createText("TabName", "NULL",4));

    // Use the property map from the first object to populate the fields. This assumes that all features have the same properties.

    if(!propertiesMapVec.empty())
    {
        auto keys = propertiesMapVec.begin()->keys();
        for(auto&& it : keys)
            tableFields.append(Field::createText(it, "NULL",4));
    }

    auto featureCollection = new FeatureCollection(this);

    auto featureCollectionTable = new FeatureCollectionTable(tableFields, GeometryType::Polygon, SpatialReference::wgs84(),this);

    featureCollection->tables()->append(featureCollectionTable);

    auto newGeojsonLayer = new FeatureCollectionLayer(featureCollection,this);

    newGeojsonLayer->setName("Terrain Roughness");
    newGeojsonLayer->setAutoFetchLegendInfos(true);

    SimpleFillSymbol* fillSymbol = new SimpleFillSymbol(SimpleFillSymbolStyle::Solid, QColor(0, 255, 0, 75), this);
    SimpleRenderer* lineRenderer = new SimpleRenderer(fillSymbol, this);
    lineRenderer->setLabel("Terrain roughness polygon");

    featureCollectionTable->setRenderer(lineRenderer);

    if(propertiesMapVec.size() != geomVec.size())
    {
        QString msg ="Error, inconsistency in geometry and properties vector size";
        this->errorMessage(msg);
        return;
    }

    for(size_t i = 0;i<propertiesMapVec.size(); ++i)
    {
        auto geom = geomVec.at(i);
        auto prop = propertiesMapVec.at(i);

        QMap<QString, QVariant> featureAttributes;
        featureAttributes.insert("AssetType", "HURRICANE_TERRAIN_GEOJSON");
        featureAttributes.insert("TabName", "Terrain Polygon");

        featureAttributes.insert(prop);

        auto feature = featureCollectionTable->createFeature(featureAttributes, geom, this);

        featureCollectionTable->addFeature(feature);
    }


    // Create a new hurricane layer
    if(selectedHurricaneLayer == nullptr)
    {
        auto name =selectedHurricaneObj.name;

        if(name.isEmpty())
            name = "USER SPECIFIED";

        selectedHurricaneLayer = new GroupLayer(QList<Layer*>{},this);
        selectedHurricaneLayer->setName("Hurricane " + name);
        selectedHurricaneLayer->setAutoFetchLegendInfos(true);

        selectedHurricaneItem = theVisualizationWidget->addSelectedFeatureLayerToMap(selectedHurricaneLayer);
    }

    theVisualizationWidget->addLayerToMap(newGeojsonLayer,selectedHurricaneItem, selectedHurricaneLayer);

}


void ArcGISHurricaneSelectionWidget::handleClearSelectAreaMap(void)
{
    auto thePolygonBoundaryTool = theVisualizationWidget->getThePolygonBoundaryTool();
    thePolygonBoundaryTool->resetPolygonBoundary();
    disconnect(theVisualizationWidget,&ArcGISVisualizationWidget::taskSelectionComplete,this,&ArcGISHurricaneSelectionWidget::handleAreaSelected);
}


void ArcGISHurricaneSelectionWidget::handleSelectAreaMap(void)
{
    connect(theVisualizationWidget,&ArcGISVisualizationWidget::taskSelectionComplete,this,&ArcGISHurricaneSelectionWidget::handleAreaSelected);

    auto thePolygonBoundaryTool = theVisualizationWidget->getThePolygonBoundaryTool();

    thePolygonBoundaryTool->getPolygonBoundaryInputs();
}


void ArcGISHurricaneSelectionWidget::handleAreaSelected(void)
{
    // Get the features from the selection query
    auto selectedFeatures = theVisualizationWidget->getFeaturesFromQueryList();

    HurricaneObject newHurricaneObj = selectedHurricaneObj;

    QVector<QStringList>& hurricaneData = newHurricaneObj.getHurricaneData();

    hurricaneData.clear();

    // Save only the features that are track points
    QList<Feature*> featureList;
    for(auto&& it : selectedFeatures)
    {
        FeatureIterator iter = it->iterator();
        while (iter.hasNext())
        {
            Feature* feature = iter.next();

            auto atrbList = feature->attributes();

            auto artbMap = atrbList->attributesMap();

            auto assetType = artbMap.value("AssetType").toString();

            if(assetType.compare("HURRICANE_TRACK_POINT") == 0)
                featureList.push_back(feature);

        }
    }

    if(featureList.empty())
        return;

    for(auto&& it : featureList)
    {
        auto atrbList = it->attributes();

        auto artbMap = atrbList->attributesMap();

        auto lat = artbMap.value("LAT").toDouble();
        auto lon = artbMap.value("LON").toDouble();

        QStringList trackPoint = selectedHurricaneObj.trackPointAtLatLon(lat,lon);

        if(trackPoint.isEmpty())
        {
            this->errorMessage("Could not get the track point");
            return;
        }

        hurricaneData.push_back(trackPoint);
    }

    // Delete the old hurricane layer
    if(hurricaneTrackItem != nullptr)
    {
        theVisualizationWidget->removeLayerFromMapAndTree(hurricaneTrackItem->getItemID());
        hurricaneTrackItem = nullptr;

        theVisualizationWidget->removeLayerFromMapAndTree(hurricaneTrackPointsItem->getItemID());
        hurricaneTrackPointsItem = nullptr;
    }

    // Check if the selected hurricane item is still there...
    if(selectedHurricaneLayer)
        selectedHurricaneItem = theVisualizationWidget->getLayersTree()->getTreeItem(selectedHurricaneLayer->layerId());

    if(selectedHurricaneItem == nullptr)
    {
        delete selectedHurricaneLayer;
        selectedHurricaneLayer = nullptr;
    }

    // Create the new hurricane layer
    this->createHurricaneVisuals(&newHurricaneObj);

    disconnect(theVisualizationWidget,&ArcGISVisualizationWidget::taskSelectionComplete,this,&ArcGISHurricaneSelectionWidget::handleAreaSelected);

    selectedHurricaneObj = newHurricaneObj;
}
