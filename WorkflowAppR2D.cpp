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
#include "AnalysisWidget.h"
#include "AssetsWidget.h"
#include "CustomizedItemModel.h"
#include "DLWidget.h"
#include "DakotaResultsSampling.h"
#include "EngDemandParameterWidget.h"
#include "GeneralInformationWidget.h"
#include "GoogleAnalytics.h"
#include "HazardToAssetWidget.h"
#include "HazardsWidget.h"
#include "InputWidgetSampling.h"
#include "LocalApplication.h"
#include "MainWindowWorkflowApp.h"
#include "ModelWidget.h"
#include "RandomVariablesContainer.h"
#include "RemoteApplication.h"
#include "RemoteJobManager.h"
#include "RemoteService.h"
#include "ResultsWidget.h"
#include "Utils/PythonProgressDialog.h"
//#include "RunLocalWidget.h"
#include "RunWidget.h"
#include "SimCenterComponentSelection.h"
#include "UQWidget.h"
#include "VisualizationWidget.h"
#include "WorkflowAppR2D.h"

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
    // Set static pointer for global procedure
    theApp = this;

    theInstance = this;

    localApp = new LocalApplication("R2DTool_workflow.py");
    remoteApp = new RemoteApplication("R2DTool_workflow.py", theService);

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
    connect(localApp,SIGNAL(runComplete()), this,SLOT(runComplete()));

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

    connect(localApp,SIGNAL(runComplete()), progressDialog, SLOT(hideProgressBar()));

    // access a web page which will increment the usage count for this tool
    manager = new QNetworkAccessManager(this);

    connect(manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinished(QNetworkReply*)));

    manager->get(QNetworkRequest(QUrl("http://opensees.berkeley.edu/OpenSees/developer/eeuq/use.php")));

}


WorkflowAppR2D::~WorkflowAppR2D()
{

}


void WorkflowAppR2D::initialize(void)
{
    // Clear action
    QMenu *editMenu = theMainWindow->menuBar()->addMenu(tr("&Edit"));
    // Set the path to the input file
    editMenu->addAction("Clear", this, &WorkflowAppR2D::clear);

    // Create the various widgets
    theGeneralInformationWidget = new GeneralInformationWidget(this);
    theRVs = new RandomVariablesContainer();
    theVisualizationWidget = new VisualizationWidget(this);
    theAssetsWidget = new AssetsWidget(this,theVisualizationWidget);
    theHazardToAssetWidget = new HazardToAssetWidget(this, theVisualizationWidget);
    theModelingWidget = new ModelWidget(this, theRVs);
    theAnalysisWidget = new AnalysisWidget(this, theRVs);
    theHazardsWidget = new HazardsWidget(this, theVisualizationWidget, theRVs);
    // theEngDemandParamWidget = new EngDemandParameterWidget(this);
    theDamageAndLossWidget = new DLWidget(this, theVisualizationWidget);
    // theDecisionVariableWidget = new DecisionVariableWidget(this);
    theUQWidget = new UQWidget(this, theRVs);
    theResultsWidget = new ResultsWidget(this, theVisualizationWidget);

    connect(theVisualizationWidget,SIGNAL(sendErrorMessage(QString)), this,SLOT(errorMessage(QString)));
    connect(theVisualizationWidget,SIGNAL(sendStatusMessage(QString)), this,SLOT(statusMessage(QString)));
    connect(theVisualizationWidget,SIGNAL(sendFatalMessage(QString)), this,SLOT(fatalMessage(QString)));
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


void WorkflowAppR2D::replyFinished(QNetworkReply */*pReply*/)
{
    return;
}


GeneralInformationWidget *WorkflowAppR2D::getGeneralInformationWidget() const
{
    return theGeneralInformationWidget;
}


AssetsWidget *WorkflowAppR2D::getAssetsWidget() const
{
    return theAssetsWidget;
}


VisualizationWidget *WorkflowAppR2D::getVisualizationWidget() const
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
    theGeneralInformationWidget->outputToJSON(jsonObjectTop);

    // ouput application data
    QJsonObject apps;

    theHazardsWidget->outputAppDataToJSON(apps);
    theAssetsWidget->outputAppDataToJSON(apps);
    theModelingWidget->outputAppDataToJSON(apps);
    theHazardToAssetWidget->outputAppDataToJSON(apps);
    theModelingWidget->outputAppDataToJSON(apps);
    theAnalysisWidget->outputAppDataToJSON(apps);
    theDamageAndLossWidget->outputAppDataToJSON(apps);
    theUQWidget->outputAppDataToJSON(apps);

    //
    // hard code for now .. EDP's coming out D&L in future to provide this .. really ugly 2 dynamic casts!!
    //

    SimCenterAppWidget *theAnalysisBuildingComponent = theAnalysisWidget->getComponent("Buildings");
    if (theAnalysisBuildingComponent != nullptr) {
        SimCenterAppSelection *theAppSelection = dynamic_cast<SimCenterAppSelection *>(theAnalysisBuildingComponent);
        if (theAppSelection != nullptr) {
            SimCenterAppWidget *theCurrentSelection = theAppSelection->getCurrentSelection();

            NoArgSimCenterApp *theNoArgWidget = dynamic_cast<NoArgSimCenterApp *>(theCurrentSelection);
            if (theNoArgWidget == nullptr || theNoArgWidget->getAppName() != "IMasEDP") {
                QJsonObject edpData;
                edpData["Application"]="StandardEarthquakeEDP_R";
                QJsonObject edpAppData;
                edpData["ApplicationData"] = edpAppData;
                apps["EDP"] = edpData;
            }
        }
    }

    jsonObjectTop.insert("Applications",apps);

    //  output regular data


    theRunWidget->outputToJSON(jsonObjectTop);

    theModelingWidget->outputToJSON(jsonObjectTop);
    theHazardsWidget->outputToJSON(jsonObjectTop);
    theAnalysisWidget->outputToJSON(jsonObjectTop);
    theDamageAndLossWidget->outputToJSON(jsonObjectTop);
    theHazardToAssetWidget->outputToJSON(jsonObjectTop);
    //theUQWidget->outputToJSON(jsonObjectTop);
    //theDamageAndLossWidget->outputAppDataToJSON(jsonObjectTop);
    theRVs->outputToJSON(jsonObjectTop);

    return true;
}


