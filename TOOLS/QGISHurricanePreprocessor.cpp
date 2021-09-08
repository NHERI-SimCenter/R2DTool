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

#include "QGISHurricanePreprocessor.h"
#include "CSVReaderWriter.h"
#include "QGISVisualizationWidget.h"

#include <QProgressBar>
#include <QList>
#include <QApplication>
#include <QObject>


QGISHurricanePreprocessor::QGISHurricanePreprocessor(QProgressBar* pBar, QGISVisualizationWidget* visWidget, QObject* parent) : theProgressBar(pBar), theVisualizationWidget(visWidget), theParent(parent)
{
    allHurricanesLayer = nullptr;
}


int QGISHurricanePreprocessor::loadHurricaneTrackData(const QString &eventFile, QString &err)
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
            if(!hurricane.empty())
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
    if(!hurricane.empty())
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

//    // Create the feature collection table/layers
//    QList<Field> trackFields;
//    trackFields.append(Field::createText("NAME", "NULL",4));
//    trackFields.append(Field::createText("SID", "NULL",4));
//    trackFields.append(Field::createText("SEASON", "NULL",4));
//    trackFields.append(Field::createText("AssetType", "NULL",4));
//    trackFields.append(Field::createText("TabName", "NULL",4));
//    trackFields.append(Field::createText("UID", "NULL",4));

//    auto trackFeatureCollection = new FeatureCollection(theParent);
//    auto trackFeatureCollectionTable = new FeatureCollectionTable(trackFields, GeometryType::Polyline, SpatialReference::wgs84(), theParent);
//    trackFeatureCollection->tables()->append(trackFeatureCollectionTable);

//    allHurricanesLayer = new FeatureCollectionLayer(trackFeatureCollection,theParent);
//    allHurricanesLayer->setName("All Hurricanes");


//    auto allHurricanesItem = theVisualizationWidget->addLayerToMap(allHurricanesLayer);

//    if(allHurricanesItem == nullptr)
//    {
//        err = "Error adding item to the map";
//        return -1;
//    }


//    // Create line symbol for the track
//    SimpleLineSymbol* lineSymbol = new SimpleLineSymbol(SimpleLineSymbolStyle::Solid,
//                                                        QColor(0, 0, 0),
//                                                        2.0f /*width*/,
//                                                        SimpleLineSymbolMarkerStyle::Arrow,
//                                                        SimpleLineSymbolMarkerPlacement::End,
//                                                        theParent);

//    // Create renderer and set symbol for the track
//    SimpleRenderer* lineRenderer = new SimpleRenderer(lineSymbol, theParent);
//    lineRenderer->setLabel("Hurricane track");

//    // Set the renderer for the feature layer
//    trackFeatureCollectionTable->setRenderer(lineRenderer);


//    for(int i = 0; i<numHurricanes; ++i)
//    {
//        theProgressBar->setValue(i);
//        QApplication::processEvents();

//        // Get the hurricane
//        HurricaneObject& hurricane = hurricanes[i];

//        auto name = hurricane.front().at(indexName);
//        auto SID = hurricane.front().at(indexSID);
//        auto season = hurricane.front().at(indexSeason);

//        hurricane.name = name;
//        hurricane.SID = SID;
//        hurricane.season = season;
//        auto nameID = name+"-"+season;

//        // Create a unique ID for this track
//        auto uid = theVisualizationWidget->createUniqueID();

//        QMap<QString, QVariant> featureAttributes;
//        featureAttributes.insert("NAME",name);
//        featureAttributes.insert("SID",SID);
//        featureAttributes.insert("SEASON",season);
//        featureAttributes.insert("TabName", nameID);
//        featureAttributes.insert("AssetType", "HURRICANE");
//        featureAttributes.insert("UID", uid);

//        auto polyline = this->getTrackGeometry(&hurricane, err);

//        if(polyline.isEmpty())
//            return -1;

//        auto trackFeat = trackFeatureCollectionTable->createFeature(featureAttributes,polyline,theParent);
//        trackFeatureCollectionTable->addFeature(trackFeat);
//    }

//    theVisualizationWidget->zoomToLayer(allHurricanesLayer->layerId());

    return 0;
}


void QGISHurricanePreprocessor::clear(void)
{
    hurricanes.clear();
    delete allHurricanesLayer;
    allHurricanesLayer = nullptr;
}


