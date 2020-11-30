#include "RuptureLocation.h"



RuptureLocation::RuptureLocation(QObject *parent):Location(parent)
{

}

RuptureLocation::RuptureLocation(double latitude, double longitude, QObject *parent):Location(latitude, longitude, parent)
{
    m_depth = 0.0;
}

double RuptureLocation::depth() const
{
    return m_depth;
}

void RuptureLocation::setDepth(double depth)
{
    if(this->m_depth != depth)
    {
        m_depth = depth;
        emit this->depthChanged(this->m_depth);
    }
}
