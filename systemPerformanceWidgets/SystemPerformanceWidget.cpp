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

#include "SystemPerformanceWidget.h"
#include "NoneWidget.h"
#include "RewetRecovery.h"

#include "SecondaryComponentSelection.h"
#include "SimCenterAppSelection.h"
#include "VisualizationWidget.h"
#include "SectionTitle.h"
#include "ResidualDemandWidget.h"

// Qt headers
#include <QDebug>

SystemPerformanceWidget::SystemPerformanceWidget(QWidget *parent)
  : MultiComponentR2D(QString("SystemPerformance"), parent)
{

  //QList<QString> waterExtraKeys; waterExtraKeys.append("WaterNewtworkPipelines"); waterExtraKeys.append("WaterNetworkNodes");
  buildingWidget = new SimCenterAppSelection(QString("Building Recovery"), QString("Buildings"), this);
  pipelineWidget = new SimCenterAppSelection(QString("Gas Pipeline Recovery"), QString("NaturalGasPipelines"), this);
  WDNWidget = new SimCenterAppSelection(QString("Water Distribution Network Recovery"), QString("WaterDistributionNetwork"), this);
  transportWidget = new SimCenterAppSelection(QString("Transportation Components Recovery"), QString("TransportationNetwork"), this);

  // Building widget apps
  SimCenterAppWidget *noneWidget = new NoneWidget(this);
  buildingWidget->addComponent(QString("None"), QString("None"), noneWidget);

  // Natural gas pipeline apps
  SimCenterAppWidget *noneWidget2 = new NoneWidget(this);
  pipelineWidget->addComponent(QString("None"), QString("None"), noneWidget2);
    
  // Water distribution network apps
  SimCenterAppWidget *noneWidget3 = new NoneWidget(this);
  SimCenterAppWidget *rewetRecovery = new RewetRecovery(this);
  WDNWidget->addComponent(QString("None"), QString("None"), noneWidget3);
  WDNWidget->addComponent(QString("REWETRecovery"), QString("REWETRecovery"), rewetRecovery);  

  // Transportation network apps
  SimCenterAppWidget *noneWidget4 = new NoneWidget(this);
  SimCenterAppWidget *residualDemand = new ResidualDemandWidget(this);
  transportWidget->addComponent(QString("None"), QString("None"), noneWidget4);
  transportWidget->addComponent(QString("Residual Demand Traffic Simulation"), QString("ResidualDemand"), residualDemand);

  this->addComponent("Buildings", buildingWidget);
  this->addComponent("Gas Network", pipelineWidget);
  this->addComponent("Water Distribution Network", WDNWidget);
  this->addComponent("Transportation Network", transportWidget);
  this->hideAll();
}

QMap<QString, SC_ResultsWidget*>  SystemPerformanceWidget::getActiveSPResultsWidgets(QWidget *parent, ResultsWidget *R2DresWidget, QMap<QString, QList<QString>> assetTypeToType)
{
  QMap<QString, SC_ResultsWidget*> activePostProcessors;
  auto activeList = this->getActiveComponents();

  for(auto&& it : activeList)
  {
      auto activeComp = dynamic_cast<SimCenterAppSelection*>(this->getComponent(it));

      if(activeComp == nullptr)
          return activePostProcessors;

      QString currComp = activeComp->getCurrentSelectionName();
      SimCenterAppWidget* currSelection = activeComp->getCurrentSelection();
      SC_ResultsWidget* currResultWidget = currSelection->getResultsWidget(parent, R2DresWidget, assetTypeToType);
      if(!currResultWidget){
          qDebug()<<QString("The SP widget of "+currComp+ " used by " + it+ " does not have a resultWidget");
      } else {
          activePostProcessors.insert(it, currResultWidget);
      }
  }

  return activePostProcessors;
}

SystemPerformanceWidget::~SystemPerformanceWidget()
{

}


void SystemPerformanceWidget::clear(void)
{
  buildingWidget->clear();
  pipelineWidget->clear();
  WDNWidget->clear();
  transportWidget->clear();
}






