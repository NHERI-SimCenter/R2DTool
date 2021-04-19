#ifndef GENERALINFORMATIONWIDGET_H
#define GENERALINFORMATIONWIDGET_H
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

// Written by: Stevan Gavrilovic, Frank McKenna

#include <SimCenterWidget.h>

class QLineEdit;
class QGridLayout;
class QComboBox;
class QCheckBox;

class GeneralInformationWidget : public SimCenterWidget
{

    Q_OBJECT
public:
    explicit GeneralInformationWidget(QWidget *parent);
    ~GeneralInformationWidget();

    bool outputToJSON(QJsonObject &jsonObject);
    bool inputFromJSON(QJsonObject &jsonObject);

    bool outputToJSON(QJsonArray &arrayObject);
    bool inputFromJSON(QJsonArray &arrayObject);

    QString getAnalysisName(void);

    void clear(void);

    enum LengthUnit{m, meter, cm, centimeter, mm, milimeter, in, inch, ft, foot};
    Q_ENUM(LengthUnit)
    enum ForceUnit{N, kN, lb, kips};
    Q_ENUM(ForceUnit)
    enum TimeUnit{sec, seconds, min, minutes, hr, hour};
    Q_ENUM(TimeUnit)
    enum TemperatureUnit{C, F, K};
    Q_ENUM(TemperatureUnit)

    bool setAssetTypeState(QString assetType, bool);
    QString getLengthUnit();
    QString getForceUnit();

signals:
    void assetChanged(QString name, bool checked);

private:

    QGridLayout* getInfoLayout(void);

    QLineEdit* nameEdit;

    QComboBox* unitsForceCombo;
    QComboBox* unitsLengthCombo;
    QComboBox* unitsTemperatureCombo;
    QComboBox* unitsTimeCombo;

    QCheckBox* buildingsCheckBox;
    QCheckBox* soilCheckBox;
    QCheckBox* waterCheckBox;
    QCheckBox* sewerCheckBox;  
    QCheckBox* gasCheckBox;
    QCheckBox* transportationCheckBox;
  
    QCheckBox* EDPCheckBox;
    QCheckBox* DMCheckBox;
    QCheckBox* DVCheckBox;
    QCheckBox* realizationCheckBox;

    template<typename UnitEnum> QString unitEnumToString(UnitEnum enumValue);
    template<typename UnitEnum> UnitEnum unitStringToEnum(QString unitString);
};


#endif // GENERALINFORMATIONWIDGET_H
