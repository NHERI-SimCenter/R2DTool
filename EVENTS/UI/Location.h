#ifndef LOCATION_H
#define LOCATION_H

#include <QObject>

class Location : public QObject
{
    Q_OBJECT
    Q_PROPERTY(double latitude READ latitude WRITE setLatitude NOTIFY latitudeChanged)
    Q_PROPERTY(double longitude READ longitude WRITE setLongitude NOTIFY longitudeChanged)

public:
    explicit Location(QObject *parent = nullptr);
    Location(double latitude, double longitude, QObject *parent = nullptr);
    Location(const Location& location);

    double latitude() const;
    double longitude() const;

    void set(double latitude, double longitude);

signals:
    void latitudeChanged(double newLatitude);
    void longitudeChanged(double newLongitude);

public slots:
    void setLatitude(double latitude);
    void setLongitude(double longitude);

private:
    double m_latitude;
    double m_longitude;
};

#endif // LOCATION_H
