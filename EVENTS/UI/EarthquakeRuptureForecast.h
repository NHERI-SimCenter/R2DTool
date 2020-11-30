#ifndef EarthquakeRuptureForecast_H
#define EarthquakeRuptureForecast_H

#include <QObject>
#include "JsonSerializable.h"

class EarthquakeRuptureForecast : public QObject, JsonSerializable
{
    Q_OBJECT

public:
    EarthquakeRuptureForecast(double magMin, double magMax, double maxDist, QString model, QString name, QObject *parent = nullptr);

    double getMagnitudeMin() const;
    double getMagnitudeMax() const;
    double getMaxDistance() const;
    QString getEQName() const;
    QString getEQModelType() const;

    QJsonObject getJson();

signals:

public slots:
    void setMagnitudeMin(double magnitude);
    void setMagnitudeMax(double magnitude);
    void setMaxDistance(double value);
    void setEQName(const QString &value);
    void setEQModelType(const QString &value);

private:
    double magnitudeMin;
    double magnitudeMax;
    double maxDistance;

    QString EQModelType;
    QString EQName;

};

#endif // EarthquakeRuptureForecast_H
