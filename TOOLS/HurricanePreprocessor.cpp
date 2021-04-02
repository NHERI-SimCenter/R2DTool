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

#include "HurricanePreprocessor.h"
#include "CSVReaderWriter.h"
#include "VisualizationWidget.h"

#include <QProgressBar>
#include <QList>
#include <QApplication>
#include <QObject>

// GIS Layers
#include "Feature.h"
#include "FeatureCollectionLayer.h"
#include "GroupLayer.h"
#include "Layer.h"
#include "LayerListModel.h"
#include "SimpleMarkerSymbol.h"
#include "Polyline.h"
#include "SimpleRenderer.h"
#include "PolygonBuilder.h"

using namespace Esri::ArcGISRuntime;

HurricanePreprocessor::HurricanePreprocessor(QProgressBar* pBar, VisualizationWidget* visWidget, QObject* parent) : theProgressBar(pBar), theVisualizationWidget(visWidget), theParent(parent)
{
    allHurricanesLayer = nullptr;
}


int HurricanePreprocessor::loadHurricaneTrackData(const QString &eventFile, QString &err)
{
    CSVReaderWriter csvTool;

    QVector<QStringList> data = csvTool.parseCSVFile(eventFile, err);

    if(!err.isEmpty())
    {
        return -1;
    }

    if(data.empty())
    {
        err = "Hurricane data is empty";
        return -1;
    }

    // Get the header information to populate the fields
    auto headerData = data.at(0);

    auto numCol = headerData.size();

    // Pop off the first two rows that contain the header and units information
    data.pop_front();
    data.pop_front();


    // Split the hurricanes up as they come in one long list
    QVectorIterator<QStringList> i(data);
    QString SID;

    HurricaneObject hurricane;
    hurricane.parameterLabels = headerData;

    auto indexLandfall = headerData.indexOf("DIST2LAND");
    auto indexSID = headerData.indexOf("SID");

    if(indexLandfall == -1 || indexSID == -1)
    {
        err = "Could not find the required column indexes in the data file";
        return -1;
    }

    // While iterating through the hurricane points, save the data at first landfall
    bool landfallFound = false;
    while (i.hasNext())
    {
        auto row = i.next();

        if(row.size() != numCol)
        {
            err = "Error, inconsistency in the data in the row and number of columns";
            return -1;
        }

        // Not all hurricanes will make landfall
        if(!landfallFound)
        {
            auto distToLand = row.at(indexLandfall);

            // If the distance to land is 0, then this is the first landfall
            if(distToLand.compare("0") == 0)
            {
                landfallFound = true;
                hurricane.landfallData = row;
                hurricane.indexLandfall = hurricane.size();
            }
        }

        auto currSID = row.at(indexSID);

        if(SID.compare(currSID) != 0)
        {
            if(!hurricane.isEmpty())
            {
                hurricanes.push_back(hurricane);
                hurricane.clear();
                landfallFound = false;
            }

            SID = currSID;
        }

        hurricane.push_back(row);
    }

    // Push back the last hurricane
    if(!hurricane.isEmpty())
        hurricanes.push_back(hurricane);

    auto numHurricanes = hurricanes.size();

    theProgressBar->setMinimum(0);
    theProgressBar->setMaximum(numHurricanes);
    theProgressBar->reset();
    QApplication::processEvents();

    // Name and storm ID
    auto indexName = headerData.indexOf("NAME");
    auto indexSeason = headerData.indexOf("SEASON");

    // Check that the indexes are found
    if(indexName == -1 || indexSID == -1 || indexSeason == -1)
    {
        err = "Could not find the required column indexes in the data file";
        return -1;
    }


    // Create the feature collection table/layers
    QList<Field> trackFields;
    trackFields.append(Field::createText("NAME", "NULL",4));
    trackFields.append(Field::createText("SID", "NULL",4));
    trackFields.append(Field::createText("SEASON", "NULL",4));
    trackFields.append(Field::createText("AssetType", "NULL",4));
    trackFields.append(Field::createText("TabName", "NULL",4));
    trackFields.append(Field::createText("UID", "NULL",4));

    auto trackFeatureCollection = new FeatureCollection(theParent);
    auto trackFeatureCollectionTable = new FeatureCollectionTable(trackFields, GeometryType::Polyline, SpatialReference::wgs84(), theParent);
    trackFeatureCollection->tables()->append(trackFeatureCollectionTable);

    allHurricanesLayer = new FeatureCollectionLayer(trackFeatureCollection,theParent);
    allHurricanesLayer->setAutoFetchLegendInfos(true);
    allHurricanesLayer->setName("All Hurricanes");


    auto allHurricanesItem = theVisualizationWidget->addLayerToMap(allHurricanesLayer);

    if(allHurricanesItem == nullptr)
    {
        err = "Error adding item to the map";
        return -1;
    }


    // Create line symbol for the track
    SimpleLineSymbol* lineSymbol = new SimpleLineSymbol(SimpleLineSymbolStyle::Solid,
                                                        QColor(0, 0, 0),
                                                        2.0f /*width*/,
                                                        SimpleLineSymbolMarkerStyle::Arrow,
                                                        SimpleLineSymbolMarkerPlacement::End,
                                                        theParent);

    // Create renderer and set symbol for the track
    SimpleRenderer* lineRenderer = new SimpleRenderer(lineSymbol, theParent);
    lineRenderer->setLabel("Hurricane track");

    // Set the renderer for the feature layer
    trackFeatureCollectionTable->setRenderer(lineRenderer);


    for(int i = 0; i<numHurricanes; ++i)
    {
        theProgressBar->setValue(i);
        QApplication::processEvents();

        // Get the hurricane
        HurricaneObject& hurricane = hurricanes[i];

        auto name = hurricane.front().at(indexName);
        auto SID = hurricane.front().at(indexSID);
        auto season = hurricane.front().at(indexSeason);

        hurricane.name = name;
        hurricane.SID = SID;
        hurricane.season = season;
        auto nameID = name+"-"+season;

        // Create a unique ID for this track
        auto uid = theVisualizationWidget->createUniqueID();

        QMap<QString, QVariant> featureAttributes;
        featureAttributes.insert("NAME",name);
        featureAttributes.insert("SID",SID);
        featureAttributes.insert("SEASON",season);
        featureAttributes.insert("TabName", nameID);
        featureAttributes.insert("AssetType", "HURRICANE");
        featureAttributes.insert("UID", uid);

        auto polyline = this->getTrackGeometry(&hurricane, err);

        if(polyline.isEmpty())
            return -1;

        auto trackFeat = trackFeatureCollectionTable->createFeature(featureAttributes,polyline,theParent);
        trackFeatureCollectionTable->addFeature(trackFeat);
    }

    theVisualizationWidget->zoomToLayer(allHurricanesLayer->layerId());

    return 0;
}


