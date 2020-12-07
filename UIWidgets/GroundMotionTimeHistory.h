#ifndef GROUNDMOTIONTIMEHISTORY_H
#define GROUNDMOTIONTIMEHISTORY_H

#include <QString>
#include <QVector>


class GroundMotionTimeHistory
{
    enum IntensityMeasureType {PGA, PGV, PGD, PSA, UNKNOWN};

public:
    GroundMotionTimeHistory(QString name);

    QVector<double> getX() const;
    void setX(const QVector<double> &value);

    QVector<double> getY() const;
    void setY(const QVector<double> &value);

    QVector<double> getZ() const;
    void setZ(const QVector<double> &value);

    double getDT() const;
    void setDT(double value);

    QString getName() const;

    double getPeakIntensityMeasureX() const;
    void setPeakIntensityMeasureX(double value);

    double getPeakIntensityMeasureY() const;
    void setPeakIntensityMeasureY(double value);

    double getPeakIntensityMeasureZ() const;
    void setPeakIntensityMeasureZ(double value);

    double getScalingFactor() const;
    void setScalingFactor(double value);

private:

    QString GMName;

    double dT;

    double scalingFactor;

    QVector<double> x;
    QVector<double> y;
    QVector<double> z;

    double peakIntensityMeasureX;
    double peakIntensityMeasureY;
    double peakIntensityMeasureZ;
};

#endif // GROUNDMOTIONTIMEHISTORY_H
