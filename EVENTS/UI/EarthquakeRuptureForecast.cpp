#include "EarthquakeRuptureForecast.h"


EarthquakeRuptureForecast::EarthquakeRuptureForecast(double magMin, double magMax, double maxDist, QString model, QString name, QObject *parent) :
    QObject(parent), magnitudeMin(magMin), magnitudeMax(magMax), maxDistance(maxDist), EQModelType(model), EQName(name)
{

}


double EarthquakeRuptureForecast::getMagnitudeMin() const
{
    return magnitudeMin;
}


double EarthquakeRuptureForecast::getMagnitudeMax() const
{
    return magnitudeMax;
}


void EarthquakeRuptureForecast::setMagnitudeMin(double magnitude)
{
    if(this->magnitudeMin != magnitude)
    {
        magnitudeMin = magnitude;
    }
}


void EarthquakeRuptureForecast::setMagnitudeMax(double magnitude)
{
    if(this->magnitudeMax != magnitude)
    {
        magnitudeMax = magnitude;
    }
}


void EarthquakeRuptureForecast::setMaxDistance(double value)
{
    maxDistance = value;
}

QString EarthquakeRuptureForecast::getEQName() const
{
    return EQName;
}

void EarthquakeRuptureForecast::setEQName(const QString &value)
{
    EQName = value;
}

QString EarthquakeRuptureForecast::getEQModelType() const
{
    return EQModelType;
}

void EarthquakeRuptureForecast::setEQModelType(const QString &value)
{
    EQModelType = value;
}


double EarthquakeRuptureForecast::getMaxDistance() const
{
    return maxDistance;
}


QJsonObject EarthquakeRuptureForecast::getJson()
{
    QJsonObject rupture;
    rupture.insert("Type", "ERF");
    rupture.insert("Model",EQModelType);
    rupture.insert("Name", EQName);
    rupture.insert("min_Mag", magnitudeMin);
    rupture.insert("max_Mag", magnitudeMax);
    rupture.insert("max_Dist", maxDistance);

    return rupture;
}

