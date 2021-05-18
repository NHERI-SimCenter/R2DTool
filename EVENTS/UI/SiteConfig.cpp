#include "SiteConfig.h"

SiteConfig::SiteConfig(QObject *parent) : QObject(parent)
{
    theType = SiteType::Grid;
    theSite.location().set(0.0, 0.0);
}

Site &SiteConfig::site()
{
    return theSite;
}

SiteGrid &SiteConfig::siteGrid()
{
    return theSiteGrid;
}

SiteConfig::SiteType SiteConfig::getType() const
{
    return theType;
}

void SiteConfig::setType(const SiteType &type)
{
    if(theType != type)
    {
        theType = type;
        emit typeChanged(theType);
    }
}

QJsonObject SiteConfig::getJson()
{
    if(theType == SiteType::Single)
        return theSite.getJson();
    else
        return theSiteGrid.getJson();
}
