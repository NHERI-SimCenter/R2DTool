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

#include "SurrogatePyFilter.h"
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
#include "SC_FileEdit.h"
#include "RandomVariablesContainer.h"
#include "SurrogateGP.h"
#include "SimCenterAppMulti.h"
#include "ModelWidget.h"
#include "SimCenterAppSelection.h"
#include "InputWidgetOpenSeesAnalysis.h"
#include "InputWidgetOpenSeesPyAnalysis.h"
#include "NoArgSimCenterApp.h"
#include "NoneWidget.h"

SurrogateFile::SurrogateFile(QWidget *parent)
:SimCenterWidget(parent)
{
   QHBoxLayout *layout = new QHBoxLayout();
   button = new QRadioButton();

   QLabel *labelFile = new QLabel();
   labelFile->setText("File Path");
   theFilePath = new SC_FileEdit("fileName");
   connect(theFilePath,SIGNAL(fileNameChanged(QString)),this,SLOT(onFilePathChanged(QString)));

   //theFilePath = new SC_StringLineEdit("fileName");
   //QPushButton *chooseFile = new QPushButton();
   //chooseFile->setText(tr("Choose"));
   //connect(chooseFile,SIGNAL(clicked()),this,SLOT(chooseFileName()));

   theModelName = new SC_StringLineEdit("modelName");
   theModelName->setText(QString(""));

   layout->addWidget(button);
   layout->addWidget(labelFile);
   layout->addWidget(theFilePath,1);
   //layout->addWidget(chooseFile);
   layout->addWidget(new QLabel("Model Name"));
   layout->addWidget(theModelName);

  // layout->addStretch();
   this->setLayout(layout);
   connect(button,SIGNAL(clicked(bool)),this,SLOT(onRemoveMod(bool)));
}

SurrogateFile::~SurrogateFile()
{

}

void
SurrogateFile::onFilePathChanged(QString newPath) {
    QFileInfo fileInfo(newPath);
    QString fileName = fileInfo.fileName();
    fileName.chop(5); // remove .json
    theModelName->setText(fileName);
}

//void
//SurrogateFile::chooseFileName(void) {
//    QString filePath=QFileDialog::getOpenFileName(this,tr("Open File"),"", "All files (*.*)");
//    if(!filePath.isEmpty()) {
//        QFileInfo fileInfo(filePath);
//        theFilePath->setFilename(filePath);
//        QString fileName = fileInfo.fileName();
//        fileName.chop(5); // remove .AT2
//        theModelName->setText(fileName);
//    }
//}


void
SurrogateFile::onRemoveMod(bool value) {
    if (value == true)
        emit removeRecord();
    return;
}

bool
SurrogateFile::outputToJSON(QJsonObject &jsonObject) {
    QFileInfo fileInfo(theFilePath->getFilename());
    jsonObject["fileName"]= fileInfo.fileName();
    jsonObject["filePath"]=fileInfo.path();
    jsonObject["modelName"]=theModelName->text();

    return true;
}

bool
SurrogateFile::inputFromJSON(QJsonObject &jsonObject) {

    QString fileName;
    QString filePath;

    if (jsonObject.contains("fileName")) {
        QJsonValue theValue = jsonObject["fileName"];
        fileName = theValue.toString();
    } else
        return false;

    if (jsonObject.contains("filePath")) {
        QJsonValue theValue = jsonObject["filePath"];
        filePath = theValue.toString();
    } else
        return false;

    QString fullFilePath = filePath + QDir::separator() + fileName;
    theFilePath->setFilename(fullFilePath);

    if (jsonObject.contains("modelName")) {
        QJsonValue theValue = jsonObject["modelName"];
        theModelName->setText(theValue.toString());
    } else
        return false;

    return true;
}

QString
SurrogateFile::getFilePath(void) {
    return theFilePath->getFilename();
}

