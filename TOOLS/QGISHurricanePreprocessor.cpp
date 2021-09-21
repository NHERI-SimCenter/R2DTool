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

#include <qgsfield.h>
#include <qgsfeature.h>
#include <qgsvectorlayer.h>
#include <qgslinesymbol.h>
#include <qgsmarkersymbol.h>


#include <QApplication>
#include <QProgressBar>
#include <QList>

QGISHurricanePreprocessor::QGISHurricanePreprocessor(QProgressBar* pBar, QGISVisualizationWidget* visWidget, QObject* parent) : theProgressBar(pBar), theVisualizationWidget(visWidget), theParent(parent)
{
    allHurricanesLayer = nullptr;

    QMap<QString, QVariant> featureAttributes;
    featureAttributes.insert("test","test");
    createLandfallVisualization(37.8717450069,-122.2609607382,featureAttributes);
}


QgsVectorLayer* QGISHurricanePreprocessor::loadHurricaneDatabaseData(const QString &eventFile, QString &err)
{
    CSVReaderWriter csvTool;

    QVector<QStringList> data = csvTool.parseCSVFile(eventFile, err);

    if(!err.isEmpty())
    {
        return nullptr;
    }

    if(data.empty())
    {
        err = "Hurricane data is empty";
        return nullptr;
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
        return nullptr;
    }

    // While iterating through the hurricane points, save the data at first landfall
    bool landfallFound = false;
    while (i.hasNext())
    {
        auto row = i.next();

        if(row.size() != numCol)
        {
            err = "Error, inconsistency in the data in the row and number of columns";
            return nullptr;
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
        err = "Error adding a vector layer";
        return nullptr;
    }

    // Create the hurricane track fields
    QList<QgsField> attrib;
    attrib.append(QgsField("NAME", QVariant::String));
    attrib.append(QgsField("SID", QVariant::String));
    attrib.append(QgsField("SEASON", QVariant::String));
    attrib.append(QgsField("TabName", QVariant::String));
    attrib.append(QgsField("AssetType", QVariant::String));
    attrib.append(QgsField("UID", QVariant::String));


    auto numAtrb = attrib.size();

    QgsFeatureList featList;

    featList.reserve(numHurricanes);

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

        QgsAttributes featureAttributes(numAtrb);
        featureAttributes[0] = name;
        featureAttributes[1] = SID;
        featureAttributes[2] = season;
        featureAttributes[3] = nameID;
        featureAttributes[4] = "HURRICANE";
        featureAttributes[5] = uid;

        QgsFeature feature;

        auto polyline = this->getTrackGeometry(&hurricane, err);

        if(polyline.isEmpty())
            return nullptr;

        feature.setGeometry(polyline);

        feature.setAttributes(featureAttributes);

        featList.push_back(feature);

    }

    // Create the buildings group layer that will hold the sublayers
    allHurricanesLayer = theVisualizationWidget->addVectorLayer("LineString","All Hurricanes");

    if(allHurricanesLayer == nullptr)
    {
        err = "Error adding item to the map";
        return nullptr;
    }

    auto pr = allHurricanesLayer->dataProvider();

    auto res = pr->addAttributes(attrib);
    if(!res)
    {
        err = "Error adding attributes";
        theVisualizationWidget->removeLayer(allHurricanesLayer);
        return nullptr;
    }

    allHurricanesLayer->updateFields(); // tell the vector layer to fetch changes from the provider

    pr->addFeatures(featList);

    allHurricanesLayer->updateExtents();

    auto lineSymbol = new QgsLineSymbol();

    lineSymbol->setWidth(0.75);

    theVisualizationWidget->createSimpleRenderer(lineSymbol,allHurricanesLayer);

    theVisualizationWidget->zoomToLayer(allHurricanesLayer);

    return allHurricanesLayer;
}


void QGISHurricanePreprocessor::clear(void)
{
    hurricanes.clear();
    delete allHurricanesLayer;
    allHurricanesLayer = nullptr;
}


