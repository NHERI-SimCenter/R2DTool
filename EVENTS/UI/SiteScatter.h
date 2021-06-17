#ifndef SITESCATTER_H
#define SITESCATTER_H

#include <QObject>
#include "JsonSerializable.h"


struct UserSpecifiedSite
{
    int SiteNum;
    QString Longitude;
    QString Latitude;
    QString Vs30;
    QString z1pt0; // z1.0
    QString z2pt5; // z2.5
    QString zTR; // depth to the bedrock
};


class SiteScatter : public QObject, public JsonSerializable
{
    Q_OBJECT

public:
    explicit SiteScatter(QObject *parent = nullptr);
    int getNumSites();

    // initialization
    void initialize(int L);
    // add site
    void addSite(const UserSpecifiedSite &userSite);
    // delete site
    void deleteSite(const UserSpecifiedSite &userSite);
    // clear
    void clearSites();
    // get site list
    QList<UserSpecifiedSite> getSiteList();

signals:

    void siteChanged();

public slots:

private:

    // Site list
    QList<UserSpecifiedSite> allSites;

public:
    QJsonObject getJson();
};

#endif // SITESCATTER_H
