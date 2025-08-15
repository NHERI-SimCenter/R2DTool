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

// Written by: Stevan Gavrilovic, Frank McKenna

#include "SimCenterPreferences.h"
#include "Utils/RelativePathResolver.h"

#include "AnalysisWidget.h"
#include "AssetsWidget.h"
#include "CustomizedItemModel.h"
#include "DLWidget.h"
#include "SystemPerformanceWidget.h"
#include "RecoveryWidget.h"
#include "DakotaResultsSampling.h"
#include "GeneralInformationWidgetR2D.h"
#include "GoogleAnalytics.h"
#include "HazardToAssetWidget.h"
#include "HazardsWidget.h"
//#include "InputWidgetSampling.h"
#include "LocalApplication.h"
#include "MainWindowWorkflowApp.h"
#include "ModelWidget.h"
//#include "LocalMappingWidget.h"
#include  <SimCenterEventRegional.h>
#include  <SimCenterAppEventSelection.h>
#include "PerformanceWidget.h"
#include "RandomVariablesContainer.h"
#include "RemoteApplication.h"
#include "RemoteJobManager.h"
#include "RemoteService.h"
#include "ResultsWidget.h"
#include "Utils/ProgramOutputDialog.h"
#include "RunWidget.h"
#include "SimCenterComponentSelection.h"
//#include <UQ_EngineSelection.h>
#include <UQWidget.h>
#include "WorkflowAppR2D.h"
#include "LoadResultsDialog.h"
#include "ToolDialog.h"

#include "VisualizationWidget.h"
#include "QGISVisualizationWidget.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QHBoxLayout>
#include <QHostInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QScrollArea>
#include <QSettings>
#include <QStackedWidget>
#include <QUuid>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <Stampede3Machine.h>
#include <SimCenterAppSelection.h>
#include <NoArgSimCenterApp.h>

// static pointer for global procedure set in constructor
static WorkflowAppR2D *theApp = nullptr;

// global procedure
int getNumParallelTasks()
{
    return theApp->getMaxNumParallelTasks();
}


WorkflowAppR2D* WorkflowAppR2D::getInstance()
{
    return theInstance;
}


WorkflowAppR2D *WorkflowAppR2D::theInstance = nullptr;


WorkflowAppR2D::WorkflowAppR2D(RemoteService *theService, QWidget *parent)
    : WorkflowAppWidget(theService, parent)
{
    resultsDialog = nullptr;

    // Set static pointer for global procedure
    theApp = this;

    theInstance = this;

    TapisMachine *theMachine = new Stampede3Machine();
    
    localApp = new LocalApplication("rWHALE.py");
    remoteApp = new RemoteApplication("rWHALE.py", theService, theMachine);

    theJobManager = new RemoteJobManager(theService);    

    SimCenterWidget *theWidgets[1];
    theRunWidget = new RunWidget(localApp, remoteApp, theWidgets, 0);

    //
    // connect signals and slots 
    //

    connect(localApp,SIGNAL(setupForRun(QString &,QString &)), this, SLOT(setUpForApplicationRun(QString &,QString &)));
    connect(localApp,SIGNAL(processResults(QString&)), this, SLOT(processResults(QString&)));
    connect(localApp,SIGNAL(runComplete()), this,SLOT(runComplete()));
    connect(localApp,SIGNAL(runComplete()), progressDialog, SLOT(hideProgressBar()));
    
    connect(localApp,SIGNAL(sendErrorMessage(QString)), this,SLOT(errorMessage(QString)));
    connect(localApp,SIGNAL(sendStatusMessage(QString)), this,SLOT(statusMessage(QString)));
    connect(localApp,SIGNAL(sendFatalMessage(QString)), this,SLOT(fatalMessage(QString)));

    connect(remoteApp,SIGNAL(setupForRun(QString &,QString &)), this, SLOT(setUpForApplicationRun(QString &,QString &)));
    connect(remoteApp,SIGNAL(successfullJobStart()), theRunWidget, SLOT(hide()));    
    connect(remoteApp,SIGNAL(sendErrorMessage(QString)), this,SLOT(errorMessage(QString)));
    connect(remoteApp,SIGNAL(sendStatusMessage(QString)), this,SLOT(statusMessage(QString)));
    connect(remoteApp,SIGNAL(sendFatalMessage(QString)), this,SLOT(fatalMessage(QString)));

    connect(theJobManager,SIGNAL(processResults(QString&)), this, SLOT(processResults(QString&)));
    connect(theJobManager,SIGNAL(loadFile(QString&)), this, SLOT(loadFile(QString&)));
    connect(theJobManager,SIGNAL(sendErrorMessage(QString)), this,SLOT(errorMessage(QString)));
    connect(theJobManager,SIGNAL(sendStatusMessage(QString)), this,SLOT(statusMessage(QString)));
    connect(theJobManager,SIGNAL(sendFatalMessage(QString)), this,SLOT(fatalMessage(QString)));        

    connect(this,SIGNAL(setUpForApplicationRunDone(QString&, QString &)), theRunWidget, SLOT(setupForRunApplicationDone(QString&, QString &)));

    // connect(theJobManager,SIGNAL(processResults(QString&)), this, SLOT(processResults(QString&)));
    
    // access a web page which will increment the usage count for this tool
    manager = new QNetworkAccessManager(this);

    connect(manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinished(QNetworkReply*)));

}


