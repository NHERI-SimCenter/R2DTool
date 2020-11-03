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

#include "IntensityMeasureWidget.h"
#include "sectiontitle.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QStackedWidget>
#include <QComboBox>
#include <QListWidget>
#include <QSpacerItem>
#include <QPushButton>
#include <QJsonObject>
#include <QJsonArray>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QDebug>
#include <QFileDialog>
#include <QPushButton>

IntensityMeasureWidget::IntensityMeasureWidget(GeneralInformationWidget* generalInfoWidget, QWidget *parent)
    : SimCenterAppWidget(parent)
{

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setMargin(0);

    QHBoxLayout *theHeaderLayout = new QHBoxLayout();
    SectionTitle *label = new SectionTitle();
    label->setText(QString("Intensity Measure (IM)"));
    label->setMinimumWidth(150);

    theHeaderLayout->addWidget(label);
    QSpacerItem *spacer = new QSpacerItem(50,10);
    theHeaderLayout->addItem(spacer);
    theHeaderLayout->addStretch(1);

    auto IMLayout = this->getIMLayout();

    mainLayout->addLayout(theHeaderLayout);
    mainLayout->addLayout(IMLayout);
    mainLayout->addStretch();

    this->setLayout(mainLayout);
    this->setMinimumWidth(640);
    this->setMaximumWidth(1000);
}


IntensityMeasureWidget::~IntensityMeasureWidget()
{

}


bool IntensityMeasureWidget::outputToJSON(QJsonObject &jsonObject)
{
    return true;
}


bool IntensityMeasureWidget::inputFromJSON(QJsonObject &jsonObject)
{
    return false;
}


void IntensityMeasureWidget::eventSelectionChanged(const QString &arg1)
{

}


bool IntensityMeasureWidget::outputAppDataToJSON(QJsonObject &jsonObject)
{

    return true;
}


bool IntensityMeasureWidget::inputAppDataFromJSON(QJsonObject &jsonObject)
{
    return true;
}


bool IntensityMeasureWidget::copyFiles(QString &destDir)
{

    return false;
}


QGridLayout* IntensityMeasureWidget::getIMLayout(void)
{
    auto smallVSpacer = new QSpacerItem(0,10);

    auto GMCharacLabel = new QLabel("Ground Motion Characterization");
    GMCharacLabel->setStyleSheet("font-weight: bold; color: black");

    auto ModelLabel = new QLabel("Model:");
    auto modelSelectCombo = new QComboBox();
    modelSelectCombo->addItem("Median NGAWest2 model (preferred, reference)");
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

    auto IMLabel = new QLabel("Intensity Measures");
    IMLabel->setStyleSheet("font-weight: bold; color: black");

    QCheckBox* PGACheckbox = new QCheckBox("Peak Ground Acceleration (PGA)");
    QCheckBox* PGVCheckbox = new QCheckBox("Peak Ground Velocity (PGV)");

    PGACheckbox->setChecked(true);
    PGVCheckbox->setChecked(true);

    auto correlationsLabel = new QLabel("Correlations");
    correlationsLabel->setStyleSheet("font-weight: bold; color: black");

    QCheckBox* spatialCorrCheckbox = new QCheckBox("Spatial Correlation");
    QCheckBox* spectralCorrCheckbox = new QCheckBox("Spectral (Cross) Correlation");

    spatialCorrCheckbox->setChecked(true);
    spectralCorrCheckbox->setChecked(true);

    QRadioButton *spatialCorr1 = new QRadioButton("Jayaram && Baker (2009)");
    QRadioButton *spatialCorr2 = new QRadioButton("Method 2");
    spatialCorr1->setChecked(true);

    QRadioButton *spectralCorr1 = new QRadioButton("Baker && Jayaram (2008)");
    QRadioButton *spectralCorr2 = new QRadioButton("Method 2");
    spectralCorr1->setChecked(true);

    auto casesListLabel = new QLabel("List of Cases to Run");
    casesListLabel->setStyleSheet("font-weight: bold; color: black");

    QListWidget *listWidget = new QListWidget();

    // Sample list widget item
    new QListWidgetItem(tr("1. Median NGAWest2 model - weight = 1"), listWidget);

    listWidget->setMaximumWidth(400);
    listWidget->setMinimumWidth(300);

    // Add a vertical spacer at the bottom to push everything up
    auto vspacer = new QSpacerItem(0,0,QSizePolicy::Minimum, QSizePolicy::Expanding);

    QGridLayout* gridLayout = new QGridLayout();

    gridLayout->addItem(smallVSpacer,0,0);
    gridLayout->addWidget(GMCharacLabel,1,0);

    gridLayout->addWidget(ModelLabel,2,0);
    gridLayout->addWidget(modelSelectCombo,2,1,1,2);

    gridLayout->addWidget(ModelParam1Label,3,0);
    gridLayout->addWidget(ModelParam1LineEdit,3,1);
    gridLayout->addWidget(param1UnitLabel,3,2);

    gridLayout->addWidget(ModelParam2Label,4,0);
    gridLayout->addWidget(ModelParam2LineEdit,4,1);
    gridLayout->addWidget(param2UnitLabel,4,2);

    gridLayout->addWidget(ModelParamNLabel,5,0);
    gridLayout->addWidget(ModelParamNLineEdit,5,1);
    gridLayout->addWidget(paramNUnitLabel,5,2);

    gridLayout->addWidget(weightLabel,6,0);
    gridLayout->addWidget(weightLineEdit,6,1);

    gridLayout->addWidget(addRunListButton,7,0,1,3,Qt::AlignCenter);

    gridLayout->addItem(smallVSpacer,8,0);

    gridLayout->addWidget(IMLabel,9,0);

    gridLayout->addWidget(PGACheckbox,10,0);
    gridLayout->addWidget(PGVCheckbox,10,1);

    gridLayout->addItem(smallVSpacer,11,0);

    gridLayout->addWidget(correlationsLabel,12,0);

    gridLayout->addWidget(spatialCorrCheckbox,13,0);
    gridLayout->addWidget(spatialCorr1,13,1);
    gridLayout->addWidget(spatialCorr2,13,2);

    gridLayout->addWidget(spectralCorrCheckbox,14,0);
    gridLayout->addWidget(spectralCorr1,14,1);
    gridLayout->addWidget(spectralCorr2,14,2);

    gridLayout->addWidget(casesListLabel,1,4);
    gridLayout->addWidget(listWidget,2,4,14,1);

    gridLayout->addItem(vspacer, 15, 0);

    return gridLayout;
}
