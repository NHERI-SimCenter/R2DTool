#ifndef HurricaneObject_H
#define HurricaneObject_H
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

// Written by: Stevan Gavrilovic, Frank McKenna

#include <QVector>
#include <QVariant>

struct HurricaneObject{

public:

    QVector<QStringList>& getHurricaneData(){
        return hurricaneData;
    }

    QStringList& operator[](int index) {

        return hurricaneData[index];
    }


    QStringList trackPointAtLatLon(double lat, double lon)
    {
        auto latIndex = parameterLabels.indexOf("LAT");
        auto lonIndex = parameterLabels.indexOf("LON");

        if(latIndex == -1 || lonIndex == -1)
            return QStringList();

        for(auto&& it : hurricaneData)
        {
            auto latD = it.at(latIndex).toDouble();
            auto lonD = it.at(lonIndex).toDouble();

            if((latD-lat)*(latD-lat) + (lonD-lon)*(lonD-lon) <= std::numeric_limits<double>::epsilon())
                return it;
        }

        return QStringList();
    }


    QStringList& front(void) {
        return hurricaneData.front();
    }


    int size(void) {
        return hurricaneData.size();
    }


    void push_back(const QStringList& data)
    {
        hurricaneData.push_back(data);
    }




    void push_back(const QList<QVariant>& data)
    {
        QStringList dataAsStringList;

        for(auto&& it : data)
            dataAsStringList.append(it.toString());

        hurricaneData.push_back(dataAsStringList);
    }


    bool empty(void) {
        return hurricaneData.isEmpty();
    }


    void clear() {
        hurricaneData.clear();
        landfallData.clear();
        name.clear();
        SID.clear();
        season.clear();
        indexLandfall = -1;
    }


    QString getValueOfParameter(const QString& paramName, const int dataPoint) {
        auto indexOfParam = parameterLabels.indexOf(paramName);

        if(indexOfParam == -1 || hurricaneData.size() < dataPoint || dataPoint < 0)
            return QString();

        return hurricaneData.at(dataPoint).at(indexOfParam);
    }


    QStringList getDataAtLandfall(void){
        return landfallData;
    }


    bool hasLandfall(void) {
        return landfallData.isEmpty();
    }


    double getLatitudeAtLandfall(void)
    {
        if(landfallData.empty() || landfallData.size() != parameterLabels.size())
            return 0.0;

        // By default will use USA_LAT and USA_LON, if not available fall back on the LAT and LON below
        auto indexUSALat = parameterLabels.indexOf("USA_LAT");
        auto indexLat = parameterLabels.indexOf("LAT");

        if(indexUSALat == -1 || indexLat == -1)
            return 0.0;

        auto USAlat = landfallData.at(indexUSALat).toDouble();

        if(USAlat != 0.0)
            return USAlat;

        return landfallData.at(indexLat).toDouble();
    }


    double getLongitudeAtLandfall(void)
    {
        if(landfallData.empty() || landfallData.size() != parameterLabels.size())
            return 0.0;

        // By default will use USA_LAT and USA_LON, if not available fall back on the LAT and LON below
        auto indexUSALon = parameterLabels.indexOf("USA_LON");
        auto indexLon = parameterLabels.indexOf("LON");

        if(indexUSALon == -1 || indexLon == -1)
            return 0.0;

        auto USAlon = landfallData.at(indexUSALon).toDouble();

        if(USAlon != 0.0)
            return USAlon;

        return landfallData.at(indexLon).toDouble();
    }


    // i.e., the storm direction at landfall
    double getLandingAngle(void)
    {
        if(landfallData.empty() || landfallData.size() != parameterLabels.size())
            return 0.0;

        auto indexStormDir = parameterLabels.indexOf("STORM_DIR");

        if(indexStormDir == -1)
            return 0.0;

        return landfallData.at(indexStormDir).toDouble();
    }


    // Speed in kts
    double getStormSpeedAtLandfall(void)
    {
        if(landfallData.empty() || landfallData.size() != parameterLabels.size())
            return 0.0;

        auto indexStormSpeed = parameterLabels.indexOf("STORM_SPEED");

        if(indexStormSpeed == -1)
            return 0.0;

        return landfallData.at(indexStormSpeed).toDouble();
    }


    // Pressure in mb
    double getPressureAtLandfall(void)
    {
        if(landfallData.empty() || landfallData.size() != parameterLabels.size())
            return 0.0;

        // Default to USA pressure and then WMO pressure if no USA pressure
        auto indexUSAPress = parameterLabels.indexOf("USA_PRES");
        auto indexWMOPress = parameterLabels.indexOf("WMO_PRES");

        if(indexUSAPress == -1 || indexWMOPress == -1)
            return 0.0;

        auto USAPress = landfallData.at(indexUSAPress).toDouble();

        if(USAPress != 0.0)
            return USAPress;


        // Check if there is WMO pressure at landfall (WMO data  can have longer intervals and may need to interpolate)
        auto WMOPress = landfallData.at(indexWMOPress).toDouble();

        if(WMOPress != 0.0)
            return WMOPress;

        // Need to interpolate WMO pressure

        // Get the WMO pressure at the timepoint before landfall
        auto pressBefore = 0.0;
        auto indexBefore = indexLandfall-1;
        while(pressBefore == 0.0 && indexBefore > 0)
        {
            pressBefore = hurricaneData.at(indexBefore).at(indexWMOPress).toDouble();
        }

        // Get the WMO pressure at the timepoint after landfall
        auto pressAfter = 0.0;
        auto indexAfter = indexLandfall+1;
        while(pressAfter == 0.0 && indexAfter < hurricaneData.size()-1)
        {
            pressAfter = hurricaneData.at(indexAfter).at(indexWMOPress).toDouble();
        }

        // Throw an error
        if(pressAfter == 0.0 || pressBefore == 0.0)
            return 0.0;

        // Return the interpolation
        return 0.5*(pressBefore + pressAfter);
    }


    // Storm radius in nautical mile nmile
    double getRadiusAtLandfall(void){

        if(landfallData.empty() || landfallData.size() != parameterLabels.size())
            return 0.0;

        auto indexOfUSARMW = parameterLabels.indexOf("USA_RMW");
        auto indexOfReunionRMW = parameterLabels.indexOf("REUNION_RMW");

        if(indexOfUSARMW == -1 || indexOfReunionRMW == -1)
            return 0.0;

        auto USARMW = landfallData.at(indexOfUSARMW).toDouble();

        if(USARMW != 0.0)
            return USARMW;

        return landfallData.at(indexOfReunionRMW).toDouble();

    }

    QVector<QStringList> hurricaneData;
    QStringList parameterLabels;

    QStringList landfallData;
    int indexLandfall = -1;

    QString name;
    QString SID; // The storm id
    QString season; // i.e., the year

};



#endif // AssetsWidget_H