void HurricanePreprocessor::clear(void)
{
    hurricanes.clear();
}



int HurricanePreprocessor::createTrackVisualization(HurricaneObject* hurricane, LayerTreeItem* parentItem, GroupLayer* parentLayer, QString& err)
{
    auto numPnts = hurricane->size();

    if(numPnts == 0)
    {
        err = "Hurricane does not have any track points";
        return -1;
    }

    // Name and storm ID
    auto name = hurricane->name;
    auto SID = hurricane->SID;
    auto season = hurricane->season;
    auto nameID = name+"-"+season;

    auto uid = theVisualizationWidget->createUniqueID();

    QMap<QString, QVariant> featureAttributes;
    featureAttributes.insert("NAME",name);
    featureAttributes.insert("SID",SID);
    featureAttributes.insert("SEASON",season);
    featureAttributes.insert("TabName", nameID);
    featureAttributes.insert("AssetType", "HURRICANE");
    featureAttributes.insert("UID", uid);

    auto polyline = this->getTrackGeometry(hurricane, err);

    if(polyline.isEmpty())
        return -1;

    // Create the feature collection table/layers
    QList<Field> trackFields;
    trackFields.append(Field::createText("NAME", "NULL",4));
    trackFields.append(Field::createText("SID", "NULL",4));
    trackFields.append(Field::createText("SEASON", "NULL",4));
    trackFields.append(Field::createText("AssetType", "NULL",4));
    trackFields.append(Field::createText("TabName", "NULL",4));
    trackFields.append(Field::createText("UID", "NULL",4));

    auto trackFeatureCollection = new FeatureCollection(theParent);
    auto trackFeatureCollectionTable = new FeatureCollectionTable(trackFields, GeometryType::Polyline, SpatialReference::wgs84(), theParent);
    trackFeatureCollection->tables()->append(trackFeatureCollectionTable);

    auto trackLayer = new FeatureCollectionLayer(trackFeatureCollection,theParent);
    trackLayer->setAutoFetchLegendInfos(true);
    trackLayer->setName("Track");

    // Create line symbol for the track
    SimpleLineSymbol* lineSymbol = new SimpleLineSymbol(SimpleLineSymbolStyle::Solid,
                                                        QColor(0, 0, 0),
                                                        2.0f /*width*/,
                                                        SimpleLineSymbolMarkerStyle::Arrow,
                                                        SimpleLineSymbolMarkerPlacement::End,
                                                        theParent);

    // Create renderer and set symbol for the track
    SimpleRenderer* lineRenderer = new SimpleRenderer(lineSymbol, theParent);
    lineRenderer->setLabel("Hurricane track");

    // Set the renderer for the feature layer
    trackFeatureCollectionTable->setRenderer(lineRenderer);

    auto trackFeat = trackFeatureCollectionTable->createFeature(featureAttributes,polyline,theParent);
    trackFeatureCollectionTable->addFeature(trackFeat);

    theVisualizationWidget->addLayerToMap(trackLayer, parentItem, parentLayer);

    return 0;
}


