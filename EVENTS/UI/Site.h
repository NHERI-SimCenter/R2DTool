#ifndef SITE_H
#define SITE_H

#include "Location.h"
#include "JsonSerializable.h"

#include <QObject>

class Site : public QObject, public JsonSerializable
{
    Q_OBJECT
    Q_PROPERTY(Location* location READ getLocationPtr CONSTANT)

public:
    explicit Site(QObject *parent = nullptr);
    explicit Site(double longitude, double latitude, QObject *parent = nullptr);
    void setLocation(const Location &location);
    void setLocation(double latitude, double longitude);
    Location& location();

    // JsonSerializable interface
    QJsonObject getJson();

signals:

public slots:

private:
    Location m_location;
    Location* getLocationPtr(){return &m_location;}
};

#endif // SITE_H
