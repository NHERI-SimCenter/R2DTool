#include "GMWidget.h"
#include "SiteWidget.h"
#include "RegionalMappingWidget.h"
#include "NGAW2Converter.h"
#include "WorkflowAppRDT.h"
#include "IntensityMeasureWidget.h"
#include "SpatialCorrelationWidget.h"
#include "RuptureWidget.h"
#include "GridNode.h"
#include "GMPEWidget.h"
#include "SiteGridWidget.h"
#include "RecordSelectionWidget.h"
#include "SiteWidget.h"
#include "CustomGraphicsView.h"
#include "SiteConfigWidget.h"
#include "GmAppConfig.h"
#include "GmAppConfigWidget.h"
#include "GmCommon.h"
#include "VisualizationWidget.h"
#include "SimCenterPreferences.h"
#include "CSVReaderWriter.h"
#include "TreeView.h"
#include "MapViewSubWidget.h"

#include "MapGraphicsView.h"
#include "Map.h"
#include "Point.h"
#include "FeatureCollection.h"
#include "FeatureCollectionLayer.h"
#include "SimpleRenderer.h"
#include "SimpleMarkerSymbol.h"
#include "PeerNgaWest2Client.h"
#include "PeerLoginDialog.h"
#include "ZipUtils.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QList>
#include <QPlainTextEdit>
#include <QDialog>
#include <QJsonObject>
#include <QVBoxLayout>
#include <QStringList>
#include <QString>

using namespace Esri::ArcGISRuntime;

GMWidget::GMWidget(QWidget *parent, VisualizationWidget* visWidget) : SimCenterAppWidget(parent), theVisualizationWidget(visWidget)
{
    initAppConfig();

    simulationComplete = false;
    progressDialog = nullptr;

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

    m_runButton = new QPushButton(tr("&Run"));
    m_settingButton = new QPushButton(tr("&Settings"));

    // Create a map view that will be used for selecting the grid points
    mapViewMainWidget = theVisualizationWidget->getMapViewWidget();
    mapViewSubWidget = std::make_unique<MapViewSubWidget>(nullptr,mapViewMainWidget);

    auto userGrid = mapViewSubWidget->getGrid();
    userGrid->createGrid();
    userGrid->setGMSiteConfig(m_siteConfig);
    userGrid->setVisualizationWidget(theVisualizationWidget);

    toolsGridLayout->addWidget(this->m_siteConfigWidget,0,0,1,3);
    toolsGridLayout->addWidget(this->spatialCorrWidget,0,3);
    toolsGridLayout->addWidget(this->m_ruptureWidget,1,0,2,3);
    toolsGridLayout->addWidget(this->m_selectionWidget,1,3);
    toolsGridLayout->addWidget(this->m_gmpeWidget,2,3);
    toolsGridLayout->addWidget(this->m_intensityMeasureWidget,3,0,1,4);
    toolsGridLayout->addWidget(this->m_settingButton, 4,0,1,2);
    toolsGridLayout->addWidget(this->m_runButton,4,2,1,2);

    this->setLayout(toolsGridLayout);

    setupConnections();

}


GMWidget::~GMWidget()
{

}


void GMWidget::setAppConfig()
{
    GmAppConfigWidget* configWidget = new GmAppConfigWidget(m_appConfig, this);
    configWidget->show();
}


