#include "SiteScatter.h"

SiteScatter::SiteScatter(QObject *parent) : QObject(parent)
{
    allSites.reserve(0);
}

int SiteScatter::getNumSites()
{
    return allSites.count();
}

void SiteScatter::initialize(int L)
{
    allSites.reserve(L);
}


void SiteScatter::addSite(const UserSpecifiedSite &userSite)
{
    allSites.push_back(userSite);
    emit siteChanged();
}


void SiteScatter::deleteSite(const UserSpecifiedSite &userSite)
{
    for (int i = 0; i != allSites.count(); ++i)
    {
        // Checking latitude and longitude
        if (userSite.Latitude.compare(allSites[i].Latitude) && userSite.Longitude.compare(allSites[i].Longitude))
        {
            allSites.removeAt(i);
            emit siteChanged();
            return;
        }
    }
}


void SiteScatter::clearSites()
{
    allSites.clear();
}


QList<UserSpecifiedSite> SiteScatter::getSiteList()
{
    return allSites;
}


QJsonObject SiteScatter::getJson()
{
    QJsonObject siteJson;
    siteJson.insert("Type","Scatter");
    return siteJson;
}
