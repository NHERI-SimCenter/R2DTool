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
// Written: fmk, Sina Naeimi

#include "Pyrecodes.h"
#include <QScrollArea>
#include <QLineEdit>
#include <QTabWidget>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QJsonObject>
#include <QDoubleValidator>
#include "LineEditSelectTool.h"

#include <SC_QRadioButton.h>
#include <SC_DoubleLineEdit.h>
#include <SC_FileEdit.h>
#include <SC_IntLineEdit.h>
#include <SC_ComboBox.h>
#include <SC_CheckBox.h>
#include <SC_TableEdit.h>
#include <RewetResults.h>
#include <QGroupBox>
#include <QPushButton>

#include <Utils/ProgramOutputDialog.h>
#include <Utils/RelativePathResolver.h>
#include <Utils/FileOperations.h>
#include <GoogleAnalytics.h>
#include <SimCenterPreferences.h>
#include <RunPythonInThread.h>


Pyrecodes::Pyrecodes(QWidget *parent)
  : SimCenterAppWidget(parent), resultWidget(0)
{
    
    QGridLayout *mainLayout = new QGridLayout(this); // Set the parent to 'this'

    QWidget *mainWidget = new QWidget();
    
    QVBoxLayout *layout = new QVBoxLayout();
    mainWidget->setLayout(layout);


    int numRow = 0;

    systemConfigFile = new SC_FileEdit("systemConfigFile");
    mainLayout->addWidget(new QLabel("System Configuration File:"), numRow, 0, 1, 1);
    mainLayout->addWidget(systemConfigFile, numRow, 1, 1, 4);
    numRow++;
    
    componentLibraryFile = new SC_FileEdit("componentLibraryFile");
    mainLayout->addWidget(new QLabel("Component Library File:"), numRow, 0, 1, 1);
    mainLayout->addWidget(componentLibraryFile, numRow, 1, 1, 4);
    numRow++;

    QGroupBox *groupBox = new QGroupBox("Run Pyrecodes without Running a Workflow");
    QGridLayout *boxLayout = new QGridLayout(groupBox);
    
    inventoryFile = new SC_FileEdit("assetFile");
    damageFile = new SC_FileEdit("damageFileFile");
    boxLayout->addWidget(new QLabel("Asset Inventory File:"),0,0);
    boxLayout->addWidget(new QLabel("Asset Damage File:"), 1, 0);
    boxLayout->addWidget(inventoryFile, 0,1 ,1,4);
    boxLayout->addWidget(damageFile,    1,1, 1,4);

    QPushButton *runLocal = new QPushButton("Run PyReCodes No Workflow");
    boxLayout->addWidget(runLocal,2,1, 2,1);
    boxLayout->setRowStretch(3,1);
    
    connect(runLocal, &QPushButton::clicked, this, &Pyrecodes::runPyReCodes);

    QWidget *spacer = new QWidget();
    spacer->setFixedHeight(20); // Adjust the height as needed
    
    mainLayout->addWidget(spacer, numRow++, 0);
    
    mainLayout->addWidget(groupBox, numRow++, 0, 1,4);
    
    mainLayout->setRowStretch(numRow, 1);
    
}


Pyrecodes::~Pyrecodes()
{

}


void Pyrecodes::clear(void)
{
    if (resultWidget != nullptr) {
        resultWidget->clear();
    }
}


bool Pyrecodes::inputFromJSON(QJsonObject &jsonObject)
{
  Q_UNUSED(jsonObject); 

  return true;
}

bool Pyrecodes::outputToJSON(QJsonObject &jsonObject)
{
  jsonObject["Application"] = "Pyrecodes";
    
  return true;
}

bool Pyrecodes::outputAppDataToJSON(QJsonObject &jsonObject) {

    //
    // per API, need to add name of application to be called in AppLication
    // and all data to be used in ApplicationDate
    //

    jsonObject["Application"] = "Pyrecodes";
    QJsonObject dataObj;
    systemConfigFile->outputToJSON(dataObj);
    componentLibraryFile->outputToJSON(dataObj);
    jsonObject["ApplicationData"] = dataObj;

    return true;
}

bool Pyrecodes::inputAppDataFromJSON(QJsonObject &jsonObject) {
    this->clear();

    if (jsonObject.contains("ApplicationData")) {
        QJsonObject dataObj = jsonObject["ApplicationData"].toObject();
        systemConfigFile->inputFromJSON(dataObj);
        componentLibraryFile->inputFromJSON(dataObj);
    }

    return true;
}


bool Pyrecodes::copyFiles(QString &destDir) {

  systemConfigFile->copyFile(destDir);
  componentLibraryFile->copyFile(destDir);

  return true;

}

bool Pyrecodes::outputCitation(QJsonObject &citation){
  QString pyrecodesCitationKey = "PyReCoDes";
  QJsonValue pyrecodesCitationValue( "\"citations\": [{\"citation\": \"Add\"}]}");
  citation.insert(pyrecodesCitationKey, pyrecodesCitationValue);
  
  return true;
}

SC_ResultsWidget* Pyrecodes::getResultsWidget(QWidget *parent, QWidget *R2DresWidget, QMap<QString, QList<QString>> assetTypeToType)
{
    if (resultWidget==nullptr){
        resultWidget = new RewetResults(parent);
    }
    
    return resultWidget;
}

void Pyrecodes::runPyReCodes() {


  //
  // create a workdir in LocalApplications folder
  //

  QDir localWorkDir(SimCenterPreferences::getInstance()->getLocalWorkDir());
  QString workDirString = localWorkDir.absoluteFilePath("PyReCodes");
  QDir workDir(workDirString);

  if(workDir.exists()) {
    if (SCUtils::isSafeToRemoveRecursivily(workDirString))
      workDir.removeRecursively();
    else {
      QString msg("The Program stopped, Running PyReCodes locally was about to recursivily remove: ");
      msg.append(workDirString);
      fatalMessage(msg);
      return;	
    }
  }
  localWorkDir.mkpath("PyReCodes");

  qDebug() << "workdirString: " << workDirString;
  
  //
  // now copy all files there
  //  

  systemConfigFile->copyFile(workDirString);
  componentLibraryFile->copyFile(workDirString);
  
  inventoryFile->copyFile(workDirString);
  damageFile->copyFile(workDirString);

  //
  // now run the python command
  //

  QStringList args;
  QString script = "hello.py";
  RunPythonInThread *thePythonProcess = new RunPythonInThread(script, args, workDirString);
  connect(thePythonProcess, &RunPythonInThread::processFinished, this, &Pyrecodes::runDone);

  thePythonProcess->runProcess();
  qDebug() << "STARTED PYTHON";  
  
}

void
Pyrecodes::runDone(int error) {
  qDebug() << "FINISHED PYTHON" << error;
}