Geometry HurricanePreprocessor::getTrackGeometry(HurricaneObject* hurricane, QString& err)
{
    // By default will use USA_LAT and USA_LON, if not available fall back on the LAT and LON below
    // Get the parameter labels or header data
    auto headerData = hurricane->parameterLabels;

    auto indexUSALat = headerData.indexOf("USA_LAT");
    auto indexUSALon = headerData.indexOf("USA_LON");
    auto indexLat = headerData.indexOf("LAT");
    auto indexLon = headerData.indexOf("LON");

    // Check that the indexes are found
    if(indexLat == -1 || indexLon == -1 || indexUSALat == -1 || indexUSALon == -1 )
    {
        err = "Could not find the required column indexes in the data file";
        return Geometry();
    }

    // Each row is a point on the hurricane track
    PartCollection* trackCollection = new PartCollection(SpatialReference::wgs84(), theParent);
    double latitude = 0.0;
    double longitude = 0.0;

    for(int j = 0; j<hurricane->size(); ++j)
    {
        QStringList trackPoint = (*hurricane)[j];

        Point pointPrev(longitude,latitude);

        // Create the geometry for visualization
        // By default will use USA_LAT and USA_LON, if not available fall back on the LAT and LON below
        latitude = trackPoint.at(indexLat).toDouble();
        longitude = trackPoint.at(indexLon).toDouble();

        //  if(latitude == 0.0 || longitude == 0.0)
        //  {
        //      latitude = trackPoint.at(indexLat).toDouble();
        //      longitude = trackPoint.at(indexLon).toDouble();

        //      if(latitude == 0.0 || longitude == 0.0)
        //      {
        //          err = "Error getting the latitude and longitude";
        //          return -1;
        //      }
        //  }

        Point point(longitude,latitude);

        if(j != 0)
        {
            Part* partj = new Part(SpatialReference::wgs84(), theParent);
            partj->addPoint(point);
            partj->addPoint(pointPrev);

            trackCollection->addPart(partj);
        }
    }

    // Add the points layer
    PolygonBuilder polylineBuilder(SpatialReference::wgs84());
    polylineBuilder.setParts(trackCollection);

    // Create the polyline feature
    auto polyline =  polylineBuilder.toPolyline();

    return polyline;
}


