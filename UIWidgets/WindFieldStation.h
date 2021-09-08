#ifndef WindFieldStation_H
#define WindFieldStation_H
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

#include <QVector>
#include <QVariant>

#ifdef ARC_GIS
namespace Esri
{
namespace ArcGISRuntime
{
class Feature;
}
}
#endif

#ifdef Q_GIS

class QgsFeature;

#endif

class WindFieldStation
{
public:
    WindFieldStation(QString name, double lat, double lon);

    bool isNull(){return stationName.isEmpty();}

    double getLatitude() const;

    double getLongitude() const;

    QString getStationFilePath() const;
    void setStationFilePath(const QString &value);

    void importWindFieldStation(void);

    // Function to convert a QString and QVariant to double
    // Throws an error exception if conversion fails
    template <typename T>
    auto objectToDouble(T obj)
    {
        // Assume a zero value if the string is empty
        if(obj.isNull())
            return 0.0;

        bool OK;
        auto val = obj.toDouble(&OK);

        if(!OK)
            throw QString("Could not convert the object to a double");

        return val;
    }

    QVector<double> getPeakWindSpeeds() const;

#ifdef ARC_GIS
    Esri::ArcGISRuntime::Feature *getStationFeature() const;
    void setStationFeature(Esri::ArcGISRuntime::Feature *value);
#endif

#ifdef Q_GIS
    QgsFeature *getStationFeature() const;
    void setStationFeature(QgsFeature *value);
#endif


    int updateFeatureAttribute(const QString& attribute, const QVariant& value);

    QVector<double> getPeakInundationHeights() const;

private:

    QString stationFilePath;

    QString stationName;

    double latitude;

    double longitude;

    QVector<double> peakWindSpeeds;
    QVector<double> peakInundationHeights;

#ifdef ARC_GIS
    Esri::ArcGISRuntime::Feature* stationFeature;
#endif

#ifdef Q_GIS
    QgsFeature* stationFeature;
#endif


};

#endif // WindFieldStation_H
