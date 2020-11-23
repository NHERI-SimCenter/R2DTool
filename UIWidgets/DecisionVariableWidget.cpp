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

#include "DecisionVariableWidget.h"
#include "sectiontitle.h"
#include "SimCenterComponentSelection.h"

#include <QListWidget>
#include <QCheckBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QComboBox>
#include <QPushButton>
#include <QJsonObject>
#include <QJsonArray>
#include <QLabel>
#include <QLineEdit>
#include <QDebug>
#include <QFileDialog>
#include <QPushButton>

DecisionVariableWidget::DecisionVariableWidget(QWidget *parent): SimCenterAppWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setMargin(0);

    QHBoxLayout *theHeaderLayout = new QHBoxLayout();
    SectionTitle *label = new SectionTitle();
    label->setText(QString("Decision Variable (DV)"));
    label->setMinimumWidth(150);

    theHeaderLayout->addWidget(label);
    QSpacerItem *spacer = new QSpacerItem(50,10);
    theHeaderLayout->addItem(spacer);

    theHeaderLayout->addStretch(1);
    mainLayout->addLayout(theHeaderLayout);

    auto theComponentSelection = new SimCenterComponentSelection();
    mainLayout->addWidget(theComponentSelection);

    theComponentSelection->setWidth(120);

    QGroupBox* numRepairsBox = this->getNumRepairsWidget();
    QGroupBox* numBreaksBox = this->getNumBreaksWidget();
    QGroupBox* serviceabilityBox = this->getServiceabilityWidget();

    QGroupBox* DM4Box = this->getDM4Widget();

    theComponentSelection->addComponent("Annual Number\nof Repairs",numRepairsBox);
    theComponentSelection->addComponent("Annual Number\nof Breaks",numBreaksBox);
    theComponentSelection->addComponent("Serviceability\nIndex",serviceabilityBox);
    theComponentSelection->addComponent("DV 4",DM4Box);

    theComponentSelection->displayComponent("Annual Number\nof Repairs");
    theComponentSelection->setWidth(150);

    this->setLayout(mainLayout);
}


DecisionVariableWidget::~DecisionVariableWidget()
{

}


bool DecisionVariableWidget::outputToJSON(QJsonObject &jsonObject)
{
    return true;
}


bool DecisionVariableWidget::inputFromJSON(QJsonObject &jsonObject)
{
    return false;
}


bool DecisionVariableWidget::outputAppDataToJSON(QJsonObject &jsonObject)
{
    return true;
}


bool DecisionVariableWidget::inputAppDataFromJSON(QJsonObject &jsonObject)
{
    return false;
}


bool DecisionVariableWidget::copyFiles(QString &destDir)
{
    return false;
}


QGroupBox* DecisionVariableWidget::getNumRepairsWidget(void)
{
    QGroupBox* groupBox = new QGroupBox("Annual Number of Repairs");
    groupBox->setFlat(true);

    auto smallVSpacer = new QSpacerItem(0,10);

    auto ModelLabel = new QLabel("Model:");
    auto modelSelectCombo = new QComboBox();
    modelSelectCombo->addItem("O’Rouke (2020, preferred)");
    modelSelectCombo->setCurrentIndex(0);
    modelSelectCombo->setMinimumWidth(300);
    modelSelectCombo->setMaximumWidth(450);

    auto sourceDemandLabel = new QLabel("Source for Demand");
    sourceDemandLabel->setStyleSheet("font-weight: bold; color: black");

    QCheckBox* GMCheckbox = new QCheckBox("Ground Motion", this);
    QCheckBox* LSCheckbox = new QCheckBox("Lateral Spreading", this);
    QCheckBox* GSCheckbox = new QCheckBox("Ground Spreading", this);
    QCheckBox* LandslideCheckbox = new QCheckBox("Landslide", this);
    QCheckBox* SFRCheckbox = new QCheckBox("Surface Fault Rupture", this);
    QCheckBox* SSFRCheckbox = new QCheckBox("Subsurface Fault Rupture", this);
    QCheckBox* GndStrnCheckbox = new QCheckBox("Ground Strain", this);
    QCheckBox* PSCheckbox = new QCheckBox("Pipe Strain", this);

    GMCheckbox->setChecked(true);
    LSCheckbox->setChecked(true);
    LandslideCheckbox->setChecked(true);


    QPushButton *addRunListButton = new QPushButton();
    addRunListButton->setText(tr("Add run to list"));
    addRunListButton->setMinimumWidth(250);

    // List widget of cases to run
    auto casesListLabel = new QLabel("List of Cases to Run");
    casesListLabel->setStyleSheet("font-weight: bold; color: black");

    QListWidget *listWidget = new QListWidget(this);

    // Sample list widget item
    new QListWidgetItem(tr("1. O’Rourke(2020)\n    a. Ground Motion\n    b. Lateral Spreading\n    c. Landslide"), listWidget);
    new QListWidgetItem(tr("2. HAZUS(2014)\n    a. Ground Motion\n    b. Lateral Spreading\n    c. Landslide"), listWidget);

    listWidget->setMaximumWidth(400);
    listWidget->setMinimumWidth(300);

    // Add a vertical spacer at the bottom to push everything up
    auto vspacer = new QSpacerItem(0,0,QSizePolicy::Minimum, QSizePolicy::Expanding);
    auto hspacer = new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Minimum);

    QGridLayout* gridLayout = new QGridLayout(this);

    gridLayout->addItem(smallVSpacer,0,0);
    gridLayout->addWidget(ModelLabel,1,0);
    gridLayout->addWidget(modelSelectCombo,1,1);

    gridLayout->addWidget(sourceDemandLabel,2,0,1,2);
    gridLayout->addWidget(GMCheckbox,3,0,1,2);
    gridLayout->addWidget(LSCheckbox,4,0,1,2);
    gridLayout->addWidget(GSCheckbox,5,0,1,2);
    gridLayout->addWidget(LandslideCheckbox,6,0,1,2);
    gridLayout->addWidget(SFRCheckbox,7,0,1,2);
    gridLayout->addWidget(SSFRCheckbox,8,0,1,2);
    gridLayout->addWidget(GndStrnCheckbox,9,0,1,2);
    gridLayout->addWidget(PSCheckbox,10,0,1,2);

    gridLayout->addWidget(addRunListButton,11,0,1,3,Qt::AlignCenter);

    gridLayout->addWidget(casesListLabel,1,3);
    gridLayout->addWidget(listWidget,2,3,9,1);

    gridLayout->addItem(hspacer, 1, 4);
    gridLayout->addItem(vspacer, 12, 0);
    groupBox->setLayout(gridLayout);

    return groupBox;
}


QGroupBox* DecisionVariableWidget::getNumBreaksWidget(void)
{
    QGroupBox* groupBox = new QGroupBox("Annual Number of Breaks");
    groupBox->setFlat(true);

    return groupBox;
}


QGroupBox* DecisionVariableWidget::getServiceabilityWidget(void)
{
    QGroupBox* groupBox = new QGroupBox("Serviceability Index");
    groupBox->setFlat(true);

    return groupBox;
}


QGroupBox* DecisionVariableWidget::getDM4Widget(void)
{
    QGroupBox* groupBox = new QGroupBox("DM 4");
    groupBox->setFlat(true);

    return groupBox;
}