WorkflowAppR2D::~WorkflowAppR2D()
{

}


void WorkflowAppR2D::initialize(void)
{

    // Add the edit menu to the menu bar, make sure it comes before help
    auto menuBar = theMainWindow->menuBar();

    QAction* menuAfter = nullptr;
    foreach (QAction *action, menuBar->actions()) {

        // First check for an examples menu and if that does not exist put it before the help menu
        auto actionText = action->text();
        if(actionText.compare("&Examples") == 0)
        {
            menuAfter = action;
            break;
        }
        else if(actionText.compare("&Help") == 0)
        {
            menuAfter = action;
            break;
        }
    }

    // Results menu
    QMenu *resultsMenu = new QMenu(tr("&Results"),menuBar);

    // Set the path to the input file
    resultsMenu->addAction("&Load Results", this, &WorkflowAppR2D::loadResults);
    menuBar->insertMenu(menuAfter, resultsMenu);

    // Create the various widgets
    theGeneralInformationWidgetR2D = new GeneralInformationWidgetR2D(this);
    theRVs = RandomVariablesContainer::getInstance();
    
    theVisualizationWidget = new QGISVisualizationWidget(theMainWindow);

    theToolDialog = new ToolDialog(this, theVisualizationWidget);
    
    // Tools menu
    QMenu *toolsMenu = new QMenu(tr("&Tools"),menuBar);

    // Set the path to the input file
    toolsMenu->addAction("&Earthquake Event Generation", theToolDialog, &ToolDialog::handleShowEQGMSimTool);
    toolsMenu->addAction("&Hurricane Scenario Simulation", theToolDialog, &ToolDialog::handleShowHurricaneSimTool);
    toolsMenu->addAction("&Census Data Allocation", theToolDialog, &ToolDialog::handleShowCensusAppTool);
    toolsMenu->addAction("&OpenQuake Source Selection", theToolDialog, &ToolDialog::handleShowOpenquakeSelectionTool);

    /*
    toolsMenu->addAction("&BRAILS-Buildings", theToolDialog, &ToolDialog::handleBrailsInventoryTool);
    toolsMenu->addAction("&BRAILS-Transportation", theToolDialog, &ToolDialog::handleBrailsTranspInventoryTool);
    */
    
//    toolsMenu->addAction("&PyReCodes", theToolDialog, &ToolDialog::handlePyrecodesTool);
    toolsMenu->addAction("&Residual Demand", theToolDialog, &ToolDialog::handleResidualDemandTool);
    menuBar->insertMenu(menuAfter, toolsMenu);

    theAssetsWidget = new AssetsWidget(this,theVisualizationWidget);
    theHazardToAssetWidget = new HazardToAssetWidget(this, theVisualizationWidget);
    theModelingWidget = new ModelWidget(this);
    theAnalysisWidget = new AnalysisWidget(this);
    theHazardsWidget = new HazardsWidget(this, theVisualizationWidget);
    //theLocalEvent = new LocalMappingWidget(this);
    
    theLocalEvent = new SimCenterAppEventSelection(QString("Events"), QString("Events"),this);
    SimCenterAppWidget *simcenterEvent = new SimCenterEventRegional();
    theLocalEvent->addComponent(QString("SimCenterEvent"), QString("SimCenterEvent"), simcenterEvent);
    
    theDamageAndLossWidget = new DLWidget(this, theVisualizationWidget);
    theSystemPerformanceWidget = new SystemPerformanceWidget(this);
    
    theUQWidget = new UQWidget(this);
    theResultsWidget = new ResultsWidget(this, theVisualizationWidget);
    thePerformanceWidget = new PerformanceWidget(this,theRVs);
    theRecoveryWidget = new RecoveryWidget(this);

    connect(theGeneralInformationWidgetR2D, SIGNAL(assetChanged(QString, bool)), this, SLOT(assetSelectionChanged(QString, bool)));
    connect(theHazardsWidget,SIGNAL(gridFileChangedSignal(QString, QString)), theHazardToAssetWidget, SLOT(hazardGridFileChangedSlot(QString,QString)));
    connect(theHazardsWidget,SIGNAL(eventTypeChangedSignal(QString)), theHazardToAssetWidget, SLOT(eventTypeChangedSlot(QString)));
    connect(theToolDialog, SIGNAL(resultProduced), this, SLOT(processResults));

    // Create layout to hold component selection
    QHBoxLayout *horizontalLayout = new QHBoxLayout();
    this->setLayout(horizontalLayout);
    this->setContentsMargins(0,5,0,5);
    horizontalLayout->setMargin(0);

    // Create the component selection & add the components to it
    theComponentSelection = new SimCenterComponentSelection();
    horizontalLayout->addWidget(theComponentSelection);

    theComponentSelection->setWidth(120);
    theComponentSelection->setItemWidthHeight(20,60);
    theComponentSelection->layout()->setSpacing(0);

    theComponentSelection->addComponent(tr("VIZ"), theVisualizationWidget);
    theComponentSelection->addComponent(tr("GI"),  theGeneralInformationWidgetR2D);
    theComponentSelection->addComponent(tr("HAZ"), theHazardsWidget);
    theComponentSelection->addComponent(tr("ASD"), theAssetsWidget);
    theComponentSelection->addComponent(tr("HTA"), theHazardToAssetWidget);
    theComponentSelection->addComponent(tr("MOD"), theModelingWidget);
    theComponentSelection->addComponent(tr("ANA"), theAnalysisWidget);
    theComponentSelection->addComponent(tr("DL"),  theDamageAndLossWidget);
    theComponentSelection->addComponent(tr("SP"), theSystemPerformanceWidget);
    theComponentSelection->addComponent(tr("REC"), theRecoveryWidget);    
    theComponentSelection->addComponent(tr("UQ"), theUQWidget);
    theComponentSelection->addComponent(tr("RV"), theRVs);
    theComponentSelection->addComponent(tr("RES"), theResultsWidget);
    /* *********************** Performance ***************************** 
    theComponentSelection->addComponent(tr("PRF"), thePerformanceWidget);    
    ******************************************************************** */


    theComponentSelection->displayComponent("VIZ");

    theComponentSelection->setItemWidthHeight(100,55);

    // for RDT select Buildings in GeneralInformation by default
    theGeneralInformationWidgetR2D->setAssetTypeState("Buildings", true);
}