SurrogatePyFilter::SurrogatePyFilter(QWidget *parent)
    : SimCenterAppWidget(parent)
{
    QTabWidget *tabWidget= new QTabWidget();

    QGridLayout *filterLayout = new QGridLayout();
    filterPath = new SC_FileEdit("filterScript");
    filterPath->setMinimumWidth(600);
    filterLayout->addWidget(new QLabel("Filter script (.py)"),0,0);
    filterLayout->addWidget(filterPath,0,1);

    //
    // Surrogates
    //

    QGroupBox* surGroupBox = new QGroupBox("List of Surrogate model files (.json)");
    surGroupBox->setContentsMargins(0,5,0,0);

    verticalLayout = new QVBoxLayout();

    // title, add & remove button
    QHBoxLayout *titleLayout = new QHBoxLayout();

    QPushButton *addEvent = new QPushButton();
    addEvent->setMinimumWidth(75);
    addEvent->setMaximumWidth(75);
    addEvent->setText(tr("Add"));

    QPushButton *removeEvent = new QPushButton();
    removeEvent->setMinimumWidth(75);
    removeEvent->setMaximumWidth(75);
    removeEvent->setText(tr("Remove"));

    QPushButton *removeAllEvent = new QPushButton();
    removeAllEvent->setMinimumWidth(100);
    removeAllEvent->setMaximumWidth(100);
    removeAllEvent->setText(tr("Remove All"));

    QPushButton *loadDirectory = new QPushButton();
    loadDirectory->setMinimumWidth(150);
    loadDirectory->setMaximumWidth(150);
    loadDirectory->setText(tr("Load Directory"));

    titleLayout->addWidget(addEvent);
    titleLayout->addSpacing(20);
    titleLayout->addWidget(removeEvent);
    titleLayout->addSpacing(20);
    titleLayout->addWidget(removeAllEvent);
    titleLayout->addSpacing(50);
    titleLayout->addWidget(loadDirectory);
    titleLayout->addStretch();

    QScrollArea *sa = new QScrollArea;
    sa->setWidgetResizable(true);
    sa->setLineWidth(0);
    sa->setFrameShape(QFrame::NoFrame);

    QWidget *eventsWidget = new QWidget;

    surLayout = new QVBoxLayout();
    surLayout->addStretch();
    eventsWidget->setLayout(surLayout);
    sa->setWidget(eventsWidget);


    verticalLayout->addLayout(titleLayout);
    verticalLayout->addWidget(sa);
    //verticalLayout->addStretch();

    surGroupBox->setLayout(verticalLayout);


    connect(addEvent, SIGNAL(pressed()), this, SLOT(addMod()));
    connect(removeEvent, SIGNAL(pressed()), this, SLOT(removeMod()));
    connect(removeAllEvent, SIGNAL(pressed()), this, SLOT(removeAllMods()));
    connect(loadDirectory, SIGNAL(pressed()), this, SLOT(loadModsFromDir()));

    filterLayout->addWidget(surGroupBox,1,0,1,-1);
    filterLayout->setRowStretch(1,1);
    filterLayout->setColumnStretch(3,1);

    QWidget *placeholderWidget = new QWidget();
    placeholderWidget->setLayout(filterLayout);
    tabWidget->addTab(placeholderWidget,"Surrogate Models");

    //
    // Default option
    //
    buildingWidget = new SimCenterAppSelection(QString("Building Analysis Method"), QString("Buildings"), this);

    // Building widget apps
    SimCenterAppWidget *openSeesPy = new InputWidgetOpenSeesPyAnalysis(this);
    InputWidgetOpenSeesAnalysis *openSees = new InputWidgetOpenSeesAnalysis();
    SimCenterAppWidget *imAsEDP = new NoArgSimCenterApp(QString("IMasEDP"));
    SimCenterAppWidget *noneWidget = new NoneWidget(this);

    buildingWidget->addComponent(QString("OpenSees"), QString("OpenSees-Simulation"), openSees);
    buildingWidget->addComponent(QString("OpenSeesPy"), QString("OpenSeesPy-Simulation"), openSeesPy);
    buildingWidget->addComponent(QString("IMasEDP"), QString("IMasEDP"), imAsEDP);
    buildingWidget->addComponent(QString("None"), QString("None"), noneWidget);

    tabWidget->addTab(buildingWidget,"Default Analysis");


    QVBoxLayout * placeholder = new QVBoxLayout();
    placeholder->addWidget(tabWidget);
    this->setLayout(placeholder);
}


SurrogatePyFilter::~SurrogatePyFilter()
{
}


void SurrogatePyFilter::clear(void)
{
    //filePath.clear();
}


bool SurrogatePyFilter::outputToJSON(QJsonObject &jsonObject)
{

    QJsonObject dataObj;
    QFileInfo fileInfo(filterPath->getFilename());
    jsonObject["filterFileName"]= fileInfo.fileName();
    jsonObject["filterFilePath"]=fileInfo.path();

    bool result = true;
    QJsonArray theArray;
    for (int i = 0; i <theModels.size(); ++i) {
        QJsonObject rv;
        if (theModels.at(i)->outputToJSON(rv)) {
            theArray.append(rv);
        } else {
            //qDebug() << "OUTPUT FAILED" << theModels.at(i)->theName->text();
            result = false;
        }
    }
    jsonObject["Models"]=theArray;

    QJsonObject defaultObj;
    buildingWidget->outputToJSON(defaultObj);
    jsonObject["DefaultAnalysis"]=defaultObj;
    return result;
}


