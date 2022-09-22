#ifndef HazardOccurrence_H
#define HazardOccurrence_H

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

#include <QObject>
#include "JsonSerializable.h"

class HazardOccurrence : public QObject, JsonSerializable
{
    Q_OBJECT

public:
    HazardOccurrence(QString model, QString name, QString hoModel, QString numCandid, int numScen, int numGMMap, QString hcInput, QString hcEd, QString imt, double imp, QObject *parent = nullptr);

    QString getEQName() const;
    QString getEQModelType() const;
    int getNumScen() const;
    int getNumGMMap() const;
    QString getHazardCurve() const;
    QString getNSHMEdition() const;
    QString getIMType() const;
    double getPeriod() const;
    QList<int> return_periods() const;
    void setReturnPeriods(const QList<int> &return_periods);
    void setReturnPeriods(const QString &return_periods);
    void addReturnPeriod(int return_period);
    QString getHOModelType() const;
    QString getCandidateEQ() const;

    bool outputToJSON(QJsonObject &jsonObject);
    bool inputFromJSON(QJsonObject &jsonObject);

    void reset(void);

signals:

public slots:
    void setEQName(const QString &value);
    void setEQModelType(const QString &value);
    void setHazardOccurrenceModel(const QString &value);
    void setNumScen(const QString value);
    void setNumGMMap(const QString value);
    void setHazardCurve(const QString &value);
    void setNSHMEdition(const QString &value);
    void setIMType(const QString &value);
    void setPeriod(const QString value);
    void setCandidateEQ(const QString value);

private:

    QString EQModelType;
    QString EQName;
    QString HOModel;
    int NumScen;
    int NumGMMap;
    QString hazardCurveInput;
    QString nshmEdition;
    QString imType;
    double imPeriod;
    QList<int> m_return_periods {224, 475, 975, 2475};
    QString returnPeriodsText;
    QString numCandidateEQ;
};

#endif // HazardOccurrence_H
