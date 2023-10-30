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

#include "CSVReaderWriter.h"
#include "GMPEWidget.h"
#include "GMWidget.h"
#include "GmAppConfig.h"
#include "GmAppConfigWidget.h"
#include "GmCommon.h"
#include "GridNode.h"
#include "IntensityMeasureWidget.h"
#include "Utils/ProgramOutputDialog.h"
#include "MapViewSubWidget.h"
#include "NGAW2Converter.h"
#include "RecordSelectionWidget.h"
#include "RuptureWidget.h"
#include "SimCenterPreferences.h"
#include "SiteConfigWidget.h"
#include "SiteGridWidget.h"
#include "SiteScatterWidget.h"
#include "SiteWidget.h"
#include "SpatialCorrelationWidget.h"
#include "VisualizationWidget.h"
#include "Vs30Widget.h"
#include "WorkflowAppR2D.h"
#include "PeerNgaWest2Client.h"
#include "PeerLoginDialog.h"
#include "ZipUtils.h"
#include "QGISSiteInputWidget.h"

#ifdef INCLUDE_USER_PASS
#include "R2DUserPass.h"
#else
#include "SampleUserPass.h"
#endif

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QList>
#include <QPlainTextEdit>
#include <QDialog>
#include <QJsonObject>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStringList>
#include <QString>
#include <QTabWidget>
#include <QScrollArea>

// GIS includes
#include "SimCenterMapcanvasWidget.h"
#include "QGISVisualizationWidget.h"
#include "MapViewWindow.h"
#include <qgsmapcanvas.h>
#include <qgsvectorlayer.h>


GMWidget::GMWidget(VisualizationWidget* visWidget, QWidget *parent) : SimCenterAppWidget(parent), theVisualizationWidget(visWidget)
{
    mapViewSubWidget = nullptr;
    userGrid = nullptr;

    initAppConfig();

    simulationComplete = false;

    process = new QProcess(this);
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &GMWidget::handleProcessFinished);
    connect(process, &QProcess::readyReadStandardOutput, this, &GMWidget::handleProcessTextOutput);
    connect(process, &QProcess::started, this, &GMWidget::handleProcessStarted);


    // Adding Site Config Widget
    this->m_siteConfig = new SiteConfig(this);
    this->m_siteConfigWidget = new SiteConfigWidget(*m_siteConfig, visWidget);

    this->m_ruptureWidget = new RuptureWidget(this);

    this->m_gmpe = new GMPE(this);
    this->m_gmpeWidget = new GMPEWidget(*this->m_gmpe);

    this->m_intensityMeasure = new IntensityMeasure(this);
    this->m_intensityMeasureWidget = new IntensityMeasureWidget(*this->m_intensityMeasure);

    spatialCorrWidget = new SpatialCorrelationWidget();

    this->m_selectionconfig = new RecordSelectionConfig(this);
    this->m_selectionWidget = new RecordSelectionWidget(*this->m_selectionconfig);

    m_runButton = new QPushButton(tr("&Run Hazard Simulation"));
    //m_settingButton = new QPushButton(tr("&Path Settings"));

    // Adding vs30 widget
    this->m_vs30 = new Vs30(this);
    this->m_vs30Widget = new Vs30Widget(*this->m_vs30, *this->m_siteConfig);

    auto buttonsLayout = new QHBoxLayout();
    //buttonsLayout->addWidget(this->m_settingButton);
    buttonsLayout->addWidget(this->m_runButton);


    /*
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0,0,0,0);

    QVBoxLayout* lhs = new QVBoxLayout();

    m_ruptureWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);

    lhs->addWidget(m_siteConfigWidget);
    lhs->addWidget(m_vs30Widget);
    lhs->addWidget(spatialCorrWidget);
    lhs->addWidget(m_eventGMDir);

    QVBoxLayout* rhs = new QVBoxLayout();
    rhs->addWidget(m_ruptureWidget);
    rhs->addWidget(m_selectionWidget);
    rhs->addWidget(m_gmpeWidget);
    rhs->addWidget(m_intensityMeasureWidget);
    rhs->addLayout(buttonsLayout);

    mainLayout->addLayout(lhs);
    mainLayout->addLayout(rhs);

    */
    
    QTabWidget *theTabWidget = new QTabWidget();

    QWidget     *mainGroup = new QWidget();
    QGridLayout *mainLayout = new QGridLayout();
    
    QLabel *generalDescriptionLabel = new QLabel("\n Steps Involved in Obtaining a Set of Motions for a Set of Sites given a Rupture: "
                                                 "\n --> Earthquake - specyfying source and ground motion models"
                                                 "\n --> Sites - specifying locations and soil conditions for sites"
                                                 "\n --> Selection - specifying ground motion database and intensity measure correlations between sites for selected records");

    mainLayout->addWidget(generalDescriptionLabel, 0, 0);
    
    QWidget *earthquakeWidget = new QWidget();
    QVBoxLayout *earthquakeLayout=new QVBoxLayout();    
    earthquakeLayout->addWidget(m_ruptureWidget);
    earthquakeLayout->addWidget(m_gmpeWidget);
    earthquakeLayout->addWidget(m_intensityMeasureWidget);
    earthquakeWidget->setLayout(earthquakeLayout);
    earthquakeLayout->addStretch();
    
    theTabWidget->addTab(earthquakeWidget, "Earthquake");

    QWidget *siteWidget = new QWidget();
    QVBoxLayout *siteLayout=new QVBoxLayout();
    siteLayout->addWidget(m_siteConfigWidget);
    siteLayout->addWidget(m_vs30Widget);
    siteWidget->setLayout(siteLayout);
    siteLayout->addStretch();
    
    theTabWidget->addTab(siteWidget, "Sites");
    
    QWidget *imWidget = new QWidget();
    QVBoxLayout *imLayout=new QVBoxLayout();
    imLayout->addWidget(m_selectionWidget);
    imLayout->addWidget(spatialCorrWidget);
    imLayout->addWidget(m_eventGMDir);
    imLayout->addLayout(buttonsLayout);
    imLayout->addStretch();
    
    imWidget->setLayout(imLayout);
    theTabWidget->addTab(imWidget, "Record Selection");
    
    mainLayout->addWidget(theTabWidget, 1, 0);

    
    mainGroup->setLayout(mainLayout);
    //mainGroup->setMaximumWidth(windowWidth);

    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setLineWidth(1);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setWidget(mainGroup);
    // scrollArea->setMaximumWidth(windowWidth + 25);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(scrollArea);
    this->setLayout(layout);

    
    setupConnections();
}


