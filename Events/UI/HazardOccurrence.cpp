/* *****************************************************************************
Copyright (c) 2016-2022, The Regents of the University of California (Regents).
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

// Written by: Kuanshi Zhong

#include "HazardOccurrence.h"

#include <QJsonArray>

HazardOccurrence::HazardOccurrence(QString model, QString name, QString hoModel, \
                                   QString numCandid, int numScen, int numGMMap, QString hcInput, \
                                   QString hcEd, QString imt, double imp, QObject *parent) :
    QObject(parent), EQModelType(model), EQName(name), HOModel(hoModel), numCandidateEQ(numCandid), NumScen(numScen), \
    NumGMMap(numGMMap), hazardCurveInput(hcInput), nshmEdition(hcEd), imType(imt), imPeriod(imp)
{

}


void HazardOccurrence::reset(void)
{

}

QString HazardOccurrence::getEQName() const
{
    return EQName;
}

void HazardOccurrence::setEQName(const QString &value)
{
    EQName = value;
}


QString HazardOccurrence::getEQModelType() const
{
    return EQModelType;
}


void HazardOccurrence::setEQModelType(const QString &value)
{
    EQModelType = value;
}


int HazardOccurrence::getNumScen() const
{
    return NumScen;
}


void HazardOccurrence::setNumScen(const QString value)
{
    if(this->NumScen != value.toInt())
    {
        NumScen = value.toInt();
    }
}


int HazardOccurrence::getNumGMMap() const
{
    return NumGMMap;
}


void HazardOccurrence::setNumGMMap(const QString value)
{
    if(this->NumGMMap != value.toInt())
    {
        NumGMMap = value.toInt();
    }
}


QString HazardOccurrence::getHazardCurve() const
{
    return hazardCurveInput;
}


void HazardOccurrence::setHazardCurve(const QString &value)
{
    hazardCurveInput = value;
}


QString HazardOccurrence::getNSHMEdition() const
{
    return nshmEdition;
}


void HazardOccurrence::setNSHMEdition(const QString &value)
{
    nshmEdition = value;
}


QString HazardOccurrence::getIMType() const
{
    return imType;
}


void HazardOccurrence::setIMType(const QString &value)
{
    imType = value;
}


double HazardOccurrence::getPeriod() const
{
    return imPeriod;
}


void HazardOccurrence::setPeriod(const QString value)
{
    if(this->imPeriod != value.toDouble())
    {
        imPeriod = value.toInt();
    }
}


QList<int> HazardOccurrence::return_periods() const
{
    return m_return_periods;
}


void HazardOccurrence::setReturnPeriods(const QList<int> &return_periods)
{
    m_return_periods = return_periods;
}


void HazardOccurrence::setReturnPeriods(const QString &return_periods)
{
    returnPeriodsText = return_periods;
}


void HazardOccurrence::addReturnPeriod(int return_period)
{
    this->m_return_periods.append(return_period);
    qSort(this->m_return_periods);
}


QString HazardOccurrence::getHOModelType() const
{
    return HOModel;
}


void HazardOccurrence::setHazardOccurrenceModel(const QString &value)
{
    HOModel = value;
}


void HazardOccurrence::setCandidateEQ(const QString value)
{
    numCandidateEQ = value;
}


QString HazardOccurrence::getCandidateEQ() const
{
    return numCandidateEQ;
}


bool HazardOccurrence::outputToJSON(QJsonObject &jsonObject)
{
    jsonObject.insert("Type", "ERF");
    jsonObject.insert("Model",EQModelType);
    jsonObject.insert("Name", EQName);
    jsonObject.insert("min_Mag", 0.0);
    jsonObject.insert("max_Mag", 10.0);
    jsonObject.insert("max_Dist", 999);
    jsonObject.insert("OccurrenceSampling",true);
    QJsonObject hoData;
    hoData.insert("Model", HOModel);
    hoData.insert("EarthquakeSampleSize", NumScen);
    hoData.insert("GroundMotionMapSize", NumGMMap);
    hoData.insert("HazardCurveInput", hazardCurveInput);
    hoData.insert("Edition", nshmEdition);
    hoData.insert("IntensityMeasure", imType);
    hoData.insert("Period", imPeriod);
    QJsonArray arrayReturnPeriods;
    for(auto&& it : m_return_periods)
    {
        arrayReturnPeriods.append(it);
    }
    hoData.insert("ReturnPeriods", arrayReturnPeriods);
    jsonObject.insert("HazardOccurrence",hoData);

    return true;
}


bool HazardOccurrence::inputFromJSON(QJsonObject &/*jsonObject*/)
{
    return true;
}
