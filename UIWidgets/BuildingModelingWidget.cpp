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

#include "BuildingModelingWidget.h"
#include "BuildingModelGeneratorWidget.h"
#include "BuildingSimulationWidget.h"
#include "CSVtoBIMModelingWidget.h"
#include "StructuralModelingWidget.h"

#include <QGroupBox>
#include <QComboBox>
#include <QTabWidget>
#include <QGridLayout>
#include <QJsonObject>

BuildingModelingWidget::BuildingModelingWidget(QWidget *parent, RandomVariablesContainer* RVContainer)
    : SimCenterAppWidget(parent), theRandomVariablesContainer(RVContainer)
{    
    QVBoxLayout* buildingModelLayout = new QVBoxLayout(this);

    theTabbedWidget = new QTabWidget(this);

    theBuildingModelGenWidget = new BuildingModelGeneratorWidget(this);
    theStructModelingWidget = new StructuralModelingWidget(this,theRandomVariablesContainer);
    theBuildingSIMWidget = new BuildingSimulationWidget(this, theRandomVariablesContainer);

    theTabbedWidget->addTab(theBuildingModelGenWidget,"Building Information Model (BIM) Generator");
    theTabbedWidget->addTab(theStructModelingWidget,"Building Response Model");
    theTabbedWidget->addTab(theBuildingSIMWidget,"Building Response Simulation");

    theTabbedWidget->setTabEnabled(1,true);

    buildingModelLayout->addWidget(theTabbedWidget);
}


BuildingModelingWidget::~BuildingModelingWidget()
{


}


bool BuildingModelingWidget::outputToJSON(QJsonObject &jsonObj)
{
    // Building model
    QJsonObject buildingObj;
    auto res1 = theBuildingModelGenWidget->outputToJSON(buildingObj);

    if(!res1)
        return res1;
    else
        jsonObj.insert("Building",buildingObj);

    // Structural model
    QJsonObject modelingObj;
    auto res2 = theStructModelingWidget->outputToJSON(modelingObj);

    if(!res2)
        return res2;
    else
        jsonObj.insert("Modeling",modelingObj);

    // SIM
    QJsonObject SIMobj;
    auto res3 = theBuildingSIMWidget->outputToJSON(SIMobj);

    if(!res3)
        return res3;
    else
        jsonObj.insert("Simulation",SIMobj);

    return true;
}



