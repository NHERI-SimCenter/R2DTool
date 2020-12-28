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

// GIS headers
#include "GroupLayer.h"
#include "FeatureCollectionLayer.h"
#include "FeatureLayer.h"
#include "FeatureCollection.h"
#include "FeatureCollectionTable.h"
#include "SimpleMarkerSymbol.h"
#include "SimpleRenderer.h"

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

    QList<Field> tableFields;
    tableFields.append(Field::createText("AssetType", "NULL",4));
    tableFields.append(Field::createText("TabName", "NULL",4));

    // Get all of the grid fields from the XML file
    QVector<QString> GMFields(numFields);

    for(int i = 0; i < GMFieldsList.count(); ++i)
    {
        auto item = GMFieldsList.item(i).toElement();
        QString fieldName = item.attribute("name","NULL");

        GMFields[i] = fieldName;

        tableFields.append(Field::createText(fieldName, "NULL",fieldName.size()));
    }

    // Get the grid data from the XML file
    QDomNodeList gridDataElements = root.elementsByTagName("grid_data");

    if(gridDataElements.size() != 1)
        return nullptr;

    auto gridData =  gridDataElements.at(0).toElement().text();

    // Each grid point is separated by a newline
    auto gridPoints = gridData.split("\n",QString::SkipEmptyParts);

    if(gridPoints.size() == 0)
        return nullptr;

    auto gridFeatureCollection = new FeatureCollection(parent);

    // Create the feature collection table/layers
    auto gridFeatureCollectionTable = new FeatureCollectionTable(tableFields, GeometryType::Point, SpatialReference::wgs84(),parent);
    gridFeatureCollection->tables()->append(gridFeatureCollectionTable);

    auto gridLayer = new FeatureCollectionLayer(gridFeatureCollection,parent);

    // Create red cross SimpleMarkerSymbol
    SimpleMarkerSymbol* crossSymbol = new SimpleMarkerSymbol(SimpleMarkerSymbolStyle::Cross, QColor("black"), 6, parent);

    // Create renderer and set symbol to crossSymbol
    SimpleRenderer* renderer = new SimpleRenderer(crossSymbol, parent);

    // Set the renderer for the feature layer
    gridFeatureCollectionTable->setRenderer(renderer);

    // Set the scale at which the layer will become visible - if scale is too high, then the entire view will be filled with symbols
    gridLayer->setMinScale(60000);

    // Iterate through the grid points to get the data at each point
    for(auto&& gp : gridPoints)
    {
        auto pointData = gp.split(" ");

        if(pointData.size() != numFields)
        {
            delete gridLayer;
            return nullptr;
        }

        // create the feature attributes
        QMap<QString, QVariant> featureAttributes;

        for(int i = 0; i<numFields; ++i)
        {
            auto attrbText = GMFields[i];
            auto attrbVal = pointData[i];
            featureAttributes.insert(attrbText,attrbVal);
        }

        featureAttributes.insert("AssetType", "ShakeMapGridPoint");
        featureAttributes.insert("TabName", "Point");

        auto longitude = pointData[0].toDouble();
        auto latitude = pointData[1].toDouble();

        // Create the point and add it to the feature table
        Point point(longitude,latitude);
        Feature* feature = gridFeatureCollectionTable->createFeature(featureAttributes, point, parent);

        gridFeatureCollectionTable->addFeature(feature);
    }


    return gridLayer;
}


QString XMLAdaptor::getEventName() const
{
    return eventName;
}
