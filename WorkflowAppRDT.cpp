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
// Latest revision: 10.08.2020

#include "AssetsWidget.h"
#include "AnalysisWidget.h"
#include "ModelWidget.h"
#include "HazardsWidget.h"
#include "HazardToAssetWidget.h"
#include "VisualizationWidget.h"
#include "ResultsWidget.h"
#include "EngDemandParameterWidget.h"
#include "DLWidget.h"

//#include "DamageMeasureWidget.h"
//#include "DecisionVariableWidget.h"
#include "WorkflowAppRDT.h"
#include "LocalApplication.h"
#include "RemoteApplication.h"
#include "RemoteJobManager.h"
#include "RunWidget.h"
//#include "InputWidgetUQ.h"
#include "RunLocalWidget.h"
#include "RemoteService.h"
#include "SimCenterComponentSelection.h"
//#include "RegionalMappingWidget.h"
#include "GeneralInformationWidget.h"
#include "MainWindowWorkflowApp.h"
#include "RandomVariablesContainer.h"
#include "InputWidgetSampling.h"
#include "DakotaResultsSampling.h"
#include "CustomizedItemModel.h"
#include "GoogleAnalytics.h"
#include "UQWidget.h"

#include <DLWidget.h>

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
#include <QMessageBox>
#include <QMenuBar>
#include <QProcess>
#include <QPushButton>
#include <QScrollArea>
#include <QSettings>
#include <QStackedWidget>
#include <QUuid>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

// static pointer for global procedure set in constructor
static WorkflowAppRDT *theApp = nullptr;

// global procedure
int getNumParallelTasks()
{
    return theApp->getMaxNumParallelTasks();
}

WorkflowAppRDT* WorkflowAppRDT::getInstance()
{
    return theInstance;
}

WorkflowAppRDT *WorkflowAppRDT::theInstance = nullptr;


WorkflowAppRDT::WorkflowAppRDT(RemoteService *theService, QWidget *parent)
    : WorkflowAppWidget(theService, parent)
{
    // Set static pointer for global procedure
    theApp = this;

    theInstance = this;

    localApp = new LocalApplication("RDT_workflow.py");
    remoteApp = new RemoteApplication("RDT_workflow.py", theService);

    theJobManager = new RemoteJobManager(theService);

    SimCenterWidget *theWidgets[1];
    theRunWidget = new RunWidget(localApp, remoteApp, theWidgets, 0);

    // connect signals and slots - error messages and signals
    //    connect(theGI,SIGNAL(sendErrorMessage(QString)), this,SLOT(errorMessage(QString)));
    //    connect(theGI,SIGNAL(sendStatusMessage(QString)), this,SLOT(statusMessage(QString)));
    //    connect(theGI,SIGNAL(sendFatalMessage(QString)), this,SLOT(fatalMessage(QString)));

    connect(theRunWidget,SIGNAL(sendErrorMessage(QString)), this,SLOT(errorMessage(QString)));
    connect(theRunWidget,SIGNAL(sendStatusMessage(QString)), this,SLOT(statusMessage(QString)));
    connect(theRunWidget,SIGNAL(sendFatalMessage(QString)), this,SLOT(fatalMessage(QString)));

    connect(localApp,SIGNAL(sendErrorMessage(QString)), this,SLOT(errorMessage(QString)));
    connect(localApp,SIGNAL(sendStatusMessage(QString)), this,SLOT(statusMessage(QString)));
    connect(localApp,SIGNAL(sendFatalMessage(QString)), this,SLOT(fatalMessage(QString)));

    connect(remoteApp,SIGNAL(sendErrorMessage(QString)), this,SLOT(errorMessage(QString)));
    connect(remoteApp,SIGNAL(sendStatusMessage(QString)), this,SLOT(statusMessage(QString)));
    connect(remoteApp,SIGNAL(sendFatalMessage(QString)), this,SLOT(fatalMessage(QString)));

    connect(localApp,SIGNAL(setupForRun(QString &,QString &)), this, SLOT(setUpForApplicationRun(QString &,QString &)));
    connect(this,SIGNAL(setUpForApplicationRunDone(QString&, QString &)), theRunWidget, SLOT(setupForRunApplicationDone(QString&, QString &)));
    connect(localApp,SIGNAL(processResults(QString, QString, QString)), this, SLOT(processResults(QString, QString, QString)));

    connect(remoteApp,SIGNAL(setupForRun(QString &,QString &)), this, SLOT(setUpForApplicationRun(QString &,QString &)));
    connect(theJobManager,SIGNAL(processResults(QString , QString, QString)), this, SLOT(processResults(QString, QString, QString)));
    connect(theJobManager,SIGNAL(loadFile(QString)), this, SLOT(loadFile(QString)));

    connect(remoteApp,SIGNAL(successfullJobStart()), theRunWidget, SLOT(hide()));

    // access a web page which will increment the usage count for this tool
    manager = new QNetworkAccessManager(this);

    connect(manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinished(QNetworkReply*)));

    manager->get(QNetworkRequest(QUrl("http://opensees.berkeley.edu/OpenSees/developer/eeuq/use.php")));

}


