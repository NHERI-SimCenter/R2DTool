#ifndef HazardConsistentScenarioWidget_H
#define HazardConsistentScenarioWidget_H
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

#include "SimCenterAppWidget.h"
#include <QValidator>

class SC_ComboBox;
class SC_IntLineEdit;
class SC_StringLineEdit;
class HazardCurveInputWidget;
class IntensityMeasure;
class SC_StringLineEdit;
class QRegExpValidator;

// CustomValidator class for custom validation logic
class CustomRPValidator : public QValidator
{
public:
    CustomRPValidator(QObject *parent = nullptr) : QValidator(parent) {}

    // Validate function is called during user input
    State validate(QString &input, int &/*pos*/) const override
    {
        // Use a regular expression for custom validation
        QRegExp rx("(\\s*\\d+\\s*,?\\s*)*");

        if (rx.exactMatch(input))
            return Acceptable;
        else
            return Invalid;
    }
};


class HazardConsistentScenarioWidget : public SimCenterAppWidget
{
    Q_OBJECT

public:
    explicit HazardConsistentScenarioWidget(QWidget *parent = nullptr);

    bool outputToJSON(QJsonObject& obj);
    bool inputFromJSON(QJsonObject& obj);

public slots:

private:

    QJsonArray getRPArray(const QString& integerListString);

    SC_ComboBox* downSamplingCombo = nullptr;
    HazardCurveInputWidget* hazCurveWidget = nullptr;
    IntensityMeasure* m_intensityMeasure = nullptr;
    SC_IntLineEdit* scenarioSampleSizeLE = nullptr;
    SC_IntLineEdit* gmSampleSizeLE = nullptr;
    SC_StringLineEdit* return_periods_lineEdit = nullptr;

};

#endif // HazardConsistentScenarioWidget_H