void WorkflowAppR2D::replyFinished(QNetworkReply */*pReply*/)
{
    return;
}


GeneralInformationWidgetR2D *WorkflowAppR2D::getGeneralInformationWidget() const
{
    return theGeneralInformationWidgetR2D;
}


AssetsWidget *WorkflowAppR2D::getAssetsWidget() const
{
    return theAssetsWidget;
}


QGISVisualizationWidget *WorkflowAppR2D::getVisualizationWidget() const
{
    return theVisualizationWidget;
}


void WorkflowAppR2D::setActiveWidget(SimCenterAppWidget* widget)
{
    auto widgetName = widget->objectName();

    this->theComponentSelection->displayComponent(widgetName);
}


bool WorkflowAppR2D::outputToJSON(QJsonObject &jsonObjectTop)
{
    // get each of the main widgets to output themselves
    theGeneralInformationWidgetR2D->outputToJSON(jsonObjectTop);

    // ouput application data
    QJsonObject apps;

    bool result = true;
    if (theHazardsWidget->outputAppDataToJSON(apps) == false) {
        this->errorMessage("Error writing HAZ data to output");
        result = false;
    }

    if (theAssetsWidget->outputAppDataToJSON(apps) == false) {
        this->errorMessage("Error writing ASD data to output");
        result = false;
    }

    if (theModelingWidget->outputAppDataToJSON(apps) == false) {
        this->errorMessage("Error writing MOD data to output");
        result = false;
    }

    if (theLocalEvent->outputAppDataToJSON(apps) == false) {
        this->errorMessage("Error writing EVENT data to output");
        result = false;
    }    

    if (theHazardToAssetWidget->outputAppDataToJSON(apps) == false) {
        this->errorMessage("Error writing HTA data to output");
        result = false;
    }
    
    if (theAnalysisWidget->outputAppDataToJSON(apps) == false) {
        this->errorMessage("Error writing ANA data to output");
        result = false;
    }

    if (theDamageAndLossWidget->outputAppDataToJSON(apps) == false) {
        this->errorMessage("Error writing DL data to output");
        result = false;
    }

    if (theSystemPerformanceWidget->outputAppDataToJSON(apps) == false) {
        this->errorMessage("Error writing SP data to output");
        result = false;
    }

    if (theRecoveryWidget->outputAppDataToJSON(apps) == false) {
        this->errorMessage("Error writing REC data to output");
        result = false;
    }    

    /* **************************** Performance ***************************** 
    if (thePerformanceWidget->outputAppDataToJSON(apps) == false) {
        this->errorMessage("Error writing PRF data to output");
        thePerformanceWidget->clear();
        result = false;
    }
    **************************** Performance ***************************** */    
    
    if (theUQWidget->outputAppDataToJSON(apps) == false) {
        this->errorMessage("Error writing UQ data to output");
        result = false;
    }

    //
    // hard code for now ..
    // EDP's coming out D&L in future to provide this ..
    //    really ugly 2 dynamic casts!!
    //

    SimCenterAppWidget *theAnalysisBuildingComponent = theAnalysisWidget->getComponent("Buildings");
    if (theAnalysisBuildingComponent != nullptr) {
        SimCenterAppSelection *theAppSelection = dynamic_cast<SimCenterAppSelection *>(theAnalysisBuildingComponent);
        if (theAppSelection != nullptr) {
            QString ANAAppName = theAppSelection->getCurrentSelectionName();
            QList<QString> noEDPAnalysisApps = {"IMasEDP", "CapacitySpectrumMethod"};
            if(!noEDPAnalysisApps.contains(ANAAppName)){
                QJsonObject buildingEdp;
                QJsonObject edpData;
                edpData["Application"]="StandardEarthquakeEDP";
                QJsonObject edpAppData;
                edpData["ApplicationData"] = edpAppData;
                buildingEdp["Buildings"] = edpData;
                apps["EDP"] = buildingEdp;
            }
//            SimCenterAppWidget *theCurrentSelection = theAppSelection->getCurrentSelection();
//            NoArgSimCenterApp *theNoArgWidget = dynamic_cast<NoArgSimCenterApp *>(theCurrentSelection);
//            if (theNoArgWidget == nullptr || (!noEDPAnalysisApps.contains(theNoArgWidget->getAppName()))) {
//                QJsonObject buildingEdp;
//                QJsonObject edpData;
//                edpData["Application"]="StandardEarthquakeEDP";
//                QJsonObject edpAppData;
//                edpData["ApplicationData"] = edpAppData;
//                buildingEdp["Buildings"] = edpData;
//                apps["EDP"] = buildingEdp;
//            }
        }
    }

    jsonObjectTop.insert("Applications",apps);
    //  output regular data
    jsonObjectTop["commonFileDir"]=commonFilePath;
    
    theRunWidget->outputToJSON(jsonObjectTop);
    theAssetsWidget->outputToJSON(jsonObjectTop);
    theModelingWidget->outputToJSON(jsonObjectTop);
    theLocalEvent->outputToJSON(jsonObjectTop);    
    theHazardsWidget->outputToJSON(jsonObjectTop);
    theAnalysisWidget->outputToJSON(jsonObjectTop);
    theDamageAndLossWidget->outputToJSON(jsonObjectTop);
    theSystemPerformanceWidget->outputToJSON(jsonObjectTop);
    theRecoveryWidget->outputToJSON(jsonObjectTop);    
    theHazardToAssetWidget->outputToJSON(jsonObjectTop);
    /* **************************** Performance *****************************
    thePerformanceWidget->outputToJSON(jsonObjectTop);       
    ******************************* Performance ***************************** */
    
    theUQWidget->outputToJSON(jsonObjectTop);
    theRVs->outputToJSON(jsonObjectTop);

    QJsonObject defaultValues;
    defaultValues["workflowInput"]=QString("scInput.json");    
    //defaultValues["filenameAIM"]=QString("AIM.json");
    defaultValues["filenameEVENT"] = QString("EVENT.json");
    defaultValues["filenameSAM"]= QString("SAM.json");
    defaultValues["filenameEDP"]= QString("EDP.json");
    defaultValues["filenameSIM"]= QString("SIM.json");
    defaultValues["driverFile"]= QString("driver");
    defaultValues["filenameDL"]= QString("BIM.json");
    defaultValues["workflowOutput"]= QString("EDP.json");
    QJsonArray rvFiles, edpFiles;
    //rvFiles.append(QString("AIM.json"));
    rvFiles.append(QString("SAM.json"));
    rvFiles.append(QString("EVENT.json"));
    rvFiles.append(QString("SIM.json"));
    edpFiles.append(QString("EDP.json"));
    defaultValues["rvFiles"]= rvFiles;
    defaultValues["edpFiles"]=edpFiles;
    jsonObjectTop["DefaultValues"]=defaultValues;    

    return result;
}


