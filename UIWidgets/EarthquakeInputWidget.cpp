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
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
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

#include "EarthquakeInputWidget.h"
#include "GMWidget.h"
#include "ShakeMapWidget.h"
#include "UserInputGMWidget.h"
#include "VisualizationWidget.h"
#include "WorkflowAppR2D.h"

#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QJsonObject>
#include <QLabel>
#include <QStackedWidget>

EarthquakeInputWidget::EarthquakeInputWidget(QWidget *parent, VisualizationWidget* visWidget)
  : SimCenterAppWidget(parent), theVisualizationWidget(visWidget)
{
    theEQWidget = nullptr;
    theRootStackedWidget = nullptr;
    theShakeMapWidget = nullptr;
    theEQSSWidget = nullptr;
    theUserInputGMWidget = nullptr;
    earthquakeSelectionCombo = nullptr;
    includeHazardCheckBox = nullptr;
}


EarthquakeInputWidget::~EarthquakeInputWidget()
{

}


QWidget* EarthquakeInputWidget::getEarthquakesWidget(void)
{
    if(theEQWidget == nullptr)
        this->createEarthquakesWidget();

    return theEQWidget;
}


bool EarthquakeInputWidget::outputToJSON(QJsonObject &jsonObj)
{

    if(includeHazardCheckBox->isChecked() == false)
        return false;

    jsonObj.insert("EventClassification", "Earthquake");
    jsonObj.insert("Application", "SimCenterEvent");

    QJsonObject appDataObj;

    auto currentSelection = earthquakeSelectionCombo->currentText();

    if(currentSelection.compare("Earthquake Scenario Simulation") == 0)
    {
        theEQSSWidget->outputToJSON(appDataObj);
    }
    else if(currentSelection.compare("ShakeMap Input") == 0)
    {
        theShakeMapWidget->outputToJSON(appDataObj);
    }
    else if(currentSelection.compare("User Specified Ground Motions") == 0)
    {
        theUserInputGMWidget->outputToJSON(appDataObj);
    }
    else
    {
        qDebug()<<"Warning, could not recognize the earthquake combobox selection of"<<earthquakeSelectionCombo->currentText();
    }

    jsonObj.insert("ApplicationData",appDataObj);

    return true;
}


bool EarthquakeInputWidget::inputFromJSON(QJsonObject &jsonObject){


    return true;
}


void EarthquakeInputWidget::createEarthquakesWidget(void)
{
    theEQWidget = new QWidget();

    QGridLayout* gridLayout = new QGridLayout(theEQWidget);

    auto smallVSpacer = new QSpacerItem(0,10);

    QLabel* selectionText = new QLabel();
    selectionText->setText("Earthquake Hazard Type:");

    includeHazardCheckBox = new QCheckBox("Include earthquake hazard in analysis");
    includeHazardCheckBox->setChecked(true);

    earthquakeSelectionCombo = new QComboBox();
    earthquakeSelectionCombo->addItem("Earthquake Scenario Simulation");
    earthquakeSelectionCombo->addItem("ShakeMap Input");
    earthquakeSelectionCombo->addItem("User Specified Ground Motions");
    earthquakeSelectionCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    connect(earthquakeSelectionCombo, SIGNAL(currentIndexChanged(QString)), this, SLOT(handleEQTypeSelection(QString)));

    theRootStackedWidget = new QStackedWidget();

    // Add a vertical spacer at the bottom to push everything up
    auto vspacer = new QSpacerItem(0,0,QSizePolicy::Minimum, QSizePolicy::Expanding);
    auto hspacer = new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Minimum);

    gridLayout->addItem(smallVSpacer,0,0);
    gridLayout->addWidget(selectionText,1,0);
    gridLayout->addWidget(earthquakeSelectionCombo,1,1);
    gridLayout->addWidget(includeHazardCheckBox,1,2);
    gridLayout->addItem(hspacer,1,3);
    gridLayout->addWidget(theRootStackedWidget,2,0,1,3);
    gridLayout->addItem(vspacer, 3, 0,1,3);

    theEQSSWidget = new GMWidget(theVisualizationWidget);
    theShakeMapWidget = new ShakeMapWidget(theVisualizationWidget);
    theUserInputGMWidget = new UserInputGMWidget(theVisualizationWidget);

    connect(theShakeMapWidget, &ShakeMapWidget::loadingComplete, this, &EarthquakeInputWidget::shakeMapLoadingFinished);

    theRootStackedWidget->addWidget(theEQSSWidget);
    theRootStackedWidget->addWidget(theShakeMapWidget->getShakeMapWidget());
    theRootStackedWidget->addWidget(theUserInputGMWidget->getUserInputGMWidget());

    theRootStackedWidget->setCurrentWidget(theEQSSWidget);

    earthquakeSelectionCombo->setCurrentText("User Specified Ground Motions");

}


void EarthquakeInputWidget::shakeMapLoadingFinished(const bool value)
{
    if(!value)
        return;

    // Shift the focus to the visualization widget
    auto mainWindowWidget = qobject_cast<WorkflowAppR2D*>(this->parent());

    if(!mainWindowWidget)
        return;

    mainWindowWidget->setActiveWidget(theVisualizationWidget);

}


void EarthquakeInputWidget::handleEQTypeSelection(const QString& selection)
{
    if(selection == "Earthquake Scenario Simulation")
        theRootStackedWidget->setCurrentWidget(theEQSSWidget);
    else if(selection == "ShakeMap Input")
        theRootStackedWidget->setCurrentWidget(theShakeMapWidget->getShakeMapWidget());
    else if(selection == "User Specified Ground Motions")
        theRootStackedWidget->setCurrentWidget(theUserInputGMWidget->getUserInputGMWidget());


}
