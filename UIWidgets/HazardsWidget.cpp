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

#include "GMWidget.h"
#include "HazardsWidget.h"
#include "ShakeMapWidget.h"
#include "HurricaneSelectionWidget.h"
#include "UserInputHurricaneWidget.h"
#include "UserInputGMWidget.h"
#include "VisualizationWidget.h"
#include "WorkflowAppR2D.h"

#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QStackedWidget>

HazardsWidget::HazardsWidget(QWidget *parent,
                             VisualizationWidget* visWidget,
                             RandomVariablesContainer * RVContainer)
    : SimCenterAppSelection(QString("Hazard Selection"),QString("Hazard"), parent),
      theRandomVariablesContainer(RVContainer), theVisualizationWidget(visWidget)
{
    this->setContentsMargins(0,0,0,0);

    theEQSSWidget = new GMWidget(this, theVisualizationWidget);
    theShakeMapWidget = new ShakeMapWidget(theVisualizationWidget);
    theUserInputGMWidget = new UserInputGMWidget(theVisualizationWidget);
    theHurricaneSelectionWidget = new HurricaneSelectionWidget(theVisualizationWidget);
    theUserInputHurricaneWidget = new UserInputHurricaneWidget(theVisualizationWidget);

    this->addComponent("Earthquake Scenario Simulation", "EQSS", theEQSSWidget);
    this->addComponent("User Specified Ground Motions", "UserInputGM", theUserInputGMWidget);
    this->addComponent("Hurricane Scenario Simulation", "HurricaneSelection", theHurricaneSelectionWidget);
    this->addComponent("User Specified Wind Field", "UserInputHurricane", theUserInputHurricaneWidget);
    this->addComponent("ShakeMap Earthquake Scenario", "UserInputShakeMap", theShakeMapWidget);

    //connect(theShakeMapWidget, &ShakeMapWidget::loadingComplete, this, &HazardsWidget::shakeMapLoadingFinished);

    connect(theShakeMapWidget, SIGNAL(outputDirectoryPathChanged(QString, QString)), this,  SLOT(gridFileChangedSlot(QString, QString)));
    connect(theEQSSWidget, SIGNAL(outputDirectoryPathChanged(QString, QString)), this,  SLOT(gridFileChangedSlot(QString, QString)));
    connect(theUserInputGMWidget, SIGNAL(outputDirectoryPathChanged(QString, QString)), this,  SLOT(gridFileChangedSlot(QString, QString)));    
    connect(theUserInputHurricaneWidget, SIGNAL(outputDirectoryPathChanged(QString, QString)), this,  SLOT(gridFileChangedSlot(QString, QString)));
    connect(theHurricaneSelectionWidget, SIGNAL(outputDirectoryPathChanged(QString, QString)), this,  SLOT(gridFileChangedSlot(QString, QString)));
}


HazardsWidget::~HazardsWidget()
{

}


void HazardsWidget::shakeMapLoadingFinished(const bool value)
{
    if(!value)
        return;

    // Shift the focus to the visualization widget
    auto mainWindowWidget = qobject_cast<WorkflowAppR2D*>(this->parent());

    if(!mainWindowWidget)
        return;

    // mainWindowWidget->setActiveWidget(theVisualizationWidget);

}


void HazardsWidget::gridFileChangedSlot(QString motionD, QString eventF)
{
    emit gridFileChangedSignal(motionD, eventF);
}


void HazardsWidget::setCurrentlyViewable(bool status) {
    this->SimCenterAppSelection::setCurrentlyViewable(status);
}