void WorkflowAppR2D::processResults(QString &resultsDir)
{
    this->statusMessage("Importing results");    
    QApplication::processEvents();

    int ok = theResultsWidget->processResults(resultsDir);
    theRunWidget->hide();

    if  (ok == 0) {
      theComponentSelection->displayComponent("RES");
      this->statusMessage("Import Success");
    }
    else
      this->errorMessage("Import Failure .. Incorrect path or Job failed");
}


void WorkflowAppR2D::clear(void)
{
    theGeneralInformationWidgetR2D->clear();
    theUQWidget->clear();
    theModelingWidget->clear();
    theLocalEvent->clear();    
    theAnalysisWidget->clear();
    theHazardToAssetWidget->clear();
    theAssetsWidget->clear();
    theHazardsWidget->clear();
    theDamageAndLossWidget->clear();
    theSystemPerformanceWidget->clear();
    theRecoveryWidget->clear();    
    /* **************************** Performance *****************************    
    thePerformanceWidget->clear();
    ****************************** Performance ***************************** */    
    theResultsWidget->clear();
    theToolDialog->clear();
    theVisualizationWidget->clear();
    // progressDialog->clear();
    theComponentSelection->displayComponent("VIZ");
}


bool WorkflowAppR2D::inputFromJSON(QJsonObject &jsonObject)
{
    //
    // get each of the main widgets to input themselves
    //

    if (theGeneralInformationWidgetR2D->inputFromJSON(jsonObject) == false) {
        this->errorMessage("R2D: failed to read GeneralInformation");
        return false;
    }

    bool result = true;
    
    if (jsonObject.contains("Applications")) {

        QJsonObject apps = jsonObject["Applications"].toObject();

        if (theUQWidget->inputAppDataFromJSON(apps) == false) {
            this->errorMessage("UQ failed to read input data");
            theUQWidget->clear();
            result = false;
        }

        if (theModelingWidget->inputAppDataFromJSON(apps) == false) {
            this->errorMessage("MOD failed to read input data");
            theModelingWidget->clear();
            result = false;
        }

        if (theLocalEvent->inputAppDataFromJSON(apps) == false) {
            this->errorMessage("LocalModeling failed to read input data");
            theLocalEvent->clear();
            result = false;
        }	
	
        if (theAnalysisWidget->inputAppDataFromJSON(apps) == false) {
            this->errorMessage("ANA failed to read input data");
            theAnalysisWidget->clear();
            result = false;
        }

        if (theHazardToAssetWidget->inputAppDataFromJSON(apps) == false) {
            this->errorMessage("HTA failed to read input data");
            theHazardToAssetWidget->clear();
            result = false;
        }

        if (theAssetsWidget->inputAppDataFromJSON(apps) == false) {
            this->errorMessage("ASD failed to read input data");
            theAssetsWidget->clear();
            result = false;
        }
	
        if (theHazardsWidget->inputAppDataFromJSON(apps) == false) {
            this->errorMessage("HAZ failed to read input data");
            theHazardsWidget->clear();
            result = false;
        }

        if (theDamageAndLossWidget->inputAppDataFromJSON(apps) == false) {
            this->errorMessage("DL failed to read input data");
            theDamageAndLossWidget->clear();
            result = false;
        }

	if (apps.contains("SystemPerformance")) {
	  if (theSystemPerformanceWidget->inputAppDataFromJSON(apps) == false) {
            this->errorMessage("SP failed to read input data");
	        theSystemPerformanceWidget->clear();
            result = false;
	  }
	}

	if (apps.contains("Recovery")) {
	  if (theRecoveryWidget->inputAppDataFromJSON(apps) == false) {
            this->errorMessage("REC failed to read input data");
	    theRecoveryWidget->clear();
	    result = false;
	  }
	}
	
    } else {
      this->errorMessage("Error, no Applications field in input file");
      return false;
    }


    
    //
    // now read app specific data
    //

    if (theRunWidget->inputFromJSON(jsonObject) == false)
        return false;

    if (theUQWidget->inputFromJSON(jsonObject) == false) {
      this->errorMessage("UQ failed to read app specific data");
      result = false;
    }
    
    if (theRVs->inputFromJSON(jsonObject) == false)
      return false;    

    if (theModelingWidget->inputFromJSON(jsonObject) == false) {
      this->errorMessage("MOD failed to read app specific data");
      result = false;
    }

    if (theLocalEvent->inputFromJSON(jsonObject) == false) {
      this->errorMessage("LocalModeling failed to read app specific data");
      result = false;
    }    
    
    if (theAnalysisWidget->inputFromJSON(jsonObject) == false) {
      this->errorMessage("ANA failed to read app specific data");
      result = false;
    }

    if (theHazardToAssetWidget->inputFromJSON(jsonObject) == false) {
      this->errorMessage("HTA failed to read app specific data");
      result = false;
    }

    if (theAssetsWidget->inputFromJSON(jsonObject) == false) {
      this->errorMessage("ASD failed to read app specific data");
      result = false;
    }

    if (theHazardsWidget->inputFromJSON(jsonObject) == false) {
      this->errorMessage("HAZ failed to read app specific data");
      result = false;
    }

    if (theDamageAndLossWidget->inputFromJSON(jsonObject) == false) {
      this->errorMessage("DL failed to read app specific data");
      result = false;
    }

    if (jsonObject.contains("SystemPerformance")) {    
      if (theSystemPerformanceWidget->inputFromJSON(jsonObject) == false) {
	    this->errorMessage("DL failed to read app specific data");
	    result = false;
      }
    }

    if (jsonObject.contains("Recovery")) {    
      if (theRecoveryWidget->inputFromJSON(jsonObject) == false) {
	    this->errorMessage("REC failed to read app specific data");
	    result = false;
      }
    }
    
    return result;
}


