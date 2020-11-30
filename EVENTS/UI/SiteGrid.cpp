#include "SiteGrid.h"

SiteGrid::SiteGrid(QObject *parent) : QObject(parent)
{
    m_latitude.set(0.0, 0.0, 10);
    m_longitude.set(0.0, 0.0, 10);
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

QJsonObject SiteGrid::getJson()
{
    QJsonObject siteJson;
    siteJson.insert("Type","Grid");

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

    siteJson.insert("Grid", gridJson);
    return siteJson;
}