void WorkflowAppRDT::initialize(void)
{
    QMenu *exampleMenu = theMainWindow->menuBar()->addMenu(tr("&Examples"));
    exampleMenu->addAction(tr("&Alameda Example"), this, &WorkflowAppRDT::loadAlamedaExample);

    // Create the various widgets
    theGeneralInformationWidget = new GeneralInformationWidget(this);
    theRVs = new RandomVariablesContainer();
    theVisualizationWidget = new VisualizationWidget(this);
    theAssetsWidget = new AssetsWidget(this,theVisualizationWidget);
    theHazardToAssetWidget = new HazardToAssetWidget(this, theVisualizationWidget);
    theModelingWidget = new ModelWidget(this, theRVs);
    theAnalysisWidget = new AnalysisWidget(this, theRVs);
    theHazardsWidget = new HazardsWidget(this, theVisualizationWidget, theRVs);
    theEngDemandParamWidget = new EngDemandParameterWidget(this);
    theDamageAndLossWidget = new DLWidget(this, theVisualizationWidget);
    //theDecisionVariableWidget = new DecisionVariableWidget(this);
    theUQWidget = new UQWidget(this, theRVs);
    theResultsWidget = new ResultsWidget(this, theVisualizationWidget);

    connect(theGeneralInformationWidget, SIGNAL(assetChanged(QString, bool)), this, SLOT(assetSelectionChanged(QString, bool)));
    connect(theHazardsWidget,SIGNAL(gridFileChangedSignal(QString, QString)), theHazardToAssetWidget, SLOT(hazardGridFileChangedSlot(QString,QString)));

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
    theComponentSelection->addComponent(tr("GI"),  theGeneralInformationWidget);
    theComponentSelection->addComponent(tr("HAZ"), theHazardsWidget);
    theComponentSelection->addComponent(tr("ASD"), theAssetsWidget);
    theComponentSelection->addComponent(tr("HTA"), theHazardToAssetWidget);
    theComponentSelection->addComponent(tr("MOD"), theModelingWidget);
    theComponentSelection->addComponent(tr("ANA"), theAnalysisWidget);
    theComponentSelection->addComponent(tr("DL"),  theDamageAndLossWidget);
    //theComponentSelection->addComponent(tr("EDP"), theEngDemandParamWidget);  // EDP from DL
    //theComponentSelection->addComponent(tr("DV"), theDecisionVariableWidget); // removed
    theComponentSelection->addComponent(tr("UQ"), theUQWidget);
    theComponentSelection->addComponent(tr("RV"), theRVs);
    theComponentSelection->addComponent(tr("RES"), theResultsWidget);

    theComponentSelection->displayComponent("VIZ");

    // for RDT select Buildings in GeneralInformation by default
    theGeneralInformationWidget->setAssetTypeState("Buildings", true);
}



WorkflowAppRDT::~WorkflowAppRDT()
{

}

void WorkflowAppRDT::replyFinished(QNetworkReply *pReply)
{
    return;
}

/*
DLWidget *WorkflowAppRDT::gettheDamageAndLossWidget() const
{
    return theDamageAndLossWidget;
}




HazardsWidget *WorkflowAppRDT::getHazardsWidget() const
{
    return theHazardsWidget;
}
*/