void WorkflowAppR2D::onRunButtonClicked() {
    theRunWidget->hide();
    theRunWidget->setMinimumWidth(this->width()*0.5);

    progressDialog->showProgressBar();
    progressDialog->setProgressBarValue(0);

    theRunWidget->showLocalApplication();
    GoogleAnalytics::ReportLocalRun();
}


void WorkflowAppR2D::onRemoteRunButtonClicked(){

    bool loggedIn = theRemoteService->isLoggedIn();

    if (loggedIn == true) {
        theRunWidget->hide();
        theRunWidget->setMinimumWidth(this->width()*0.5);
        theRunWidget->showRemoteApplication();

    } else {
        errorMessage("ERROR - You Need to Login");
    }

    GoogleAnalytics::ReportDesignSafeRun();
}


void WorkflowAppR2D::onRemoteGetButtonClicked(){

    bool loggedIn = theRemoteService->isLoggedIn();

    if (loggedIn == true) {

        theJobManager->hide();
        theJobManager->updateJobTable("");
        theJobManager->show();

    } else {
        this->errorMessage("ERROR - You need to Login");
    }
}


void WorkflowAppR2D::onExitButtonClicked(){

}


void WorkflowAppR2D::setUpForApplicationRun(QString &workingDir, QString &subDir) {

    //    errorMessage("");

    //
    // create temporary directory in working dir
    // and copy all files needed to this directory by invoking copyFiles() on app widgets
    //

    QString tmpDirName = QString("tmp.SimCenter");
    QDir workDir(workingDir);

    QString tmpDirectory = workDir.absoluteFilePath(tmpDirName);
    QDir destinationDirectory(tmpDirectory);

    if(destinationDirectory.exists())
    {
        destinationDirectory.removeRecursively();
    }
    else
        destinationDirectory.mkpath(tmpDirectory);

    theResultsWidget->clear();
    subDir = "input_data";

    QString templateDirectory  = destinationDirectory.absoluteFilePath(subDir);
    destinationDirectory.mkpath(templateDirectory);

    commonFilePath = subDir;
    
    // FMK
    
    // copyPath(path, tmpDirectory, false);
    //    theSIM->copyFiles(templateDirectory);
    //    theEventSelection->copyFiles(templateDirectory);
    //    theAnalysisSelection->copyFiles(templateDirectory);

    bool res = false;

    // Copy the files
    this->statusMessage("Copying files");

    QApplication::processEvents();

    res = theUQWidget->copyFiles(templateDirectory);
    if(!res)
    {
        errorMessage("Error in copy files in "+theUQWidget->objectName());
        progressDialog->hideProgressBar();
        return;
    }

    res = theModelingWidget->copyFiles(templateDirectory);
    if(!res)
    {
        errorMessage("Error in copy files in "+theModelingWidget->objectName());
        progressDialog->hideProgressBar();
        return;
    }

    res = theAssetsWidget->copyFiles(templateDirectory);
    if(!res)
    {
        errorMessage("Error in copy files in "+theAssetsWidget->objectName());
        progressDialog->hideProgressBar();
        return;
    }
    res = theHazardsWidget->copyFiles(templateDirectory);
    if(!res)
    {
        theComponentSelection->displayComponent("HAZ");
        errorMessage("Error in copy files in "+theHazardsWidget->objectName());
        progressDialog->hideProgressBar();
        return;
    }

    res = theAnalysisWidget->copyFiles(templateDirectory);
    if(!res)
    {
        errorMessage("Error in copy files in "+theAnalysisWidget->objectName());
        progressDialog->hideProgressBar();
        return;
    }

    res = theHazardToAssetWidget->copyFiles(templateDirectory);
    if(!res)
    {
        errorMessage("Error in copy files in "+theHazardToAssetWidget->objectName());
        progressDialog->hideProgressBar();
        return;
    }

    res = theDamageAndLossWidget->copyFiles(templateDirectory);
    if(!res) {
        errorMessage("Error in copy files in "+theDamageAndLossWidget->objectName());
        progressDialog->hideProgressBar();
        return;
    }

    res = theSystemPerformanceWidget->copyFiles(templateDirectory);
    if(!res) {
        errorMessage("Error in copy files in "+theSystemPerformanceWidget->objectName());
        progressDialog->hideProgressBar();
        return;
    }

    res = theRecoveryWidget->copyFiles(templateDirectory);
    if(!res) {
        errorMessage("Error in copy files in "+theRecoveryWidget->objectName());
        progressDialog->hideProgressBar();
        return;
    }        
    

    // Generate the input file
    this->statusMessage("Generating .json input file");

    QApplication::processEvents();
    //
    // in new templatedir dir save the UI data into dakota.json file (same result as using saveAs)
    // NOTE: we append object workingDir to this which points to template dir
    //

    //QString inputFile = templateDirectory + QDir::separator() + tr("inputRWHALE.json");
    QString inputFile = tmpDirectory + QDir::separator() + tr("inputRWHALE.json");

    QFile file(inputFile);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        //errorMessage();
        progressDialog->hideProgressBar();
        return;
    }

    QJsonObject json;
    res = this->outputToJSON(json);
    if(!res)
    {
        errorMessage("Error in creating .json input file");
        progressDialog->hideProgressBar();
        return;
    }

    /* FMK THINKING ABOUT THIS ONE ****************************
    // rel paths in input file for loading later
    QFileInfo fileInfo(inputFile);
    SCUtils::ResolveRelativePaths(json, fileInfo.dir());
    * ******************************************************* */
    
    json["runDir"]=tmpDirectory;
    json["WorkflowType"]="Regional Simulation";
    
    QJsonDocument doc(json);
    file.write(doc.toJson());
    file.close();
    
    statusMessage("Setup done. Now starting application.");

    QJsonObject citations;
    QString citeFile = templateDirectory + QDir::separator() + tr("please_cite.json");        
    this->createCitation(citations, citeFile);
    
    QApplication::processEvents();

    emit setUpForApplicationRunDone(tmpDirectory, inputFile);
}