GMWidget::~GMWidget()
{

}


void GMWidget::setAppConfig(void)
{
    GmAppConfigWidget* configWidget = new GmAppConfigWidget(m_appConfig, this);
    configWidget->show();
}


void GMWidget::setupConnections()
{
    // GMPE options (link between source type and GMPE options)
    connect(m_ruptureWidget, SIGNAL(widgetTypeChanged(QString)),
            m_gmpeWidget, SLOT(handleAvailableGMPE(QString)));

    // correlation model options (link between source type and correlation model options)
    connect(m_ruptureWidget, SIGNAL(widgetTypeChanged(QString)),
            spatialCorrWidget, SLOT(handleAvailableModel(QString)));

    // Intensity Measure Levels options (link between source type and intensity measure levels options)
    connect(m_ruptureWidget, SIGNAL(widgetTypeChanged(QString)),
            m_intensityMeasureWidget, SLOT(handleIntensityMeasureLevels(QString)));

    //Connecting the run button
    connect(m_runButton, &QPushButton::clicked, this, [this]()
    {

        // Get the type of site definition, i.e., single or grid
        auto type = m_siteConfig->getType();

        if(type == SiteConfig::SiteType::Single)
        {
            QString msg = "Single site selection not supported yet";
            this->infoMessage(msg);
            return;
        }
        else if(type == SiteConfig::SiteType::Grid)
        {
            if(!m_siteConfigWidget->getSiteGridWidget()->getGridCreated())
            {
                QString msg = "Please select a grid before continuing";
                this->statusMessage(msg);
                return;
            }
        }
        else if(type == SiteConfig::SiteType::Scatter)
        {
            if(!m_siteConfigWidget->getSiteScatterWidget()->siteFileExists())
            {
                QString msg = "Please choose a site file before continuing";
                this->statusMessage(msg);
                return;
            }
        }

        // kz: only contact PEER NGA when the databse is set to "NGA West"
        if(this->m_selectionconfig->getDatabase().compare("NGAWest2")==0) {
            QString userName = getPEERUserName();
            QString password = getPEERPassWord();
            peerClient.signIn(userName, password);
        }
        /***
        // Here you need the file "PEERUserPass.h", it is not included in the repo. Set your own username and password below.
        QString userName = getPEERUserName();
        QString password = getPEERPassWord();

        peerClient.signIn(userName, password);
        ***/


        runHazardSimulation();
    });

    connect(&peerClient, &PeerNgaWest2Client::statusUpdated, this, [this](QString statusUpdate)
    {
        this->statusMessage(statusUpdate);
    });

    //connect(m_settingButton, &QPushButton::clicked, this, &GMWidget::setAppConfig);

    // connect output director path change to EventGMDirWidget
    connect(m_appConfig, &GmAppConfig::outputDirectoryPathChanged, m_eventGMDir, &EventGMDirWidget::setEventFile);
    connect(m_appConfig, &GmAppConfig::outputDirectoryPathChanged, m_eventGMDir, &EventGMDirWidget::setMotionDir);

    connect(m_siteConfigWidget->getSiteGridWidget(), &SiteGridWidget::selectGridOnMap, this, &GMWidget::showGISWindow);


    connect(&peerClient, &PeerNgaWest2Client::recordsDownloaded, this, [this](QString zipFile)
    {
        this->parseDownloadedRecords(zipFile);
        this->getProgressDialog()->hideProgressBar();
    });

    // connect use event and ground motion dir to Hazard widget to switch
    connect(m_eventGMDir, &EventGMDirWidget::useEventFileMotionDir, this, &GMWidget::sendEventFileMotionDir);

    // connect configuration change to simulation tag updating
    connect(this, &GMWidget::configUpdated, this, &GMWidget::updateSimulationTag);

}