GeneralInformationWidget *WorkflowAppRDT::getGeneralInformationWidget() const
{
    return theGeneralInformationWidget;
}

AssetsWidget *WorkflowAppRDT::getAssetsWidget() const
{
    return theAssetsWidget;
}


VisualizationWidget *WorkflowAppRDT::getVisualizationWidget() const
{
    return theVisualizationWidget;
}


void WorkflowAppRDT::setActiveWidget(SimCenterAppWidget* widget)
{
    auto widgetName = widget->objectName();

    this->theComponentSelection->displayComponent(widgetName);
}


bool WorkflowAppRDT::outputToJSON(QJsonObject &jsonObjectTop)
{
    // get each of the main widgets to output themselves
    theGeneralInformationWidget->outputToJSON(jsonObjectTop);

    // ouput application data
    QJsonObject apps;

    theModelingWidget->outputAppDataToJSON(apps);
    theUQWidget->outputAppDataToJSON(apps);
    theAssetsWidget->outputAppDataToJSON(apps);
    theHazardsWidget->outputAppDataToJSON(apps);
    theAnalysisWidget->outputAppDataToJSON(apps);
    theDamageAndLossWidget->outputAppDataToJSON(apps);
    theHazardToAssetWidget->outputAppDataToJSON(apps);

    //
    // hard copde for now .. EDP's coming out D&L to provide
    //

    QJsonObject edpData;
    edpData["Application"]="StandardEarthquakeEDP_R";
    QJsonObject edpAppData;
    edpData["ApplicationData"] = edpAppData;
    apps["EDP"] = edpData;


    jsonObjectTop.insert("Applications",apps);

    //  output regular data

    theRunWidget->outputToJSON(jsonObjectTop);
    //theModelingWidget->outputToJSON(jsonObjectTop);
    theHazardsWidget->outputToJSON(jsonObjectTop);
    //theAnalysisWidget->outputToJSON(jsonObjectTop);
    //theDamageAndLossWidget->outputToJSON(jsonObjectTop);
    //theHazardToAssetWidget->outputToJSON(jsonObjectTop);
    //theUQWidget->outputToJSON(jsonObjectTop);
    //theDamageAndLossWidget->outputAppDataToJSON(jsonObjectTop);
    theRVs->outputToJSON(jsonObjectTop);

    return true;
}


void WorkflowAppRDT::processResults(QString /*dakotaOut*/, QString /*dakotaTab*/, QString /*inputFile*/)
{
    theResultsWidget->processResults();
    theRunWidget->hide();
    theComponentSelection->displayComponent("RES");

}


void WorkflowAppRDT::clear(void)
{

}


void WorkflowAppRDT::loadAlamedaExample()
{
    this->loadFile("/Users/steve/Documents/RDT/Examples/Alameda/Alameda.json");
}


bool WorkflowAppRDT::inputFromJSON(QJsonObject &jsonObject)
{
    //
    // get each of the main widgets to input themselves
    //

    if (theGeneralInformationWidget->inputFromJSON(jsonObject) == false) {
        emit errorMessage("RDT: failed to read GeneralInformation");
    }


    if (jsonObject.contains("Applications")) {

        QJsonObject apps = jsonObject["Applications"].toObject();

        theModelingWidget->inputAppDataFromJSON(apps);
        theUQWidget->inputAppDataFromJSON(apps);
        theAssetsWidget->inputAppDataFromJSON(apps);
        theHazardsWidget->inputAppDataFromJSON(apps);
        theAnalysisWidget->inputAppDataFromJSON(apps);
        theDamageAndLossWidget->inputAppDataFromJSON(apps);
        theHazardToAssetWidget->inputAppDataFromJSON(apps);

    } else
        return false;

    /*
    ** Note to me - others
    */

    theRunWidget->inputFromJSON(jsonObject);
    //theModelingWidget->outputToJSON(jsonObjectTop);
    theHazardsWidget->inputFromJSON(jsonObject);
    //theAnalysisWidget->outputToJSON(jsonObjectTop);
    //theDamageAndLossWidget->outputToJSON(jsonObjectTop);
    //theHazardToAssetWidget->outputToJSON(jsonObjectTop);
    //theUQWidget->outputToJSON(jsonObjectTop);
    //theDamageAndLossWidget->outputAppDataToJSON(jsonObjectTop);
    theRVs->inputFromJSON(jsonObject);



    return true;
}


