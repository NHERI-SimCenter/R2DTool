/* *****************************************************************************
Copyright (c) 2016-2017, The Regents of the University of California (Regents).
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

#include "BuildingSimulationWidget.h"
#include "InputWidgetOpenSeesAnalysis.h"
#include "InputWidgetOpenSeesPyAnalysis.h"

#include <QGridLayout>
#include <QGroupBox>
#include <QComboBox>
#include <QDebug>
#include <QStackedWidget>

BuildingSimulationWidget::BuildingSimulationWidget(QWidget *parent, RandomVariablesContainer* RVContainer)
    : SimCenterAppWidget(parent), theRandomVariablesContainer(RVContainer)
{
    buildingSIMSelectCombo = new QComboBox(this);
    buildingSIMSelectCombo->addItem("OpenSees Simulation");
    buildingSIMSelectCombo->addItem("OpenSeesPy Simulation");

    connect(buildingSIMSelectCombo,QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BuildingSimulationWidget::handleBuildingSIMSelectionChanged);

    QVBoxLayout* buildingModelLayout = new QVBoxLayout(this);

    theStackedWidget = new QStackedWidget(this);

    openSeesInputWidget = new InputWidgetOpenSeesAnalysis(theRandomVariablesContainer, this);
    openSeesPyInputWidget = new InputWidgetOpenSeesPyAnalysis(theRandomVariablesContainer, this);

    theStackedWidget->addWidget(openSeesInputWidget);
    theStackedWidget->addWidget(openSeesPyInputWidget);

    buildingModelLayout->addWidget(buildingSIMSelectCombo);
    buildingModelLayout->addWidget(theStackedWidget);

    buildingSIMSelectCombo->setCurrentText("OpenSeesPy Simulation");
}


BuildingSimulationWidget::~BuildingSimulationWidget()
{

}


bool BuildingSimulationWidget::outputToJSON(QJsonObject &jsonObj)
{

    auto currentSelection = buildingSIMSelectCombo->currentText();

    if(currentSelection.compare("OpenSees Simulation") == 0)
    {
        return openSeesInputWidget->outputToJSON(jsonObj);
    }
    else if(currentSelection.compare("OpenSeesPy Simulation") == 0)
    {
        return openSeesPyInputWidget->outputToJSON(jsonObj);
    }
    else
    {
        qDebug()<<"Warning, selection "<<currentSelection<<" not handled";
        return false;
    }

    return true;
}


void BuildingSimulationWidget::handleBuildingSIMSelectionChanged(const int index)
{
    theStackedWidget->setCurrentIndex(index);
}




