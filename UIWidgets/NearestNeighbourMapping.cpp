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

#include "NearestNeighbourMapping.h"
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

NearestNeighbourMapping::NearestNeighbourMapping(QWidget *parent) : SimCenterAppWidget(parent)
{
    QGridLayout* regionalMapLayout = new QGridLayout(this);

    QLabel* samplesLabel = new QLabel("Number of samples",this);
    QLabel* neighborsLabel = new QLabel("Number of neighbors",this);\

    QIntValidator *validator = new QIntValidator(this);
    validator->setBottom(1);

    samplesLineEdit = new QLineEdit(this);
    samplesLineEdit->setText("5");
    neighborsLineEdit = new QLineEdit(this);
    neighborsLineEdit->setText("4");

    samplesLineEdit->setValidator(validator);
    neighborsLineEdit->setValidator(validator);

    regionalMapLayout->addWidget(samplesLabel, 0, 0);
    regionalMapLayout->addWidget(samplesLineEdit, 0, 1);
    regionalMapLayout->addWidget(neighborsLabel, 1, 0);
    regionalMapLayout->addWidget(neighborsLineEdit, 1, 1);
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

    QJsonObject data;
    QFileInfo theFile(eventGridPath);
    if (theFile.exists()) {
        nearestNeigborObj["filenameEVENTgrid"]=theFile.fileName();
        nearestNeigborObj["pathToEventFile"]=theFile.path();

    } else {
         nearestNeigborObj.insert("filenameEVENTgrid",eventGridPath);
         nearestNeigborObj["filenameEVENTgrid"]="None";
         nearestNeigborObj["pathToEventFile"]="";
        return false;
    }

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

        QString filePath;

        if (appData.contains("pathToEventFile")) {
            filePath = appData["pathToEventFile"].toString();
            if (filePath != "")
             eventGridPath = filePath + QDir::separator();
        }

        if (appData.contains("filenameEVENTgrid"))
            eventGridPath += appData["filenameEVENTgrid"].toString();
    }

    return true;
}


void NearestNeighbourMapping::handleFileNameChanged(QString motionDir, QString eventFile)
{
    eventGridPath = eventFile;
    motionDirPath = motionDir;
}


bool NearestNeighbourMapping::copyFiles(QString &destName)
{
    QFileInfo theFile(eventGridPath);
    if (theFile.exists()) {
        return this->copyPath(theFile.path(), destName, false);
    }
}