void GMWidget::initAppConfig()
{
    m_appConfig = new GmAppConfig(this);
    // Adding the EventGMDir widget
    this->m_eventGMDir = new EventGMDirWidget();

    //First, We will look into settings
    QSettings settings;
    m_appConfig->setWorkDirectoryPath(settings.value("WorkingDirectoryPath", "").toString());
    m_appConfig->setInputFilePath(settings.value("InputFilePath", "").toString());
    m_appConfig->setOutputFilePath(settings.value("OutputFilePath", "").toString());
    m_appConfig->setUsername(settings.value("RDBUsername", "").toString());
    m_appConfig->setPassword(settings.value("RDBPassword", "").toString());
    m_eventGMDir->setEventFile(settings.value("OutputFilePath", "").toString());
    m_eventGMDir->setMotionDir(settings.value("OutputFilePath", "").toString());

    //If path is missing, create a working directory in Applications working directory
    if(m_appConfig->getWorkDirectoryPath().isEmpty() || !QFile::exists(m_appConfig->getWorkDirectoryPath()))
    {

        QString workingDir = SimCenterPreferences::getInstance()->getLocalWorkDir();

        if(workingDir.isEmpty())
        {
            QString errorMessage = QString("Set the Local Jobs Directory location in preferences.");

            this->errorMessage(errorMessage);

            return;
        }

        QDir dirWork(workingDir);

        if (!dirWork.exists())
            if (!dirWork.mkpath(workingDir))
            {
                QString errorMessage = QString("Could not load the Working Directory: ") + workingDir
                        + QString(". Change the Local Jobs Directory location in preferences.");

                this->errorMessage(errorMessage);

                return;
            }

        // Store data in a ground motions folder under hazard simulation
        workingDir += QDir::separator() + QString("HazardSimulation") + QDir::separator() + QString("GroundMotions");

        m_appConfig->setWorkDirectoryPath(workingDir);

    }

    if(m_appConfig->getInputDirectoryPath().isEmpty() || !QFile::exists(m_appConfig->getInputDirectoryPath()))
    {

        QString workingDir = SimCenterPreferences::getInstance()->getLocalWorkDir();

        if(workingDir.isEmpty())
        {
            QString errorMessage = QString("Set the Local Jobs Directory location in preferences.");

            this->errorMessage(errorMessage);

            return;
        }

        QDir dirWork(workingDir);

        if (!dirWork.exists())
            if (!dirWork.mkpath(workingDir))
            {
                QString errorMessage = QString("Could not load the Input File Directory: ") + workingDir
                        + QString(". Change the Local Jobs Directory location in preferences.");

                this->errorMessage(errorMessage);

                return;
            }

        QString inputFilePath = workingDir + QDir::separator() + "HazardSimulation" +  QDir::separator() + "GroundMotions" +  QDir::separator() +  "Input";

        m_appConfig->setInputFilePath(inputFilePath);

    }

    if(m_appConfig->getOutputDirectoryPath().isEmpty() || !QFile::exists(m_appConfig->getOutputDirectoryPath()))
    {

        QString workingDir = SimCenterPreferences::getInstance()->getLocalWorkDir();

        if(workingDir.isEmpty())
        {
            QString errorMessage = QString("Set the Local Jobs Directory location in preferences.");

            this->errorMessage(errorMessage);

            return;
        }

        QDir dirWork(workingDir);

        if (!dirWork.exists())
            if (!dirWork.mkpath(workingDir))
            {
                QString errorMessage = QString("Could not load the Input File Directory: ") + workingDir
                        + QString(". Change the Local Jobs Directory location in preferences.");

                this->errorMessage(errorMessage);

                return;
            }

        QString outputFilePath = workingDir + QDir::separator() + "HazardSimulation" +  QDir::separator() + "GroundMotions" +  QDir::separator() +  "Output";

        m_appConfig->setOutputFilePath(outputFilePath);
        eventPath = outputFilePath + QDir::separator() + "EventGrid.csv";
        motionFolder = outputFilePath;
        m_eventGMDir->setEventFile(eventPath);
        m_eventGMDir->setMotionDir(motionFolder);
    }

}


void GMWidget::saveAppSettings(void)
{
    QSettings settings;
    settings.setValue("WorkingDirectoryPath", m_appConfig->getWorkDirectoryPath());
    settings.setValue("InputFilePath", m_appConfig->getInputDirectoryPath());
    settings.setValue("OutputFilePath", m_appConfig->getOutputDirectoryPath());
    settings.setValue("RDBUsername", m_appConfig->getUsername());
    settings.setValue("RDBPassword", m_appConfig->getPassword());
}


void GMWidget::resetAppSettings(void)
{
    QSettings settings;
    settings.setValue("WorkingDirectoryPath", "");
    settings.setValue("InputFilePath", "");
    settings.setValue("OutputFilePath", "");
    settings.setValue("RDBUsername", "");
    settings.setValue("RDBPassword","");
}


void GMWidget::showGISWindow(void)
{
    //    theVisualizationWidget->testNewMapCanvas();

    if(mapViewSubWidget == nullptr)
    {
        auto mapViewWidget = theVisualizationWidget->getMapViewWidget("Select grid on map");
        mapViewSubWidget = new MapViewWindow(mapViewWidget);

        auto mapCanvas = mapViewWidget->mapCanvas();

        userGrid = new RectangleGrid(mapCanvas);

        // Also important to get events from QGIS
        mapCanvas->setMapTool(userGrid);
        userGrid->createGrid();
        userGrid->setSiteGridConfig(m_siteConfig);
        userGrid->setVisualizationWidget(theVisualizationWidget);
    }


    mapViewSubWidget->show();
    userGrid->show();
}


bool
GMWidget::outputAppDataToJSON(QJsonObject &jsonObj) {

    emit eventTypeChangedSignal("Earthquake");

    jsonObj["Application"] = "EQSS";

    if(simulationComplete == false)
    {
        this->errorMessage("Earthquake Scenario Simulation is not completed - please click \"Run Hazard Simulation\"");
        return false;
    }
    QJsonObject appData;
    QFileInfo theFile(eventPath);
    qDebug() << eventPath;
    if (theFile.exists()) {
        appData["eventFile"]=theFile.fileName();
        appData["eventFileDir"]=theFile.path();
    } else {
        appData["eventFile"]=eventPath; // may be valid on others computer
        appData["eventFileDir"]=QString("");
    }
    QFileInfo theDir(motionFolder);
    qDebug() << motionFolder;
    if (theDir.exists()) {
        appData["motionDir"]=theDir.absoluteFilePath();
    } else {
        appData["motionDir"]=QString("None");
    }
    jsonObj["ApplicationData"]=appData;

    return true;
}

