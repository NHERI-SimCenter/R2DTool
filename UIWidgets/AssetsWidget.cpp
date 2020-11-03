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
// Latest revision: 10.08.2020

#include "AssetsWidget.h"
#include "ComponentInputWidget.h"
#include "VisualizationWidget.h"
#include "sectiontitle.h"
#include "SimCenterComponentSelection.h"

// Qt headers
#include <QHBoxLayout>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QPushButton>
#include <QGroupBox>
#include <QJsonObject>
#include <QJsonArray>
#include <QTableWidget>
#include <QColorTransform>
#include <QLineEdit>
#include <QListWidget>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QPointer>
#include <QPushButton>
#include <QCheckBox>


AssetsWidget::AssetsWidget(QWidget *parent, VisualizationWidget* visWidget)
    : SimCenterAppWidget(parent), visualizationWidget(visWidget)
{
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setMargin(0);

    QHBoxLayout *theHeaderLayout = new QHBoxLayout();
    SectionTitle *label = new SectionTitle();
    label->setText(QString("Input Asset Classes"));
    label->setMinimumWidth(150);

    theHeaderLayout->addWidget(label);
    QSpacerItem *spacer = new QSpacerItem(50,10);
    theHeaderLayout->addItem(spacer);

    theHeaderLayout->addStretch(1);
    mainLayout->addLayout(theHeaderLayout);

    auto theComponentSelection = new SimCenterComponentSelection();
    mainLayout->addWidget(theComponentSelection);

    theComponentSelection->setMaxWidth(120);

    buildingWidget = std::make_unique<ComponentInputWidget>(this, "Buildings");
    pipelineWidget = std::make_unique<ComponentInputWidget>(this, "Pipelines");

    visualizationWidget->setBuildingWidget(buildingWidget.get());
    visualizationWidget->setPipelineWidget(pipelineWidget.get());

    QGroupBox* buildingInfoBox = buildingWidget->getComponentsWidget();
    QGroupBox* pipelineInfoBox = pipelineWidget->getComponentsWidget();

    theComponentSelection->addComponent("Buildings",buildingInfoBox);
    theComponentSelection->addComponent("Pipelines",pipelineInfoBox);
    theComponentSelection->displayComponent("Pipelines");

    QString pathToPipelineInfoFile =  "/Users/steve/Desktop/SimCenter/Examples/CECPipelineExample/sample_input.csv";
    pipelineWidget->testFileLoad(pathToPipelineInfoFile);

    QString pathToBuildingInfoFile =  "/Users/steve/Desktop/SimCenter/Examples/AnchorageExample/AnchorageBuildings.csv";
    buildingWidget->testFileLoad(pathToBuildingInfoFile);

    this->setLayout(mainLayout);
}


AssetsWidget::~AssetsWidget()
{

}


bool AssetsWidget::outputToJSON(QJsonObject &jsonObject)
{
    return true;
}


bool AssetsWidget::inputFromJSON(QJsonObject &jsonObject)
{
    return false;
}


bool AssetsWidget::outputAppDataToJSON(QJsonObject &jsonObject)
{
    return true;
}


bool AssetsWidget::inputAppDataFromJSON(QJsonObject &jsonObject)
{
    return false;
}


bool AssetsWidget::copyFiles(QString &destDir)
{
    return false;
}




