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
#include "PyrecodesResults.h"

#include <QGuiApplication>
#include <QScreen>
//#include <QDialog>
#include <QTabWidget>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QJsonObject>
#include <QDoubleValidator>
#include "LineEditSelectTool.h"
#include <QTabWidget>

#include <SC_QRadioButton.h>
#include <SC_DoubleLineEdit.h>
#include <SC_FileEdit.h>
#include <SC_DirEdit.h>
#include <SC_IntLineEdit.h>
#include <SC_ComboBox.h>
#include <SC_CheckBox.h>
#include <SC_TableEdit.h>
#include <SC_AssetInputDelegate.h>
#include <QGroupBox>
#include <QPushButton>

#include <Utils/ProgramOutputDialog.h>
#include <Utils/RelativePathResolver.h>
#include <Utils/FileOperations.h>
#include <GoogleAnalytics.h>
#include <SimCenterPreferences.h>
#include <RunPythonInThread.h>

#include <QCoreApplication>
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>


static QString citeText = "Blagojević, Nikola and Stojadinović, Božidar. pyrecodes: an open-source library for regional recovery simulation and disaster resilience assessment of the built environment. (2023) Chair of Structural Dynamics and Earthquake Engineering, ETH Zurich.  https://doi.org/10.5905/ethz-1007-700";


Pyrecodes::Pyrecodes(QWidget *parent)
: SimCenterAppWidget(parent), resultsWidget(0)
{
  //    QWidget     *inputWidget = new QWidget();  
  //    QGridLayout *inputLayout = new QGridLayout(inputWidget); // Set the parent to 'this'
  QGridLayout *inputLayout = new QGridLayout(this); // Set the parent to 'this'  

    int numRow = 0;

    mainFile = new SC_FileEdit("mainFile");
    inputLayout->addWidget(new QLabel("Main File:"), numRow, 0, 1, 1);
    inputLayout->addWidget(mainFile, numRow, 1, 1, 4);
    numRow++;

    connect(mainFile, &SC_FileEdit::fileNameChanged, this, &Pyrecodes::parseMainFile);
    
    systemConfigFile = new SC_FileEdit("SystemConfigurationFile");
    inputLayout->addWidget(new QLabel("System Configuration File:"), numRow, 0, 1, 1);
    inputLayout->addWidget(systemConfigFile, numRow, 1, 1, 4);
    numRow++;
    
    componentLibraryFile = new SC_FileEdit("ComponentLibraryFile");
    inputLayout->addWidget(new QLabel("Component Library File:"), numRow, 0, 1, 1);
    inputLayout->addWidget(componentLibraryFile, numRow, 1, 1, 4);
    numRow++;

    inputLayout->addWidget(new QLabel("Realizations to Analyze:"), numRow, 0, 1, 1);
    realizationEntriesToRun = new SC_AssetInputDelegate("Filter");
    inputLayout->addWidget(realizationEntriesToRun, numRow, 1, 1, 2);
    numRow++;			  
    
    QGroupBox *groupBox = new QGroupBox("Run Pyrecodes without Running a Workflow");
    QGridLayout *boxLayout = new QGridLayout(groupBox);
    
    boxLayout->addWidget(new QLabel("Input Data Folder:"), 0, 0);
    boxLayout->addWidget(new QLabel("R2D Results Folder:"),1,0);
    r2dResultsFolder = new SC_DirEdit("r2dRunDir");
    inputDataFolder = new SC_DirEdit("inputDataDir");
    boxLayout->addWidget(inputDataFolder,  0,1, 1,4);    
    boxLayout->addWidget(r2dResultsFolder, 1,1 ,1,4);

    QHBoxLayout *runLayout = new QHBoxLayout();

    runLayout->addStretch();
    runLocal = new QPushButton("Launch pyrecodes without Workflow");
    QPushButton *showResults = new QPushButton("Show Results");
    
    runLayout->addWidget(runLocal);
    runLayout->addWidget(showResults);    
    runLayout->addStretch();    
    //boxLayout->addWidget(runLocal,2,0, 1,5);
    boxLayout->addLayout(runLayout,2,0,1,5);
    boxLayout->setRowStretch(3,1);
    
    connect(runLocal, &QPushButton::clicked, this, &Pyrecodes::runPyrecodes);
    connect(showResults, &QPushButton::clicked, this, [=]() {
	popupResultsDialog->show();	
      });

    QWidget *spacer = new QWidget();
    spacer->setFixedHeight(20); // Adjust the height as needed
    
    inputLayout->addWidget(spacer, numRow++, 0);
    inputLayout->addWidget(groupBox, numRow++, 0, 1, 5);
    
    QLabel *citeLabel = new QLabel(citeText);
    citeLabel->setWordWrap(true);
    inputLayout->addWidget(citeLabel, numRow++,0, 1, 5);
    
    inputLayout->setRowStretch(numRow, 1);
    
    //
    // create a PyrecodesResults widget and a popup Dialog for showing results if in Dialog
    //    - trying something different
    
    theResultsWidget = new PyrecodesResults(this, false);

    popupResultsDialog = new QDialog(nullptr, Qt::Window);
    popupResultsDialog->setWindowTitle("pyrecodes Results Window");
    QGridLayout *popupResultDialogLayout = new QGridLayout();
    popupResultDialogLayout->addWidget(theResultsWidget, 0,0);
    popupResultsDialog->setLayout(popupResultDialogLayout);
    QRect rec = QGuiApplication::primaryScreen()->geometry();
    int height = this->height()<int(rec.height())?int(rec.height()):this->height();
    int width  = this->width()<int(rec.width())?int(rec.width()):this->width();
    height = abs(0.75*height);
    width = abs(0.75*width);
    popupResultsDialog->resize(width,height);

    /* ** to stop growing when displaying gif **************************************
    popupResultsDialog->setMinimumWidth(width);
    popupResultsDialog->setMaximumWidth(width);
    popupResultsDialog->setMinimumHeight(height);
    popupResultsDialog->setMaximumHeight(height);    
    popupResultsDialog->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);    
    popupResultsDialog->resize(width, height);
    ******************************************************************************* */

    /* ** for when putting INputs and Results in a Tabbed widget *******************
    QVBoxLayout *layout = new QVBoxLayout(this);
    theTabbedWidget = new QTabWidget();
    theTabbedWidget->addTab(inputWidget, "          Inputs          ");
    theTabbedWidget->addTab(theResultsWidget, "          Results         ");    
    theTabbedWidget->tabBar()->hide();

    layout->addWidget(theTabbedWidget);
    **********************************************************************************/
    
}