bool GMWidget::inputAppDataFromJSON(QJsonObject &jsonObj)
{
    if (jsonObj.contains("ApplicationData")) {
        QJsonObject appData = jsonObj["ApplicationData"].toObject();

        QString fileName;
        QString pathToFile;

        if (appData.contains("eventFile"))
            fileName = appData["eventFile"].toString();
        if (appData.contains("eventFileDir"))
            pathToFile = appData["eventFileDir"].toString();
        else
            pathToFile=QDir::currentPath();

        QString fullFilePath= pathToFile + QDir::separator() + fileName;

        if (!QFileInfo::exists(fullFilePath)){
            fullFilePath = pathToFile + QDir::separator() + "input_data" + QDir::separator() + fileName;

            if (!QFile::exists(fullFilePath)) {
                this->errorMessage("GMWidget - could not find event file");
                return false;
            }
        }

        eventPath = fullFilePath;
        m_eventGMDir->setEventFile(eventPath);

        if (appData.contains("motionDir"))
            motionFolder = appData["motionDir"].toString();

        QDir motionD(motionFolder);

        if (!motionD.exists()){
            QString trialDir = QDir::currentPath() +
                    QDir::separator() + "input_data" + motionFolder;
            if (motionD.exists(trialDir)) {
                motionFolder = trialDir;
                m_eventGMDir->setMotionDir(motionFolder);
            } else {
                this->errorMessage("GMWidget - could not find motion dir" + motionFolder + " " + trialDir);
                return false;
            }
        } else {
            m_eventGMDir->setMotionDir(motionFolder);
        }

        //this->loadUserGMData();

        return true;
    }

    return false;
}

bool GMWidget::outputToJSON(QJsonObject &jsonObj)
{

    QJsonObject unitsObj;

    // Only IMs
    if (m_selectionconfig->getDatabase().size() == 0)
    {
        auto IMType = m_intensityMeasure->type();
        unitsObj[IMType] = "g";
    }
    else // Time history download selected
    {
        unitsObj["TH_file"] = "g";
        unitsObj["factor"] = "scalar";
    }

    jsonObj["units"] = unitsObj;

    return true;
}


bool GMWidget::inputFromJSON(QJsonObject &/*jsonObject*/){

    return true;
}


