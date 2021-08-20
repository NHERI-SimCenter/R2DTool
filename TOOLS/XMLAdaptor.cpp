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

#include "XMLAdaptor.h"
#include "VisualizationWidget.h"

// GIS headers
#include "GroupLayer.h"
#include "FeatureCollectionLayer.h"
#include "FeatureLayer.h"
#include "FeatureCollection.h"
#include "FeatureCollectionTable.h"
#include "SimpleMarkerSymbol.h"
#include "SimpleRenderer.h"
#include "PointCollection.h"
#include "MultipointBuilder.h"

// To deletge
#include "GraphicsOverlay.h"
#include "SimCenterMapGraphicsView.h"

#include <QtXml>
#include <QFile>

using namespace Esri::ArcGISRuntime;

XMLAdaptor::XMLAdaptor()
{

}


FeatureCollectionLayer* XMLAdaptor::parseXMLFile(const QString& filePath, QString& errMessage, QObject* parent)
{
    // QDomDocument used to import XML data
    QDomDocument xmlGMs;

    // Load xml file
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly ))
    {
        // Error while loading file
        errMessage = "Error while loading file";
        return nullptr;
    }

    // Set raw XML content into the QDomDocument
    xmlGMs.setContent(&file);

    // Close the file now that we are done with it
    file.close();

    // Extract the root element
    QDomElement root=xmlGMs.documentElement();

    // Get root names and attributes
    QString Type = root.tagName();

    // Check that the XML file is actually a shake map grid
    if(Type.compare("shakemap_grid") != 0)
    {
        errMessage = "Error, XML file is not a ShakeMap grid";
        return nullptr;
    }

    // Get some information from the file
    shakemapID = root.attribute("shakemap_id","NULL");

    //    QString shakemapVersion = root.attribute("shakemap_version","NULL");

    //    if(shakemapVersion.compare("10") != 0)
    //    {
    //        errMessage = "Error: only shakemap version 10 is supported";
    //        return nullptr;
    //    }

    QDomNodeList eventList = root.elementsByTagName("event");

    QDomNodeList GMFieldsList = root.elementsByTagName("grid_field");

    auto numFields = GMFieldsList.size();

    if(numFields == 0 || eventList.size() == 0)
        return nullptr;

    // Get the event name
    eventName = eventList.item(0).toElement().attribute("event_description","NULL");

    // Get all of the grid fields from the XML file
    QStringList pointFields;

    pointFields.reserve(numFields);

    for(int i = 0; i < GMFieldsList.count(); ++i)
    {
        auto item = GMFieldsList.item(i).toElement();
        QString fieldName = item.attribute("name","NULL");

        pointFields.push_back(fieldName);
    }

    // Get the grid data from the XML file
    QDomNodeList gridDataElements = root.elementsByTagName("grid_data");

    if(gridDataElements.size() != 1)
    {
        errMessage = "Error, no grid data in XML file";
        return nullptr;
    }

    auto gridData =  gridDataElements.at(0).toElement().text();

    // Each grid point is separated by a newline
    auto gridPoints = gridData.split("\n",Qt::SkipEmptyParts);

    if(gridPoints.size() == 0)
        return nullptr;


    auto indexLon = pointFields.indexOf("LON");
    auto indexLat = pointFields.indexOf("LAT");

    if(indexLat == -1 || indexLon == -1)
    {
        errMessage = "Getting the lat and/or lon indexes in the grid xml file";
        return nullptr;
    }

    MultipointBuilder* multiPointBuilder = new MultipointBuilder(SpatialReference::wgs84(), parent);

    PointCollection* pc = new PointCollection(SpatialReference::wgs84(), parent);

    // Over 50000 points causes arc gis library to crash... even though, too many points makes the visualization too cluttered
    int decim = gridPoints.size()/40000;

    if(decim == 0)
        decim = 1;

    // Iterate through the grid points to get the data at each point

    auto count = 0;
    for(auto&& gp : gridPoints)
    {
        ++count;

        // Change for windows compilation
        gp.remove("\r");

        if(gp.isEmpty())
            continue;

        auto pointData = gp.split(" ");


        if(pointData.size() != numFields)
        {
            errMessage = "Error the number of columns in a point does not equal the number of fields";
            delete multiPointBuilder;
            delete pc;
            return nullptr;
        }

        // create the feature attributes
        QMap<QString, QVariant> featureAttributes;
        for(int i = 0; i<numFields; ++i)
        {
            auto attrbText = pointFields[i];
            auto attrbVal = pointData[i];
            featureAttributes.insert(attrbText,attrbVal);
        }

        bool OK = true;

        auto longitude = pointData[indexLon].toDouble(&OK);

        if(!OK)
        {
            errMessage = "Error converting longitude to double";
            delete multiPointBuilder;
            delete pc;
            return nullptr;
        }

        auto latitude = pointData[indexLat].toDouble(&OK);

        if(!OK)
        {
            errMessage = "Error converting latitude to double";
            delete multiPointBuilder;
            delete pc;
            return nullptr;
        }

        if(longitude == 0.0 || latitude == 0.0)
        {
            errMessage = "Error, zero lat lon values";
            delete multiPointBuilder;
            delete pc;
            return nullptr;
        }

        GroundMotionStation station("NULL",latitude,longitude);

        station.setStationAttributes(featureAttributes);

        stationList.push_back(station);


        if(count%decim != 0)
            continue;

        auto res = pc->addPoint(longitude,latitude);

        if(res == -1)
        {
            errMessage = "Error, adding point to the point collection";
            delete multiPointBuilder;
            delete pc;
            return nullptr;
        }
    }


