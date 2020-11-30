#ifndef SITECONFIG_H
#define SITECONFIG_H

#include <QObject>
#include <Site.h>
#include <SiteGrid.h>
#include <JsonSerializable.h>

class SiteConfig : public QObject, public JsonSerializable
{
    Q_OBJECT
    Q_PROPERTY(SiteType type READ getType WRITE setType NOTIFY typeChanged)
    Q_ENUMS(SiteType)

public:
    explicit SiteConfig(QObject *parent = nullptr);
    enum SiteType {Single = 0, Grid = 1};

    Site& site();
    SiteGrid& siteGrid();
    SiteType getType() const;

signals:
    void typeChanged(SiteType newType);

public slots:
    void setType(const SiteType &type);

private:
    Site m_site;
    SiteGrid m_siteGrid;
    SiteType m_type;

    // JsonSerializable interface
public:
    QJsonObject getJson();
};

#endif // SITECONFIG_H