QgsVectorLayer* QGISHurricanePreprocessor::createTrackVisualization(HurricaneObject* hurricane, QgsVectorLayer* parentLayer, QString& err)
{
    auto numPnts = hurricane->size();

    if(numPnts == 0)
    {
        err = "Hurricane does not have any track points";
        return nullptr;
    }

    // Name and storm ID
    auto name = hurricane->name;
    auto SID = hurricane->SID;
    auto season = hurricane->season;
    auto nameID = name+"-"+season;

    auto uid = theVisualizationWidget->createUniqueID();

//    QMap<QString, QVariant> featureAttributes;
//    featureAttributes.insert("NAME",name);
//    featureAttributes.insert("SID",SID);
//    featureAttributes.insert("SEASON",season);
//    featureAttributes.insert("TabName", nameID);
//    featureAttributes.insert("AssetType", "HURRICANE_TRACK");
//    featureAttributes.insert("UID", uid);

//    auto polyline = this->getTrackGeometry(hurricane, err);

//    if(polyline.isEmpty())
//        return nullptr;

//    // Create the feature collection table/layers
//    QList<Field> trackFields;
//    trackFields.append(Field::createText("NAME", "NULL",4));
//    trackFields.append(Field::createText("SID", "NULL",4));
//    trackFields.append(Field::createText("SEASON", "NULL",4));
//    trackFields.append(Field::createText("AssetType", "NULL",4));
//    trackFields.append(Field::createText("TabName", "NULL",4));
//    trackFields.append(Field::createText("UID", "NULL",4));

//    auto trackFeatureCollection = new FeatureCollection(theParent);
//    auto trackFeatureCollectionTable = new FeatureCollectionTable(trackFields, GeometryType::Polyline, SpatialReference::wgs84(), theParent);
//    trackFeatureCollection->tables()->append(trackFeatureCollectionTable);

//    auto trackLayer = new FeatureCollectionLayer(trackFeatureCollection,theParent);
//    trackLayer->setAutoFetchLegendInfos(true);
//    trackLayer->setName("Track");

//    // Create line symbol for the track
//    SimpleLineSymbol* lineSymbol = new SimpleLineSymbol(SimpleLineSymbolStyle::Solid,
//                                                        QColor(0, 0, 0),
//                                                        2.0f /*width*/,
//                                                        SimpleLineSymbolMarkerStyle::Arrow,
//                                                        SimpleLineSymbolMarkerPlacement::End,
//                                                        theParent);

//    // Create renderer and set symbol for the track
//    SimpleRenderer* lineRenderer = new SimpleRenderer(lineSymbol, theParent);
//    lineRenderer->setLabel("Hurricane track");

//    // Set the renderer for the feature layer
//    trackFeatureCollectionTable->setRenderer(lineRenderer);

//    auto trackFeat = trackFeatureCollectionTable->createFeature(featureAttributes,polyline,theParent);
//    trackFeatureCollectionTable->addFeature(trackFeat);

//    return theVisualizationWidget->addLayerToMap(trackLayer, parentItem, parentLayer);

    return nullptr;
}


