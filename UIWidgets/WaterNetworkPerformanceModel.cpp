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

#include "WaterNetworkPerformanceModel.h"
#include "RandomVariablesContainer.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>


WaterNetworkPerformanceModel::WaterNetworkPerformanceModel(RandomVariablesContainer *theRandomVariableIW, QWidget *parent)
    : SimCenterAppWidget(parent), theRandomVariablesContainer(theRandomVariableIW)
{
    auto mainLayout = new QGridLayout(this);

    QLabel* pathText = new QLabel("Path to .inp file:");

    inpFileLineEdit = new QLineEdit();
    //    componentFileLineEdit->setMaximumWidth(750);
    inpFileLineEdit->setMinimumWidth(400);
    inpFileLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    QPushButton *browseFileButton = new QPushButton();
    browseFileButton->setText(tr("Browse"));
    browseFileButton->setMaximumWidth(150);

    connect(browseFileButton,SIGNAL(clicked()),this,SLOT(chooseFileDialog()));

    mainLayout->addWidget(pathText,0,0);
    mainLayout->addWidget(inpFileLineEdit,0,1);
    mainLayout->addWidget(browseFileButton,0,2);

    QLabel* methodText = new QLabel("Water Network Peformance Methodology:");

    perfomanceMethodCombo = new QComboBox();
    perfomanceMethodCombo->addItem("CB-Cities pipe-network",QVariant("CBCities"));
    perfomanceMethodCombo->addItem("Water Network Tool for Resilience (WNTR)",QVariant("WNTR"));

    mainLayout->addWidget(methodText,1,0);
    mainLayout->addWidget(perfomanceMethodCombo,1,1,1,2);
    mainLayout->setRowStretch(2,1);

    //    inpFileLineEdit->setText("/Users/steve/Desktop/SogaExample/central.inp");

}


WaterNetworkPerformanceModel::~WaterNetworkPerformanceModel()
{
    // remove old random variables
    QStringList names;
    for (int i=0; i<varNamesAndValues.size()-1; i+=2) {
        names.append(varNamesAndValues.at(i));
    }

    theRandomVariablesContainer->removeRandomVariables(names);
}


void WaterNetworkPerformanceModel::clear(void)
{
    pathToINPInputFile.clear();
    inpFileLineEdit->clear();
    varNamesAndValues.clear();
}


bool WaterNetworkPerformanceModel::outputToJSON(QJsonObject &/*jsonObject*/)
{


    return true;
}


bool WaterNetworkPerformanceModel::inputFromJSON(QJsonObject &/*jsonObject*/)
{

    return true;
}


bool WaterNetworkPerformanceModel::outputAppDataToJSON(QJsonObject &jsonObject) {

    bool res = true;

    //
    // per API, need to add name of application to be called in AppLication
    // and all data to be used in ApplicationDate
    //

    jsonObject["Application"] = "WaterNetworkPerformance";
    QJsonObject dataObj;

    // First the inp file (in EPANET format)
    QFileInfo inpFile(inpFileLineEdit->text());

    if (inpFile.exists()) {

        dataObj["waterNetworkSourceFile"]=inpFile.fileName();
        dataObj["pathToWaterNetworkSource"]=inpFile.path();
    }
    else
    {
        dataObj["waterNetworkSourceFile"]=QString("None");
        dataObj["pathToWaterNetworkSource"]=QString("");
        return false;
    }

    auto performanceMethod = perfomanceMethodCombo->currentData().toString();
    dataObj["performanceMethod"] = performanceMethod;

    jsonObject["ApplicationData"] = dataObj;

    return res;
}


bool WaterNetworkPerformanceModel::inputAppDataFromJSON(QJsonObject &jsonObject) {

    if (jsonObject.contains("ApplicationData")) {
        QJsonObject appData = jsonObject["ApplicationData"].toObject();


        // First load the .inp file
        QFileInfo inpFileInfo;
        QString inpFileName;
        QString pathToInpFile;
        bool foundFile = false;
        if (appData.contains("waterNetworkSourceFile"))
            inpFileName = appData["waterNetworkSourceFile"].toString();

        if (inpFileInfo.exists(inpFileName)) {

            pathToINPInputFile = inpFileName;
            inpFileLineEdit->setText(inpFileName);

            foundFile = true;

        } else {

            if (appData.contains("pathToWaterNetworkSource"))
                pathToInpFile = appData["pathToWaterNetworkSource"].toString();
            else
                pathToInpFile=QDir::currentPath();

            pathToINPInputFile = pathToInpFile + QDir::separator() + inpFileName;

            if (inpFileInfo.exists(pathToINPInputFile)) {
                inpFileLineEdit->setText(pathToINPInputFile);
                foundFile = true;

            } else {
                // adam .. adam .. adam
                pathToINPInputFile = pathToInpFile + QDir::separator()
                        + "input_data" + QDir::separator() + inpFileName;
                if (inpFileInfo.exists(pathToINPInputFile)) {
                    inpFileLineEdit->setText(pathToINPInputFile);
                    foundFile = true;
                }
                else
                {
                    QString errMessage = "WaterNetworkPerformanceModel no file found at: " + inpFileName;
                    this->errorMessage(errMessage);
                    return false;
                }
            }
        }

        if(foundFile == false)
        {
            QString errMessage = "WaterNetworkPerformanceModel no file found: " + inpFileName;
            this->errorMessage(errMessage);
            return false;
        }


        if(appData.contains("performanceMethod"))
        {

            auto performanceMethod = appData.value("performanceMethod").toVariant();

            auto idx = perfomanceMethodCombo->findData(performanceMethod);

            if(idx != -1)
                perfomanceMethodCombo->setCurrentIndex(idx);
            else
                this->errorMessage("Could not find the 'performanceMethod' "+performanceMethod.toString());

        }
        else
        {
            this->infoMessage("WaterNetworkPerformanceModel no 'performanceMethod' supplied, using default");
        }

    }


    this->errorMessage("WaterNetworkPerformanceModel no ApplicationData");
    return true;
}


void WaterNetworkPerformanceModel::chooseFileDialog(void)
{
    pathToINPInputFile = QFileDialog::getOpenFileName(this,tr("Water Network .inp File"));

    // Return if the user cancels
    if(pathToINPInputFile.isEmpty())
    {
        pathToINPInputFile = "NULL";
        return;
    }

    // Set file name & entry in qLine edit
    inpFileLineEdit->setText(pathToINPInputFile);
}


bool WaterNetworkPerformanceModel::copyFiles(QString &dirName) {

    QString fileName = inpFileLineEdit->text();
    QFileInfo fileInfo(fileName);

    if (fileInfo.exists()) {
        return this->copyFile(fileName, dirName);
    }

    return true;
}