void GMWidget::setupConnections()
{
    //Connecting the run button
    connect(m_runButton, &QPushButton::released, this, [this]() {

        if(!peerClient.loggedIn())
        {
            PeerLoginDialog loginDialog(&peerClient, this);
            loginDialog.setWindowModality(Qt::ApplicationModal);
            loginDialog.exec();
            loginDialog.close();
            if(loginDialog.result() != QDialog::Accepted)
                return;
        }

        this->runHazardSimulation();
    });

    connect(m_settingButton, &QPushButton::released, this, &GMWidget::setAppConfig);

    connect(m_siteConfigWidget->getSiteGridWidget(), &SiteGridWidget::selectGridOnMap, this, &GMWidget::showGISWindow);

    //    connect(&peerClient, &PeerNgaWest2Client::recordsDownloaded, this, [](QString recordsFile)
    //    {
    //        qDebug() << "RECORDS DOWNLOADED: " << recordsFile;
    //        /*
    //        auto tempRecordsDir = QDir(groundMotionsFolder.path());
    //        //Cleaning up previous search results
    //        if(tempRecordsDir.exists("_SearchResults.csv"))
    //            tempRecordsDir.remove("_SearchResults.csv");
    //        ZipUtils::UnzipFile(recordsFile, tempRecordsDir);
    //        processPeerRecords(tempRecordsDir);
    //        */
    //    });

    //connect(&peerClient, &PeerNgaWest2Client::statusUpdated, this, &PEER_NGA_Records::updateStatus);

    connect(&peerClient, &PeerNgaWest2Client::selectionStarted, this, [this]()
    {
        // this->progressBar->setHidden(false);
        this->m_runButton->setEnabled(false);
        this->m_runButton->setDown(true);
    });


    connect(&peerClient, &PeerNgaWest2Client::selectionFinished, this, [this]()
    {
        //  this->progressBar->setHidden(true);
        this->m_runButton->setEnabled(true);
        this->m_runButton->setDown(false);
    });

    connect(&peerClient, &PeerNgaWest2Client::recordsDownloaded, this, [this](QString zipFile)
    {
        this->parseDownloadedRecords(zipFile);
        this->m_runButton->setEnabled(true);
        this->m_runButton->setDown(false);
    });
}


void GMWidget::initAppConfig()
{

    m_appConfig = new GmAppConfig(this);

    auto regMapWidget = WorkflowAppRDT::getInstance()->getTheRegionalMappingWidget();
    connect(m_appConfig,&GmAppConfig::outputDirectoryPathChanged,regMapWidget,&RegionalMappingWidget::handleFileNameChanged);

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

            this->userMessageDialog(errorMessage);

            return;
        }

        QDir dirWork(workingDir);

        if (!dirWork.exists())
            if (!dirWork.mkpath(workingDir))
            {
                QString errorMessage = QString("Could not load the Working Directory: ") + workingDir
                        + QString(". Change the Local Jobs Directory location in preferences.");

                this->userMessageDialog(errorMessage);

                return;
            }

        m_appConfig->setWorkDirectoryPath(workingDir+"/HazardSimulation/");

    }

    if(m_appConfig->getInputDirectoryPath().isEmpty() || !QFile::exists(m_appConfig->getInputDirectoryPath()))
    {

        QString workingDir = SimCenterPreferences::getInstance()->getLocalWorkDir();

        if(workingDir.isEmpty())
        {
            QString errorMessage = QString("Set the Local Jobs Directory location in preferences.");

            this->userMessageDialog(errorMessage);

            return;
        }

        QDir dirWork(workingDir);

        if (!dirWork.exists())
            if (!dirWork.mkpath(workingDir))
            {
                QString errorMessage = QString("Could not load the Input File Directory: ") + workingDir
                        + QString(". Change the Local Jobs Directory location in preferences.");

                this->userMessageDialog(errorMessage);

                return;
            }

        m_appConfig->setInputFilePath(workingDir+"/HazardSimulation/Input/");

    }

    if(m_appConfig->getOutputDirectoryPath().isEmpty() || !QFile::exists(m_appConfig->getOutputDirectoryPath()))
    {

        QString workingDir = SimCenterPreferences::getInstance()->getLocalWorkDir();

        if(workingDir.isEmpty())
        {
            QString errorMessage = QString("Set the Local Jobs Directory location in preferences.");

            this->userMessageDialog(errorMessage);

            return;
        }

        QDir dirWork(workingDir);

        if (!dirWork.exists())
            if (!dirWork.mkpath(workingDir))
            {
                QString errorMessage = QString("Could not load the Input File Directory: ") + workingDir
                        + QString(". Change the Local Jobs Directory location in preferences.");

                this->userMessageDialog(errorMessage);

                return;
            }

        m_appConfig->setOutputFilePath(workingDir+"/HazardSimulation/Output/");
    }

}