void WorkflowAppRDT::onRunButtonClicked() {
    theRunWidget->hide();
    theRunWidget->setMinimumWidth(this->width()*0.5);
    theRunWidget->showLocalApplication();
    GoogleAnalytics::ReportLocalRun();
}


void WorkflowAppRDT::onRemoteRunButtonClicked(){
    emit errorMessage("");

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


void WorkflowAppRDT::onRemoteGetButtonClicked(){

    emit errorMessage("");

    bool loggedIn = theRemoteService->isLoggedIn();

    if (loggedIn == true) {

        theJobManager->hide();
        theJobManager->updateJobTable("");
        theJobManager->show();

    } else {
        errorMessage("ERROR - You Need to Login");
    }
}


void WorkflowAppRDT::onExitButtonClicked(){

}


void WorkflowAppRDT::setUpForApplicationRun(QString &workingDir, QString &subDir) {

    errorMessage("");

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


    qDebug() << "WorkflowAppRDT is changinging subDir to input_data";
    subDir = "input_data";

    QString templateDirectory  = destinationDirectory.absoluteFilePath(subDir);
    destinationDirectory.mkpath(templateDirectory);

    // copyPath(path, tmpDirectory, false);
    //    theSIM->copyFiles(templateDirectory);
    //    theEventSelection->copyFiles(templateDirectory);
    //    theAnalysisSelection->copyFiles(templateDirectory);
    theUQWidget->copyFiles(templateDirectory);
    theModelingWidget->copyFiles(templateDirectory);
    theAssetsWidget->copyFiles(templateDirectory);
    //theHazardsWidget->outputAppDataToJSON(apps);
    theAnalysisWidget->copyFiles(templateDirectory);
    theDamageAndLossWidget->copyFiles(templateDirectory);
    theHazardToAssetWidget->copyFiles(templateDirectory);
    theDamageAndLossWidget->copyFiles(templateDirectory);
    //    theEDP_Selection->copyFiles(templateDirectory);

    //
    // in new templatedir dir save the UI data into dakota.json file (same result as using saveAs)
    // NOTE: we append object workingDir to this which points to template dir
    //

    //QString inputFile = templateDirectory + QDir::separator() + tr("inputRWHALE.json");
    QString inputFile = tmpDirectory + QDir::separator() + tr("inputRWHALE.json");

    QFile file(inputFile);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        //errorMessage();
        return;
    }
    QJsonObject json;
    this->outputToJSON(json);

    json["runDir"]=tmpDirectory;
    json["WorkflowType"]="Regional Simulation";


    QJsonDocument doc(json);
    file.write(doc.toJson());
    file.close();


    statusMessage("SetUp Done .. Now starting application");

    emit setUpForApplicationRunDone(tmpDirectory, inputFile);
}


void WorkflowAppRDT::loadFile(const QString fileName){

    //
    // open file
    //

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        emit errorMessage(QString("Could Not Open File: ") + fileName);
        return;
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
    // clear current and input from new JSON
    //

    this->clear();
    this->inputFromJSON(jsonObj);
}


int WorkflowAppRDT::getMaxNumParallelTasks() {
    return theUQWidget->getNumParallelTasks();
}

void WorkflowAppRDT::assetSelectionChanged(QString text, bool value)
{
    if (value == true)
    {
        theAssetsWidget->show(text);
        theHazardToAssetWidget->show(text);
        theModelingWidget->show(text);
        theAnalysisWidget->show(text);
        theEngDemandParamWidget->show(text);
        theDamageAndLossWidget->show(text);
        theUQWidget->show(text);
        //theDecisionVariableWidget->show(text);
    }
    else
    {
        theAssetsWidget->hide(text);
        theHazardToAssetWidget->hide(text);
        theModelingWidget->hide(text);
        theAnalysisWidget->hide(text);
        theEngDemandParamWidget->hide(text);
        theDamageAndLossWidget->hide(text);
        theUQWidget->hide(text);
        //theDecisionVariableWidget->hide(text);
    }

}
