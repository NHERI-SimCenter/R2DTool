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

#include "GMERFWidget.h"
#include "RuptureWidget.h"
#include "JsonSerializable.h"
#include "VisualizationWidget.h"
#include "PointSourceRuptureWidget.h"
#include "OpenQuakeUserSpecifiedWidget.h"
#include "OpenQuakeScenarioWidget.h"

#include <QVBoxLayout>
#include <QGroupBox>
#include <QComboBox>
#include <QPushButton>
#include <QStackedWidget>

GMERFWidget::GMERFWidget(VisualizationWidget* visWidget, QWidget *parent) : QWidget(parent), theVisualizationWidget(visWidget)
{
    ruptureSelectCombo = new QComboBox();
    mainStackedWidget =  new QStackedWidget();

    // Connect the combo box signal to the stacked widget slot
    QObject::connect(ruptureSelectCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                     mainStackedWidget, &QStackedWidget::setCurrentIndex);

    // Create the widgets that will be in the stacked box
    ERFruptureWidget = new RuptureWidget();
    pointSourceWidget = new PointSourceRuptureWidget();
    oqcpuWidget = new OpenQuakeUserSpecifiedWidget();
    oqsbWidget = new OpenQuakeScenarioWidget();

    // OpenQuake classical PSHA
    //    oqcpWidget = new OpenQuakeClassicalWidget(parent);

    // Hazard Occurrence (KZ-08/22)
    //    hoWidget = new HazardOccurrenceWidget(parent);
    //    hoWidget->setToolTip("Hazard occurrence models reduce the number of earthquake scenarios and/or \nground motion maps to be analyzed in regional risk assessment");
    //    hoWidget->setToolTipDuration(5000);

    ruptureSelectCombo->addItem("OpenSHA ERF");
    ruptureSelectCombo->addItem("Point Source");
    ruptureSelectCombo->addItem("OpenQuake Source Model");
    ruptureSelectCombo->addItem("OpenQuake Scenario-based");

    mainStackedWidget->addWidget(ERFruptureWidget);
    mainStackedWidget->addWidget(pointSourceWidget);
    mainStackedWidget->addWidget(oqcpuWidget);
    mainStackedWidget->addWidget(oqsbWidget);
//    mainStackedWidget->addWidget(oqcpWidget);
//    mainStackedWidget->addWidget(hoWidget);

    auto forecastRupScenButton = new QPushButton("Forecast Rupture Scenarios");

    auto mapView = theVisualizationWidget->getMapViewWidget("RuptureScenarioView");
    mapViewSubWidget = std::unique_ptr<SimCenterMapcanvasWidget>(mapView);

    QVBoxLayout *earthquakeLayout=new QVBoxLayout(this);
    earthquakeLayout->addWidget(ruptureSelectCombo);
    earthquakeLayout->addWidget(mainStackedWidget);
    earthquakeLayout->addWidget(forecastRupScenButton);
    earthquakeLayout->addWidget(mapViewSubWidget.get());

}

bool GMERFWidget::inputFromJSON(QJsonObject& /*obj*/)
{

    return true;
}


bool GMERFWidget::outputToJSON(QJsonObject& obj)
{

    QJsonObject scenarioObj;
    scenarioObj.insert("Type", "Earthquake");
//    // get scenario number
//    QString numEQ = this->ERFruptureWidget->getEQNum();
//    if (numEQ.compare("All")==0) {
//        scenarioObj.insert("Number", "All");
//    } else {
//        scenarioObj.insert("Number", numEQ.toInt());
//    }
//    scenarioObj.insert("Generator", "Selection");

    QJsonObject EqRupture;
    ERFruptureWidget->outputToJSON(EqRupture);

//    // number of scenarios for ERF widget
//    if (ERFruptureWidget->getWidgetType().compare("OpenSHA ERF")==0)
//    {
//        if (EqRupture.contains("Number"))
//            scenarioObj["Number"] = EqRupture["Number"];
//    }

    if(EqRupture.isEmpty())
        return false;

    scenarioObj.insert("EqRupture",EqRupture);

    obj.insert("Scenario",scenarioObj);

    return true;
}

RuptureWidget *GMERFWidget::ruptureWidget() const
{
    return ERFruptureWidget;
}