QgsVectorLayer* QGISHurricanePreprocessor::createTrackVisualization(HurricaneObject* hurricane, QString& err)
{
    auto numPnts = hurricane->size();

    if(numPnts == 0)
    {
        err = "Hurricane does not have any track points";
        return nullptr;
    }

    auto geom = this->getTrackGeometry(hurricane, err);

    if(geom.isEmpty())
        return nullptr;

    // Create the hurricane track fields
    QList<QgsField> attrib;
    attrib.append(QgsField("NAME", QVariant::String));
    attrib.append(QgsField("SID", QVariant::String));
    attrib.append(QgsField("SEASON", QVariant::String));
    attrib.append(QgsField("TabName", QVariant::String));
    attrib.append(QgsField("AssetType", QVariant::String));
    attrib.append(QgsField("UID", QVariant::String));

    // Name and storm ID
    auto name = hurricane->name;
    auto SID = hurricane->SID;
    auto season = hurricane->season;
    auto nameID = name+"-"+season;
    auto uid = theVisualizationWidget->createUniqueID();

    auto numAtrb = attrib.size();

    QgsAttributes featureAttributes(numAtrb);
    featureAttributes[0] = name;
    featureAttributes[1] = SID;
    featureAttributes[2] = season;
    featureAttributes[3] = nameID;
    featureAttributes[4] = "HURRICANE_TRACK";
    featureAttributes[5] = uid;

    // Create the group layer that will hold the sublayers
    auto trackLayer = theVisualizationWidget->addVectorLayer("LineString","Track");

    if(trackLayer == nullptr)
    {
        err = "Error adding a vector layer";
        return nullptr;
    }

    QgsFeature feature;

    // Set the feature geometry
    feature.setGeometry(geom);

    // Set the feature attributes
    feature.setAttributes(featureAttributes);

    QgsFeatureList featList;
    featList.push_back(feature);

    auto pr = trackLayer->dataProvider();

    auto res = pr->addAttributes(attrib);

    if(!res)
        qDebug()<<"Error adding attributes";

    trackLayer->updateFields(); // tell the vector layer to fetch changes from the provider

    pr->addFeatures(featList);

    trackLayer->updateExtents();

    auto lineSymbol = new QgsLineSymbol();

    lineSymbol->setWidth(0.75);

    theVisualizationWidget->createSimpleRenderer(lineSymbol,trackLayer);


    return trackLayer;
}


QgsVectorLayer*  QGISHurricanePreprocessor::createTrackPointsVisualization(HurricaneObject* hurricane, QString& err)
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

    // Create the table to store the fields
    QList<QgsField> attrib = {QgsField("AssetType", QVariant::String),
                              QgsField("TabName",  QVariant::String),
                              QgsField("UID", QVariant::String)};


    // Append Full fields
    for(auto&& it : headerData)
        attrib.append(QgsField(it, QVariant::String));


    auto layer = theVisualizationWidget->addVectorLayer("Point", "Track Points");

    auto pr = layer->dataProvider();

    auto res = pr->addAttributes(attrib);

    if(!res)
        qDebug()<<"Error adding attributes";

    pr->addAttributes(attrib);

    layer->updateFields(); // tell the vector layer to fetch changes from the provider

    QgsFeatureList featList;

    // Each row is a point on the hurricane track
    for(int j = 0; j<numPnts; ++j)
    {
        QStringList trackPoint = (*hurricane)[j];

        //create the feature attributes
        QgsAttributes featAttrb(attrib.size());

        auto uid = theVisualizationWidget->createUniqueID();

        featAttrb[0] = "HURRICANE_TRACK_POINT";
        featAttrb[1]  = "Track Point";
        featAttrb[2] = uid;

        for(int k = 0; k<headerData.size(); ++k)
        {
            if(!trackPoint.at(k).isEmpty())
                featAttrb[3+k] = trackPoint.at(k);
        }

        // Create the geometry for visualization
        // By default will use USA_LAT and USA_LON, if not available fall back on the LAT and LON below
        auto latitude = trackPoint.at(indexLat).toDouble();
        auto longitude = trackPoint.at(indexLon).toDouble();

        QgsFeature fet;
        fet.setGeometry(QgsGeometry::fromPointXY(QgsPointXY(longitude,latitude)));

        fet.setAttributes(featAttrb);

        featList.append(fet);


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
    }

    pr->addFeatures(featList);

    layer->updateExtents();

    QgsMarkerSymbol* pointMarkerSymbol = new QgsMarkerSymbol();

    pointMarkerSymbol->setColor(Qt::green);

    theVisualizationWidget->createSimpleRenderer(pointMarkerSymbol,layer);

    return layer;
}


