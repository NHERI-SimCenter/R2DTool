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

// Written by: Frank McKenna, Stevan Gavrilovic

#include "DLWidget.h"
#include "PelicunDLWidget.h"
#include "NoneWidget.h"
#include "SecondaryComponentSelection.h"
#include "NoArgSimCenterApp.h"
#include "SimCenterAppSelection.h"
#include "VisualizationWidget.h"
#include "sectiontitle.h"

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

DLWidget::DLWidget(QWidget *parent, VisualizationWidget* visWidget)
    : MultiComponentR2D(parent), visualizationWidget(visWidget)
{

    buildingWidget = new SimCenterAppSelection(QString("Building Damage & Loss Application"), QString("DL"), QString(), QString("Buildings"), this);
    pipelineWidget = new SimCenterAppSelection(QString("Gas Network Damage & Loss Application"), QString("DL"), QString(), QString("NaturalGasPipelines"), this);
    WDNWidget = new SimCenterAppSelection(QString("Water Distribution Network Damage & Loss Application"), QString("DL"), QString(), QString("WaterDistributionNetwork"), this);

    // Building widget apps
    SimCenterAppWidget *buildingPelicun = new PelicunDLWidget;
    SimCenterAppWidget *noneWidget = new NoneWidget(this);

    buildingWidget->addComponent(QString("Pelicun"), QString("pelicun"), buildingPelicun);
    buildingWidget->addComponent(QString("None"), QString("None"), noneWidget);

    // Natural gas pipeline apps
    SimCenterAppWidget *noneWidget2 = new NoneWidget(this);

    pipelineWidget->addComponent(QString("None"), QString("None"), noneWidget2);

    // Water distribution network apps
    SimCenterAppWidget *noneWidget3 = new NoneWidget(this);

    SimCenterAppWidget *WDNDL = new NoArgSimCenterApp(QString("CBCitiesDL"));

    WDNWidget->addComponent(QString("None"), QString("None"), noneWidget3);
    WDNWidget->addComponent(QString("CBCities"), QString("CBCitiesDL"), WDNDL);

    this->addComponent("Buildings", buildingWidget);
    this->addComponent("Gas Network",pipelineWidget);
    this->addComponent("Water Network",WDNWidget);

    this->hideAll();
}


DLWidget::~DLWidget()
{

}


QList<QString> DLWidget::getActiveDLApps(void)
{
    QList<QString> activeDLapps;
    auto activeList = this->getActiveComponents();

    for(auto&& it : activeList)
    {
        auto activeComp = dynamic_cast<SimCenterAppSelection*>(this->getComponent(it));

        if(activeComp == nullptr)
            return activeDLapps;

        auto currComp = activeComp->getCurrentSelectionName();

        if(currComp.isEmpty())
        {
            this->errorMessage("Could not get the active DL apps in DLWidget");
            return activeDLapps;
        }

        activeDLapps.append(currComp);
    }

    return activeDLapps;
}


void DLWidget::clear(void)
{
    buildingWidget->clear();
    pipelineWidget->clear();
    WDNWidget->clear();
}