void GMWidget::runHazardSimulation(void)
{

    simulationComplete = false;

    QString pathToGMFilesDirectory = m_appConfig->getOutputDirectoryPath() + QDir::separator();

    // Remove old csv files in the output folder
    const QFileInfo existingFilesInfo(pathToGMFilesDirectory);

    // Get the existing files in the folder to see if we already have the record
    QStringList acceptableFileExtensions = {"*.csv"};
    QStringList existingCSVFiles = existingFilesInfo.dir().entryList(acceptableFileExtensions, QDir::Files);

    // Remove the csv files - in case we have less sites that previous and they are not overwritten
    for(auto&& it : existingCSVFiles)
    {
        QFile file(pathToGMFilesDirectory + it);
        file.remove();
    }

    // Clean out any existing input files
    auto pathToInputDir = m_appConfig->getInputDirectoryPath() + QDir::separator();

    QStringList nameFilters = {"SiteFile.csv","OpenQuakeSiteModel.csv","sites_oq.csv","EQHazardConfiguration.json","oq_job.ini","rupture_model_example.xml"};

    QDir dir(pathToInputDir);
    dir.setNameFilters(nameFilters);
    dir.setFilter(QDir::Files);
    foreach(QString dirFile, dir.entryList())
    {
        dir.remove(dirFile);
    }

    // Remove the grid from the visualization screen
    if(userGrid)
        userGrid->hide();

    // First check if the settings are valid
    QString err;
    if(!m_appConfig->validate(err))
    {
        this->errorMessage(err);
        return;
    }

    //int maxID = m_siteConfig->siteGrid().getNumSites() - 1;
    int minID = 0;
    int maxID = 1;
    if(m_siteConfig->getType() == SiteConfig::SiteType::Grid)
    {
        maxID = m_siteConfig->siteGrid().getNumSites() - 1;
    }
    else if(m_siteConfig->getType() == SiteConfig::SiteType::Scatter)
    {
        minID = m_siteConfigWidget->getSiteScatterWidget()->getMinID();
        maxID = m_siteConfigWidget->getSiteScatterWidget()->getMaxID();
    }
    else if(m_siteConfig->getType() == SiteConfig::SiteType::UserCSV)
    {
        QString filterIDs = m_siteConfigWidget->getCsvSiteWidget()->getFilterString();
        if (filterIDs.isEmpty())
        {
            this->statusMessage("Warning: no filters defined - will load all sites.");
            m_siteConfigWidget->getCsvSiteWidget()->selectAllComponents();
            filterIDs = m_siteConfigWidget->getCsvSiteWidget()->getFilterString();
        }
        QStringList IDs = filterIDs.split(QRegExp(",|-"), QString::SkipEmptyParts);
        qDebug() << IDs;
        int tmpMin = 10000000;
        int tmpMax = 0;
        for (int i = 0; i < IDs.size(); i++) {
            if (IDs[i].toInt() > tmpMax)
                tmpMax = IDs[i].toInt();
            if (IDs[i].toInt() < tmpMin)
                tmpMin = IDs[i].toInt();
        }
        minID = tmpMin;
        maxID = tmpMax;
    }

    //    maxID = 5;


    QJsonObject siteObj;
    siteObj.insert("Type", "From_CSV");
    siteObj.insert("input_file", "SiteFile.csv");
    siteObj.insert("min_ID", minID);
    siteObj.insert("max_ID", maxID);
    // add an output_file field for preparing OpenQuake site model
    siteObj.insert("output_file", "OpenQuakeSiteModel.csv");

    QJsonObject scenarioObj;
    scenarioObj.insert("Type", "Earthquake");
    // get scenario number
    QString numEQ = this->m_ruptureWidget->getEQNum();
    if (numEQ.compare("All")==0) {
        scenarioObj.insert("Number", "All");
    } else {
        scenarioObj.insert("Number", numEQ.toInt());
    }
    scenarioObj.insert("Generator", "Selection");

    QJsonObject EqRupture;
    m_ruptureWidget->outputToJSON(EqRupture);

    // number of scenarios for ERF widget
    if (m_ruptureWidget->getWidgetType().compare("OpenSHA ERF")==0)
    {
        if (EqRupture.contains("Number"))
            scenarioObj["Number"] = EqRupture["Number"];
    }

    if(EqRupture.isEmpty())
    {
        QString err = "Error in getting the earthquake rupture .JSON";
        this->errorMessage(err);
        return;
    }

    scenarioObj.insert("EqRupture",EqRupture);

    // Get the GMPE Json object
    QJsonObject GMPEobj;
    qDebug() << QString(m_ruptureWidget->getWidgetType());
    if (m_ruptureWidget->getWidgetType().compare("OpenQuake Classical")==0 || m_ruptureWidget->getWidgetType().compare("OpenQuake User-Specified")==0)
    {
        GMPEobj.insert("Type", "LogicTree");
        GMPEobj.insert("Parameters", m_ruptureWidget->getGMPELogicTree());
    }
    else
    {
        m_gmpe->outputToJSON(GMPEobj);
    }

    // Get the Vs30 Json object
    QJsonObject Vs30obj;
    m_vs30->outputToJSON(Vs30obj);
    siteObj.insert("Vs30", Vs30obj);

    // Get the correlation model Json object
    auto corrModObj = spatialCorrWidget->getJsonCorr();

    // Get the scaling Json object
    auto scalingObj = spatialCorrWidget->getJsonScaling();

    // Get the intensity measure Json object
    auto IMObj = m_intensityMeasure->getJson();

    auto numGM =  m_selectionWidget->getNumberOfGMPerSite();

    if(numGM == -1)
    {
        QString err = "Error in getting the number of ground motions at a site";
        this->errorMessage(err);
        return;
    }

    QJsonObject eventObj;
    eventObj.insert("NumberPerSite", numGM);
    eventObj.insert("GMPE", GMPEobj);
    eventObj.insert("CorrelationModel", corrModObj);
    eventObj.insert("IntensityMeasure", IMObj);
    eventObj.insert("ScalingFactor", scalingObj);
    eventObj.insert("SaveIM", true);
    eventObj.insert("Database",  m_selectionconfig->getDatabase());
    eventObj.insert("OutputFormat", "SimCenterEvent");

    QJsonObject configFile;
    configFile.insert("Site",siteObj);
    configFile.insert("Scenario",scenarioObj);
    configFile.insert("Event",eventObj);
    configFile.insert("Directory",m_appConfig->getJson());

    // Get the type of site definition, i.e., single or grid
    auto type = m_siteConfig->getType();

    QVector<QStringList> gridData;

    QStringList headerRow = {"Station", "Latitude", "Longitude"};

    gridData.push_back(headerRow);

    bool writeSiteFile = true;

    if(type == SiteConfig::SiteType::Single)
    {
        //qDebug()<<"Single site selection not supported yet";
        QStringList stationRow;

        // The station id
        stationRow.push_back(QString::number(0));

        // The latitude and longitude
        auto longitude = m_siteConfig->site().location().longitude();
        auto latitude = m_siteConfig->site().location().latitude();

        stationRow.push_back(QString::number(latitude));
        stationRow.push_back(QString::number(longitude));

        gridData.push_back(stationRow);
    }
    else if(type == SiteConfig::SiteType::Grid)
    {
        if(!m_siteConfigWidget->getSiteGridWidget()->getGridCreated())
        {
            QString msg = "Select a grid before continuing";
            this->statusMessage(msg);
            return;
        }

        // Get the vector of grid nodes

        if(userGrid == nullptr)
        {
            QString err = "Please create a user-grid";
            this->errorMessage(err);
            return;
        }

        auto gridNodeVec = userGrid->getGridNodeVec();
        auto mapCanvas = mapViewSubWidget->getMapCanvasWidget()->mapCanvas();

        for(int i = 0; i<gridNodeVec.size(); ++i)
        {
            auto gridNode = gridNodeVec.at(i);

            QStringList stationRow;

            // The station id
            stationRow.push_back(QString::number(i));

            auto screenPoint = gridNode->getPoint();

            // The latitude and longitude
            auto longitude = theVisualizationWidget->getLongFromScreenPoint(screenPoint,mapCanvas);
            auto latitude = theVisualizationWidget->getLatFromScreenPoint(screenPoint,mapCanvas);

            stationRow.push_back(QString::number(latitude));
            stationRow.push_back(QString::number(longitude));

            gridData.push_back(stationRow);
        }
    }
    else if(type == SiteConfig::SiteType::Scatter)
    {
        // Site file will be copied to the input directory
        writeSiteFile = false;

        if(!m_siteConfigWidget->getSiteScatterWidget()->copySiteFile(pathToInputDir))
        {
            this->errorMessage("Error copying site file to inputput directory");
        }
    }
    else if(type == SiteConfig::SiteType::UserCSV)
    {
        // Site file will be copied to the input directory
        writeSiteFile = false;
        if(!m_siteConfigWidget->getCsvSiteWidget()->copyFiles(pathToInputDir))
        {
            this->errorMessage("Error copying site file to inputput directory");
        }
        else
        {
            QFileInfo csv_origin(m_siteConfigWidget->getCsvSiteWidget()->getPathToComponentFile());
            QFile csv_file(pathToInputDir + QDir::separator() + csv_origin.fileName());
            if (!csv_file.fileName().contains("SiteFile.csv", Qt::CaseSensitive))
            {
                QFileInfo csv_t(pathToInputDir + QDir::separator() + "SiteFile.csv");
                if (csv_t.exists() && csv_t.isFile())
                {
                    QFile csv_tf(pathToInputDir + QDir::separator() + "SiteFile.csv");
                    csv_tf.remove();
                }
                csv_file.rename(pathToInputDir + QDir::separator() + "SiteFile.csv");
            }
        }
    }

    if(writeSiteFile)
    {
        QString pathToSiteLocationFile = pathToInputDir + QDir::separator() + "SiteFile.csv";

        CSVReaderWriter csvTool;

        auto res = csvTool.saveCSVFile(gridData, pathToSiteLocationFile, err);

        if(res != 0)
        {
            this->errorMessage(err);
            return;
        }
    }

    QString strFromObj = QJsonDocument(configFile).toJson(QJsonDocument::Indented);

    // Hazard sim
    QString pathToConfigFile = pathToInputDir + QDir::separator() + "EQHazardConfiguration.json";

    QFile file(pathToConfigFile);

    if(!file.open(QIODevice::WriteOnly))
    {
        file.close();
    }
    else
    {
        QTextStream out(&file); out << strFromObj;
        file.close();
    }

    auto pythonPath = SimCenterPreferences::getInstance()->getPython();

    // TODO: make this a relative link once we figure out the folder structure
    // auto pathToHazardSimScript = "/Users/steve/Desktop/SimCenter/HazardSimulation/HazardSimulation.py";
    // auto pathToHazardSimScript = "/Users/fmckenna/release/HazardSimulation/HazardSimulation.py";
    QString pathToHazardSimScript = SimCenterPreferences::getInstance()->getAppDir() + QDir::separator()
            + "applications" + QDir::separator() + "performRegionalEventSimulation" + QDir::separator()
            + "regionalGroundMotion" + QDir::separator() + "HazardSimulation.py";

    QFileInfo hazardFileInfo(pathToHazardSimScript);
    if (!hazardFileInfo.exists()) {
        QString errorMessage = QString("ERROR - hazardApp does not exist") + pathToHazardSimScript;
        this->errorMessage(errorMessage);
        qDebug() << errorMessage;
        return;
    }
    QStringList args = {pathToHazardSimScript,"--hazard_config",pathToConfigFile};

    qDebug()<<"Hazard Simulation Command:"<<args[0]<<" "<<args[1]<<" "<<args[2];

    this->getProgressDialog()->showProgressBar();

    process->start(pythonPath, args);
    //process->waitForStarted();
    process->waitForFinished();
}


