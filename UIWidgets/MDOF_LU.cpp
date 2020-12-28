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

// Written by: Frank McKenna

#include "MDOF_LU.h"
#include "SimCenterPreferences.h"
#include "RandomVariablesContainer.h"

#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

MDOF_LU::MDOF_LU(RandomVariablesContainer *theRandomVariableIW, QWidget *parent)
    : SimCenterAppWidget(parent), theRandomVariablesContainer(theRandomVariableIW)
{
    QGridLayout *layout = new QGridLayout();

    QLabel *hazusLabel = new QLabel("Hazus Data File");
    hazusDataFile = new QLineEdit;
    QPushButton *hazusButton = new QPushButton("Choose");

    auto SCPrefs = SimCenterPreferences::getInstance();

    QString filePath = SCPrefs->getAppDir() + QDir::separator()
            + "applications" + QDir::separator() + "createSAM"
            + QDir::separator() + "MDOF-LU" + QDir::separator()
            + "data" + QDir::separator() + "HazusData.txt";
    QFileInfo hFile(filePath);
    if (hFile.exists()) {
        hazusDataFile->setText(filePath);
    }

    connect(hazusButton,SIGNAL(clicked()),this,SLOT(chooseFileName1()));
    layout->addWidget(hazusLabel,    0,0);
    layout->addWidget(hazusDataFile, 0,1);
    layout->addWidget(hazusButton,   0,2);

    QLabel *stiffnessLabel= new QLabel("stdStiffness");
    stdStiffness = new QLineEdit;
    stdStiffness->setText("0.1");
    layout->addWidget(stiffnessLabel, 1,0);
    layout->addWidget(stdStiffness,   1,1);

    QLabel *dampingLabel= new QLabel("stdDamping");
    stdDamping = new QLineEdit;
    stdDamping->setText("0.1");
    layout->addWidget(dampingLabel, 2,0);
    layout->addWidget(stdDamping,   2,1);

    layout->setRowStretch(3,1);

    this->setLayout(layout);
}


MDOF_LU::~MDOF_LU()
{

}


void MDOF_LU::clear(void)
{

}


bool MDOF_LU::outputToJSON(QJsonObject &jsonObject)
{
    return true;
}


bool MDOF_LU::inputFromJSON(QJsonObject &jsonObject)
{
    return true;
}


bool MDOF_LU::outputAppDataToJSON(QJsonObject &jsonObject) {

    bool res = true;

    jsonObject["Application"] = "MDOF-LU";
    QJsonObject dataObj;

    dataObj["stdStiffness"] = stdStiffness->text();
    dataObj["stdDamping"]   = stdDamping->text();

    QFileInfo theFile(hazusDataFile->text());
    if (theFile.exists()) {
        dataObj["hazusData"]=theFile.fileName();
        dataObj["pathToHazusFile"]=theFile.path();
    } else {
        dataObj["hazusData"]=QString("None");
        dataObj["pathToHazusFile"]=QString("");
        return false;
    }


    jsonObject["ApplicationData"] = dataObj;

    return res;
}


bool MDOF_LU::inputAppDataFromJSON(QJsonObject &jsonObject) {

    if (jsonObject.contains("ApplicationData")) {
        QJsonObject appData = jsonObject["ApplicationData"].toObject();


        QString fileName;
        QString pathToFile;
        if (appData.contains("stdStiffness"))
            stdStiffness->setText(appData["stdStiffness"].toString());
        if (appData.contains("stdDamping"))
            stdDamping->setText(appData["stdDamping"].toString());
        if (appData.contains("hazusData"))
            fileName = appData["hazusData"].toString();
        if (appData.contains("pathToHazusFile"))
            pathToFile = appData["pathToHazusFile"].toString();
        hazusDataFile->setText(pathToFile + QDir::separator() + fileName);
    }
    return true;
}


void MDOF_LU::chooseFileName1(void) {
    QString filePath=QFileDialog::getOpenFileName(this,tr("Open File"),"C://", "All Files (*.*)");
    hazusDataFile->setText(filePath);
}


bool MDOF_LU::copyFiles(QString &dirName) {

    QString fileName = hazusDataFile->text();
    QFileInfo fileInfo(fileName);

    if (fileInfo.exists()) {
        return this->copyFile(fileName, dirName);
    }

    return true;
}

