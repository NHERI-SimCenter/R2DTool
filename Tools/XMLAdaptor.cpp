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

#include "QGISVisualizationWidget.h"
#include <qgsvectorlayer.h>

#include <QtXml>
#include <QFile>

XMLAdaptor::XMLAdaptor()
{

}


QgsVectorLayer* XMLAdaptor::parseXMLFile(const QString& filePath, QString& errMessage, QGISVisualizationWidget* GISVisWidget)
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

    int indexLon = -1;
    int indexLat = -1;

    // Get all of the grid fields from the XML file
    QgsFields featFields;

    QList<QgsField> attribFields;
    attribFields.push_back(QgsField("AssetType", QVariant::String));
    attribFields.push_back(QgsField("TabName", QVariant::String));

    featFields.append(QgsField("AssetType", QVariant::String));
    featFields.append(QgsField("TabName", QVariant::String));

    for(int i = 0; i < GMFieldsList.count(); ++i)
    {
        auto item = GMFieldsList.item(i).toElement();

        QString fieldName = item.attribute("name","NULL");
        attribFields.push_back(QgsField(fieldName, QVariant::String));

        featFields.append(QgsField(fieldName, QVariant::String));

        if(fieldName.compare("LAT") == 0)
            indexLat = i;
        if(fieldName.compare("LON") == 0)
            indexLon = i;
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

    if(indexLat == -1 || indexLon == -1)
    {
        errMessage = "Error getting the lat and/or lon indexes in the grid xml file";
        return nullptr;
    }   

//    // Over 50000 points causes arc gis library to crash... even though, too many points makes the visualization too cluttered
//    int decim = gridPoints.size()/40000;

//    if(decim == 0)
//        decim = 1;

    // Iterate through the grid points to get the data at each point
    QgsFeatureList featureList;

    auto count = 0;
    for(auto&& gp : gridPoints)
    {
        ++count;

//        if(count%decim != 0)
//            continue;

        // Change for windows compilation
        gp.remove("\r");

        if(gp.isEmpty())
            continue;

        QStringList pointData;

        if(gp.contains("\t"))
            pointData = gp.split("\t");
        else
            pointData = gp.split(" ");

        if(pointData.size() != numFields)
        {
            errMessage = "Error the number of columns in a point does not equal the number of fields";
            return nullptr;
        }

        // create the feature attributes
        QgsAttributes featAttributes(attribFields.size());

        featAttributes[0]= "SHAKEMAP_GRID"; // Asset type
        featAttributes[1]= "ShakeMap Grid Point"; // Tab Name

        for(int i = 0; i<numFields; ++i)
        {
            auto attrbVal = pointData[i];
            featAttributes[2+i] = attrbVal;
        }

        bool OK = true;

        auto longitude = pointData[indexLon].toDouble(&OK);

        if(!OK)
        {
            errMessage = "Error converting longitude to double";
            return nullptr;
        }

        auto latitude = pointData[indexLat].toDouble(&OK);

        if(!OK)
        {
            errMessage = "Error converting latitude to double";
            return nullptr;
        }

        if(longitude == 0.0 || latitude == 0.0)
        {
            errMessage = "Error, zero lat lon values";
            return nullptr;
        }

        // Create the feature
        QgsFeature feature;
        feature.setFields(featFields);
        feature.setGeometry(QgsGeometry::fromPointXY(QgsPointXY(longitude,latitude)));
        feature.setAttributes(featAttributes);
        featureList.append(feature);

        // Create the ground motion station
        GroundMotionStation station("NULL",latitude,longitude);
        station.setStationFeature(feature);
        stationList.push_back(station);
    }


    auto vectorLayer = GISVisWidget->addVectorLayer("Point", "ShakeMap Grid");
    if(vectorLayer == nullptr)
    {
        errMessage = "Error creating a layer";
        return nullptr;
    }

    auto dProvider = vectorLayer->dataProvider();
    auto res = dProvider->addAttributes(attribFields);

    if(!res)
    {
        errMessage = "Error adding attribute fields to layer";
        GISVisWidget->removeLayer(vectorLayer);
        return nullptr;
    }

    vectorLayer->updateFields(); // tell the vector layer to fetch changes from the provider

    dProvider->addFeatures(featureList);
    vectorLayer->updateExtents();

    GISVisWidget->createSymbolRenderer(Qgis::MarkerShape::Cross,Qt::black,2.0,vectorLayer);

    return vectorLayer;
}


QString XMLAdaptor::getEventName() const
{
    return eventName;
}


QVector<GroundMotionStation> XMLAdaptor::getStationList() const
{
    return stationList;
}