//    pc->addPoint(-122.2609607382,37.8717450069);
//    pc->addPoint(-122.3609607382,37.9717450069);
//    pc->addPoint(-122.1609607382,37.7717450069);


    multiPointBuilder->setPoints(pc);

    Multipoint mPoint(multiPointBuilder->toGeometry());

    if(!mPoint.isValid() || mPoint.isEmpty())
    {
        errMessage = "Error creating the multipoint geometry in XMLAdaptor";
        delete multiPointBuilder;
        delete pc;
        return nullptr;
    }


    auto gridFeatureCollection = new FeatureCollection(parent);

    // Create red cross SimpleMarkerSymbol
    SimpleMarkerSymbol* crossSymbol = new SimpleMarkerSymbol(SimpleMarkerSymbolStyle::Cross, QColor("black"), 7, parent);

    // Create renderer and set symbol to crossSymbol
    SimpleRenderer* renderer = new SimpleRenderer(crossSymbol, parent);
    renderer->setLabel("ShakeMap Grid Point");


    QList<Field> tableFields;
    tableFields.append(Field::createText("AssetType", "NULL",4));
    tableFields.append(Field::createText("TabName", "NULL",4));

    // Option 1 start
    // Create the feature collection table/layers
//    auto gridFeatureCollectionTable = new FeatureCollectionTable(tableFields, GeometryType::Point, SpatialReference::wgs84(), parent);
//    gridFeatureCollection->tables()->append(gridFeatureCollectionTable);

//    // Set the renderer for the feature layer
//    gridFeatureCollectionTable->setRenderer(renderer);

//    for(int i =0; i<pc->size(); ++i)
//    {

//        // Create the point and add it to the feature table
//        // create the feature attributes
//        QMap<QString, QVariant> featureAttributes;
//        featureAttributes.insert("AssetType", "SHAKEMAP_GRID");
//        featureAttributes.insert("TabName", "ShakeMapGrid");

//        auto pnt =pc->point(i);

//        Feature* feature = gridFeatureCollectionTable->createFeature(featureAttributes, pnt, parent);

//        gridFeatureCollectionTable->addFeature(feature);
//    }
    // Option 1 end



    // Option 2 start

    // Create the feature collection table/layers
    auto gridFeatureCollectionTable = new FeatureCollectionTable(tableFields, GeometryType::Multipoint, SpatialReference::wgs84(), parent);
    gridFeatureCollection->tables()->append(gridFeatureCollectionTable);


    // Set the renderer for the feature layer
    gridFeatureCollectionTable->setRenderer(renderer);

    // Set the scale at which the layer will become visible - if scale is too high, then the entire view will be filled with symbols

    // Create the point and add it to the feature table
    // create the feature attributes
    QMap<QString, QVariant> featureAttributes;
    featureAttributes.insert("AssetType", "SHAKEMAP_GRID");
    featureAttributes.insert("TabName", "ShakeMapGrid");

    Feature* feature = gridFeatureCollectionTable->createFeature(featureAttributes, mPoint, parent);

    gridFeatureCollectionTable->addFeature(feature);

    // Option 2 end


    // Option 3 start
//    QMap<QString, QVariant> featureAttributes;
//    featureAttributes.insert("AssetType", "SHAKEMAP_GRID");
//    featureAttributes.insert("TabName", "ShakeMapGrid");
//    Point point1(-122.2609607382,37.8717450069);
//    Point point2(-122.3609607382,37.9717450069);
//    Feature* feature1 = gridFeatureCollectionTable->createFeature(featureAttributes, point1, parent);
//    gridFeatureCollectionTable->addFeature(feature1);
//    Feature* feature2 = gridFeatureCollectionTable->createFeature(featureAttributes, point2, parent);
//    gridFeatureCollectionTable->addFeature(feature2);
    // Option 3 end


    auto gridLayer = new FeatureCollectionLayer(gridFeatureCollection, parent);

    gridLayer->setMinScale(1000000);

//    // Test start
//    auto vizWidget = dynamic_cast<VisualizationWidget*>(parent);

//    auto m_originalMultipointGraphic = new Graphic(mPoint, crossSymbol, parent);
//    m_originalMultipointGraphic->setZIndex(0);

//    auto m_graphicsOverlay = new GraphicsOverlay(parent);
//    vizWidget->getMapViewWidget()->graphicsOverlays()->append(m_graphicsOverlay);
//    m_graphicsOverlay->graphics()->append(m_originalMultipointGraphic);
    // Test end

    return gridLayer;
}


QString XMLAdaptor::getEventName() const
{
    return eventName;
}

QVector<GroundMotionStation> XMLAdaptor::getStationList() const
{
    return stationList;
}
