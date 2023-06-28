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
#include "EarthquakeRuptureForecastWidget.h"
#include "OpenQuakeScenario.h"
#include "OpenQuakeScenarioWidget.h"
#include "OpenQuakeClassical.h"
#include "OpenQuakeClassicalWidget.h"
#include "OpenQuakeUserSpecifiedWidget.h"
#include "PointSourceRupture.h"
#include "PointSourceRuptureWidget.h"
#include "RuptureWidget.h"
#include "HazardOccurrence.h"
#include "HazardOccurrenceWidget.h"

#include <QVBoxLayout>
#include <QStackedWidget>

RuptureWidget::RuptureWidget(QWidget *parent) : SimCenterAppWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    theRootStackedWidget = new QStackedWidget(this);
    theRootStackedWidget->setContentsMargins(0,0,0,0);

    pointSourceWidget = new PointSourceRuptureWidget(parent);
    erfWidget = new EarthquakeRuptureForecastWidget(parent);
    // add widgets connecting the OpenQuake-type hazard
    // OpenQuake scenario-based
    oqsbWidget = new OpenQuakeScenarioWidget(parent);
    // OpenQuake classical PSHA
    oqcpWidget = new OpenQuakeClassicalWidget(parent);
    // OpenQuake classical PSHA User
    oqcpuWidget = new OpenQuakeUserSpecifiedWidget(parent);
    // Hazard Occurrence (KZ-08/22)
    hoWidget = new HazardOccurrenceWidget(parent);
    hoWidget->setToolTip("Hazard occurrence models reduce the number of earthquake scenarios and/or \nground motion maps to be analyzed in regional risk assessment");
    hoWidget->setToolTipDuration(5000);

    theRootStackedWidget->addWidget(pointSourceWidget);
    theRootStackedWidget->addWidget(erfWidget);
    theRootStackedWidget->addWidget(oqsbWidget);
    theRootStackedWidget->addWidget(oqcpWidget);
    theRootStackedWidget->addWidget(oqcpuWidget);
    theRootStackedWidget->addWidget(hoWidget);

    theRootStackedWidget->setCurrentWidget(erfWidget);

    ruptureGroupBox = new QGroupBox(tr("Earthquake Scenario"));
    QVBoxLayout* boxLayout = new QVBoxLayout(ruptureGroupBox);

    ruptureSelectionCombo = new QComboBox(this);

    ruptureSelectionCombo->addItem("Earthquake Rupture Forecast");
    ruptureSelectionCombo->addItem("Point Source");
    ruptureSelectionCombo->addItem("OpenQuake Scenario-Based");
    ruptureSelectionCombo->addItem("OpenQuake Classifcal PSHA");
    ruptureSelectionCombo->addItem("OpenQuake User-Specified");
    ruptureSelectionCombo->addItem("Hazard Occurrence");

    connect(ruptureSelectionCombo,&QComboBox::currentTextChanged,this,&RuptureWidget::handleSelectionChanged);

    boxLayout->addWidget(ruptureSelectionCombo);
    boxLayout->addWidget(theRootStackedWidget);

    layout->addWidget(ruptureGroupBox);

    this->setLayout(layout);
}



bool RuptureWidget::outputToJSON(QJsonObject &jsonObject)
{
    qDebug() << "RuptureWidget: starting output JSON...";
    if(ruptureSelectionCombo->currentText().compare("Point Source") == 0)
        pointSourceWidget->getRuptureSource()->outputToJSON(jsonObject);
    else if(ruptureSelectionCombo->currentText().compare("Earthquake Rupture Forecast") == 0)
        erfWidget->getRuptureSource()->outputToJSON(jsonObject);
    else if(ruptureSelectionCombo->currentText().compare("OpenQuake Scenario-Based") == 0)
        oqsbWidget->getRuptureSource()->outputToJSON(jsonObject);
    else if(ruptureSelectionCombo->currentText().compare("OpenQuake Classifcal PSHA") == 0)
        oqcpWidget->getRuptureSource()->outputToJSON(jsonObject);
    else if(ruptureSelectionCombo->currentText().compare("OpenQuake User-Specified") == 0)
        oqcpuWidget->getRuptureSource()->outputToJSON(jsonObject);
    else if(ruptureSelectionCombo->currentText().compare("Hazard Occurrence") == 0)
        hoWidget->getRuptureSource()->outputToJSON(jsonObject);
    
    return true;
}

bool RuptureWidget::inputFromJSON(QJsonObject &/*jsonObject*/)
{
    return true;
}


void RuptureWidget::handleSelectionChanged(const QString& selection)
{
    if(selection.compare("Point Source") == 0)
    {
        theRootStackedWidget->setCurrentWidget(pointSourceWidget);
        widgetType = "OpenSHA Point";
    }
    else if(selection.compare("Earthquake Rupture Forecast") == 0)
    {
        theRootStackedWidget->setCurrentWidget(erfWidget);
        widgetType = "OpenSHA ERF";
    }
    else if(selection.compare("OpenQuake Scenario-Based") == 0)
    {
        theRootStackedWidget->setCurrentWidget(oqsbWidget);
        widgetType = "OpenQuake Scenario";
    }
    else if(selection.compare("OpenQuake Classifcal PSHA") == 0)
    {
        theRootStackedWidget->setCurrentWidget(oqcpWidget);
        widgetType = "OpenQuake Classical";
    }
    else if(selection.compare("OpenQuake User-Specified") == 0)
    {
        theRootStackedWidget->setCurrentWidget(oqcpuWidget);
        widgetType = "OpenQuake User-Specified";
    }
    else if(selection.compare("Hazard Occurrence") == 0)
    {
        theRootStackedWidget->setCurrentWidget(hoWidget);
        widgetType = "Hazard Occurrence";
    }
    emit widgetTypeChanged(widgetType);

}


QString RuptureWidget::getWidgetType() const
{
    return widgetType;
}


QString RuptureWidget::getEQNum() const
{
    QString numEQ;
    if (widgetType.compare("Hazard Occurrence")==0) {
        numEQ = hoWidget->getRuptureSource()->getCandidateEQ();
    } else {
        //KZ: update the scenario number for OpenSHA ERF
        //numEQ = "1";
        if (widgetType.compare("OpenSHA ERF")==0) {
            numEQ = erfWidget->getNumScen();
        } else {
            numEQ = "1";
        }
    }
    return numEQ;
}


QString RuptureWidget::getGMPELogicTree() const
{
    QString gmpeLT = "";
    if (widgetType.compare("OpenQuake Classical")==0)
    {
        gmpeLT = oqcpWidget->getRuptureSource()->getGMPEFilename();
    }
    else if (widgetType.compare("OpenQuake User-Specified")==0)
    {
        gmpeLT = oqcpuWidget->getRuptureSource()->getGMPEFilename();
    }

    return gmpeLT;
}
