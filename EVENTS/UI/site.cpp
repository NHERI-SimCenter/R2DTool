#include "Site.h"

Site::Site(QObject *parent) : QObject(parent)
{
    this->setLocation(0.0, 0.0);
}

Site::Site(double latitude, double longitude, QObject *parent) : QObject(parent)
{
    this->setLocation(latitude, longitude);
}

void Site::setLocation(double latitude, double longitude)
{
    this->m_location.set(latitude, longitude);
}

Location &Site::location()
{
    return this->m_location;
}

QJsonObject Site::getJson()
{
    QJsonObject siteJson;
    siteJson.insert("Type","SingleLocation");
    QJsonObject locationJson;

    locationJson.insert("Latitude", this->location().latitude());
    locationJson.insert("Longitude", this->location().longitude());
    siteJson.insert("Location", locationJson);

    return siteJson;
}