QgsVectorLayer*  QGISHurricanePreprocessor::createTrackPointsVisualization(HurricaneObject* hurricane, QgsVectorLayer* parentLayer, QString& err)
{
    auto numPnts = hurricane->size();

    if(numPnts == 0)
    {
        err = "Hurricane does not have any track points";
        return nullptr;
    }

    // Get the parameter labels or header data
    auto headerData = hurricane->parameterLabels;

    // By default will use USA_LAT and USA_LON, if not available fall back on the LAT and LON below
    auto indexUSALat = headerData.indexOf("USA_LAT");
    auto indexUSALon = headerData.indexOf("USA_LON");
    auto indexLat = headerData.indexOf("LAT");
    auto indexLon = headerData.indexOf("LON");

    // Check that the lat/lon indexes are found
    if((indexLat == -1 || indexLon == -1) && (indexUSALat == -1 || indexUSALon == -1))
    {
        err = "Could not find the required column indexes in the data file";
        return nullptr;
    }

//    // Create the table to store the fields
//    QList<Field> pointFields;
//    // Common fields
//    pointFields.append(Field::createText("AssetType", "NULL",4));
//    pointFields.append(Field::createText("TabName", "NULL",4));
//    pointFields.append(Field::createText("UID", "NULL",4));

//    // Full fields
//    for(auto&& it : headerData)
//    {
//        pointFields.append(Field::createText(it, "NULL",4));
//    }

//    // Create the feature collection table/layers
//    auto trackPntsFeatureCollection = new FeatureCollection(theParent);
//    auto trackPntsTable = new FeatureCollectionTable(pointFields, GeometryType::Point, SpatialReference::wgs84(), theParent);
//    trackPntsFeatureCollection->tables()->append(trackPntsTable);

//    auto trackPntsLayer = new FeatureCollectionLayer(trackPntsFeatureCollection,theParent);
//    trackPntsLayer->setAutoFetchLegendInfos(true);
//    trackPntsLayer->setName("Track Points");

//    // Create cross SimpleMarkerSymbol
//    SimpleMarkerSymbol* markerSymbol = new SimpleMarkerSymbol(SimpleMarkerSymbolStyle::Circle, QColor("black"), 6, theParent);

//    // Create renderer and set symbol to crossSymbol
//    SimpleRenderer* pointRenderer = new SimpleRenderer(markerSymbol, theParent);
//    pointRenderer->setLabel("Hurricane track points");

//    // Set the renderer for the feature layer
//    trackPntsTable->setRenderer(pointRenderer);


//    // Each row is a point on the hurricane track
//    for(int j = 0; j<numPnts; ++j)
//    {

//        QStringList trackPoint = (*hurricane)[j];

//        //create the feature attributes
//        QMap<QString, QVariant> featureAttributes;
//        for(int k = 0; k<headerData.size(); ++k)
//        {
//            if(!trackPoint.at(k).isEmpty())
//                featureAttributes.insert(headerData.at(k), trackPoint.at(k));
//        }

//        auto uid = theVisualizationWidget->createUniqueID();

//        featureAttributes.insert("AssetType", "HURRICANE_TRACK_POINT");
//        featureAttributes.insert("TabName", "Track Point");
//        featureAttributes.insert("UID", uid);

//        // Create the geometry for visualization
//        // By default will use USA_LAT and USA_LON, if not available fall back on the LAT and LON below
//        auto latitude = trackPoint.at(indexLat).toDouble();
//        auto longitude = trackPoint.at(indexLon).toDouble();

//        //  if(latitude == 0.0 || longitude == 0.0)
//        //  {
//        //      latitude = trackPoint.at(indexLat).toDouble();
//        //      longitude = trackPoint.at(indexLon).toDouble();

//        //      if(latitude == 0.0 || longitude == 0.0)
//        //      {
//        //          err = "Error getting the latitude and longitude";
//        //          return -1;
//        //      }
//        //  }

//        Point point(longitude,latitude);

//        auto feature = trackPntsTable->createFeature(featureAttributes, point, theParent);
//        trackPntsTable->addFeature(feature);
//    }


//    LayerTreeItem* trackPointsItem = theVisualizationWidget->addLayerToMap(trackPntsLayer,parentItem, parentLayer);

    return nullptr;
}


QgsVectorLayer* QGISHurricanePreprocessor::createLandfallVisualization(const double latitude,const double longitude, const QMap<QString, QVariant>& featureAttributes, QgsVectorLayer* parentLayer)
{

//    QList<Field> pointFields;

//    auto fieldKeys = featureAttributes.keys();

//    for(auto&& it : fieldKeys)
//        pointFields.append(Field::createText(it, "NULL",4));

//    // Create the feature collection table/layers
//    auto landfallFeatCollection = new FeatureCollection(theParent);
//    auto landFallPntTable = new FeatureCollectionTable(pointFields, GeometryType::Point, SpatialReference::wgs84(), theParent);
//    landfallFeatCollection->tables()->append(landFallPntTable);

//    auto landFallPointLayer = new FeatureCollectionLayer(landfallFeatCollection,theParent);
//    landFallPointLayer->setAutoFetchLegendInfos(true);
//    landFallPointLayer->setName("Landfall");

//    // Create cross SimpleMarkerSymbol
//    SimpleMarkerSymbol* landfallMarkerSymbol = new SimpleMarkerSymbol(SimpleMarkerSymbolStyle::Diamond, QColor("blue"), 12, theParent);

//    // Create renderer and set symbol to crossSymbol
//    SimpleRenderer* landfallPointRenderer = new SimpleRenderer(landfallMarkerSymbol, theParent);
//    landfallPointRenderer->setLabel("Hurricane landfall");

//    // Set the renderer for the feature layer
//    landFallPntTable->setRenderer(landfallPointRenderer);

//    // Create the point geometry
//    Point point(longitude,latitude);

//    auto feature = landFallPntTable->createFeature(featureAttributes, point, theParent);
//    landFallPntTable->addFeature(feature);

//    // Add the point layer
//    LayerTreeItem* landfallItem = theVisualizationWidget->addLayerToMap(landFallPointLayer,parentItem, parentLayer);

    return nullptr;
}


HurricaneObject* QGISHurricanePreprocessor::getHurricane(const QString& SID)
{
    for(auto&& it : hurricanes)
    {
        if(SID.compare(it.SID) == 0)
            return &it;
    }

    return nullptr;
}


QgsVectorLayer *QGISHurricanePreprocessor::getAllHurricanesLayer() const
{
    return allHurricanesLayer;
}

