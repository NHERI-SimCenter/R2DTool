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

// Written by: Stevan Gavrilovic

#include "NearestNeighbourMapping.h"
#include "SimCenterPreferences.h"

#include <QComboBox>
#include <QDebug>
#include <QDir>
#include <QGridLayout>
#include <QGroupBox>
#include <QIntValidator>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>

NearestNeighbourMapping::NearestNeighbourMapping(QWidget *parent) : SimCenterAppWidget(parent)
{
    QGridLayout* regionalMapLayout = new QGridLayout(this);

    QLabel* samplesLabel = new QLabel("Number of samples",this);
    QLabel* neighborsLabel = new QLabel("Number of neighbors",this);\

    // create label and entry for seed to layout
    srand(time(NULL));
    int randomNumber = rand() % 1000 + 1;
    randomSeed = new QLineEdit();
    randomSeed->setText(QString::number(randomNumber));
    randomSeed->setToolTip("Set the seed");
						       
    QIntValidator *validator = new QIntValidator(this);
    validator->setBottom(1);

    samplesLineEdit = new QLineEdit(this);
    samplesLineEdit->setText("5");
    neighborsLineEdit = new QLineEdit(this);
    neighborsLineEdit->setText("4");

    samplesLineEdit->setValidator(validator);
    neighborsLineEdit->setValidator(validator);
    randomSeed->setValidator(validator);

    regionalMapLayout->addWidget(samplesLabel, 0, 0);
    regionalMapLayout->addWidget(samplesLineEdit, 0, 1);
    regionalMapLayout->addWidget(neighborsLabel, 1, 0);
    regionalMapLayout->addWidget(neighborsLineEdit, 1, 1);
    regionalMapLayout->addWidget(new QLabel("Seed"), 2, 0);
    regionalMapLayout->addWidget(randomSeed, 2, 1);

    regionalMapLayout->setRowStretch(3,1);
    
}


NearestNeighbourMapping::~NearestNeighbourMapping()
{

}


bool NearestNeighbourMapping::outputAppDataToJSON(QJsonObject &jsonObj)
{
    jsonObj["Application"] = QString("NearestNeighborEvents");

    QJsonObject nearestNeigborObj;

    nearestNeigborObj.insert("samples",samplesLineEdit->text().toInt());
    nearestNeigborObj.insert("neighbors",neighborsLineEdit->text().toInt());
    nearestNeigborObj.insert("seed",randomSeed->text().toInt());    

    jsonObj.insert("ApplicationData",nearestNeigborObj);

    return true;
}


bool NearestNeighbourMapping::inputAppDataFromJSON(QJsonObject &jsonObject)
{

    //qDebug() << __PRETTY_FUNCTION__ << jsonObject;

    if (jsonObject.contains("ApplicationData"))
    {
        QJsonObject appData = jsonObject["ApplicationData"].toObject();

        if (appData.contains("neighbors"))
            neighborsLineEdit->setText(QString::number(appData["neighbors"].toInt()));

        if (appData.contains("samples"))
            samplesLineEdit->setText(QString::number(appData["samples"].toInt()));

        if (appData.contains("seed"))
            randomSeed->setText(QString::number(appData["seed"].toInt()));	

    }

    return true;
}


void NearestNeighbourMapping::clear(void)
{
    samplesLineEdit->clear();
    neighborsLineEdit->clear();
    
    srand(time(NULL));
    int randomNumber = rand() % 1000 + 1;
    randomSeed->setText(QString::number(randomNumber));    
}


bool NearestNeighbourMapping::copyFiles(QString &destName)
{
  return true;
}