Pyrecodes::~Pyrecodes()
{

}


void Pyrecodes::clear(void)
{
    if (theResultsWidget != nullptr) {
        theResultsWidget->clear();
    }
    if (resultsWidget != nullptr) {
        resultsWidget->clear();
    }
  QString empty;
  resultsDir = QString("");
  if (mainFile != nullptr) {
    mainFile->setFilename(empty);
  }
  if (systemConfigFile != nullptr) {
    systemConfigFile->setFilename(empty);
  }
  if (componentLibraryFile != nullptr) {
    componentLibraryFile->setFilename(empty);
  }
  realizationEntriesToRun->clear();
  if (r2dResultsFolder != nullptr) {
    r2dResultsFolder->setDirName(empty);
  }
  if (inputDataFolder != nullptr) {
    inputDataFolder->setDirName(empty);
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
    mainFile->outputToJSON(dataObj);    
    systemConfigFile->outputToJSON(dataObj);
    componentLibraryFile->outputToJSON(dataObj);
    realizationEntriesToRun->outputToJSON(dataObj);
    jsonObject["ApplicationData"] = dataObj;

    return true;
}

bool Pyrecodes::inputAppDataFromJSON(QJsonObject &jsonObject) {
    this->clear();

    if (jsonObject.contains("ApplicationData")) {
        QJsonObject dataObj = jsonObject["ApplicationData"].toObject();
	mainFile->inputFromJSON(dataObj);
        systemConfigFile->inputFromJSON(dataObj);
        componentLibraryFile->inputFromJSON(dataObj);
	realizationEntriesToRun->inputFromJSON(dataObj);	
    }

    return true;
}

bool Pyrecodes::copyFiles(QString &destDir) {

  // copy main file and all files in the dir
  QFileInfo mainF = QFile(mainFile->getFilename());
  if (mainF.exists()) {
    SimCenterAppWidget::copyPath(mainF.absolutePath(), destDir, false);
  }

  QString filename = mainF.fileName();
  QString newMainFile = destDir + QDir::separator() + filename;

  this->writeMainFile(newMainFile);
  
  // copy systemconfig & components library file
  systemConfigFile->copyFile(destDir);
  componentLibraryFile->copyFile(destDir);

  // open up system config file and copy some extra files that user has provided
  
  return true;

}

  
bool Pyrecodes::outputCitation(QJsonObject &citation){
  citation.insert("Recovery",citeText);
  
  return true;
}

SC_ResultsWidget* Pyrecodes::getResultsWidget(QWidget *parent, QWidget *R2DresWidget, QMap<QString, QList<QString>> assetTypeToType)
{
  if (resultsWidget == 0)
    resultsWidget = new PyrecodesResults(this, true);

  return resultsWidget;
}


SC_ResultsWidget* Pyrecodes::getResultsWidget(QWidget *parent)
{
  if (resultsWidget==nullptr){
    resultsWidget = new PyrecodesResults(parent);
  }
  
  return resultsWidget;
}

void Pyrecodes::runPyrecodes() {

  //
  // create a workdir in LocalApplications folder named pyrecodes
  //

  QDir localWorkDir(SimCenterPreferences::getInstance()->getLocalWorkDir());
  QString workDirString = localWorkDir.absoluteFilePath("pyrecodes");
  QDir workDir(workDirString);

  if(workDir.exists()) {
    if (SCUtils::isSafeToRemoveRecursivily(workDirString))
      workDir.removeRecursively();
    else {
      QString msg("The Program stopped, Running pyrecodes locally was about to recursivily remove: ");
      msg.append(workDirString);      
      fatalMessage(msg);
      return;	
    }
  }

  localWorkDir.mkpath("pyrecodes");

  //
  // in that directory create input_data and results folders
  //
  
  QDir finalWorkDir(workDirString);
  finalWorkDir.mkdir("input_data");
  finalWorkDir.mkdir("results");      
  QString inputDataDir = finalWorkDir.absoluteFilePath("input_data");
  resultsDir = finalWorkDir.absoluteFilePath("results");  
  
  //
  // now copy all files there
  //  

  inputDataFolder->copyFile(inputDataDir);  
  r2dResultsFolder->copyFile(resultsDir);
  this->copyFiles(inputDataDir);

  //
  // now run the python command
  //

  // need to create names for mainFile componentLibrartyFile and systemConfigFile in new dir
  QString mainFileNew = inputDataDir + QDir::separator() + mainFile->getName();
  QString configFileNew = inputDataDir + QDir::separator() + systemConfigFile->getName();
  QString componentFileNew = inputDataDir + QDir::separator() + componentLibraryFile->getName(); 

  // set up args to script
  QStringList args;
  args << "--mainFile" << mainFileNew
       << "--SystemConfigurationFile" << configFileNew
       << "--ComponentLibraryFile" << componentFileNew
       << "--r2dRunDir" << resultsDir
       << "--inputDataDir" << inputDataDir;

  // set python script to run
  QString appDir = SimCenterPreferences::getInstance()->getAppDir();
  QString pyScript = appDir + QDir::separator() + "applications" + QDir::separator() +
    "performREC" + QDir::separator() + "pyrecodes" + QDir::separator() + "run_pyrecodes.py";  

  qDebug() << "SCRIPT: " << pyScript << " ARGS: " << args << " workDir: " << workDirString;

  runLocal->setText("pyrecodes is now Running");
  runLocal->setDisabled(true);

  this->statusMessage("pyrecodes is now running in the background ..");
  
  // finally run, connect when done
  RunPythonInThread *thePythonProcess = new RunPythonInThread(pyScript, args, workDirString);
  connect(thePythonProcess, &RunPythonInThread::processFinished, this, &Pyrecodes::runDone);

  thePythonProcess->runProcess();
}

void
Pyrecodes::runDone(int error) {
  runLocal->setText("Launch pyrecodes without Workflow");
  runLocal->setDisabled(false);
  QString blank;
  theResultsWidget->processResults(blank, resultsDir);
  /* TABBED
  theTabbedWidget->tabBar()->show();
  theTabbedWidget->setCurrentIndex(1);
  */
   popupResultsDialog->show();
}

void
Pyrecodes::parseMainFile(QString filename) {

  //
  // Open the file in read-only mode & get JSON object
  //
  
  QFile file(filename);
  if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
    return;
  }

  QFileInfo fileInfo(filename);
  QDir fileDir = fileInfo.dir();
  QString filePath = fileInfo.path();

  if (!fileDir.exists()) {
    errorMessage("Pyrecodes: mainFile directory must exist and must contain pyrecodes files");
    return;
  }
  
  // Read the file contents
  QByteArray fileData = file.readAll();
  
  // Parse the JSON document
  QJsonDocument jsonDoc = QJsonDocument::fromJson(fileData);
  if (jsonDoc.isNull() || !jsonDoc.isObject()) { 
    errorMessage("Pyrecodes: file specified is not in JSON format");
    return;
  }
  QJsonObject data = jsonDoc.object();
  
  //
  // given the JSON object data, go see if it has the 2 key value pairs we need
  //   - if there, read and set the filename if in current dir of mainFile
  //

  if (data.contains("ComponentLibrary") && data["ComponentLibrary"].isObject()) {
    QJsonObject compData = data["ComponentLibrary"].toObject();
    if (compData.contains("ComponentLibraryFile") && compData["ComponentLibraryFile"].isString()) {
      QString compDataFile = compData["ComponentLibraryFile"].toString();
      if (fileDir.exists(compDataFile)) {
	QString fPath = fileDir.absoluteFilePath(compDataFile);
	componentLibraryFile->setFilename(fPath);
      }
    } 
  } 
  
  if (data.contains("System") && data["System"].isObject()) {
    QJsonObject sysConfigData = data["System"].toObject();
    if (sysConfigData.contains("SystemConfigurationFile") && sysConfigData["SystemConfigurationFile"].isString()) {
      QString sysConfigDataFile = sysConfigData["SystemConfigurationFile"].toString();
      if (fileDir.exists(sysConfigDataFile)) {
	QString fullPath = fileDir.absoluteFilePath(sysConfigDataFile);
	systemConfigFile->setFilename(fullPath);
      }
    }
  }
	
  if (data.contains("DamageInput") && data["DamageInput"].isObject()) {
    QJsonObject dataObj = data["DamageInput"].toObject();
    qDebug() << "DAMAGE_INPUT OBJ: " << dataObj;
    if (dataObj.contains("Parameters") && dataObj["Parameters"].isObject()) {    
      QJsonObject paramObj = dataObj["Parameters"].toObject();
      qDebug() << "PARAMETERS OBJ: " << paramObj;
      realizationEntriesToRun->inputFromJSON(paramObj);
    }
  }
}


