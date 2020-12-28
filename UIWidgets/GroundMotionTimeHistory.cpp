/* *****************************************************************************
Copyright (c) 2016-2021, The Regents of the University of California (Regents).
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the FreeBSD Project.

REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS
PROVIDED "AS IS". REGENTS HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT,
UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

*************************************************************************** */

// Written by: Stevan Gavrilovic

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
