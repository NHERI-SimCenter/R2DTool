#include "GroundMotionTimeHistory.h"

GroundMotionTimeHistory::GroundMotionTimeHistory(QString name) : GMName(name)
{
    dT = 0.0;
    scalingFactor = 1.0;
    peakIntensityMeasureX = 0.0;
    peakIntensityMeasureY = 0.0;
    peakIntensityMeasureZ = 0.0;
}


QVector<double> GroundMotionTimeHistory::getX() const
{
    return x;
}


void GroundMotionTimeHistory::setX(const QVector<double> &value)
{
    x = value;
}


QVector<double> GroundMotionTimeHistory::getY() const
{
    return y;
}


void GroundMotionTimeHistory::setY(const QVector<double> &value)
{
    y = value;
}


QVector<double> GroundMotionTimeHistory::getZ() const
{
    return z;
}


void GroundMotionTimeHistory::setZ(const QVector<double> &value)
{
    z = value;
}


double GroundMotionTimeHistory::getDT() const
{
    return dT;
}


void GroundMotionTimeHistory::setDT(double value)
{
    dT = value;
}


QString GroundMotionTimeHistory::getName() const
{
    return GMName;
}

double GroundMotionTimeHistory::getPeakIntensityMeasureX() const
{
    return peakIntensityMeasureX;
}

void GroundMotionTimeHistory::setPeakIntensityMeasureX(double value)
{
    peakIntensityMeasureX = value;
}

double GroundMotionTimeHistory::getPeakIntensityMeasureY() const
{
    return peakIntensityMeasureY;
}

void GroundMotionTimeHistory::setPeakIntensityMeasureY(double value)
{
    peakIntensityMeasureY = value;
}

double GroundMotionTimeHistory::getPeakIntensityMeasureZ() const
{
    return peakIntensityMeasureZ;
}

void GroundMotionTimeHistory::setPeakIntensityMeasureZ(double value)
{
    peakIntensityMeasureZ = value;
}

double GroundMotionTimeHistory::getScalingFactor() const
{
    return scalingFactor;
}

void GroundMotionTimeHistory::setScalingFactor(double value)
{
    scalingFactor = value;
}