void GMWidget::saveAppSettings()
{
    QSettings settings;
    settings.setValue("WorkingDirectoryPath", m_appConfig->getWorkDirectoryPath());
    settings.setValue("InputFilePath", m_appConfig->getInputDirectoryPath());
    settings.setValue("OutputFilePath", m_appConfig->getOutputDirectoryPath());
    settings.setValue("RDBUsername", m_appConfig->getUsername());
    settings.setValue("RDBPassword", m_appConfig->getPassword());
}


void GMWidget::showGISWindow()
{
    auto scene = mapViewMainWidget->scene();
    auto sceneRect = scene->sceneRect();
    if(sceneRect.isNull())
        return;

    mapViewSubWidget->show();
    mapViewSubWidget->addGridToScene();

    mapViewSubWidget->setWindowTitle(QApplication::translate("toplevel", "Select ground motion grid"));
}


bool GMWidget::outputToJSON(QJsonObject &jsonObj)
{
    //    if(simulationComplete == false)
    //    {
    //        return false;
    //    }

    auto pathToEventGrid = m_appConfig->getOutputDirectoryPath() + "/";

    jsonObj.insert("pathEventData", pathToEventGrid);

    return true;
}


bool GMWidget::inputFromJSON(QJsonObject &jsonObject){


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


    // Remove the grid from the visualization screen
    mapViewSubWidget->removeGridFromScene();

    // First check if the settings are valid
    QString err;
    if(!m_appConfig->validate(err))
    {
        this->userMessageDialog(err);
        qDebug()<<err;
        return;
    }

    int maxID = m_siteConfig->siteGrid().getNumSites() - 1;

    //    maxID = 5;

    QJsonObject siteObj;
    siteObj.insert("Type", "From_CSV");
    siteObj.insert("input_file", "SiteFile.csv");
    siteObj.insert("min_ID", 0);
    siteObj.insert("max_ID", maxID);

    QJsonObject scenarioObj;
    scenarioObj.insert("Type", "Earthquake");
    scenarioObj.insert("Number", 1);
    scenarioObj.insert("Generator", "Selection");

    auto EqRupture = m_ruptureWidget->getJson();

    if(EqRupture.isEmpty())
    {
        QString err = "Error in getting the earthquake rupture .JSON";
        qDebug()<<err;
        this->userMessageDialog(err);
        return;
    }

    scenarioObj.insert("EqRupture",EqRupture);

    // Get the GMPE Json object
    auto GMPEobj = m_gmpe->getJson();

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
        qDebug()<<err;
        this->userMessageDialog(err);
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

    if(type == SiteConfig::SiteType::Single)
    {
        qDebug()<<"Single site selection not supported yet";
    }
    else if(type == SiteConfig::SiteType::Grid)
    {
        if(!m_siteConfigWidget->getSiteGridWidget()->getGridCreated())
        {
            QString msg = "Select a grid before continuing";
            this->userMessageDialog(msg);
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

    QString pathToSiteLocationFile = m_appConfig->getInputDirectoryPath() + "SiteFile.csv";

    CSVReaderWriter csvTool;

    auto res = csvTool.saveCSVFile(gridData, pathToSiteLocationFile, err);

    if(res != 0)
    {
        this->userMessageDialog(err);
        qDebug()<<err;
        return;
    }

    QString strFromObj = QJsonDocument(configFile).toJson(QJsonDocument::Indented);

    // Hazard sim
    QString pathToConfigFile = m_appConfig->getInputDirectoryPath() + "EQHazardConfiguration.json";

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
    auto pathToHazardSimScript = "/Users/steve/Desktop/SimCenter/HazardSimulation/HazardSimulation.py";
    //    auto pathToHazardSimScript = "/Users/fmckenna/release/HazardSimulation/HazardSimulation.py";

    QStringList args = {pathToHazardSimScript,"--hazard_config",pathToConfigFile};

    process->start(pythonPath, args);
    process->waitForStarted();
}


void GMWidget::handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{

    if(exitStatus == QProcess::ExitStatus::CrashExit)
    {
        QString errText("Error, the process running the hazard simulation script crashed");
        this->handleErrorMessage(errText);
        progressBar->hide();

        return;
    }

    if(exitCode != 0)
    {
        QString errText("An error occurred in the Hazard Simulation script, the exit code is " + QString::number(exitCode));
        this->handleErrorMessage(errText);
        progressBar->hide();

        return;
    }

    progressTextEdit->appendPlainText("Contacting PEER server to download ground motion records.\n");

    QApplication::processEvents();

    auto res = this->downloadRecords();

    if(res != 0)
    {
        QString errText("Error downloading the ground motion records");
        this->handleErrorMessage(errText);
        progressBar->hide();

        return;
    }

    progressTextEdit->appendPlainText("\nDownload and parsing of ground motion records complete.");

    progressBar->hide();

    progressTextEdit->appendPlainText("\nEarthquake hazard simulation complete.");

    simulationComplete = true;
}


void GMWidget::handleProcessStarted(void)
{
    this->showInfoDialog();
}


void GMWidget::handleProcessTextOutput(void)
{
    QByteArray output = process->readAllStandardOutput();

    progressTextEdit->appendPlainText("\n***Output from script start***\n");
    progressTextEdit->appendPlainText(QString(output));
    progressTextEdit->appendPlainText("***Output from script end***\n");

}


int GMWidget::downloadRecords(void)
{

    QString pathToGMFilesDirectory = m_appConfig->getOutputDirectoryPath() + QDir::separator();
    QString recordsListFilename = QString(pathToGMFilesDirectory + QString("RSN.csv"));
    QFile theRecordsListFile = QFile(recordsListFilename);

    if (!theRecordsListFile.exists())
    {
        QString errorMessage = QString("GMWidget::Record selection failed, no file ") +  recordsListFilename + QString(" exists");
        emit sendErrorMessage(errorMessage);
        return -1;
    }

    QStringList recordsList;

    if (theRecordsListFile.open(QIODevice::ReadOnly))
    {
        //file opened successfully, parse csv file for records
        while (!theRecordsListFile.atEnd())
        {
            QByteArray line = theRecordsListFile.readLine();
            foreach (const QByteArray &item, line.split(','))
            {
                recordsList.append(QString::fromLocal8Bit(item).trimmed()); // Assuming local 8-bit.
            }
        }


        // Check if any of the records exist, do not need to download them again
        const QFileInfo existingFilesInfo(pathToGMFilesDirectory);

        // Get the existing files in the folder to see if we already have the record
        QStringList acceptableFileExtensions = {"*.json"};
        QStringList existingFiles = existingFilesInfo.dir().entryList(acceptableFileExtensions, QDir::Files);

        QStringList recordsToDownload;

        if(!existingFiles.empty())
        {
            for(auto&& it : recordsList)
            {
                auto fileToCheck = "RSN" + it + ".json";
                if(!existingFiles.contains(fileToCheck))
                    recordsToDownload.append(it);
            }
        }
        else
        {
            recordsToDownload = recordsList;
        }

        peerClient.selectRecords(recordsToDownload);

    }

    return 0;
}


int GMWidget::processDownloadedRecords(QString& errorMessage)
{
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

    progressBar->setRange(0,inputFiles.size());

    progressBar->setValue(0);

    // Create the table to store the fields
    QList<Field> tableFields;
    tableFields.append(Field::createText("AssetType", "NULL",4));
    tableFields.append(Field::createText("TabName", "NULL",4));
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

    // Pop off the row that contains the header information
    data.pop_front();

    auto numRows = data.size();

    // Get the data
    for(int i = 0; i<numRows; ++i)
    {
        auto vecValues = data.at(i);

        if(vecValues.size() != 3)
        {
            qDebug()<<"Error in importing ground motions";
            return -1;
        }

        bool ok;
        auto lon = vecValues[1].toDouble(&ok);

        if(!ok)
        {
            errorMessage = "Error converting longitude object " + vecValues[1] + " to a double";
            return -1;
        }

        auto lat = vecValues[2].toDouble(&ok);

        if(!ok)
        {
            errorMessage = "Error converting latitude object " + vecValues[2] + " to a double";
            return -1;
        }

        auto stationName = vecValues[0];

        auto stationPath = inputFile.dir().absolutePath() + QDir::separator() + stationName;

        GroundMotionStation GMStation(stationPath,lat,lon);

        auto res = GMStation.importGroundMotions();

        if(res == 0)
        {
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
        else
        {
            errorMessage = "Error importing ground motion file: " + stationName;
            return -1;
        }

    }

    // Create a new layer
    auto layersTreeView = theVisualizationWidget->getLayersTree();

    // Check if there is a 'User Ground Motions' root item in the tree
    auto userInputTreeItem = layersTreeView->getTreeItem("EQ Hazard Simulation Grid", nullptr);

    // If there is no item, create one
    if(userInputTreeItem == nullptr)
        userInputTreeItem = layersTreeView->addItemToTree("EQ Hazard Simulation Grid", QString());

    // Add the event layer to the layer tree
    auto eventItem = layersTreeView->addItemToTree(fileName, QString(), userInputTreeItem);

    // Add the event layer to the map
    theVisualizationWidget->addLayerToMap(gridLayer,eventItem);

    return 0;
}


int GMWidget::parseDownloadedRecords(QString zipFile)
{

    QString pathToOutputDirectory = m_appConfig->getOutputDirectoryPath();
    bool result =  ZipUtils::UnzipFile(zipFile, pathToOutputDirectory);
    if (result == false)
    {
        QString errMsg = "Error in unziping the downloaded ground motion files";
        this->handleErrorMessage(errMsg);
        return -1;
    }

    NGAW2Converter tool;

    QJsonObject createdRecords;
    QString errMsg;
    auto res = tool.convertToSimCenterEvent(pathToOutputDirectory + QDir::separator(), errMsg, &createdRecords);
    if(res != 0)
    {
        if(res == -2)
        {
            errMsg.prepend("Error downloading ground motion files from PEER server.\n");
        }

        this->handleErrorMessage(errMsg);
        return res;
    }

    auto res2 = this->processDownloadedRecords(errMsg);
    if(res2 != 0)
    {
        this->handleErrorMessage(errMsg);
        return res2;
    }

    progressTextEdit->appendPlainText("The folder containing the results: "+m_appConfig->getOutputDirectoryPath() + "\n");

    return 0;
}




void GMWidget::showInfoDialog(void)
{
    if (!progressDialog)
    {
        progressDialog = new QDialog(this);

        auto progressBarLayout = new QVBoxLayout(progressDialog);
        progressDialog->setLayout(progressBarLayout);

        progressDialog->setMinimumHeight(500);
        progressDialog->setMinimumWidth(640);

        progressTextEdit = new QPlainTextEdit(this);
        progressTextEdit->setWordWrapMode(QTextOption::WrapMode::WordWrap);
        progressTextEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        progressTextEdit->setReadOnly(true);
        progressBar = new QProgressBar(progressDialog);
        progressBar->setRange(0,0);

        progressBarLayout->addWidget(progressTextEdit);
        progressBarLayout->addWidget(progressBar);
    }

    progressTextEdit->clear();
    progressTextEdit->appendPlainText("Earthquake hazard simulation started.\n \nThis may take a while! The script is using OpenSHA and determining which records to select from the PEER NGA West 2 database.");
    progressBar->show();
    progressDialog->show();
    progressDialog->raise();
    progressDialog->activateWindow();
}


void GMWidget::handleErrorMessage(const QString& errorMessage)
{
    progressTextEdit->appendPlainText("\n");

    auto msgStr = QString("<font color=%1>").arg("red") + errorMessage + QString("</font>") + QString("<font color=%1>").arg("black") + QString("&nbsp;") + QString("</font>");

    // Output to console and to text edit
    qDebug()<<msgStr;
    progressTextEdit->appendHtml(msgStr);

    progressTextEdit->appendPlainText("\n");
}

