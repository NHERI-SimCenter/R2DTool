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
#include "GroundMotionStation.h"

#include <QFileInfo>
#include <QString>
#include <QDir>
#include <QStringList>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>

GroundMotionStation::GroundMotionStation(QString path, double lat, double lon) : stationFilePath(path), latitude(lat), longitude(lon)
{
}


double GroundMotionStation::getLatitude() const
{
    return latitude;
}


double GroundMotionStation::getLongitude() const
{
    return longitude;
}


void GroundMotionStation::importGroundMotions(void)
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
    QStringList tableHeadings = data.first();

    // Pop off the row that contains the header information
    data.pop_front();

    stationData = data;

    auto numRows = data.size();
    auto numCols = tableHeadings.size();

    if(tableHeadings.at(0).compare("GM_file") == 0)
    {
        if(numCols != 2)
            throw "The number of columns in the header should be 2";

        QFileInfo stationInfo(stationFilePath);

        auto baseDir = stationInfo.dir().absolutePath();

        // Get the data
        for(int i = 0; i<numRows; ++i)
        {
            auto rowStringList = data[i];

            if(rowStringList.size() != numCols)
                throw "The number of columns in the row " + QString::number(i) + " should be " + QString::number(numCols);

            auto GMFile = rowStringList[0];

            bool ok;
            auto factor = rowStringList[1].toDouble(&ok);

            if(!ok)
                throw "Error converting the string " + rowStringList[1] + " to a double";

            auto GMFilePath = baseDir + QDir::separator() + GMFile + ".json";

            this->importGroundMotionTimeHistory(GMFilePath, factor);
        }
    }

}


void GroundMotionStation::importGroundMotionTimeHistory(const QString& filePath,const double scalingFactor)
{
    QFile file(filePath);
    if (!file.open(QFile::ReadOnly | QFile::Text))
        throw "Could not open the file at: "+ filePath;


    // place contents of file into json object
    QString val;
    val=file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(val.toUtf8());
    QJsonObject jsonObj = doc.object();

    // close file
    file.close();

    // Get the name
    auto gmNameObj = jsonObj.value("name");

    if(gmNameObj.isNull())
        throw "NUll JSON object for field 'name'";

    QString gmName = gmNameObj.toString();

    // Get the time=step size
    auto dTObj = jsonObj.value("dT");

    if(dTObj.isNull())
        throw "NUll JSON object for field 'dT'";

    double dT = dTObj.toDouble();

    GroundMotionTimeHistory newGM(gmName);

    newGM.setDT(dT);

    // Get the time history in the x-direction
    if(!jsonObj.value("data_x").isNull())
    {
        auto xArray = jsonObj.value("data_x").toArray();

        QVector<double> data_x(xArray.size());

        for(int i = 0; i<xArray.size(); ++i)
        {
            data_x[i] = xArray.at(i).toDouble(0.0);
        }

        newGM.setX(data_x);
    }

    // Get the time history in the y-direction
    if(!jsonObj.value("data_y").isNull())
    {
        auto yArray = jsonObj.value("data_y").toArray();

        QVector<double> data_y(yArray.size());

        for(int i = 0; i<yArray.size(); ++i)
        {
            data_y[i] = yArray.at(i).toDouble(0.0);
        }

        newGM.setY(data_y);
    }

    // Get the time history in the y-direction
    if(!jsonObj.value("data_z").isNull())
    {
        auto zArray = jsonObj.value("data_z").toArray();

        QVector<double> data_z(zArray.size());

        for(int i = 0; i<zArray.size(); ++i)
        {
            data_z[i] = zArray.at(i).toDouble(0.0);
        }

        newGM.setZ(data_z);
    }

    // Set PGA if avail.
    if(!jsonObj.value("PGA_x").isNull())
    {
        double PGA_x = jsonObj.value("PGA_x").toDouble(0.0);

        newGM.setPeakIntensityMeasureX(PGA_x);
    }

    if(!jsonObj.value("PGA_y").isNull())
    {
        double PGA_y = jsonObj.value("PGA_y").toDouble(0.0);

        newGM.setPeakIntensityMeasureY(PGA_y);
    }

    if(!jsonObj.value("PGA_z").isNull())
    {
        double PGA_z = jsonObj.value("PGA_z").toDouble(0.0);

        newGM.setPeakIntensityMeasureZ(PGA_z);
    }

    newGM.setScalingFactor(scalingFactor);

    groundMotionTimeHistories.push_back(std::move(newGM));

}

//#ifdef Q_GIS
QgsFeature GroundMotionStation::getStationFeature() const
{
    return stationFeature;
}

void GroundMotionStation::setStationFeature(const QgsFeature &value)
{
    stationFeature = value;
}
//#endif


//#ifdef ARC_GIS
//QMap<QString, QVariant> GroundMotionStation::getStationAttributes() const
//{
//    return stationAttributes;
//}


//void GroundMotionStation::setStationAttributes(const QMap<QString, QVariant> &value)
//{
//    stationAttributes = value;
//}
//#endif


QVector<GroundMotionTimeHistory> GroundMotionStation::getStationGroundMotions() const
{
    return groundMotionTimeHistories;
}


QString GroundMotionStation::getStationFilePath() const
{
    return stationFilePath;
}

QVector<QStringList> GroundMotionStation::getStationData() const
{
    return stationData;
}


