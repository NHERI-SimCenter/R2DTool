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

#include "OpenSeesPyBuildingModel.h"
#include "OpenSeesParser.h"
#include "RandomVariablesContainer.h"
#include "sectiontitle.h"

#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QPushButton>
#include <QScrollArea>

#include <string>
#include <sstream>
#include <iostream>

using namespace std;

OpenSeesPyBuildingModel::OpenSeesPyBuildingModel(RandomVariablesContainer *theRandomVariableIW, QWidget *parent)
    : SimCenterAppWidget(parent), responseNodes(0), theRandomVariablesContainer(theRandomVariableIW)
{
    responseNodes = NULL;
    layout = new QGridLayout();

    QLabel *label1 = new QLabel();
    label1->setText("Input Script");

    filePathLineEdit = new QLineEdit;
    QPushButton *chooseFile1 = new QPushButton();
    chooseFile1->setText(tr("Choose"));
    connect(chooseFile1,SIGNAL(clicked()),this,SLOT(chooseFileName1()));
    layout->addWidget(label1,0,0);
    layout->addWidget(filePathLineEdit,0,1);
    layout->addWidget(chooseFile1,0,2);

    QLabel *label2 = new QLabel();
    label2->setText("DOF Mapping:");
    responseNodes = new QLineEdit;
    responseNodes->setText("1,2,3");

    QRegExp LERegExp ("[1-3][ ]*,[ ]*[1-3][ ]*,[ ]*[1-3]");
    QRegExpValidator* LEValidator = new QRegExpValidator(LERegExp);
    responseNodes->setValidator(LEValidator);

    layout->addWidget(label2,1,0);
    layout->addWidget(responseNodes,1,1);

    QLabel *label3 = new QLabel();
    label3->setText("Analysis Spatial Dimensions:");
    ndm = new QLineEdit();
    ndm->setText("3");

    QRegExp LERegExp2 ("[1-3]");
    QRegExpValidator* LEValidator2 = new QRegExpValidator(LERegExp2);
    ndm->setValidator(LEValidator2);

    ndm->setMaximumWidth(50);
    layout->addWidget(label3,2,0);
    layout->addWidget(ndm,2,1);

    QLabel *label4 = new QLabel();
    label4->setText("Number Nodal DOF:");
    ndf = new QLineEdit();

    QRegExp LERegExp3 ("[1-9]");
    QRegExpValidator* LEValidator3 = new QRegExpValidator(LERegExp3);
    ndf->setValidator(LEValidator3);

    ndf->setText("6");
    ndf->setMaximumWidth(50);
    layout->addWidget(label4,3,0);
    layout->addWidget(ndf,3,1);


    columnLine = new QLineEdit();
    layout->addWidget(new QLabel("Column Line Nodes:"), 4, 0);
    layout->addWidget(columnLine);

    QWidget *dummyR = new QWidget();
    layout->addWidget(dummyR,4,0);
    layout->setRowStretch(5,1);

    this->setLayout(layout);
}


OpenSeesPyBuildingModel::~OpenSeesPyBuildingModel()
{
    // remove old random variables
    QStringList names;
    for (int i=0; i<varNamesAndValues.size()-1; i+=2) {
        names.append(varNamesAndValues.at(i));
    }

    theRandomVariablesContainer->removeRandomVariables(names);
}


void OpenSeesPyBuildingModel::clear(void)
{

}


bool OpenSeesPyBuildingModel::outputToJSON(QJsonObject &jsonObject)
{


    return true;
}


bool OpenSeesPyBuildingModel::inputFromJSON(QJsonObject &jsonObject)
{

    return true;
}


bool OpenSeesPyBuildingModel::outputAppDataToJSON(QJsonObject &jsonObject) {

    bool res = true;
   qDebug() << __PRETTY_FUNCTION__ ;
    //
    // per API, need to add name of application to be called in AppLication
    // and all data to be used in ApplicationDate
    //

    jsonObject["Application"] = "OpenSeesPyInput";
    QJsonObject dataObj;

    QString fileName = filePathLineEdit->text();
    QFileInfo fileInfo(fileName);


    dataObj["dofMap"]=responseNodes->text();
    dataObj["ndm"]=ndm->text().toInt();
    dataObj["modelPath"] = "";
    dataObj["columnLine"]=columnLine->text();

    QFileInfo theModelFile(fileName);
    if (theModelFile.exists()) {
        dataObj["mainScript"] = theModelFile.fileName();
        dataObj["filePath"] = theModelFile.absolutePath();
    } else {
        QString errorMessage("OpenSeesPyBuilding Model Generator - no modeling script set");
        emit sendErrorMessage(errorMessage);
        dataObj["mainScript"] = "None";
        dataObj["filePath"] = "";
        res = false;
    }

    jsonObject["ApplicationData"] = dataObj;
  qDebug() << __PRETTY_FUNCTION__ << dataObj;

    return res;
}