QgsVectorLayer* QGISHurricanePreprocessor::createLandfallVisualization(const double latitude,const double longitude, const QMap<QString, QVariant>& featureAttributes)
{
    // Create the table to store the fields
    QList<QgsField> attrib;

    QMap<QString, QVariant>::const_iterator it;
    for (it = featureAttributes.begin(); it != featureAttributes.end(); ++it)
        attrib.append(QgsField(it.key(),it.value().type()));

    auto layer = theVisualizationWidget->addVectorLayer("Point", "Landfall");

    auto pr = layer->dataProvider();

    auto res = pr->addAttributes(attrib);

    if(!res)
        qDebug()<<"Error adding attributes";

    layer->updateFields(); // tell the vector layer to fetch changes from the provider

    QgsFeatureList featList;

    QgsFeature fet;
    fet.setGeometry(QgsGeometry::fromPointXY(QgsPointXY(longitude,latitude)));

    //create the feature attributes
    QgsAttributes featAttrb;

    featAttrb.reserve(attrib.size());

    QMap<QString, QVariant>::const_iterator it2;
    for (it2 = featureAttributes.begin(); it2 != featureAttributes.end(); ++it2)
        featAttrb.push_back(it2.value());

    fet.setAttributes(featAttrb);

    featList.append(fet);

    pr->addFeatures(featList);

    layer->updateExtents();

    theVisualizationWidget->createSymbolRenderer(QgsSimpleMarkerSymbolLayerBase::Diamond,Qt::blue,4.0,layer);

    return layer;
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



QgsGeometry QGISHurricanePreprocessor::getTrackGeometry(HurricaneObject* hurricane, QString& err)
{
    // By default will use USA_LAT and USA_LON, if not available fall back on the LAT and LON below
    // Get the parameter labels or header data
    auto headerData = hurricane->parameterLabels;

    auto indexUSALat = headerData.indexOf("USA_LAT");
    auto indexUSALon = headerData.indexOf("USA_LON");
    auto indexLat = headerData.indexOf("LAT");
    auto indexLon = headerData.indexOf("LON");

    // Check that the indexes are found
    if((indexLat == -1 || indexLon == -1) && (indexUSALat == -1 || indexUSALon == -1))
    {
        err = "Could not find the required column indexes in the data file";
        return QgsGeometry();
    }

    // Each row is a point on the hurricane track
    //    PartCollection* trackCollection = new PartCollection(SpatialReference::wgs84(), theParent);
    double latitude = 0.0;
    double longitude = 0.0;

    QgsPolylineXY polyLine;

    for(int j = 0; j<hurricane->size(); ++j)
    {
        QStringList trackPoint = (*hurricane)[j];

        //        QgsPointXY pointPrev(longitude,latitude);

        // Create the geometry for visualization
        // By default will use USA_LAT and USA_LON, if not available fall back on the LAT and LON below
        latitude = trackPoint.at(indexLat).toDouble();
        longitude = trackPoint.at(indexLon).toDouble();


        QgsPointXY pointNow(longitude,latitude);

        polyLine.push_back(pointNow);

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

        //        Point point(longitude,latitude);

        //        if(j != 0)
        //        {
        //            Part* partj = new Part(SpatialReference::wgs84(), theParent);
        //            partj->addPoint(point);
        //            partj->addPoint(pointPrev);

        //            trackCollection->addPart(partj);
        //        }
    }

    QgsGeometry geom = QgsGeometry::fromPolylineXY(polyLine);

    return geom;
}
