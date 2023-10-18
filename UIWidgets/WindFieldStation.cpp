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

#include "CSVReaderWriter.h"
#include "WindFieldStation.h"

#include <QFileInfo>
#include <QString>
#include <QDir>
#include <QStringList>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>

WindFieldStation::WindFieldStation(QString name, double lat, double lon) : stationName(name), latitude(lat), longitude(lon)
{
}


double WindFieldStation::getLatitude() const
{
    return latitude;
}


double WindFieldStation::getLongitude() const
{
    return longitude;
}


void WindFieldStation::importWindFieldStation(void)
{
    CSVReaderWriter csvTool;

    QString err;
    QVector<QStringList> data = csvTool.parseCSVFile(stationFilePath,err);

    // Return if there is an error or the data is empty
    if(!err.isEmpty())
        throw err;

    if(data.size() < 2)
        throw "The file " + stationFilePath + " is empty";

    // Get the header file
    tableHeadings = data.first();

    // Pop off the row that contains the header information
    data.pop_front();

    stationData = data;
}


QgsFeature WindFieldStation::getStationFeature() const
{
    return stationFeature;
}

void WindFieldStation::setStationFeature(QgsFeature value)
{
    stationFeature = value;
}


void WindFieldStation::setStationFilePath(const QString &value)
{
    stationFilePath = value;
}


QString WindFieldStation::getStationFilePath() const
{
    return stationFilePath;
}

int WindFieldStation::updateFeatureAttribute(const QString& attribute, const QVariant& value)
{
    stationFeature.setAttribute(attribute,value);

    return 0;
}

QVector<QStringList> WindFieldStation::getStationData() const
{
    return stationData;
}

QStringList WindFieldStation::getStationDataHeaders() const
{
    return tableHeadings;
}