void
Pyrecodes::writeMainFile(QString filepath) {

  /* want this: 
     "ComponentLibrary": {
        "ComponentLibraryCreatorFileName": "json_component_library_creator",

        "ComponentLibraryFile": "Alameda_ComponentLibrary.json"
     },
     "System": {
        "SystemCreatorClassName": "ConcreteSystemCreator",
        "SystemCreatorFileName": "concrete_system_creator",
        "SystemClassName": "BuiltEnvironment",
        "SystemFileName": "built_environment",
        "SystemConfigurationFile": "Alameda_SystemConfiguration.json"
     },
     "DamageInput": {
        "Type": "SpecificRealization",
        "Parameters":{
            "Filter": "0,1"
        }
     }
  */     

  QJsonObject componentLibrary;
  componentLibrary["ComponentLibraryCreatorFileName"]="json_component_library_creator";
  componentLibrary["ComponentLibraryCreatorClassName"]="JSONComponentLibraryCreator";
  componentLibraryFile->outputToJSON(componentLibrary);

  QJsonObject system;  
  system["SystemCreatorClassName"] = "ConcreteSystemCreator";
  system["SystemCreatorFileName"] = "concrete_system_creator";
  system["SystemClassName"] = "BuiltEnvironment";
  system["SystemFileName"] = "built_environment";
  systemConfigFile->outputToJSON(system);

  QJsonObject damage;
  damage["Type"] = "SpecificRealization";
  QJsonObject typeParamaters;
  realizationEntriesToRun->outputToJSON(typeParamaters);
  damage["Parameters"]=typeParamaters;

  QJsonObject jsonObject;
  jsonObject["ComponentLibrary"] = componentLibrary;
  jsonObject["System"]=system;
  jsonObject["DamageInput"]=damage;

  //
  // write to file
  //
  
  // Open file for writing
  QFile file(filepath);
  if (!file.open(QIODevice::WriteOnly)) {
    qWarning() << "Couldn't open file for writing:" << file.errorString();
    return;
  }

  // Convert QJsonObject to QJsonDocument
  QJsonDocument jsonDoc(jsonObject);

  // Write JSON doc to file
  file.write(jsonDoc.toJson(QJsonDocument::Indented)); // Indented for readability
  file.close();
}