bool SurrogatePyFilter::inputFromJSON(QJsonObject &jsonObject)
{

    bool result = true;

    // clean out current list
    this->clear();
    QString filterFilePath = jsonObject["filterFilePath"].toString();
    QString filterFileName = jsonObject["filterFileName"].toString();
    auto  myPath = filterFilePath + QDir::separator() + filterFileName;
    filterPath->setFilename(myPath);
    // get array
    if (jsonObject.contains("Models")) {
        if (jsonObject["Models"].isArray()) {

            QJsonArray modelArray = jsonObject["Models"].toArray();

            // foreach object in array
            foreach (const QJsonValue &modelValue, modelArray) {

                // get data, create an event, read it and then add to layout

                QJsonObject modelObject = modelValue.toObject();
                SurrogateFile *theModel = new SurrogateFile();

                if (theModel->inputFromJSON(modelObject)) { // this method is where type is set
                    theModels.append(theModel);
                    surLayout->insertWidget(surLayout->count()-1, theModel);
                } else {
                    delete theModel;
                    result = false;
                }
            }
        }
    }

    auto defaultObject = jsonObject["DefaultAnalysis"].toObject();
    buildingWidget->inputFromJSON(defaultObject);

    return result;

}


bool SurrogatePyFilter::outputAppDataToJSON(QJsonObject &jsonObject) {

    bool res = true;

    //
    // per API, need to add name of application to be called in AppLication
    // and all data to be used in ApplicationDate
    //

    jsonObject["Application"] = "SurrogateRegionalPy";


    QJsonObject dataObj;
    QFileInfo fileInfo(filterPath->getFilename());
    dataObj["fileName"]= fileInfo.fileName();
    dataObj["filePath"]=fileInfo.path();
    jsonObject["ApplicationData"] = dataObj;

    return 1;

}


bool SurrogatePyFilter::inputAppDataFromJSON(QJsonObject &jsonObject) {

    if (jsonObject.contains("ApplicationData")) {
        QJsonObject appData = jsonObject["ApplicationData"].toObject();
        filterPath->outputToJSON(appData);
    }
    return true;
}


bool SurrogatePyFilter::copyFiles(QString &dirName) {

    QString fileName = filterPath->getFilename();
    QFileInfo fileInfo(fileName);

    if (fileInfo.exists()) {
        this->copyFile(fileName, dirName);
    } else {
        return false;
    }

    //enumerate files
    int numInputWidgetExistingEvents = theModels.size();
    for (int i = 0; i < numInputWidgetExistingEvents; i++) {
      SurrogateFile *theModel = theModels.at(i);
      auto a = theModel->getFilePath();
      this->copyFile(theModel->getFilePath(), dirName);
    }

    return true;
}


void SurrogatePyFilter::addMod(void)
{
   SurrogateFile *theModel = new SurrogateFile();
   theModels.append(theModel);
   surLayout->insertWidget(surLayout->count()-1, theModel);
   //connect(this,SLOT(InputWidgetExistingEventErrorMessage(QString)), theModel, SIGNAL(sendErrorMessage(QString)));
}

void SurrogatePyFilter::removeMod(void)
{
    // find the ones selected & remove them
    int numInputWidgetExistingEvents = theModels.size();
    for (int i = numInputWidgetExistingEvents-1; i >= 0; i--) {
      SurrogateFile *theModel = theModels.at(i);
      if (theModel->button->isChecked()) {
          theModel->close();
          surLayout->removeWidget(theModel);
          theModels.remove(i);
          theModel->setParent(0);
          delete theModel;
      }
    }
}


void SurrogatePyFilter::loadModsFromDir(void) {
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                 "",
                                                 QFileDialog::ShowDirsOnly
                                                 | QFileDialog::DontResolveSymlinks);

    this->clear();

    QDir directory(dir);
    QStringList fileList= directory.entryList(QStringList() << "*.json",QDir::Files);
    foreach(QString fileName, fileList) {

        SurrogateFile *theModel = new SurrogateFile();
        QString name = fileName;
        name.chop(5); // remove .AT2
        theModel->theModelName->setText(name);

        if (theModel != NULL) {
            QString filePathStr = directory.filePath(fileName);
            theModel->theFilePath->setFilename(filePathStr);
        }
        theModels.append(theModel);
        surLayout->insertWidget(surLayout->count()-1, theModel);
    }}

void SurrogatePyFilter::removeAllMods(void)
{
    // find the ones selected & remove them
    int numInputWidgetExistingEvents = theModels.size();
    for (int i = numInputWidgetExistingEvents-1; i >= 0; i--) {
      SurrogateFile *theModel = theModels.at(i);
      theModel->close();
      surLayout->removeWidget(theModel);
      theModels.remove(i);
      theModel->setParent(0);
      delete theModel;
    }
}