void WorkflowAppR2D::processResults(QString resultsDir, QString /*dakotaTab*/, QString /*inputFile*/)
{
    theResultsWidget->processResults(resultsDir);
    theRunWidget->hide();
    theComponentSelection->displayComponent("RES");
}


void WorkflowAppR2D::clear(void)
{
    theGeneralInformationWidget->clear();
    theUQWidget->clear();
    theModelingWidget->clear();
    theAnalysisWidget->clear();
    theHazardToAssetWidget->clear();
    theAssetsWidget->clear();
    theHazardsWidget->clear();
    theDamageAndLossWidget->clear();
    theResultsWidget->clear();
    theVisualizationWidget->clear();
    progressDialog->clear();
}


bool WorkflowAppR2D::inputFromJSON(QJsonObject &jsonObject)
{
    //
    // get each of the main widgets to input themselves
    //

    if (theGeneralInformationWidget->inputFromJSON(jsonObject) == false) {
        emit errorMessage("R2D: failed to read GeneralInformation");
    }


    if (jsonObject.contains("Applications")) {

        QJsonObject apps = jsonObject["Applications"].toObject();

        theUQWidget->inputAppDataFromJSON(apps);
        theModelingWidget->inputAppDataFromJSON(apps);
        theAnalysisWidget->inputAppDataFromJSON(apps);
        theHazardToAssetWidget->inputAppDataFromJSON(apps);
        theAssetsWidget->inputAppDataFromJSON(apps);
        theHazardsWidget->inputAppDataFromJSON(apps);
        theDamageAndLossWidget->inputAppDataFromJSON(apps);

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


void WorkflowAppR2D::onRunButtonClicked() {
    theRunWidget->hide();
    theRunWidget->setMinimumWidth(this->width()*0.5);

    progressDialog->showDialog(true);
    progressDialog->showProgressBar();
    theRunWidget->showLocalApplication();
    GoogleAnalytics::ReportLocalRun();
}


void WorkflowAppR2D::onRemoteRunButtonClicked(){
    //    emit errorMessage("");

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

    // emit errorMessage("");

    bool loggedIn = theRemoteService->isLoggedIn();

    if (loggedIn == true) {

        theJobManager->hide();
        theJobManager->updateJobTable("");
        theJobManager->show();

    } else {
        errorMessage("ERROR - You need to Login");
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


    qDebug() << "WorkflowAppR2D is changinging subDir to input_data";
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


    statusMessage("Setup done. Now starting application.");

    emit setUpForApplicationRunDone(tmpDirectory, inputFile);
}


void WorkflowAppR2D::loadFile(const QString fileName){

    // check file exists & set apps current dir of it does
    QFileInfo fileInfo(fileName);
    if (!fileInfo.exists()){
        emit errorMessage(QString("File foes not exist: ") + fileName);
        return;
    }

    QString dirPath = fileInfo.absoluteDir().absolutePath();
    QDir::setCurrent(dirPath);
    // qDebug() << "WorkflowAppR2D: setting current dir" << dirPath;

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

    progressDialog->showProgressBar();
    QApplication::processEvents();

    this->inputFromJSON(jsonObj);
    progressDialog->hideProgressBar();

    if(qobject_cast<RemoteJobManager*>(QObject::sender()) == nullptr)
        this->statusMessage("Done loading. Click on the 'RUN' button to run an analysis.");

    // Automatically hide after n seconds
    // progressDialog->hideAfterElapsedTime(4);
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
        theAnalysisWidget->show(text);
        // theEngDemandParamWidget->show(text);
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
        // theEngDemandParamWidget->hide(text);
        theDamageAndLossWidget->hide(text);
        theUQWidget->hide(text);
        //theDecisionVariableWidget->hide(text);
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
    progressDialog->hideAfterElapsedTime(2);
}

