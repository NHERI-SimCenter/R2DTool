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

#include "HazardConsistentScenarioWidget.h"
#include "HazardCurveInputWidget.h"
#include "SC_ComboBox.h"
#include "SC_IntLineEdit.h"
#include "SC_StringLineEdit.h"
#include "IntensityMeasureWidget.h"
#include "IntensityMeasure.h"

#include <QGridLayout>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>

HazardConsistentScenarioWidget::HazardConsistentScenarioWidget(QWidget *parent) : SimCenterAppWidget(parent)
{
    //We use a grid layout for the Rupture widget
    auto mainLayout = new QGridLayout(this);

    auto DownSamplingAlgoLabel = new QLabel("Downsampling Algorithm");
    downSamplingCombo = new SC_ComboBox("Model",QStringList({"Manzour & Davidson (2016)"}));

    downSamplingCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

    hazCurveWidget = new HazardCurveInputWidget("Generator");

    m_intensityMeasure = new IntensityMeasure(this);
    auto m_intensityMeasureWidget = new IntensityMeasureWidget(*m_intensityMeasure);
    m_intensityMeasureWidget->typeBox()->setCurrentIndex(1); // Set to PGA

    scenarioSampleSizeLE = new SC_IntLineEdit("EarthquakeSampleSize",40);
    gmSampleSizeLE = new SC_IntLineEdit("GroundMotionMapSize",100);

    QLabel* scenarioSampleSize_label = new QLabel(tr("Earthquake scenario sample size:"));
    QLabel* gmSampleSize_label = new QLabel(tr("Ground motion map sample size:"));
    QLabel* return_period_label = new QLabel(tr("Return periods (yr):"));

    return_periods_lineEdit = new SC_StringLineEdit("ReturnPeriods","50, 224, 475, 975, 2475");
    return_periods_lineEdit->setValidator(new CustomRPValidator(return_periods_lineEdit));

    mainLayout->addWidget(DownSamplingAlgoLabel,0,0);
    mainLayout->addWidget(downSamplingCombo,0,1);
    mainLayout->addWidget(hazCurveWidget,1,0,1,2);
    mainLayout->addWidget(m_intensityMeasureWidget,2,0,1,2);
    mainLayout->addWidget(scenarioSampleSize_label,3,0);
    mainLayout->addWidget(scenarioSampleSizeLE,3,1);
    mainLayout->addWidget(gmSampleSize_label,4,0);
    mainLayout->addWidget(gmSampleSizeLE,4,1);
    mainLayout->addWidget(return_period_label,5,0);
    mainLayout->addWidget(return_periods_lineEdit,5,1);
    mainLayout->setRowStretch(6,1);

}




bool HazardConsistentScenarioWidget::inputFromJSON(QJsonObject& /*obj*/)
{

    return true;
}


QJsonArray HazardConsistentScenarioWidget::getRPArray(const QString& integerListString)
{
    // Split the string into a QStringList using commas as the delimiter
    QStringList integerStringList = integerListString.split(", ");

    // Convert the QStringList to a QJsonArray of integers
    QJsonArray jsonArray;
    for (const QString &integerString : integerStringList) {
        bool conversionOk;
        int integerValue = integerString.toInt(&conversionOk);
        if (conversionOk) {
            jsonArray.append(integerValue);
        } else {
            qDebug() << "Invalid integer value:" << integerString;
            // Handle the error as needed
        }
    }

    return jsonArray;
}


bool HazardConsistentScenarioWidget::outputToJSON(QJsonObject& obj)
{
    obj["method"] = "Subsampling";

    QJsonObject paramObj;
    downSamplingCombo->outputToJSON(paramObj);
    scenarioSampleSizeLE->outputToJSON(paramObj);
    gmSampleSizeLE->outputToJSON(paramObj);
    hazCurveWidget->outputToJSON(paramObj);

    QJsonObject im;
    m_intensityMeasure->outputToJSON(im);

    paramObj["IntensityMeasure"] = im["Type"];

    if (im["Type"]=="PGA")
        paramObj["Period"] = im["Period"];
    else
        paramObj["Periods"] = im["Periods"];

    auto rpArr = getRPArray(return_periods_lineEdit->text());
    paramObj["ReturnPeriods"] = rpArr;

    obj["Parameters"] = paramObj;

    return true;
}

