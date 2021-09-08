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
#include "Utils/PythonProgressDialog.h"
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


#ifdef ARC_GIS
// GIS includes
#include "MapGraphicsView.h"
#include "Map.h"
#include "Point.h"
#include "FeatureCollection.h"
#include "FeatureCollectionLayer.h"
#include "SimpleRenderer.h"
#include "SimpleMarkerSymbol.h"
#include "LayerTreeView.h"
#include "ArcGISVisualizationWidget.h"
using namespace Esri::ArcGISRuntime;
#endif

GMWidget::GMWidget(QWidget *parent, VisualizationWidget* visWidget) : SimCenterAppWidget(parent), theVisualizationWidget(visWidget)
{
    initAppConfig();

    simulationComplete = false;

    process = new QProcess(this);
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &GMWidget::handleProcessFinished);
    connect(process, &QProcess::readyReadStandardOutput, this, &GMWidget::handleProcessTextOutput);
    connect(process, &QProcess::started, this, &GMWidget::handleProcessStarted);

    QGridLayout* toolsGridLayout = new QGridLayout(this);
    toolsGridLayout->setContentsMargins(0,0,0,0);

    // Adding Site Config Widget
    this->m_siteConfig = new SiteConfig();
    this->m_siteConfigWidget = new SiteConfigWidget(*m_siteConfig);

    this->m_ruptureWidget = new RuptureWidget(this);

    this->m_gmpe = new GMPE();
    this->m_gmpeWidget = new GMPEWidget(*this->m_gmpe, this);

    this->m_intensityMeasure = new IntensityMeasure();
    this->m_intensityMeasureWidget = new IntensityMeasureWidget(*this->m_intensityMeasure, this);

    spatialCorrWidget = new SpatialCorrelationWidget(this);

    this->m_selectionconfig = new RecordSelectionConfig();
    this->m_selectionWidget = new RecordSelectionWidget(*this->m_selectionconfig, this);

    m_runButton = new QPushButton(tr("&Run Hazard Simulation"));
    m_settingButton = new QPushButton(tr("&Settings"));

    // Create a map view that will be used for selecting the grid points
    mapViewSubWidget = std::make_unique<MapViewSubWidget>(nullptr);

    // Adding vs30 widget
    this->m_vs30 = new Vs30();
    this->m_vs30Widget = new Vs30Widget(*this->m_vs30, *this->m_siteConfig, this);

    auto userGrid = mapViewSubWidget->getGrid();
    userGrid->createGrid();
    userGrid->setSiteGridConfig(m_siteConfig);
    userGrid->setVisualizationWidget(theVisualizationWidget);

    auto buttonsLayout = new QHBoxLayout();
    buttonsLayout->addWidget(this->m_settingButton);
    buttonsLayout->addWidget(this->m_runButton);


    toolsGridLayout->addWidget(this->m_siteConfigWidget, 0,0,2,1);
    toolsGridLayout->addWidget(this->m_ruptureWidget, 3,0,4,1);

    toolsGridLayout->addWidget(this->spatialCorrWidget,  0,1);
    toolsGridLayout->addWidget(this->m_selectionWidget,  1,1);
    toolsGridLayout->addWidget(this->m_vs30Widget, 3,1,1,1); // vs30 widget
    toolsGridLayout->addWidget(this->m_gmpeWidget,        4,1,1,1);
    toolsGridLayout->addWidget(this->m_intensityMeasureWidget,5,1,1,1);
    toolsGridLayout->addLayout(buttonsLayout,6,1,1,1);

    toolsGridLayout->setHorizontalSpacing(5);
    toolsGridLayout->setVerticalSpacing(0);

    this->setLayout(toolsGridLayout);

    setupConnections();

    //Test
    //    // Here you need the file "PEERUserPass.h", it is not included in the repo. Set your own username and password below.
    //    QString userName = getPEERUserName();
    //    QString password = getPEERPassWord();

    //    peerClient.signIn(userName, password);
    //    this->showInfoDialog();

    //    this->handleProcessFinished(0,QProcess::NormalExit);

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

        // Here you need the file "PEERUserPass.h", it is not included in the repo. Set your own username and password below.
        QString userName = getPEERUserName();
        QString password = getPEERPassWord();

        peerClient.signIn(userName, password);

        runHazardSimulation();
    });

    connect(&peerClient, &PeerNgaWest2Client::statusUpdated, this, [this](QString statusUpdate)
    {
        this->statusMessage(statusUpdate);
    });

    connect(m_settingButton, &QPushButton::clicked, this, &GMWidget::setAppConfig);

    connect(m_siteConfigWidget->getSiteGridWidget(), &SiteGridWidget::selectGridOnMap, this, &GMWidget::showGISWindow);


    connect(&peerClient, &PeerNgaWest2Client::recordsDownloaded, this, [this](QString zipFile)
    {
       this->parseDownloadedRecords(zipFile);
       this->getProgressDialog()->hideProgressBar();
    });

}