void GMWidget::handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    this->m_runButton->setEnabled(true);

    if(exitStatus == QProcess::ExitStatus::CrashExit)
    {
        QString errText("Error, the process running the hazard simulation script crashed");
        this->errorMessage(errText);
        this->getProgressDialog()->hideProgressBar();

        return;
    }

    if(exitCode != 0)
    {
        QString errText("An error occurred in the Hazard Simulation script, the exit code is " + QString::number(exitCode));
        this->errorMessage(errText);
        this->getProgressDialog()->hideProgressBar();

        return;
    }

    // Checking if the ground motion selection is requested by the user
    if (m_selectionconfig->getDatabase().size() == 0)
    {
        // ground motion selection is not requested -> completed the job
        this->statusMessage("The folder containing the results: "+m_appConfig->getOutputDirectoryPath() + "\n");
        this->statusMessage("Earthquake hazard simulation complete.\n");
        simulationComplete = true;
        // Saving the event grid path
        auto eventGridFile = m_appConfig->getOutputDirectoryPath() + QDir::separator() + QString("EventGrid.csv");
        emit outputDirectoryPathChanged(m_appConfig->getOutputDirectoryPath(), eventGridFile);

        // Load the results
        QString errMsg;
        auto res = this->processDownloadedRecords(errMsg);
        if(res != 0)
            this->errorMessage("Failed to process event grid file with the following error: " + errMsg);

        this->getProgressDialog()->hideProgressBar();

        return;
    }

    this->statusMessage("Contacting PEER server to download ground motion records.");

    QApplication::processEvents();

    numDownloaded = 0;
    downloadComplete = false;
    recordsListToDownload.clear();
    NGA2Results = QJsonObject();

    auto res = this->downloadRecords();

    if(res != 0)
    {
        QString errText("Error downloading the ground motion records");
        this->errorMessage(errText);
        this->getProgressDialog()->hideProgressBar();
    }

}


void GMWidget::handleProcessStarted(void)
{
    this->statusMessage("Running script in the background");
    this->m_runButton->setEnabled(false);
}


void GMWidget::handleProcessTextOutput(void)
{
    QByteArray output = process->readAllStandardOutput();

    this->statusMessage(QString(output));
}


int GMWidget::downloadRecords(void)
{
    QString pathToGMFilesDirectory = m_appConfig->getOutputDirectoryPath() + QDir::separator();

    // read file of selected record and create a list containing records to download
    recordsListToDownload.clear();

    QStringList recordsToDownload;

    QString recordsListFilename = QString(pathToGMFilesDirectory + QString("RSN.csv"));
    QFile theRecordsListFile = QFile(recordsListFilename);

    if (!theRecordsListFile.exists())
    {
        QString errorMessage = QString("GMWidget::Record selection failed, no file ") +  recordsListFilename + QString(" exists");
        this->errorMessage(errorMessage);
        return -1;
    }

    if (theRecordsListFile.open(QIODevice::ReadOnly))
    {
        //file opened successfully, parse csv file for records
        while (!theRecordsListFile.atEnd())
        {
            QByteArray line = theRecordsListFile.readLine();
            foreach (const QByteArray &item, line.split(','))
            {
                recordsToDownload.append(QString::fromLocal8Bit(item).trimmed()); // Assuming local 8-bit.
            }
        }
    }

    theRecordsListFile.close();

    qDebug()<<QString::number(recordsListToDownload.empty());

    // Check if any of the records exist, do not need to download them again
    const QFileInfo existingFilesInfo(pathToGMFilesDirectory);

    // Get the existing files in the folder to see if we already have the record
    QStringList acceptableFileExtensions = {"*.json"};
    QStringList existingFiles = existingFilesInfo.dir().entryList(acceptableFileExtensions, QDir::Files);

    if(!existingFiles.empty())
    {
        for(auto&& it : recordsToDownload)
        {
            auto fileToCheck = "RSN" + it + ".json";
            if(!existingFiles.contains(fileToCheck))
                recordsListToDownload.append(it);
        }
    }
    else
    {
        recordsListToDownload = recordsToDownload;
    }


    qDebug()<<QString::number(recordsListToDownload.isEmpty());
    qDebug()<<QString::number(recordsListToDownload.empty());
    if (!recordsListToDownload.isEmpty())
    {
            this->downloadRecordBatch();
    }

    return 0;
}


