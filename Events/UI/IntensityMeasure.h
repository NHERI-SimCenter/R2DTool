#ifndef INTENSITYMEASURE_H
#define INTENSITYMEASURE_H
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

#include "JsonSerializable.h"

#include <QObject>
#include <QJsonArray>

class IntensityMeasure : public QObject, JsonSerializable
{
    Q_OBJECT
public:
    explicit IntensityMeasure(QObject *parent = nullptr);

    QString type() const;

    const QStringList& validTypes();

    QList<double> periods() const;
    void setPeriods(const QList<double> &periods);
    void setPeriods(const QString &periods);
    void addPeriod(double period);
    double getImtTruc() const;

    bool outputToJSON(QJsonObject &jsonObject);
    bool inputFromJSON(QJsonObject &jsonObject);

    void reset(void);

signals:
    void typeChanged(QString newType);
    void imtScaleChanged(QString newScale);

public slots:
    bool setType(const QString &type);
    void setImtLevels(const QString &value);
    void setImtScale(const QString &value);
    void setImtTruc(double value);

private:
    QString m_type;
    QList<double> m_periods;

    QString periodsText;

    QJsonArray imtLevels = {0.01,10.0,100}; // default intensity measure levels
    QString imtScale = "Log"; // default intensity measure scale is log

    double imtTruc = 3.0; // default trucation levels 3 \sigma

public:
    QJsonObject getJson();
};

#endif // INTENSITYMEASURE_H
