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

// Written by: Frank McKenna

#include "NearestNeighbourMapping.h"
#include "SiteSpecifiedMapping.h"
#include "GISBasedMapping.h"
#include "NoArgSimCenterApp.h"

#include "HazardToAssetWidget.h"
#include "SecondaryComponentSelection.h"
#include "SimCenterAppSelection.h"
#include "VisualizationWidget.h"


HazardToAssetWidget::HazardToAssetWidget(QWidget *parent, VisualizationWidget* visWidget)
: MultiComponentR2D(QString("RegionalMapping"), parent)
{

  QList<QString> waterExtraKeys; waterExtraKeys.append("WaterNetworkPipelines"); waterExtraKeys.append("WaterNetworkNodes");
  
  buildingWidget = new SimCenterAppSelection(QString("Building Mapping"), QString("Buildings"), this);
  gasWidget = new SimCenterAppSelection(QString("Gas Network Mapping"), QString("NaturalGasPipelines"), this);
  wdnWidget = new SimCenterAppSelection(QString("Water Distribution Network Mapping"), QString("WaterDistributionNetwork"), waterExtraKeys);
  
  NearestNeighbourMapping *theNNMapB = new NearestNeighbourMapping();
  SiteSpecifiedMapping *theSSMapB = new SiteSpecifiedMapping();
  GISBasedMapping *theGISMapB = new GISBasedMapping();
  
  buildingWidget->addComponent(QString("Nearest Neighbour"), QString("NearestNeighborEvents"), theNNMapB);
  buildingWidget->addComponent(QString("Site Specified"), QString("SiteSpecifiedEvents"), theSSMapB);
  buildingWidget->addComponent(QString("GIS Specified"), QString("GISSpecifiedEvents"), theGISMapB);

  NearestNeighbourMapping *theNNMapG = new NearestNeighbourMapping();
  SiteSpecifiedMapping *theSSMapG = new SiteSpecifiedMapping();
  GISBasedMapping *theGISMapG = new GISBasedMapping();
  
  gasWidget->addComponent(QString("Nearest Neighbour"), QString("NearestNeighborEvents"), theNNMapG);
  gasWidget->addComponent(QString("Site Specified"), QString("SiteSpecifiedEvents"), theSSMapG);
  gasWidget->addComponent(QString("GIS Specified"), QString("GISSpecifiedEvents"), theGISMapG);      

  NearestNeighbourMapping *theNNMapWDN = new NearestNeighbourMapping();
  SiteSpecifiedMapping *theSSMapWDN = new SiteSpecifiedMapping();
  GISBasedMapping *theGISMapWDN = new GISBasedMapping();
  
  wdnWidget->addComponent(QString("Nearest Neighbour"), QString("NearestNeighborEvents"), theNNMapWDN);
  wdnWidget->addComponent(QString("Site Specified"), QString("SiteSpecifiedEvents"), theSSMapWDN);
  wdnWidget->addComponent(QString("GIS Specified"), QString("GISSpecifiedEvents"), theGISMapWDN);      

  this->addComponent("Buildings", buildingWidget);
  this->addComponent("Gas Network",gasWidget);
  this->addComponent("Water Network", wdnWidget);
  this->hideAll();  
}


HazardToAssetWidget::~HazardToAssetWidget()
{

}


void HazardToAssetWidget::clear(void)
{
    buildingWidget->clear();
    gasWidget->clear();
    wdnWidget->clear();    
}
