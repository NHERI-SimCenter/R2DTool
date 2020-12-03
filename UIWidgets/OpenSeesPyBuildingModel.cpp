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

// Written: fmckenna

#include "OpenSeesPyBuildingModel.h"
#include <QPushButton>
#include <QScrollArea>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QDebug>
#include <QFileDialog>
#include <QPushButton>
#include <sectiontitle.h>
#include <QFileInfo>
#include <string>
#include <sstream>
#include <iostream>
using namespace std;
#include <QGridLayout>

#include <OpenSeesParser.h>
#include <RandomVariablesContainer.h>

//#include <InputWidgetParameters.h>

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
    label2->setText("Response Nodes:");
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
    label4->setText("Number of degrees-of-freedom at nodes:");
    ndf = new QLineEdit();

    QRegExp LERegExp3 ("[1-9]");
    QRegExpValidator* LEValidator3 = new QRegExpValidator(LERegExp3);
    ndf->setValidator(LEValidator3);

    ndf->setText("6");
    ndf->setMaximumWidth(50);
    layout->addWidget(label4,3,0);
    layout->addWidget(ndf,3,1);

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



bool
OpenSeesPyBuildingModel::outputToJSON(QJsonObject &jsonObject)
{
    filePath = "/Users/steve/Desktop/SimCenter/Examples/rWhaleExample/input_data_rdt/model/cantilever.py";

    // just need to send the class type here.. type needed in object in case user screws up
    jsonObject["Application"]="OpenSeesPyInput";

    QJsonObject appData;

    appData["dofMap"]=responseNodes->text();
    appData["ndm"]=ndm->text().toInt();

    QFileInfo modelPath(filePath);

    appData["mainScript"] = modelPath.fileName();
    appData["modelPath"] = modelPath.absolutePath() +"/";

//    QJsonArray rvArray;
//    for (int i=0; i<varNamesAndValues.size()-1; i+=2) {
//        QJsonObject rvObject;
//        QString name = varNamesAndValues.at(i);
//        rvObject["name"]=name;
//        rvObject["value"]=QString("RV.")+name;
//        rvArray.append(rvObject);
//    }

//    appData["randomVar"]=rvArray;

    jsonObject["ApplicationData"] = appData;

    return true;
}


bool
OpenSeesPyBuildingModel::inputFromJSON(QJsonObject &jsonObject)
{
    varNamesAndValues.clear();

    this->clear();

    if (jsonObject.contains("responseNodes")) {
        QString stringResponseNodes;
        QJsonArray nodeResponseTags = jsonObject["responseNodes"].toArray();
        foreach (const QJsonValue & value, nodeResponseTags) {
            int tag = value.toInt();
            stringResponseNodes = stringResponseNodes + " " +  QString::number(tag);
        }
        responseNodes->setText(stringResponseNodes);
    }

    // backward compatability .. response nodes used to be nodes
    if (jsonObject.contains("nodes")) {
        QString stringResponseNodes;
        QJsonArray nodeResponseTags = jsonObject["nodes"].toArray();
        foreach (const QJsonValue & value, nodeResponseTags) {
            int tag = value.toInt();
            stringResponseNodes = stringResponseNodes + " " +  QString::number(tag);
        }
        responseNodes->setText(stringResponseNodes);
    }

    if (jsonObject.contains("randomVar")) {
        QJsonArray randomVars = jsonObject["randomVar"].toArray();
        foreach (const QJsonValue & value, randomVars) {
            QJsonObject theRV = value.toObject();
            QString name = theRV["name"].toString();
            QString zero = "0";
            varNamesAndValues.append(name);
            varNamesAndValues.append(zero);
        }
    }

    int theNDM = jsonObject["ndm"].toInt();
    int theNDF = 1;
    if (theNDM == 2)
        theNDF = 3;
    else if (theNDM == 3)
        theNDF = 6;

    if (jsonObject.contains("ndf")) {
        theNDF = jsonObject["ndf"].toInt();
    }

    ndm->setText(QString::number(theNDM));
    ndf->setText(QString::number(theNDF));

    return true;
}


bool
OpenSeesPyBuildingModel::outputAppDataToJSON(QJsonObject &jsonObject) {

    //
    // per API, need to add name of application to be called in AppLication
    // and all data to be used in ApplicationDate
    //

    jsonObject["Application"] = "OpenSeesPyInput";
    QJsonObject dataObj;
    QString fileName = filePathLineEdit->text();
    QFileInfo fileInfo(fileName);

    dataObj["fileName"]= fileInfo.fileName();
    dataObj["filePath"]=fileInfo.path();

    jsonObject["ApplicationData"] = dataObj;

    return true;
}


bool
OpenSeesPyBuildingModel::inputAppDataFromJSON(QJsonObject &jsonObject) {

    //
    // from ApplicationData
    //

    if (jsonObject.contains("ApplicationData")) {
        QJsonObject dataObject = jsonObject["ApplicationData"].toObject();

        //
        // retrieve filename and path, set the QLIne Edit
        //

        QString fileName;
        QString filePath;

        if (dataObject.contains("fileName")) {
            QJsonValue theName = dataObject["fileName"];
            fileName = theName.toString();
        } else
            return false;

        if (dataObject.contains("filePath")) {
            QJsonValue theName = dataObject["filePath"];
            filePath = theName.toString();
        } else
            return false;

        filePathLineEdit->setText(QDir(filePath).filePath(fileName));

    } else {
        return false;
    }
    return true;
}


void
OpenSeesPyBuildingModel::setFilename1(QString name1){

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


void
OpenSeesPyBuildingModel::chooseFileName1(void) {
    filePath=QFileDialog::getOpenFileName(this,tr("Open File"),"C://", "Python Script (*.py)");
    this->setFilename1(filePath);
}


void
OpenSeesPyBuildingModel::specialCopyMainInput(QString fileName, QStringList varNames) {
    // if OpenSees or FEAP parse the file for the variables
    if (varNames.size() > 0) {
        OpenSeesParser theParser;
        theParser.writeFile(filePathLineEdit->text(), fileName, varNames);
    }
}


QString OpenSeesPyBuildingModel::getMainInput() {
    return filePath;
}


bool
OpenSeesPyBuildingModel::copyFiles(QString &dirName) {

    QString fileName = filePathLineEdit->text();

    if (fileName.isEmpty()) {
        emit sendErrorMessage("OpenSeesInput - no file set");
        return false;
    }
    QFileInfo fileInfo(fileName);

    QString theFile = fileInfo.fileName();
    QString thePath = fileInfo.path();

    SimCenterAppWidget::copyPath(thePath, dirName, false);

    QStringList varNames = theRandomVariablesContainer->getRandomVariableNames();

    // now create special copy of original main script that handles the RV
    OpenSeesParser theParser;
    QString copiedFile = dirName + QDir::separator() + theFile;
    theParser.writeFile(fileName, copiedFile, varNames);

    return true;
}

