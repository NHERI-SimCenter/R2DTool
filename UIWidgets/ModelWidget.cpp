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

// Written by: Frank McKenna

#include "MDOF_LU.h"
#include "ModelWidget.h"
#include "MultiFidelityBuildingModel.h"
#include "NoArgSimCenterApp.h"
#include "NoneWidget.h"
#include "OpenSeesPyBuildingModel.h"
#include <CustomPy.h>
#include "SecondaryComponentSelection.h"
#include "SimCenterAppSelection.h"
#include "VisualizationWidget.h"
#include "SectionTitle.h"

// Qt headers
#include <QCheckBox>
#include <QColorTransform>
#include <QDebug>
#include <QFileDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonObject>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPointer>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

ModelWidget::ModelWidget(QWidget *parent)
  : MultiComponentR2D(QString("Modeling"), parent)
{

  QList<QString> waterExtraKeys; waterExtraKeys.append("WaterNewtworkPipelines"); waterExtraKeys.append("WaterNetworkNodes");
  QList<QString> transportExtraKeys;
  transportExtraKeys.append("TransportRoads");
  transportExtraKeys.append("TransportBridges");
  transportExtraKeys.append("TransportTunnels");
  buildingWidget = new SimCenterAppSelection(QString("Building Modeling"), QString("Buildings"), this);
    pipelineWidget = new SimCenterAppSelection(QString("Gas Pipeline Modeling"), QString("NaturalGasPipelines"), this);
    WDNWidget = new SimCenterAppSelection(QString("Water Distribution Network Modeling"), QString("WaterDistributionNetwork"), waterExtraKeys);
//    transportWidget = new SimCenterAppSelection(QString("Transportation Components Modeling"), QString("TransportationNetwork"), transportExtraKeys);
    transportWidget = new SimCenterAppSelection(QString("Transportation Components Modeling"), QString("TransportationNetwork"), this);

    // Building widget apps
    SimCenterAppWidget *mdofLU = new MDOF_LU();
    SimCenterAppWidget *multiFidelityBuildingModel = new MultiFidelityBuildingModel();
    SimCenterAppWidget *openSeesPy = new OpenSeesPyBuildingModel(this);

    SimCenterAppWidget *noneWidget = new NoneWidget(this);

    buildingWidget->addComponent(QString("MDOF-LU"), QString("MDOF-LU"), mdofLU);
    buildingWidget->addComponent(QString("High-Fidelity Models and MDOF-LU"), QString("multiFidelityBuildingModel"), multiFidelityBuildingModel);
    buildingWidget->addComponent(QString("OpenSeesPy Script Generator"), QString("OpenSeesPyInput"), openSeesPy);
    buildingWidget->addComponent(QString("None"), QString("None"), noneWidget);

    // KZ: adding CustomPy
    SimCenterAppWidget *custom_py = new CustomPy();
    buildingWidget->addComponent(QString("CustomPy"), QString("CustomPyInput"), custom_py);

    // Natural gas pipeline apps
    SimCenterAppWidget *noneWidget2 = new NoneWidget(this);

    pipelineWidget->addComponent(QString("None"), QString("None"), noneWidget2);

    // Water distribution network apps
    SimCenterAppWidget *noneWidget3 = new NoneWidget(this);

    WDNWidget->addComponent(QString("None"), QString("None"), noneWidget3);

    // Transportation network apps
    SimCenterAppWidget *noneWidget4 = new NoneWidget(this);
    transportWidget->addComponent(QString("None"), QString("None"), noneWidget4);

    this->addComponent("Buildings", buildingWidget);
    this->addComponent("Gas Network", pipelineWidget);
    this->addComponent("Water Distribution Network", WDNWidget);
    this->addComponent("Transportation Network", transportWidget);
    this->hideAll();
}


ModelWidget::~ModelWidget()
{

}

bool ModelWidget::outputCitation(QJsonObject &citation){
    QJsonObject ModelingCitations;
    buildingWidget->outputCitation(ModelingCitations);
    pipelineWidget->outputCitation(ModelingCitations);
    WDNWidget->outputCitation(ModelingCitations);
    transportWidget->outputCitation(ModelingCitations);
    citation.insert("Model", ModelingCitations);
    return true;
}


void ModelWidget::clear(void)
{
    buildingWidget->clear();
    pipelineWidget->clear();
    WDNWidget->clear();
    transportWidget->clear();
}