int WorkflowAppR2D::loadFile(QString &fileName){

    //
    // check file exists & set apps current dir of it does
    //
    
    QFileInfo fileInfo(fileName);
    if (!fileInfo.exists()){
        this->errorMessage(QString("File does not exist: ") + fileName);
        return -1;
    }

    QString dirPath = fileInfo.absoluteDir().absolutePath();
    QDir::setCurrent(dirPath);
    
    //
    // open file
    //

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        this->errorMessage(QString("Could Not Open File: ") + fileName);
        return -1;
    }

    //
    // place contents of file into json object
    //

    QString val;
    val=file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(val.toUtf8());
    QJsonObject jsonObj = doc.object();

    // close file
    file.close();

    //
    // Clean up and find the relative paths if the paths are wrong
    //
    
    SCUtils::ResolveAbsolutePaths(jsonObj, fileInfo.dir());
    SCUtils::PathFinder(jsonObj,dirPath);
    
    //
    // clear current and input from new JSON
    //

    this->clear();
    bool res = this->inputFromJSON(jsonObj);
    if(res == false) {
        this->errorMessage("Failed to load the input file: " + fileName);
        return -1;
    } 

    this->statusMessage("Done Loading File");
    /*
    auto fileSender = qobject_cast<RemoteJobManager*>(QObject::sender());
    if(fileSender == nullptr)
    this->statusMessage("Done Loading File");
    else
        this->statusMessage("Done Downloading & Loading Remote File");
    */
    
    return 0;
}