void GMWidget::downloadRecordBatch(void)
{
    if(recordsListToDownload.empty())
        return;

    auto maxBatchSize = 100;

    if(recordsListToDownload.size() < maxBatchSize)
    {
        peerClient.selectRecords(recordsListToDownload);
        numDownloaded = recordsListToDownload.size();

        recordsListToDownload.clear();
    }
    else
    {
        auto recordsBatch = recordsListToDownload.mid(0,maxBatchSize-1);

        numDownloaded += recordsBatch.size();

        peerClient.selectRecords(recordsBatch);

        recordsListToDownload = recordsListToDownload.mid(maxBatchSize-1,recordsListToDownload.size()-1);
    }
}

int GMWidget::processDownloadedRecords(QString& errorMessage)
{

    auto qgisVizWidget = static_cast<QGISVisualizationWidget*>(theVisualizationWidget);

    if(qgisVizWidget == nullptr)
    {
        qDebug()<<"Failed to cast to QGISVisualizationWidget";
        return -1;
    }

    QString pathToOutputDirectory = m_appConfig->getOutputDirectoryPath() + QDir::separator();

    // Account for the different directory structure if only want IMs
    if(m_selectionconfig->getDatabase().size() == 0)
        pathToOutputDirectory += "IMs" + QString(QDir::separator());

    pathToOutputDirectory += "EventGrid.csv";

    const QFileInfo inputFile(pathToOutputDirectory);

    if (!inputFile.exists() || !inputFile.isFile())
    {
        errorMessage ="A file does not exist at the path: " + pathToOutputDirectory;
        return -1;
    }

    QStringList acceptableFileExtensions = {"*.CSV", "*.csv"};

    QStringList inputFiles = inputFile.dir().entryList(acceptableFileExtensions,QDir::Files);

    if(inputFiles.empty())
    {
        errorMessage ="No files with .csv extensions were found at the path: "+pathToOutputDirectory;
        return -1;
    }

    // QString fileName = inputFile.fileName();

    CSVReaderWriter csvTool;

    QString err;
    QVector<QStringList> data = csvTool.parseCSVFile(pathToOutputDirectory,err);

    if(!err.isEmpty())
    {
        errorMessage = err;
        return -1;
    }

    if(data.empty())
        return -1;

    auto motionDir = inputFile.dir().absolutePath() ;

    QApplication::processEvents();

    this->getProgressDialog()->setProgressBarRange(0,inputFiles.size());
    this->getProgressDialog()->setProgressBarValue(0);

    QApplication::processEvents();

    // Get the headers in the first station file - assume that the rest will be the same
    auto rowStr = data.at(1);
    auto stationName = rowStr[0];

    // Path to station files, e.g., site0.csv
    auto stationFilePath = motionDir + QDir::separator() + stationName;

    QString err2;
    QVector<QStringList> sampleStationData = csvTool.parseCSVFile(stationFilePath,err);

    // Return if there is an error or the station data is empty
    if(!err2.isEmpty())
    {
        errorMessage = "Could not parse the first station with the following error: "+err2;
        return -1;
    }

    if(sampleStationData.size() < 2)
    {
        errorMessage = "The file " + stationFilePath + " is empty";
        return -1;
    }

    // Get the header file
    auto stationDataHeadings = sampleStationData.first();

    // Create the fields
    QList<QgsField> attribFields;
    attribFields.push_back(QgsField("AssetType", QVariant::String));
    attribFields.push_back(QgsField("TabName", QVariant::String));
    attribFields.push_back(QgsField("Station Name", QVariant::String));
    attribFields.push_back(QgsField("Latitude", QVariant::Double));
    attribFields.push_back(QgsField("Longitude", QVariant::Double));

    for(auto&& it : stationDataHeadings)
        attribFields.push_back(QgsField(it, QVariant::String));


    // Pop off the row that contains the header information
    data.pop_front();

    auto numRows = data.size();

    int count = 0;

    auto maxToDisp = 20;

    QgsFeatureList featureList;
    // Get the data
    for(int i = 0; i<numRows; ++i)
    {
        auto rowStr = data.at(i);

        auto stationName = rowStr[0];

        // Path to station files, e.g., site0.csv
        auto stationPath = motionDir + QDir::separator() + stationName;

        bool ok;
        auto lon = rowStr[1].toDouble(&ok);

        if(!ok)
        {
            errorMessage = "Error longitude to a double, check the value";
            return -1;
        }

        auto lat = rowStr[2].toDouble(&ok);

        if(!ok)
        {
            errorMessage = "Error latitude to a double, check the value";
            return -1;
        }

        GroundMotionStation GMStation(stationPath,lat,lon);

        try
        {
            GMStation.importGroundMotions();
        }
        catch(QString msg)
        {
            errorMessage = "Error importing ground motion file: " + stationName+"\n"+msg;
            return -1;
        }

        auto stationData = GMStation.getStationData();

        // create the feature attributes
        QgsAttributes featAttributes(attribFields.size());

        auto latitude = GMStation.getLatitude();
        auto longitude = GMStation.getLongitude();

        featAttributes[0] = "GroundMotionGridPoint";     // "AssetType"
        featAttributes[1] = "Ground Motion Grid Point";  // "TabName"
        featAttributes[2] = stationName;                 // "Station Name"
        featAttributes[3] = latitude;                    // "Latitude"
        featAttributes[4] = longitude;                   // "Longitude"

        // The number of headings in the file
        auto numParams = stationData.front().size();

        maxToDisp = (maxToDisp<stationData.size() ? maxToDisp : stationData.size());

        QVector<QString> dataStrs(numParams);

        for(int i = 0; i<maxToDisp-1; ++i)
        {
            auto stationParams = stationData[i];

            for(int j = 0; j<numParams; ++j)
            {
                dataStrs[j] += stationParams[j] + ", ";
            }
        }

        for(int j = 0; j<numParams; ++j)
        {
            auto str = dataStrs[j] ;
            str += stationData[maxToDisp-1][j];

            if(maxToDisp<stationData.size())
                str += "...";

            featAttributes[5+j] = str;
        }

        // Create the feature
        QgsFeature feature;
        feature.setGeometry(QgsGeometry::fromPointXY(QgsPointXY(longitude,latitude)));
        feature.setAttributes(featAttributes);
        featureList.append(feature);

        ++count;
        this->getProgressDialog()->setProgressBarValue(count);

        QApplication::processEvents();
    }


    auto vectorLayer = qgisVizWidget->addVectorLayer("Point", "Ground Motion Grid");

    if(vectorLayer == nullptr)
    {
        errorMessage = "Error creating a layer";
        return -1;
    }

    auto dProvider = vectorLayer->dataProvider();
    auto res = dProvider->addAttributes(attribFields);

    if(!res)
    {
        errorMessage = "Error adding attribute fields to layer";
        qgisVizWidget->removeLayer(vectorLayer);
        return -1;
    }

    vectorLayer->updateFields(); // tell the vector layer to fetch changes from the provider

    dProvider->addFeatures(featureList);
    vectorLayer->updateExtents();

    qgisVizWidget->createSymbolRenderer(Qgis::MarkerShape::Cross,Qt::black,2.0,vectorLayer);

    return 0;
}


