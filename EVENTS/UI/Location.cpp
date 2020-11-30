#include "Location.h"

Location::Location(QObject *parent) : QObject(parent)
{

}

Location::Location(double latitude, double longitude, QObject *parent): QObject(parent)
{
    this->set(latitude, longitude);
}

Location::Location(const Location &location)
{
    this->m_latitude = location.m_latitude;
    this->m_longitude = location.m_longitude;
}

double Location::latitude() const
{
    return m_latitude;
}

void Location::setLatitude(double latitude)
{
    if(m_latitude != latitude)
    {
        m_latitude = latitude;
        emit latitudeChanged(m_latitude);
    }
}

double Location::longitude() const
{
    return m_longitude;
}

void Location::setLongitude(double longitude)
{
    if(m_longitude != longitude)
    {
        m_longitude = longitude;
        emit longitudeChanged(m_longitude);
    }
}

void Location::set(double latitude, double longitude)
{
    this->setLatitude(latitude);
    this->setLongitude(longitude);
}

