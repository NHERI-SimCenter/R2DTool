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

#include "RegionalMappingWidget.h"
#include "SimCenterPreferences.h"

#include <QGridLayout>
#include <QGroupBox>
#include <QJsonObject>
#include <QComboBox>
#include <QDebug>
#include <QLabel>
#include <QLineEdit>
#include <QIntValidator>
#include <QDir>

RegionalMappingWidget::RegionalMappingWidget(QWidget *parent) : SimCenterAppWidget(parent)
{
    QGridLayout* regionalMapLayout = new QGridLayout(this);

    QLabel* appLabel = new QLabel("Application",this);
    QComboBox* appSelectCombo = new QComboBox(this);
    appSelectCombo->addItem("Nearest Neighbour Event");

    QLabel* samplesLabel = new QLabel("Number of samples",this);
    QLabel* neighborsLabel = new QLabel("Number of neighbors",this);\
//    QLabel* eventGridLabel = new QLabel("Filename of event grid",this);

    QIntValidator *validator = new QIntValidator(this);
    validator->setBottom(1);

    samplesLineEdit = new QLineEdit(this);
    samplesLineEdit->setText("5");
    neighborsLineEdit = new QLineEdit(this);
    neighborsLineEdit->setText("4");

    samplesLineEdit->setValidator(validator);
    neighborsLineEdit->setValidator(validator);

//    filenameLineEdit = new QLineEdit(this);
//    filenameLineEdit->setText("EventGrid");

    regionalMapLayout->addWidget(appLabel,0, 0);
    regionalMapLayout->addWidget(appSelectCombo, 0, 1);
    regionalMapLayout->addWidget(samplesLabel, 1, 0);
    regionalMapLayout->addWidget(samplesLineEdit, 1, 1);
    regionalMapLayout->addWidget(neighborsLabel, 2, 0);
    regionalMapLayout->addWidget(neighborsLineEdit, 2, 1);
//    regionalMapLayout->addWidget(eventGridLabel, 3, 0);
//    regionalMapLayout->addWidget(filenameLineEdit, 3, 1);
}


RegionalMappingWidget::~RegionalMappingWidget()
{

}


bool RegionalMappingWidget::outputAppDataToJSON(QJsonObject &jsonObj)
{

    QJsonObject regionalMapObj;

    regionalMapObj.insert("Application","NearestNeighborEvents");

    QJsonObject nearestNeigborObj;

    nearestNeigborObj.insert("samples",samplesLineEdit->text().toInt());
    nearestNeigborObj.insert("neighbors",neighborsLineEdit->text().toInt());
    nearestNeigborObj.insert("filenameEVENTgrid",eventGridPath);

    regionalMapObj.insert("ApplicationData",nearestNeigborObj);

    jsonObj.insert("RegionalMapping", regionalMapObj);

    return true;
}


bool RegionalMappingWidget::inputAppDataFromJSON(QJsonObject &jsonObject){


    return true;
}


void RegionalMappingWidget::handleFileNameChanged(const QString &value)
{
    eventGridPath = value;
}




