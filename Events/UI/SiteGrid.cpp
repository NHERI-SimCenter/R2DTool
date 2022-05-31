/* *****************************************************************************
Copyright (c) 2016-2017, The Regents of the University of California (Regents).
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

// Written: Stevan Gavrilovic

#include "SiteGrid.h"

SiteGrid::SiteGrid(QObject *parent) : QObject(parent)
{
    m_latitude.set(0.0, 0.0, 5);
    m_longitude.set(0.0, 0.0, 5);
}


GridDivision &SiteGrid::latitude()
{
    return m_latitude;
}


GridDivision &SiteGrid::longitude()
{
    return m_longitude;
}


int SiteGrid::getNumSites()
{
    return (latitude().divisions() + 1) * (longitude().divisions() + 1);
}


void SiteGrid::setCenter(double latitude, double longitude)
{
    double centerlatitude = (m_latitude.min() + m_latitude.max())/2;
    double centerlongitude = (m_longitude.min() + m_longitude.max())/2;

    //Calculating deltas
    double delLat = latitude - centerlatitude;
    double delLon = longitude - centerlongitude;

    //Moving
    if(delLat > 0)
    {
        m_latitude.setMax(m_latitude.max() + delLat);
        m_latitude.setMin(m_latitude.min() + delLat);
    }
    else
    {
        m_latitude.setMin(m_latitude.min() + delLat);
        m_latitude.setMax(m_latitude.max() + delLat);
    }

    if(delLon > 0)
    {
        m_longitude.setMax(m_longitude.max() + delLon);
        m_longitude.setMin(m_longitude.min() + delLon);
    }
    else
    {
        m_longitude.setMin(m_longitude.min() + delLon);
        m_longitude.setMax(m_longitude.max() + delLon);
    }
}


GridDivision *SiteGrid::getLatitudePtr()
{
    return &m_latitude;
}


GridDivision *SiteGrid::getLongitudePtr()
{
    return &m_longitude;
}


bool SiteGrid::outputToJSON(QJsonObject &jsonObject)
{
    jsonObject.insert("Type","Grid");

    QJsonObject gridJson;
    QJsonObject latitudeJson;
    latitudeJson.insert("Min", m_latitude.min());
    latitudeJson.insert("Max", m_latitude.max());
    latitudeJson.insert("Divisions", m_latitude.divisions());
    gridJson.insert("Latitude", latitudeJson);

    QJsonObject longitudeJson;
    longitudeJson.insert("Min", m_longitude.min());
    longitudeJson.insert("Max", m_longitude.max());
    longitudeJson.insert("Divisions", m_longitude.divisions());
    gridJson.insert("Longitude", longitudeJson);

    jsonObject.insert("Grid", gridJson);

    return true;
}


bool SiteGrid::inputFromJSON(QJsonObject &/*jsonObject*/)
{
    return true;
}


void SiteGrid::reset(void)
{

}

