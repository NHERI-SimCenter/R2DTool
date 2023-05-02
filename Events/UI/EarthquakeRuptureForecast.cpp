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

#include "EarthquakeRuptureForecast.h"

EarthquakeRuptureForecast::EarthquakeRuptureForecast(double magMin, double magMax, double maxDist, QString model, QString name, QString samplingMethod, int numScen, QObject *parent) :
    QObject(parent), magnitudeMin(magMin), magnitudeMax(magMax), maxDistance(maxDist), EQModelType(model), EQName(name), SamplingMethod(samplingMethod), NumScen(numScen)
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


void EarthquakeRuptureForecast::reset(void)
{

}


QString EarthquakeRuptureForecast::getEQName() const
{
    return EQName;
}

void EarthquakeRuptureForecast::setEQName(const QString &value)
{
    EQName = value;
}

QString EarthquakeRuptureForecast::getSamplingMethod() const
{
    return SamplingMethod;
}

void EarthquakeRuptureForecast::setSamplingMethod(const QString &value)
{
    SamplingMethod = value;
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

int EarthquakeRuptureForecast::getNumScen() const
{
    return NumScen;
}


void EarthquakeRuptureForecast::setNumScen(const QString value)
{
    if(this->NumScen != value.toInt())
    {
        NumScen = value.toInt();
    }
}


bool EarthquakeRuptureForecast::outputToJSON(QJsonObject &jsonObject)
{
    jsonObject.insert("Type", "ERF");
    jsonObject.insert("Model",EQModelType);
    jsonObject.insert("Name", EQName);
    jsonObject.insert("min_Mag", magnitudeMin);
    jsonObject.insert("max_Mag", magnitudeMax);
    jsonObject.insert("max_Dist", maxDistance);
    //jsonObject.insert("Sampling", SamplingMethod);
    jsonObject.insert("Number", NumScen);

    return true;
}


bool EarthquakeRuptureForecast::inputFromJSON(QJsonObject &/*jsonObject*/)
{
    return true;
}