bool OpenSeesPyBuildingModel::inputAppDataFromJSON(QJsonObject &jsonObject) {

    if (jsonObject.contains("ApplicationData")) {
        QJsonObject appData = jsonObject["ApplicationData"].toObject();


        QFileInfo fileInfo;
        QString fileName;
        QString pathToFile;

        if (appData.contains("dofMap"))
            responseNodes->setText(appData["dofMap"].toString());
        if (appData.contains("ndm"))
            ndm->setText(QString::number(appData["ndm"].toInt()));
        if (appData.contains("columnLine"))
            columnLine->setText(appData["columnLine"].toString());

        //
        // mainScript file .. a number of options
        //  1 is user created input the full path can be specified
        //  2 if user specified, it can be relative to current dir
        //  3 if use specified it can be in input_Data
        //  4 if RDT created the path is seperate


        if (appData.contains("mainScript"))
            fileName = appData["mainScript"].toString();
        if (appData.contains("modelPath"))
              pathToFile = appData["modelPath"].toString();

        if (pathToFile != "")
            fileName = pathToFile + QDir::separator() + fileName;

        if (fileInfo.exists(fileName)) {
            filePathLineEdit->setText(fileName);
        } else  {

            if (appData.contains("filePath"))
                pathToFile = appData["filePath"].toString();
            else
                pathToFile = QDir::currentPath();

            QString fullFile = pathToFile + QDir::separator() + fileName;

            if (fileInfo.exists(fullFile)) {

                // option 2 or 4
                filePathLineEdit->setText(fullFile);
                return true;

            } else {

                // option 3
                // adam .. adam .. adam
                fullFile = pathToFile + QDir::separator()
                        + "input_data" + QDir::separator() + fileName;

                 qDebug() << __PRETTY_FUNCTION__ << "path1 " << fullFile;

                if (fileInfo.exists(fullFile)) {
                    filePathLineEdit->setText(fullFile);;
                    return true;
                }
            }
        }
        emit sendErrorMessage("OpenSeesPyBuilder could not find script file");
        return false;
    }

    emit sendErrorMessage("OpenSeesPyBuilder no ApplicationData");
    return true;
}


void OpenSeesPyBuildingModel::setFilename1(QString name1){

    // remove old random variables
    QStringList names;
    for (int i=0; i<varNamesAndValues.size()-1; i+=2) {
        names.append(varNamesAndValues.at(i));
    }

    theRandomVariablesContainer->removeRandomVariables(names);

    // set file name & ebtry in qLine edit

    filePath = name1;
    filePathLineEdit->setText(name1);

    //
    // parse file for random variables and add them
    //

    OpenSeesParser theParser;
    varNamesAndValues = theParser.getVariables(filePath);

    theRandomVariablesContainer->addConstantRVs(varNamesAndValues);

    return;
}


void OpenSeesPyBuildingModel::chooseFileName1(void) {
    filePath=QFileDialog::getOpenFileName(this,tr("Open File"),"C://", "Python Script (*.py)");
    this->setFilename1(filePath);
}


void OpenSeesPyBuildingModel::specialCopyMainInput(QString fileName, QStringList varNames) {
    // if OpenSees or FEAP parse the file for the variables
    if (varNames.size() > 0) {
        OpenSeesParser theParser;
        theParser.writeFile(filePathLineEdit->text(), fileName, varNames);
    }
}


QString OpenSeesPyBuildingModel::getMainInput() {
    return filePath;
}


bool OpenSeesPyBuildingModel::copyFiles(QString &dirName) {

    QString fileName = filePathLineEdit->text();
    QFileInfo fileInfo(fileName);

    if (fileInfo.exists()) {
        return this->copyFile(fileName, dirName);
    }

    return true;
}