int GMWidget::parseDownloadedRecords(QString zipFile)
{
    QString pathToOutputDirectory = m_appConfig->getOutputDirectoryPath();
    bool result =  ZipUtils::UnzipFile(zipFile, pathToOutputDirectory);
    if (result == false)
    {
        QString errMsg = "Error in unziping the downloaded ground motion files";
        this->errorMessage(errMsg);
        return -1;
    }

    NGAW2Converter tool;

    // Import the search results overview file provided by the PEER Ground Motion Database for this batch - this file will get overwritten on the next batch
    QString errMsg;
    auto res1 = tool.parseNGAW2SearchResults(pathToOutputDirectory,NGA2Results,errMsg);
    if(res1 != 0)
    {
        this->errorMessage(errMsg);
        return -1;
    }

    // if more records to download due to 100 record limit .. go download them
    if (!recordsListToDownload.empty())
    {
        this->downloadRecordBatch();
        return 0;
    }

    QJsonObject createdRecords;
    auto res = tool.convertToSimCenterEvent(pathToOutputDirectory + QDir::separator(), NGA2Results, errMsg, &createdRecords);
    if(res != 0)
    {
        if(res == -2)
        {
            errMsg.prepend("Error downloading ground motion files from PEER server.\n");
        }

        this->errorMessage(errMsg);
        return res;
    }

    auto res2 = this->processDownloadedRecords(errMsg);
    if(res2 != 0)
    {
        this->errorMessage("Failed to process event grid file with the following error: " + errMsg);
        return res2;
    }

    this->statusMessage("Download and parsing of ground motion records complete.");

    this->statusMessage("The folder containing the results: "+m_appConfig->getOutputDirectoryPath());

    this->statusMessage("Earthquake hazard simulation complete.");

    simulationComplete = true;
    auto eventGridFile = m_appConfig->getOutputDirectoryPath() + QDir::separator() + QString("EventGrid.csv");

    emit outputDirectoryPathChanged(m_appConfig->getOutputDirectoryPath(), eventGridFile);

    this->getProgressDialog()->hideProgressBar();

    return 0;
}

GmAppConfig *GMWidget::appConfig() const
{
    return m_appConfig;
}

void GMWidget::sendEventFileMotionDir(const QString &eventFile, const QString &motionDir)
{
    eventPath = eventFile;
    motionFolder = motionDir;
    m_appConfig->setOutputFilePath(motionDir);
    emit useEventFileMotionDir(motionDir, eventFile);
}

bool GMWidget::copyFiles(QString &destDir)
{
    // checking if the simulation is done
    // this helps to avoid copy previous run's files
    if (!simulationComplete) {
        // try to run the Hazard Simulation first
        m_runButton->animateClick();
    }
    // create dir and copy motion files
    QDir destDIR(destDir);
    if (!destDIR.exists()) {
        qDebug() << "GMWidget::copyFiles dest dir does not exist: " << destDir;
        return false;
    }

    QFileInfo eventFileInfo(eventPath);
    if (eventFileInfo.exists()) {
        this->copyFile(eventPath, destDir);
    } else {
        qDebug() << "GMWidget::copyFiles eventFile does not exist: " << eventPath;
        return false;
    }

    QDir motionDirInfo(motionFolder);
    if (motionDirInfo.exists()) {
        return this->copyPath(motionFolder, destDir, false);
    } else {
        qDebug() << "GMWidget::copyFiles motionFolder does not exist: " << motionFolder;
        return false;
    }

    // should never get here
    return false;
}

void GMWidget::updateSimulationTag()
{
    simulationComplete = false;
}

bool GMWidget::getSimulationStatus()
{
    bool tmp = simulationComplete;
    return tmp;
}

