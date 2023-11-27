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

#include "GeoJSONReaderWriter.h"

#include <QVector>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QStringList>
#include <QFile>

GeoJSONReaderWriter::GeoJSONReaderWriter()
{

}


int GeoJSONReaderWriter::saveGeoJsonFile(const QVector<QStringList>& data,
                                         const QStringList& headers,
                                         const QString assetType,
                                         const QString& pathToFile,
                                         QString& err)
{

    auto indexFootprint = this->getIndexOfVal(headers, "footprint");
    auto indexLatitude = -1;
    auto indexLongitude = -1;

    if(indexFootprint == -1)
    {
        // First check if a footprint was provided
        indexLatitude = this->getIndexOfVal(headers, "latitude");
        indexLongitude = this->getIndexOfVal(headers, "longitude");

        if(indexLongitude == -1 || indexLatitude == -1)
        {
            indexLatitude = this->getIndexOfVal(headers, "lat");
            indexLongitude = this->getIndexOfVal(headers, "lon");

            if(indexLongitude == -1 || indexLatitude == -1){
                err = "Could not find geometry or latitude and longitude in the header columns";
                return -1;
            }
        }
    }

    QJsonObject assetDictionary;
    assetDictionary["type"]="FeatureCollection";

    QJsonObject crs;
    crs["type"] = "name";
    QJsonObject properties;

    // All simcenter tables should be in the 4326 CRS, i.e., lat./lon.
    properties["name"] = "urn:ogc:def:crs:EPSG::4326";
    crs["properties"] = properties;

    assetDictionary["crs"]=crs;

    auto numCol = data.first().size();

    // Check that there are items in each row and that the number of items is consistent
    if(numCol==0)
    {
        err = "Empty data vector came into the function save data.";
        return false;
    }

    auto dataCopy = data;

    dataCopy.removeFirst();

    QJsonArray featuresArray;

    for(auto&& row : dataCopy)
    {
        // Each row in the table is a feature
        QJsonObject feature;

        // Parse the geometry
        if (indexFootprint != -1)
        {
            QString footPrintString = row[indexFootprint];

            // First, convert the json string to a QByteArray
            QByteArray jsonByteArray = footPrintString.toUtf8();

            // Then, parse the json byte array to a QJsonDocument
            QJsonDocument jsonDocument = QJsonDocument::fromJson(jsonByteArray);

            // Finally, extract the QJsonObject from the QJsonDocument
            feature = jsonDocument.object();
        }
        else
        {
            feature["type"] = "Feature";

            QJsonObject geometry;
            geometry["type"] = "Point";

            auto latitude = row[indexLatitude];
            auto longitude = row[indexLongitude];

            QJsonArray coords = QJsonArray {longitude.toDouble(),latitude.toDouble()};
            geometry["coordinates"] = coords;

            feature["geometry"] = geometry;
        }



        QJsonObject featureProperties;

        // Each column in the table is a feature attribute
        for(int i = 0; i<numCol; ++i)
        {
            auto featureKey = headers[i];
            auto rowColData = row[i];

            featureProperties[featureKey] = rowColData;
        }

        featureProperties["type"] = assetType;

        feature["properties"] = featureProperties;

        featuresArray.append(feature);
    }

    assetDictionary["features"]=featuresArray;


    QFile file(pathToFile);
    if (!file.open(QFile::WriteOnly | QFile::Text))
    {
        err = "Error creating the asset output json file in GeojsonAssetInputWidget";
        return -1;
    }

    // Write the data to the file
    QJsonDocument doc(assetDictionary);
    file.write(doc.toJson());
    file.close();

    return 0;
}


int GeoJSONReaderWriter::getIndexOfVal(const QStringList& headersStr, const QString val)
{
    for(int i =0; i<headersStr.size(); ++i)
    {
        QString headerStr = headersStr.at(i);
        if(headerStr.contains(val, Qt::CaseInsensitive))
            return i;
    }

    return -1;
}