int WorkflowAppR2D::getMaxNumParallelTasks() {
    return theUQWidget->getNumParallelTasks();
}


void WorkflowAppR2D::assetSelectionChanged(QString text, bool value)
{
    if (value == true)
    {
        theAssetsWidget->show(text);
        theHazardToAssetWidget->show(text);
        theModelingWidget->show(text);
	//        theLocalEvent->show(text);
        theAnalysisWidget->show(text);
	/* ********* PERFORMANCE **********
        thePerformanceWidget->show(text);
	* *********************************/
        theDamageAndLossWidget->show(text);
        theSystemPerformanceWidget->show(text);	
        theUQWidget->show(text);
    }
    else
    {
        theAssetsWidget->hide(text);
        theHazardToAssetWidget->hide(text);
        theModelingWidget->hide(text);
        //theLocalEvent->hide(text);	
        theAnalysisWidget->hide(text);
	/* ********* PERFORMANCE **********	
        thePerformanceWidget->hide(text);
	*********************************** */	
        theDamageAndLossWidget->hide(text);
        theSystemPerformanceWidget->hide(text);	
        theUQWidget->hide(text);
    }

}


void WorkflowAppR2D::statusMessage(QString message)
{
    progressDialog->appendText(message);
}


void WorkflowAppR2D::infoMessage(QString message)
{
    progressDialog->appendInfoMessage(message);
}


