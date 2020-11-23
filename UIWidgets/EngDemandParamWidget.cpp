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
// Latest revision: 10.01.2020

#include "EngDemandParamWidget.h"
#include "sectiontitle.h"
#include "SimCenterComponentSelection.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QStackedWidget>
#include <QComboBox>
#include <QListWidget>
#include <QPushButton>
#include <QJsonObject>
#include <QJsonArray>
#include <QLabel>
#include <QLineEdit>
#include <QDebug>
#include <QFileDialog>
#include <QPushButton>

EngDemandParamWidget::EngDemandParamWidget(QWidget *parent) : SimCenterAppWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setMargin(0);

    QHBoxLayout *theHeaderLayout = new QHBoxLayout();
    SectionTitle *label = new SectionTitle();
    label->setText(QString("Engineering Demand Parameter (EDP)"));
    label->setMinimumWidth(150);

    theHeaderLayout->addWidget(label);
    QSpacerItem *spacer = new QSpacerItem(50,10);
    theHeaderLayout->addItem(spacer);

    theHeaderLayout->addStretch(1);
    mainLayout->addLayout(theHeaderLayout);

    auto theComponentSelection = new SimCenterComponentSelection();
    mainLayout->addWidget(theComponentSelection);

    theComponentSelection->setWidth(120);

    QGroupBox* liquefactionBox = this->getLiquefactionBox();
    QGroupBox* latSpreadBox = this->getLateralSpreadingBox();
    QGroupBox* groundSetBox = this->getGroundSettlementBox();
    QGroupBox* LandslideBox = this->getLandSlideBox();
    QGroupBox* surfFaultRup = this->getSurfaceFaultRupBox();
    QGroupBox* subsurfFaultRup = this->getSubsurfaceFaultRupBox();
    QGroupBox* groundStrn = this->getGroundStrainBox();

    theComponentSelection->addComponent("Liquefaction",liquefactionBox);
    theComponentSelection->addComponent("Lateral\nSpreading",latSpreadBox);
    theComponentSelection->addComponent("Ground\nSettlement",groundSetBox);
    theComponentSelection->addComponent("Landslide",LandslideBox);
    theComponentSelection->addComponent("Surface\nFault Rupture",surfFaultRup);
    theComponentSelection->addComponent("Subsurface\nFault Rupture",subsurfFaultRup);
    theComponentSelection->addComponent("Ground\nStrain",groundStrn);

    theComponentSelection->displayComponent("Landslide");

    this->setLayout(mainLayout);
}

EngDemandParamWidget::~EngDemandParamWidget()
{

}


bool
EngDemandParamWidget::outputToJSON(QJsonObject &jsonObject)
{
    return true;
}


bool EngDemandParamWidget::inputFromJSON(QJsonObject &jsonObject)
{

    return false;
}


bool EngDemandParamWidget::outputAppDataToJSON(QJsonObject &jsonObject)
{
    return true;
}


bool EngDemandParamWidget::inputAppDataFromJSON(QJsonObject &jsonObject)
{
    return true;
}

bool EngDemandParamWidget::copyFiles(QString &destDir)
{

    return false;
}

