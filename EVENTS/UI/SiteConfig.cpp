#include "SiteConfig.h"

SiteConfig::SiteConfig(QObject *parent) : QObject(parent)
{
    m_type = SiteType::Grid;
    m_site.location().set(0.0, 0.0);
}

Site &SiteConfig::site()
{
    return m_site;
}

SiteGrid &SiteConfig::siteGrid()
{
    return m_siteGrid;
}

SiteConfig::SiteType SiteConfig::getType() const
{
    return m_type;
}

void SiteConfig::setType(const SiteType &type)
{
    if(m_type != type)
    {
        m_type = type;
        emit typeChanged(m_type);
    }
}

QJsonObject SiteConfig::getJson()
{
    if(m_type == SiteType::Single)
        return m_site.getJson();
    else
        return m_siteGrid.getJson();
}