int HurricanePreprocessor::createTrackPointsVisualization(HurricaneObject* hurricane, LayerTreeItem* parentItem, GroupLayer* parentLayer, QString& err)
{
    auto numPnts = hurricane->size();

    if(numPnts == 0)
    {
        err = "Hurricane does not have any track points";
        return -1;
    }

    // Get the parameter labels or header data
    auto headerData = hurricane->parameterLabels;

    // Name and storm ID
    auto indexName = headerData.indexOf("NAME");
    auto indexSID = headerData.indexOf("SID");
    auto indexSeason = headerData.indexOf("SEASON");

    // By default will use USA_LAT and USA_LON, if not available fall back on the LAT and LON below
    auto indexUSALat = headerData.indexOf("USA_LAT");
    auto indexUSALon = headerData.indexOf("USA_LON");
    auto indexLat = headerData.indexOf("LAT");
    auto indexLon = headerData.indexOf("LON");

    // Check that the indexes are found
    if(indexName == -1 || indexSID == -1 || indexLat == -1 || indexLon == -1 || indexSeason == -1 || indexUSALat == -1 || indexUSALon == -1 )
    {
        err = "Could not find the required column indexes in the data file";
        return -1;
    }

    auto name = hurricane->front().at(indexName);
    auto SID = hurricane->front().at(indexSID);
    auto season = hurricane->front().at(indexSeason);

    hurricane->name = name;
    hurricane->SID = SID;
    hurricane->season = season;

    auto nameID = name+"-"+season;

    // Create the layers and tree item

    // Save the pointers to the layer object

    FeatureCollectionLayer* trackPntsLayer = nullptr;
    FeatureCollectionTable* trackPntsTable = nullptr;

    // Create the table to store the fields
    QList<Field> pointFields;

    // Common fields
    pointFields.append(Field::createText("AssetType", "NULL",4));
    pointFields.append(Field::createText("TabName", "NULL",4));
    pointFields.append(Field::createText("UID", "NULL",4));

    // Full fields
    for(auto&& it : headerData)
        pointFields.append(Field::createText(it, "NULL",4));

    // Create the feature collection table/layers
    auto trackPntsFeatureCollection = new FeatureCollection(theParent);
    trackPntsTable = new FeatureCollectionTable(pointFields, GeometryType::Point, SpatialReference::wgs84(), theParent);
    trackPntsFeatureCollection->tables()->append(trackPntsTable);

    trackPntsLayer = new FeatureCollectionLayer(trackPntsFeatureCollection,theParent);
    trackPntsLayer->setAutoFetchLegendInfos(true);
    trackPntsLayer->setName("Track Points");

    // Create cross SimpleMarkerSymbol
    SimpleMarkerSymbol* markerSymbol = new SimpleMarkerSymbol(SimpleMarkerSymbolStyle::Circle, QColor("black"), 6, theParent);

    // Create renderer and set symbol to crossSymbol
    SimpleRenderer* pointRenderer = new SimpleRenderer(markerSymbol, theParent);
    pointRenderer->setLabel("Hurricane track points");

    // Set the renderer for the feature layer
    trackPntsTable->setRenderer(pointRenderer);


    // Each row is a point on the hurricane track
    for(int j = 0; j<numPnts; ++j)
    {

        QStringList trackPoint = (*hurricane)[j];

        //create the feature attributes
        QMap<QString, QVariant> featureAttributes;
        for(int k = 0; k<headerData.size(); ++k)
        {
            if(!trackPoint.at(k).isEmpty())
                featureAttributes.insert(headerData.at(k), trackPoint.at(k));
        }

        auto uid = theVisualizationWidget->createUniqueID();

        featureAttributes.insert("AssetType", "HURRICANE");
        featureAttributes.insert("TabName", "Track Point");
        featureAttributes.insert("UID", uid);

        // Create the geometry for visualization
        // By default will use USA_LAT and USA_LON, if not available fall back on the LAT and LON below
        auto latitude = trackPoint.at(indexLat).toDouble();
        auto longitude = trackPoint.at(indexLon).toDouble();

        //  if(latitude == 0.0 || longitude == 0.0)
        //  {
        //      latitude = trackPoint.at(indexLat).toDouble();
        //      longitude = trackPoint.at(indexLon).toDouble();

        //      if(latitude == 0.0 || longitude == 0.0)
        //      {
        //          err = "Error getting the latitude and longitude";
        //          return -1;
        //      }
        //  }

        Point point(longitude,latitude);

        auto feature = trackPntsTable->createFeature(featureAttributes, point, theParent);
        trackPntsTable->addFeature(feature);
    }


    theVisualizationWidget->addLayerToMap(trackPntsLayer,parentItem, parentLayer);


    return 0;
}


int HurricanePreprocessor::createLandfallVisualization(const double latitude,const double longitude, const QMap<QString, QVariant>& featureAttributes, LayerTreeItem* parentItem, GroupLayer* parentLayer)
{

    QList<Field> pointFields;

    auto fieldKeys = featureAttributes.keys();

    for(auto&& it : fieldKeys)
        pointFields.append(Field::createText(it, "NULL",4));

    // Create the feature collection table/layers
    auto landfallFeatCollection = new FeatureCollection(theParent);
    auto landFallPntTable = new FeatureCollectionTable(pointFields, GeometryType::Point, SpatialReference::wgs84(), theParent);
    landfallFeatCollection->tables()->append(landFallPntTable);

    auto landFallPointLayer = new FeatureCollectionLayer(landfallFeatCollection,theParent);
    landFallPointLayer->setAutoFetchLegendInfos(true);
    landFallPointLayer->setName("Landfall");

    // Create cross SimpleMarkerSymbol
    SimpleMarkerSymbol* landfallMarkerSymbol = new SimpleMarkerSymbol(SimpleMarkerSymbolStyle::Diamond, QColor("blue"), 12, theParent);

    // Create renderer and set symbol to crossSymbol
    SimpleRenderer* landfallPointRenderer = new SimpleRenderer(landfallMarkerSymbol, theParent);
    landfallPointRenderer->setLabel("Hurricane Landfall");

    // Set the renderer for the feature layer
    landFallPntTable->setRenderer(landfallPointRenderer);

    // Create the point geometry
    Point point(longitude,latitude);

    auto feature = landFallPntTable->createFeature(featureAttributes, point, theParent);
    landFallPntTable->addFeature(feature);

    // Add the points layer
    theVisualizationWidget->addLayerToMap(landFallPointLayer,parentItem, parentLayer);

    return 0;
}


HurricaneObject* HurricanePreprocessor::getHurricane(const QString& SID)
{
    for(auto&& it : hurricanes)
    {
        if(SID.compare(it.SID) == 0)
            return &it;
    }

    return nullptr;
}


Esri::ArcGISRuntime::Layer *HurricanePreprocessor::getAllHurricanesLayer() const
{
    return allHurricanesLayer;
}