QGroupBox* EngDemandParamWidget::getLandSlideBox(void)
{
    QGroupBox* groupBox = new QGroupBox("Landslide");
    groupBox->setFlat(true);

    auto smallVSpacer = new QSpacerItem(0,10);

    auto ModelLabel = new QLabel("Model:");
    auto modelSelectCombo = new QComboBox();
    modelSelectCombo->addItem("Median model (preferred, reference)");
    modelSelectCombo->setCurrentIndex(0);
    modelSelectCombo->setMinimumWidth(300);
    modelSelectCombo->setMaximumWidth(450);

    auto ModelParam1Label = new QLabel("Model Parameter 1:");
    auto ModelParam1LineEdit = new QLineEdit();
    ModelParam1LineEdit->setText("100");
    ModelParam1LineEdit->setMaximumWidth(100);
    auto param1UnitLabel = new QLabel("Unit");

    auto ModelParam2Label = new QLabel("Model Parameter 2:");
    auto ModelParam2LineEdit = new QLineEdit();
    ModelParam2LineEdit->setText("100");
    ModelParam2LineEdit->setMaximumWidth(100);
    auto param2UnitLabel = new QLabel("Unit");

    auto ModelParamNLabel = new QLabel("Model Parameter N:");
    auto ModelParamNLineEdit = new QLineEdit();
    ModelParamNLineEdit->setText("100");
    ModelParamNLineEdit->setMaximumWidth(100);
    auto paramNUnitLabel = new QLabel("Unit");

    auto weightLabel = new QLabel("Weight:");
    auto weightLineEdit = new QLineEdit();
    weightLineEdit->setText("1");
    weightLineEdit->setMaximumWidth(100);

    QPushButton *addRunListButton = new QPushButton();
    addRunListButton->setText(tr("Add run to list"));
    addRunListButton->setMinimumWidth(250);

    // List widget of cases to run
    auto casesListLabel = new QLabel("List of Cases to Run");
    casesListLabel->setStyleSheet("font-weight: bold; color: black");

    QListWidget *listWidget = new QListWidget();

    // Sample list widget item
    new QListWidgetItem(tr("1. Median model - weight = 1"), listWidget);

    listWidget->setMaximumWidth(400);
    listWidget->setMinimumWidth(300);

    // Add a vertical spacer at the bottom to push everything up
    auto vspacer = new QSpacerItem(0,0,QSizePolicy::Minimum, QSizePolicy::Expanding);

    auto hspacer = new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Minimum);

    QGridLayout* gridLayout = new QGridLayout();

    gridLayout->addItem(smallVSpacer,0,0);
    gridLayout->addWidget(ModelLabel,1,0);
    gridLayout->addWidget(modelSelectCombo,1,1,1,2);

    gridLayout->addWidget(ModelParam1Label,2,0);
    gridLayout->addWidget(ModelParam1LineEdit,2,1);
    gridLayout->addWidget(param1UnitLabel,2,2);

    gridLayout->addWidget(ModelParam2Label,3,0);
    gridLayout->addWidget(ModelParam2LineEdit,3,1);
    gridLayout->addWidget(param2UnitLabel,3,2);

    gridLayout->addWidget(ModelParamNLabel,4,0);
    gridLayout->addWidget(ModelParamNLineEdit,4,1);
    gridLayout->addWidget(paramNUnitLabel,4,2);

    gridLayout->addWidget(weightLabel,5,0);
    gridLayout->addWidget(weightLineEdit,5,1);

    gridLayout->addWidget(addRunListButton,6,0,1,3,Qt::AlignCenter);

    gridLayout->addWidget(casesListLabel,1,3);
    gridLayout->addWidget(listWidget,2,3,6,1);

    gridLayout->addItem(hspacer, 1, 4);
    gridLayout->addItem(vspacer, 7, 0);
    groupBox->setLayout(gridLayout);

    return groupBox;
}

QGroupBox* EngDemandParamWidget::getLiquefactionBox(void)
{
    QGroupBox* groupBox = new QGroupBox("Liquefaction");
    groupBox->setFlat(true);


    return groupBox;
}


QGroupBox* EngDemandParamWidget::getLateralSpreadingBox(void)
{
    QGroupBox* groupBox = new QGroupBox("Lateral Spreading");
    groupBox->setFlat(true);


    return groupBox;
}


QGroupBox* EngDemandParamWidget::getGroundSettlementBox(void)
{
    QGroupBox* groupBox = new QGroupBox("Ground Settlement");
    groupBox->setFlat(true);


    return groupBox;
}


QGroupBox* EngDemandParamWidget::getSurfaceFaultRupBox(void)
{
    QGroupBox* groupBox = new QGroupBox("Surface Fault Rupture");
    groupBox->setFlat(true);


    return groupBox;
}


QGroupBox* EngDemandParamWidget::getSubsurfaceFaultRupBox(void)
{
    QGroupBox* groupBox = new QGroupBox("Subsurface Fault Rupture");
    groupBox->setFlat(true);


    return groupBox;
}


QGroupBox* EngDemandParamWidget::getGroundStrainBox(void)
{
    QGroupBox* groupBox = new QGroupBox("Ground Strain");
    groupBox->setFlat(true);


    return groupBox;
}

