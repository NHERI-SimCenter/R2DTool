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
#include "SimpleRenderer.h"
#include "PolygonBuilder.h"

using namespace Esri::ArcGISRuntime;

HurricanePreprocessor::HurricanePreprocessor(QProgressBar* pBar, VisualizationWidget* visWidget, QObject* parent) : theProgressBar(pBar), theVisualizationWidget(visWidget), theParent(parent)
{

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
    auto topHeaderData = data.at(0);
    //    auto unitsData = data.at(1);

    auto numCol = topHeaderData.size();

    auto indexName = -1;
    auto indexSID = -1;
    auto indexLat = -1;
    auto indexLon = -1;
    auto indexSeason = -1;

    for(int col = 0; col < numCol; ++col)
    {
        if(topHeaderData.at(col) == "LAT")
            indexLat = col;
        else if(topHeaderData.at(col) == "LON")
            indexLon = col;
        else if(topHeaderData.at(col) == "NAME")
            indexName = col;
        else if(topHeaderData.at(col) == "SID")
            indexSID = col;
        else if(topHeaderData.at(col) == "SEASON")
            indexSeason = col;
    }

    if(indexName == -1 || indexSID == -1 || indexLat == -1 || indexLon == -1 || indexSeason == -1)
    {
        err = "Could not find the required column indexes in the data file";
        return -1;
    }

    // Create the table to store the fields
    QList<Field> tableFields;

    // Common fields
    tableFields.append(Field::createText("AssetType", "NULL",4));
    tableFields.append(Field::createText("TabName", "NULL",4));
    tableFields.append(Field::createText("UID", "NULL",4));

    for(auto&& it : topHeaderData)
    {
        tableFields.append(Field::createText(it, "NULL",4));
    }

    // Pop off the first two rows that contain the header information
    data.pop_front();
    data.pop_front();

    // Create the buildings group layer that will hold the sublayers
    auto allHurricanesLayer = new GroupLayer(QList<Layer*>{},theParent);
    allHurricanesLayer->setName("Hurricanes");

    auto allHurricanesItem = theVisualizationWidget->addLayerToMap(allHurricanesLayer);

    if(allHurricanesItem == nullptr)
    {
        err = "Error adding item to the map";
        return -1;
    }


    // Split the hurricanes up as they come in one long list
    QVector<QVector<QStringList>> hurricanes;

    QVectorIterator<QStringList> i(data);
    QString SID;

    QVector<QStringList> hurricane;
    while (i.hasNext())
    {
        auto row = i.next();

        if(row.size() != numCol)
        {
            err = "Error, inconsistency in the data in the row and number of columns";
            return -1;
        }

        auto currSID = row.at(indexSID);

        if(SID.compare(currSID) != 0)
        {
            if(!hurricane.isEmpty())
            {
                hurricanes.push_back(hurricane);
                hurricane.clear();
            }

            SID = currSID;
        }
        else
        {
            hurricane.push_back(row);
        }

    }

    // Push back
    if(!hurricane.isEmpty())
        hurricanes.push_back(hurricane);

    auto numHurricanes = hurricanes.size();

    theProgressBar->setMinimum(0);
    theProgressBar->setMaximum(numHurricanes);
    theProgressBar->reset();
    QApplication::processEvents();

    for(int i = 0; i<numHurricanes; ++i)
    {
        theProgressBar->setValue(i);
        QApplication::processEvents();

        // Get the hurricane
        auto hurricane = hurricanes.at(i);

        auto numPnts = hurricane.size();

        auto name = hurricane.front().at(indexName);
        auto SID = hurricane.front().at(indexSID);
        auto season = hurricane.front().at(indexSeason);

        auto nameID = name+"-"+season;

        auto thisHurricaneLayer = new GroupLayer(QList<Layer*>{},theParent);
        thisHurricaneLayer->setName(nameID);

        auto thisHurricanesItem = theVisualizationWidget->addLayerToMap(thisHurricaneLayer,allHurricanesItem,allHurricanesLayer);

        //        auto trackPntsFeatureCollection = new FeatureCollection(this);

        //        // Create the feature collection table/layers
        //        auto trackPntsFeatureCollectionTable = new FeatureCollectionTable(tableFields, GeometryType::Point, SpatialReference::wgs84(), this);
        //        trackPntsFeatureCollection->tables()->append(trackPntsFeatureCollectionTable);

        //        auto trackPntsLayer = new FeatureCollectionLayer(trackPntsFeatureCollection,this);

        //        trackPntsLayer->setName(name+" - Points");

        //        // Create cross SimpleMarkerSymbol
        //        SimpleMarkerSymbol* markerSymbol = new SimpleMarkerSymbol(SimpleMarkerSymbolStyle::Circle, QColor("black"), 4, this);

        //        // Create renderer and set symbol to crossSymbol
        //        SimpleRenderer* pointRenderer = new SimpleRenderer(markerSymbol, this);
        //        pointRenderer->setLabel("Hurricane track points");

        //        // Set the renderer for the feature layer
        //        trackPntsFeatureCollectionTable->setRenderer(pointRenderer);

        // Each row is a point on the hurricane track
        PartCollection* trackCollection = new PartCollection(SpatialReference::wgs84(), theParent);
        double latitude = 0.0;
        double longitude = 0.0;
        for(int j = 0; j<numPnts; ++j)
        {
            auto trackPoint = hurricane.at(j);

            // create the feature attributes
            //  QMap<QString, QVariant> featureAttributes;
            //  for(int k = 0; k<numCol; ++k)
            //  {
            //      featureAttributes.insert(topHeaderData.at(k), trackPoint.at(k));
            //  }

            //  featureAttributes.insert("AssetType", buildingIDStr);
            //  featureAttributes.insert("TabName", buildingIDStr);
            //  featureAttributes.insert("UID", uid);

            Point pointPrev(longitude,latitude);

            // Create the geometry for visualization
            latitude = trackPoint.at(indexLat).toDouble();
            longitude = trackPoint.at(indexLon).toDouble();

            Point point(longitude,latitude);

            if(j != 0)
            {
                Part* partj = new Part(SpatialReference::wgs84(), theParent);
                partj->addPoint(point);
                partj->addPoint(pointPrev);

                trackCollection->addPart(partj);
            }

            // Create the point feature
            //            auto feature = trackPntsFeatureCollectionTable->createFeature(featureAttributes, point, this);

            //            trackPntsFeatureCollectionTable->addFeature(feature);
        }

        // Add the points layer
        //        theVisualizationWidget->addLayerToMap(trackPntsLayer,thisHurricanesItem, thisHurricaneLayer);

        PolygonBuilder polylineBuilder(SpatialReference::wgs84());
        polylineBuilder.setParts(trackCollection);

        // Add the track polyline layer
        // if(!polylineBuilder.isSketchValid())
        // {
        //     qDebug()<<"Error, cannot create a feature with the latitude and longitude provided";
        //     return;
        // }

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

        trackLayer->setName(nameID+" - Track");

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

        auto uid = theVisualizationWidget->createUniqueID();

        QMap<QString, QVariant> featureAttributes;
        featureAttributes.insert("NAME",name);
        featureAttributes.insert("SID",SID);
        featureAttributes.insert("SEASON",season);
        featureAttributes.insert("TabName", nameID);
        featureAttributes.insert("AssetType", "HURRICANE");
        featureAttributes.insert("UID", uid);

        // Create the polyline feature
        auto polyline =  polylineBuilder.toPolyline();

        auto trackFeat = trackFeatureCollectionTable->createFeature(featureAttributes,polyline,theParent);
        trackFeatureCollectionTable->addFeature(trackFeat);

        theVisualizationWidget->addLayerToMap(trackLayer, thisHurricanesItem, thisHurricaneLayer);
    }
}