void GMWidget::initAppConfig()
{

    m_appConfig = new GmAppConfig(this);


    //First, We will look into settings
    QSettings settings;
    m_appConfig->setWorkDirectoryPath(settings.value("WorkingDirectoryPath", "").toString());
    m_appConfig->setInputFilePath(settings.value("InputFilePath", "").toString());
    m_appConfig->setOutputFilePath(settings.value("OutputFilePath", "").toString());
    m_appConfig->setUsername(settings.value("RDBUsername", "").toString());
    m_appConfig->setPassword(settings.value("RDBPassword", "").toString());

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
    mapViewSubWidget->show();
    mapViewSubWidget->addGridToScene();

    mapViewSubWidget->setWindowTitle(QApplication::translate("toplevel", "Select ground motion grid"));
}


bool
GMWidget::outputAppDataToJSON(QJsonObject &jsonObj) {
    jsonObj["Application"] = "EQSS";

    QJsonObject appData;
    jsonObj["ApplicationData"]=appData;

    return true;
}

bool GMWidget::outputToJSON(QJsonObject &jsonObj)
{
    /*
    if(simulationComplete == false)
    {
        return true;
    }

    auto pathToEventGridFolder = m_appConfig->getOutputDirectoryPath() + QDir::separator();

    jsonObj.insert("pathEventData", pathToEventGridFolder);
    */

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
    mapViewSubWidget->removeGridFromScene();

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
    scenarioObj.insert("Number", 1);
    scenarioObj.insert("Generator", "Selection");

    QJsonObject EqRupture;
    m_ruptureWidget->outputToJSON(EqRupture);

    if(EqRupture.isEmpty())
    {
        QString err = "Error in getting the earthquake rupture .JSON";
        this->errorMessage(err);
        return;
    }

    scenarioObj.insert("EqRupture",EqRupture);

    // Get the GMPE Json object
    QJsonObject GMPEobj;
    m_gmpe->outputToJSON(GMPEobj);

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

    auto numGM =  m_siteConfigWidget->getNumberOfGMPerSite();

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
        qDebug()<<"Single site selection not supported yet";
    }
    else if(type == SiteConfig::SiteType::Grid)
    {
        if(!m_siteConfigWidget->getSiteGridWidget()->getGridCreated())
        {
            QString msg = "Select a grid before continuing";
            this->statusMessage(msg);
            return;
        }

        // Create the objects needed to visualize the grid in the GIS
        auto siteGrid = mapViewSubWidget->getGrid();

        // Get the vector of grid nodes
        auto gridNodeVec = siteGrid->getGridNodeVec();

        for(int i = 0; i<gridNodeVec.size(); ++i)
        {
            auto gridNode = gridNodeVec.at(i);

            QStringList stationRow;

            // The station id
            stationRow.push_back(QString::number(i));

            auto screenPoint = gridNode->getPoint();

            // The latitude and longitude
            auto longitude = theVisualizationWidget->getLongFromScreenPoint(screenPoint);
            auto latitude = theVisualizationWidget->getLatFromScreenPoint(screenPoint);

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
    process->waitForStarted();
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


    this->downloadRecordBatch();

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

#ifdef Q_GIS
int GMWidget::processDownloadedRecords(QString& errorMessage)
{

    qDebug()<<"GMWidget::processDownloadedRecords";

    return 0;
}
#endif

#ifdef ARC_GIS
int GMWidget::processDownloadedRecords(QString& errorMessage)
{

    auto arcVizWidget = static_cast<ArcGISVisualizationWidget*>(theVisualizationWidget);

    if(arcVizWidget == nullptr)
    {
        qDebug()<<"Failed to cast to ArcGISVisualizationWidget";
        return -1;
    }

    auto pathToOutputDirectory = m_appConfig->getOutputDirectoryPath() + QDir::separator() + "EventGrid.csv";

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

    QString fileName = inputFile.fileName();

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


    QApplication::processEvents();

    this->getProgressDialog()->setProgressBarRange(0,inputFiles.size());
    this->getProgressDialog()->setProgressBarValue(0);

    // Create the table to store the fields
    QList<Field> tableFields;
    tableFields.append(Field::createText("AssetType", "NULL",4));
    tableFields.append(Field::createText("TabName", "NULL",4));
    tableFields.append(Field::createText("Station Name", "NULL",4));
    tableFields.append(Field::createText("Latitude", "NULL",8));
    tableFields.append(Field::createText("Longitude", "NULL",9));
    tableFields.append(Field::createText("Number of Ground Motions","NULL",4));
    tableFields.append(Field::createText("Ground Motions","",1));

    auto gridFeatureCollection = new FeatureCollection(this);

    // Create the feature collection table/layers
    auto gridFeatureCollectionTable = new FeatureCollectionTable(tableFields, GeometryType::Point, SpatialReference::wgs84(), this);
    gridFeatureCollection->tables()->append(gridFeatureCollectionTable);

    auto gridLayer = new FeatureCollectionLayer(gridFeatureCollection,this);

    // Create red cross SimpleMarkerSymbol
    SimpleMarkerSymbol* crossSymbol = new SimpleMarkerSymbol(SimpleMarkerSymbolStyle::Cross, QColor("black"), 6, this);

    // Create renderer and set symbol to crossSymbol
    SimpleRenderer* renderer = new SimpleRenderer(crossSymbol, this);

    // Set the renderer for the feature layer
    gridFeatureCollectionTable->setRenderer(renderer);

    // Set the scale at which the layer will become visible - if scale is too high, then the entire view will be filled with symbols
    // gridLayer->setMinScale(80000);

    auto headers = data.front();

    auto indexFile = headers.indexOf("GP_file");
    auto indexLon = headers.indexOf("Longitude");
    auto indexLat = headers.indexOf("Latitude");

    if(indexLon == -1 || indexLat == -1 || indexFile == -1)
    {
        errorMessage = "Error could not find latitude and longitude in headers";
        return -1;
    }

    // Pop off the row that contains the header information
    data.pop_front();

    auto numRows = data.size();

    // Get the data
    for(int i = 0; i<numRows; ++i)
    {
        this->getProgressDialog()->setProgressBarValue(i+1);

        auto vecValues = data.at(i);

        if(vecValues.size() < 3)
        {
            errorMessage = "Error in importing ground motions";
            return -1;
        }

        bool ok;
        auto lon = vecValues[indexLon].toDouble(&ok);

        if(!ok)
        {
            errorMessage = "Error converting longitude object " + vecValues[1] + " to a double";
            return -1;
        }

        auto lat = vecValues[indexLat].toDouble(&ok);

        if(!ok)
        {
            errorMessage = "Error converting latitude object " + vecValues[2] + " to a double";
            return -1;
        }

        auto stationName = vecValues[indexFile];

        auto stationPath = inputFile.dir().absolutePath() + QDir::separator() + stationName;

        GroundMotionStation GMStation(stationPath,lat,lon);

        try
        {
            GMStation.importGroundMotions();
        }
        catch(QString msg)
        {
            auto errorMessage = "Error importing ground motion file: " + stationName+"\n"+msg;

            this->errorMessage(errorMessage);

            return -1;
        }

        stationList.push_back(GMStation);

        // create the feature attributes
        QMap<QString, QVariant> featureAttributes;

        auto vecGMs = GMStation.getStationGroundMotions();
        featureAttributes.insert("Number of Ground Motions", vecGMs.size());

        QString GMNames;
        for(int i = 0; i<vecGMs.size(); ++i)
        {
            auto GMName = vecGMs.at(i).getName();

            GMNames.append(GMName);

            if(i != vecGMs.size()-1)
                GMNames.append(", ");

        }

        featureAttributes.insert("Station Name", stationName);
        featureAttributes.insert("Ground Motions", GMNames);
        featureAttributes.insert("AssetType", "GroundMotionGridPoint");
        featureAttributes.insert("TabName", "Ground Motion Grid Point");

        auto latitude = GMStation.getLatitude();
        auto longitude = GMStation.getLongitude();

        featureAttributes.insert("Latitude", latitude);
        featureAttributes.insert("Longitude", longitude);

        // Create the point and add it to the feature table
        Point point(longitude,latitude);
        Feature* feature = gridFeatureCollectionTable->createFeature(featureAttributes, point, this);

        gridFeatureCollectionTable->addFeature(feature);
    }

    // Create a new layer
    LayerTreeView *layersTreeView = arcVizWidget->getLayersTree();

    // Check if there is a 'User Ground Motions' root item in the tree
    auto userInputTreeItem = layersTreeView->getTreeItem("EQ Hazard Simulation Grid", nullptr);

    // If there is no item, create one
    if(userInputTreeItem == nullptr)
        userInputTreeItem = layersTreeView->addItemToTree("EQ Hazard Simulation Grid", QString());

    // Add the event layer to the layer tree
    auto eventItem = layersTreeView->addItemToTree(fileName, QString(), userInputTreeItem);

    // Add the event layer to the map
    arcVizWidget->addLayerToMap(gridLayer,eventItem);

    return 0;
}
#endif




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
        this->errorMessage(errMsg);
        return res2;
    }

    this->statusMessage("Download and parsing of ground motion records complete.");

    this->statusMessage("The folder containing the results: "+m_appConfig->getOutputDirectoryPath());

    this->statusMessage("Earthquake hazard simulation complete.");

    simulationComplete = true;

    auto eventGridFile = m_appConfig->getOutputDirectoryPath() + QDir::separator() + QString("EventGrid.csv");

    emit outputDirectoryPathChanged(m_appConfig->getOutputDirectoryPath(), eventGridFile);
    // progressDialog->hide();

    return 0;
}

GmAppConfig *GMWidget::appConfig() const
{
    return m_appConfig;
}


void GMWidget::setCurrentlyViewable(bool status){

    if (status == true)
        mapViewSubWidget->setCurrentlyViewable(status);
}
