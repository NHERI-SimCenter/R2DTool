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
#include <QCheckBox>

HazardConsistentScenarioWidget::HazardConsistentScenarioWidget(QWidget *parent) : SimCenterAppWidget(parent)
{
    //We use a grid layout for the Rupture widget
    auto mainLayout = new QGridLayout(this);

    auto DownSamplingAlgoLabel = new QLabel("Downsampling Algorithm");
    downSamplingCombo = new SC_ComboBox("Model",QStringList({"Manzour & Davidson (2016)", "Wang et al. (2023)"}));

    downSamplingCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

    hazCurveWidget = new HazardCurveInputWidget("Generator");

//    m_intensityMeasure = new IntensityMeasure(this);
//    auto m_intensityMeasureWidget = new IntensityMeasureWidget(*m_intensityMeasure);
//    m_intensityMeasureWidget->typeBox()->setCurrentIndex(1); // Set to PGA

    // Intensity measure type
    QLabel* imt_label= new QLabel(tr("Intensity measure:"),this);
    IMT_Combo = new QComboBox(this);
    IMT_Combo->addItem("PGA");
    IMT_Combo->addItem("SA");
    IMT_Combo->setToolTip("Select intensity measure type for the hazard curves");

    // Sa period
    IMT_period= new QLabel(tr("Hazard curve Sa period:"),this);
    PeriodEdit = new QLineEdit(this);
    PeriodEdit->setText("0.0");
    PeriodEdit->setDisabled(1);
    PeriodEdit->setStyleSheet("background-color: lightgray");
    PeriodEdit->setToolTip("Fill in a float number for the period if 'SA' is the intensity measure type");


    scenarioSampleSizeLE = new SC_IntLineEdit("EarthquakeSampleSize",40);
    gmSampleSizeLE = new SC_IntLineEdit("GroundMotionMapSize",100);

    QLabel* scenarioSampleSize_label = new QLabel(tr("Earthquake scenario sample size:"));
    QLabel* gmSampleSize_label = new QLabel(tr("Ground motion map sample size:"));
    QLabel* return_period_label = new QLabel(tr("Return periods (yr):"));

    return_periods_lineEdit = new SC_StringLineEdit("ReturnPeriods","50, 224, 475, 975, 2475");
    return_periods_lineEdit->setValidator(new CustomRPValidator(return_periods_lineEdit));

    lassoTuningParameterLabel = new QLabel(tr("LASSO Tuning Parameters: "));
    lassoTuningParameterLE =  new SC_StringLineEdit("LassoTuningParameter","");
    setLassoTuning = new QCheckBox("Set LASSO Tuning Parameter");

    mainLayout->addWidget(DownSamplingAlgoLabel,0,0);
    mainLayout->addWidget(downSamplingCombo,0,1);
    mainLayout->addWidget(hazCurveWidget,1,0,1,4);
//    mainLayout->addWidget(m_intensityMeasureWidget,2,0,1,2);
    mainLayout->addWidget(imt_label,2,0);
    mainLayout->addWidget(IMT_Combo,2,1,1,1);
    mainLayout->addWidget(IMT_period,2,2,1,1);
    mainLayout->addWidget(PeriodEdit,2,3,1,1);

    mainLayout->addWidget(scenarioSampleSize_label,3,0);
    mainLayout->addWidget(scenarioSampleSizeLE,3,1);
    mainLayout->addWidget(gmSampleSize_label,4,0);
    mainLayout->addWidget(gmSampleSizeLE,4,1);
    mainLayout->addWidget(return_period_label,5,0);
    mainLayout->addWidget(return_periods_lineEdit,5,1);
    mainLayout->addWidget(setLassoTuning, 6, 0);
    mainLayout->addWidget(lassoTuningParameterLabel,6,1);
    mainLayout->addWidget(lassoTuningParameterLE,6,2,1,2);
    mainLayout->setRowStretch(7,1);
    mainLayout->setColumnStretch(4,3);
    mainLayout->setColumnStretch(1,2);
    mainLayout->setAlignment(lassoTuningParameterLabel, Qt::AlignRight);

    connect(this->IMT_Combo, &QComboBox::currentTextChanged,
            this, &HazardConsistentScenarioWidget::handleTypeChanged);

    connect(this->downSamplingCombo, &QComboBox::currentTextChanged,
            this, &HazardConsistentScenarioWidget::handleModelChanged);

    connect(this->setLassoTuning, &QCheckBox::stateChanged, this,
            &HazardConsistentScenarioWidget::handleSetLassoChanged);

    setLassoTuning->hide();
    lassoTuningParameterLabel->hide();
    lassoTuningParameterLE->hide();

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

QJsonArray HazardConsistentScenarioWidget::getDoubleArray(const QString& doubleListString)
{
    // Split the string into a QStringList using commas as the delimiter
    QStringList doubleStringList = doubleListString.split(", ");

    // Convert the QStringList to a QJsonArray of integers
    QJsonArray jsonArray;
    for (const QString &doubleString : doubleStringList) {
        bool conversionOk;
        double value = doubleString.toDouble(&conversionOk);
        if (conversionOk) {
            jsonArray.append(value);
        } else {
            qDebug() << "Invalid double value:" << doubleString;
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

    paramObj.insert("IntensityMeasure", IMT_Combo->currentText());
    paramObj.insert("Period", PeriodEdit->text().toDouble());

    auto rpArr = getRPArray(return_periods_lineEdit->text());
    paramObj["ReturnPeriods"] = rpArr;

    if(downSamplingCombo->currentText().compare("Wang et al. (2023)") == 0)
    {
        QJsonArray tpArr;
        if (setLassoTuning->isChecked()){
            tpArr = getDoubleArray(lassoTuningParameterLE->text());
        }
        paramObj["LassoTuningParameter"] = tpArr;
    }

    obj["Parameters"] = paramObj;

    return true;
}

void HazardConsistentScenarioWidget::handleTypeChanged(const QString &val)
{
    if(val.compare("SA") == 0)
    {
        PeriodEdit->setEnabled(1);
        PeriodEdit->setStyleSheet("background-color: white");
        PeriodEdit->setText("0.1");
    }
    else
    {
        PeriodEdit->setText("0.0");
        PeriodEdit->setDisabled(1);
        PeriodEdit->setStyleSheet("background-color: lightgray");
    }
}

void HazardConsistentScenarioWidget::handleModelChanged(const QString &val)
{
    if(val.compare("Wang et al. (2023)") == 0)
    {
        setLassoTuning->show();
        lassoTuningParameterLabel->hide();
        lassoTuningParameterLE->hide();
    }
    else
    {
        setLassoTuning->hide();
        lassoTuningParameterLabel->hide();
        lassoTuningParameterLE->hide();
    }
    setLassoTuning->setChecked(false);
}

void HazardConsistentScenarioWidget::handleSetLassoChanged()
{
    if(setLassoTuning->isChecked())
    {
        lassoTuningParameterLabel->show();
        lassoTuningParameterLE->show();
        lassoTuningParameterLE->setText("0.001, 0.01, 0.1, 1, 10");
    }
    else
    {
        lassoTuningParameterLabel->hide();
        lassoTuningParameterLE->hide();
    }
}