void WorkflowAppR2D::errorMessage(QString message)
{
    progressDialog->appendErrorMessage(message);
}


void WorkflowAppR2D::fatalMessage(QString message)
{
    progressDialog->appendErrorMessage(message);
}


void WorkflowAppR2D::runComplete()
{
    //    progressDialog->hideAfterElapsedTime(2);
    progressDialog->hideProgressBar();
}


DLWidget *WorkflowAppR2D::getTheDamageAndLossWidget() const
{
    return theDamageAndLossWidget;
}

ResultsWidget* WorkflowAppR2D::getTheResultsWidget() const{
    return theResultsWidget;
}

SystemPerformanceWidget *WorkflowAppR2D::getTheSystemPerformanceWidget() const
{
    return theSystemPerformanceWidget;
}

RecoveryWidget *WorkflowAppR2D::getTheRecoveryWidget() const
{
    return theRecoveryWidget;
}


LocalApplication *WorkflowAppR2D::getLocalApp() const
{
    return localApp;
}


void WorkflowAppR2D::loadResults(void)
{
    if(resultsDialog == nullptr)
        resultsDialog= new LoadResultsDialog(this);

    resultsDialog->show();
}

int
WorkflowAppR2D::createCitation(QJsonObject &citation, QString citeFile) {

  QString cit("{\"R2D\": { \"citations\": [{\"citation\": \"Frank McKenna, Stevan Gavrilovic, Jinyan Zhao, Kuanshi Zhong, Adam Zsarnoczay, Barbaros Cetiner, Sina Naeimi, Sang-ri Yi, Aakash Bangalore Satish, Amin Pakzad, Pedro Arduino, & Wael Elhaddad. (2025). NHERI-SimCenter/R2DTool: Version 5.5.0 (v5.5.0). Zenodo. https://doi.org/10.5281/zenodo.16884221\",\"description\": \"This is the overall tool reference used to indicate the version of the tool.\"}},{\"citation\": \"Gregory G. Deierlein, Frank McKenna, Adam Zsarnóczay, Tracy Kijewski-Correa, Ahsan Kareem, Wael Elhaddad, Laura Lowes, Matthew J. Schoettler, and Sanjay Govindjee (2020) A Cloud-Enabled Application Framework for Simulating Regional-Scale Impacts of Natural Hazards on the Built Environment. Frontiers in the Built Environment. 6:558706. doi: 10.3389\/fbuil.2020.558706\",\"description\": \" This marker paper describes the SimCenter application framework, which was designed to simulate the impacts of natural hazards on the built environment.It  is a necessary attribute for publishing work resulting from the use of SimCenter tools, software, and datasets.\"}]}}");

  QJsonDocument docC = QJsonDocument::fromJson(cit.toUtf8());
  if(!docC.isNull()) {
    if(docC.isObject()) {
      citation = docC.object();        
    }  else {
      qDebug() << "WorkflowdAppR2D citation text is not valid JSON: \n" << cit << endl;
    }
  }

  theAssetsWidget->outputCitation(citation);
  theHazardsWidget->outputCitation(citation);
  theModelingWidget->outputCitation(citation);
  theAnalysisWidget->outputCitation(citation);
  theDamageAndLossWidget->outputCitation(citation);
  theUQWidget->outputCitation(citation);
  theSystemPerformanceWidget->outputCitation(citation);
  theRecoveryWidget->outputCitation(citation);

  QFile file(citeFile);
  if (!file.open(QFile::WriteOnly | QFile::Text)) {
    //errorMessage();
    progressDialog->hideProgressBar();
    return 0;
  }
  QJsonDocument doc(citation);
  file.write(doc.toJson());
  file.close();

  
  return 0;    
}
