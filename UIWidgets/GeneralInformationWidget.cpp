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

// Written by: Stevan Gavrilovic
// Latest revision: 09.30.2020

#include "GeneralInformationWidget.h"
#include "sectiontitle.h"

#include <QGridLayout>
#include <QGroupBox>
#include <QJsonArray>
#include <QPushButton>
#include <QRadioButton>
#include <QJsonObject>
#include <QComboBox>
#include <QDebug>
#include <QList>
#include <QLabel>
#include <QLineEdit>
#include <QMetaEnum>

GeneralInformationWidget* GeneralInformationWidget::getInstance()
{
    if (theInstance == 0)
        theInstance = new GeneralInformationWidget();

    return theInstance;
}


GeneralInformationWidget *GeneralInformationWidget::theInstance = nullptr;


GeneralInformationWidget::GeneralInformationWidget(QWidget *parent) : SimCenterWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setMargin(0);

    QHBoxLayout *theHeaderLayout = new QHBoxLayout();
    SectionTitle *label = new SectionTitle();
    label->setText(QString("General Information"));
    label->setMinimumWidth(150);

    theHeaderLayout->addWidget(label);
    QSpacerItem *spacer = new QSpacerItem(50,10);
    theHeaderLayout->addItem(spacer);
    theHeaderLayout->addStretch(1);

    auto infoLayout = this->getInfoLayout();

    mainLayout->addLayout(theHeaderLayout);
    mainLayout->addLayout(infoLayout);
    mainLayout->addStretch();

    this->setLayout(mainLayout);
    this->setMinimumWidth(640);
    this->setMaximumWidth(750);
}


GeneralInformationWidget::~GeneralInformationWidget()
{

}


bool GeneralInformationWidget::outputToJSON(QJsonObject &jsonObj){


    return true;
}


bool GeneralInformationWidget::inputFromJSON(QJsonObject &jsonObject){


    return true;
}


void GeneralInformationWidget::clear(void)
{
    analysisLineEdit->clear();
}


QGridLayout* GeneralInformationWidget::getInfoLayout(void)
{
    auto analysisLabel = new QLabel("Analysis ID:");
    analysisLineEdit = new QLineEdit();
    analysisLineEdit->setText("Analysis_1");
    analysisLineEdit->setMaximumWidth(250);

    auto unitSystemLabel = new QLabel("Unit System:");
    unitsCombo = new QComboBox();
    unitsCombo->addItem("SI (km)");
    unitsCombo->setCurrentIndex(0);
    unitsCombo->setMaximumWidth(260);

    QPushButton *loadFileButton = new QPushButton();
    loadFileButton->setText(tr("Browse"));
    loadFileButton->setMaximumWidth(150);
    auto workingDirectoryLabel = new QLabel("Working Directory:");
    workingDirectoryLineEdit = new QLineEdit();
    workingDirectoryLineEdit->setMaximumWidth(400);

    auto UQLabel = new QLabel("Uncertainty Quantification (UQ)");
    UQLabel->setStyleSheet("font-weight: bold; color: black");

    auto dataGenLabel = new QLabel("Method for Data Generation");
    dataGenCombo = new QComboBox();
    dataGenCombo->addItem("Latin Hypercube Sampling");
    dataGenCombo->setCurrentIndex(0);
    dataGenCombo->setMaximumWidth(250);

    auto numSamplesLabel = new QLabel("Number of Samples:");
    numSamplesLineEdit = new QLineEdit();
    numSamplesLineEdit->setText("100");
    numSamplesLineEdit->setMaximumWidth(100);

    auto seedLabel = new QLabel("Seed:");
    seedLineEdit = new QLineEdit();
    seedLineEdit->setText("1");
    seedLineEdit->setMaximumWidth(100);


    UQLabel->hide();
    dataGenLabel->hide();
    dataGenCombo->hide();
    numSamplesLabel->hide();
    numSamplesLineEdit->hide();
    seedLabel->hide();
    seedLineEdit->hide();

    auto assessmentSetupLabel = new QLabel("Assessment Setup");
    assessmentSetupLabel->setStyleSheet("font-weight: bold; color: black");

    QRadioButton *button1 = new QRadioButton("Pre-configured setup for risk assessment (preferred)");
    QRadioButton *button2 = new QRadioButton("Allow for user customization (e.g., source models, evaluation methods to use)");
    button1->setChecked(true);

    auto warningLabel = new QLabel();
    warningLabel->setText("Warning: Only choose this option if you have read the user manual and are familiar with the program");
    warningLabel->setStyleSheet("color: red");

    // Layout the UI components in a grid
    QGridLayout* layout = new QGridLayout();

    layout->addWidget(analysisLabel, 0, 0);
    layout->addWidget(analysisLineEdit, 0, 1);

    layout->addWidget(unitSystemLabel, 1, 0);
    layout->addWidget(unitsCombo, 1, 1);

    layout->addWidget(workingDirectoryLabel, 2, 0);
    layout->addWidget(workingDirectoryLineEdit, 2, 1);
    layout->addWidget(loadFileButton, 2, 2);

    layout->addWidget(UQLabel, 3, 0);

    layout->addWidget(dataGenLabel, 4, 0);
    layout->addWidget(dataGenCombo, 4, 1);

    layout->addWidget(numSamplesLabel, 5, 0);
    layout->addWidget(numSamplesLineEdit, 5, 1);

    layout->addWidget(seedLabel, 6, 0);
    layout->addWidget(seedLineEdit, 6, 1);

    layout->addWidget(assessmentSetupLabel, 7, 0);

    layout->addWidget(button1, 8, 0, 1, 3);
    layout->addWidget(button2, 9, 0, 1, 3);

    layout->addWidget(warningLabel, 10, 0, 1, 3);

    return layout;
}



