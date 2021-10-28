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

// Written by: Stevan Gavrilovic

#include "LayerTreeView.h"
#include "HurricaneSelectionWidget.h"
#include "VisualizationWidget.h"
#include "WorkflowAppR2D.h"
#include "HurricaneParameterWidget.h"
#include "SimCenterPreferences.h"
#include "SiteConfig.h"
#include "GridNode.h"
#include "NodeHandle.h"
#include "LayerTreeItem.h"
#include "CSVReaderWriter.h"
#include "Utils/PythonProgressDialog.h"

//Test
#include <qgisapp.h>

#include <QApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QComboBox>
#include <QDialog>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QSpinBox>
#include <QSpacerItem>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QDir>

#ifdef Q_GIS
#include "SimCenterMapcanvasWidget.h"
#include "RectangleGrid.h"
#include <qgsmapcanvas.h>
#endif

#ifdef ARC_GIS
using namespace Esri::ArcGISRuntime;
#include "PolygonBoundary.h"
#endif

HurricaneSelectionWidget::HurricaneSelectionWidget(VisualizationWidget* visWidget, QWidget *parent) : theVizWidget(visWidget), SimCenterAppWidget(parent)
{
    fileInputWidget = nullptr;

    hurricaneParamsWidget = nullptr;
    loadDbButton = nullptr;
    numIMsLineEdit = nullptr;
    progressBar = nullptr;
    progressBarWidget = nullptr;
    progressLabel = nullptr;
    selectHurricaneWidget = nullptr;
    selectedHurricaneName = nullptr;
    selectedHurricaneSID = nullptr;
    selectedHurricaneSeason = nullptr;
    siteConfig = nullptr;
    siteGrid = nullptr;
    specifyHurricaneWidget = nullptr;
    terrainLineEdit = nullptr;
    theStackedWidget = nullptr;
    trackLineEdit = nullptr;
    typeOfScenarioWidget = nullptr;
    runButton = nullptr;
    divLatSpinBox = nullptr;
    divLonSpinBox = nullptr;

    process = new QProcess(this);
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &HurricaneSelectionWidget::handleProcessFinished);
    connect(process, &QProcess::readyReadStandardOutput, this, &HurricaneSelectionWidget::handleProcessTextOutput);
    connect(process, &QProcess::started, this, &HurricaneSelectionWidget::handleProcessStarted);

    eventDatabaseFile = "";

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(5,0,0,0);

    siteConfig = new SiteConfig(this);
    siteGrid = &siteConfig->siteGrid();

    layout->addWidget(this->getHurricaneSelectionWidget());

#ifdef ARC_GIS
    auto userGrid = mapViewSubWidget->getGrid();
#endif

#ifdef Q_GIS
    auto mapCanvas = mapViewSubWidget->mapCanvas();
    userGrid = std::make_unique<RectangleGrid>(mapCanvas);
    userPoint = std::make_unique<NodeHandle>(nullptr,mapCanvas);
#endif

    userGrid->createGrid();
    userGrid->setSiteGridConfig(siteConfig);
    userGrid->setVisualizationWidget(theVizWidget);

}


HurricaneSelectionWidget::~HurricaneSelectionWidget()
{

}


bool HurricaneSelectionWidget::outputAppDataToJSON(QJsonObject &jsonObject) {

    jsonObject["Application"] = "UserInputHurricane";

    QJsonObject appData;
    //    QFileInfo theFile(eventFile);
    //    if (theFile.exists()) {
    //        appData["eventFile"]=theFile.fileName();
    //        appData["eventFileDir"]=theFile.path();
    //    } else {
    //        appData["eventFile"]=eventFile; // may be valid on others computer
    //        appData["eventFileDir"]=QString("");
    //    }
    //    QFileInfo theDir(motionDir);
    //    if (theDir.exists()) {
    //        appData["motionDir"]=theDir.absoluteFilePath();
    //    } else {
    //        appData["motionDir"]=QString("None");
    //    }

    jsonObject["ApplicationData"]=appData;

    return true;
}

 bool
 HurricaneSelectionWidget::copyFiles(QString &destDir)
 {

    return true;
 }
bool HurricaneSelectionWidget::outputToJSON(QJsonObject &/*jsonObj*/)
{
    // qDebug() << "USER Hurricane outputPLAIN";

    return true;
}


bool HurricaneSelectionWidget::inputAppDataFromJSON(QJsonObject &jsonObj)
{
    if (jsonObj.contains("ApplicationData")) {
        //        QJsonObject appData = jsonObj["ApplicationData"].toObject();

        //        QString fileName;
        //        QString pathToFile;

        //        if (appData.contains("eventFile"))
        //            fileName = appData["eventFile"].toString();
        //        if (appData.contains("eventFileDir"))
        //            pathToFile = appData["eventFileDir"].toString();
        //        else
        //            pathToFile=QDir::currentPath();

        //        QString fullFilePath= pathToFile + QDir::separator() + fileName;

        //        // adam .. adam .. adam
        //        if (!QFileInfo::exists(fullFilePath)){
        //            fullFilePath = pathToFile + QDir::separator()
        //                    + "input_data" + QDir::separator() + fileName;

        //            if (!QFile::exists(fullFilePath)) {
        //                qDebug() << "UserInputGM - could not find event file";
        //                return false;
        //            }
        //        }

        //        eventFileLineEdit->setText(fullFilePath);
        //        eventFile = fullFilePath;

        //        if (appData.contains("motionDir"))
        //            motionDir = appData["motionDir"].toString();

        //        QDir motionD(motionDir);

        //        if (!motionD.exists()){
        //            QString trialDir = QDir::currentPath() +
        //                    QDir::separator() + "input_data" + motionDir;
        //            if (motionD.exists(trialDir)) {
        //                motionDir = trialDir;
        //                motionDirLineEdit->setText(trialDir);
        //            } else {
        //                qDebug() << "UserInputGM - could not find motion dir" << motionDir << " " << trialDir;
        //                return false;
        //            }
        //        } else {
        //            motionDirLineEdit->setText(motionDir);
        //        }

        //        this->loadUserGMData();
        //        return true;
    }

    return false;
}


QStackedWidget* HurricaneSelectionWidget::getHurricaneSelectionWidget(void)
{
    if (theStackedWidget)
        return theStackedWidget;

    theStackedWidget = new QStackedWidget();
    theStackedWidget->setContentsMargins(0,0,0,0);

    theStackedWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    //
    // file and dir input
    //

    fileInputWidget = new QWidget();
    fileInputWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    fileInputWidget->setContentsMargins(0,0,0,0);

    QGridLayout *mainLayout = new QGridLayout(fileInputWidget);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);

#ifdef ARC_GIS
    auto mapView = theVizWidget->getMapViewWidget("HurricaneSelectionWidget");
    mapViewSubWidget = std::make_unique<EmbeddedMapViewWidget>(mapView);
#endif

#ifdef Q_GIS
    auto mapView = theVizWidget->getMapViewWidget("HurricaneSelectionWidget");
    mapViewSubWidget = std::unique_ptr<SimCenterMapcanvasWidget>(mapView);

    // Enable the selection tool
    mapViewSubWidget->enableSelectionTool();
#endif

    QComboBox* simulationTypeComboBox = new QComboBox();
    simulationTypeComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

    simulationTypeComboBox->addItem("Specify Hurricane Track");
    simulationTypeComboBox->addItem("Select Hurricane Track from Database");
    typeOfScenarioWidget = new QStackedWidget();
    typeOfScenarioWidget->setContentsMargins(5,0,5,0);

    // Widget to select hurricane from database
    selectHurricaneWidget = new QWidget();

    selectHurricaneWidget->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum);

    QGridLayout *selectHurricaneLayout = new QGridLayout(selectHurricaneWidget);
    selectHurricaneLayout->setContentsMargins(0,5,0,0);

    loadDbButton = new QPushButton("Load Hurricane Database");
    loadDbButton->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum);

    connect(loadDbButton,SIGNAL(clicked()),this,SLOT(loadHurricaneButtonClicked()));

    QPushButton *selectHurricaneButton = new QPushButton("Select Hurricane");

    connect(selectHurricaneButton,&QPushButton::clicked,this,&HurricaneSelectionWidget::handleHurricaneSelect);

    QLabel* selectedHurricaneLabel = new QLabel("Hurricane Selected: ");
    QLabel* SIDLabel = new QLabel("SID: ");
    QLabel* seasonLabel = new QLabel("Season: ");

    selectedHurricaneName = new QLabel("None");
    selectedHurricaneSID = new QLabel("None");
    selectedHurricaneSeason = new QLabel("None");

    hurricaneParamsWidget = new HurricaneParameterWidget();

    selectHurricaneLayout->addWidget(loadDbButton,0,0);
    selectHurricaneLayout->addWidget(selectHurricaneButton,0,1);
    selectHurricaneLayout->addWidget(selectedHurricaneLabel,1,0);
    selectHurricaneLayout->addWidget(selectedHurricaneName,1,1);
    selectHurricaneLayout->addWidget(seasonLabel,2,0);
    selectHurricaneLayout->addWidget(selectedHurricaneSeason,2,1);
    selectHurricaneLayout->addWidget(SIDLabel,3,0);
    selectHurricaneLayout->addWidget(selectedHurricaneSID,3,1);
    selectHurricaneLayout->rowStretch(3);

    // Widget to specify hurricane track
    specifyHurricaneWidget = new QWidget();
    QGridLayout* specifyHurricaneLayout = new QGridLayout(specifyHurricaneWidget);
    specifyHurricaneLayout->setContentsMargins(0,0,0,0);
    //    specifyHurricaneLayout->setSpacing(8);

    QLabel* trackLabel = new QLabel("Hurricane Track (.csv)");
    trackLineEdit = new QLineEdit();
    QPushButton *browseTrackButton = new QPushButton("Browse");

    connect(browseTrackButton,&QPushButton::clicked,this,&HurricaneSelectionWidget::handleHurricaneTrackImport);

    QLabel* terrainLabel = new QLabel("Terrain Roughness (.geojson)");
    terrainLineEdit = new QLineEdit();
    browseTerrainButton = new QPushButton("Browse");

    specifyHurricaneLayout->addWidget(trackLabel,0,0);
    specifyHurricaneLayout->addWidget(trackLineEdit,0,1);
    specifyHurricaneLayout->addWidget(browseTrackButton,0,2);
    specifyHurricaneLayout->addWidget(terrainLabel,1,0);
    specifyHurricaneLayout->addWidget(terrainLineEdit,1,1);
    specifyHurricaneLayout->addWidget(browseTerrainButton,1,2);

    //    connect(browseFileButton,SIGNAL(clicked()),this,SLOT(chooseEventFileDialog()));

    typeOfScenarioWidget->addWidget(specifyHurricaneWidget);
    typeOfScenarioWidget->addWidget(selectHurricaneWidget);

    connect(simulationTypeComboBox,SIGNAL(currentIndexChanged(int)),typeOfScenarioWidget,SLOT(setCurrentIndex(int)));


    // Grid selection
    auto defineGridButton = new QPushButton(tr("&Define Grid on Map"));
    auto selectGridButton = new QPushButton(tr("&Select Grid"));
    auto clearGridButton = new QPushButton(tr("&Clear Grid"));

    //    defineGridButton->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum);
    //    selectGridButton->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum);
    //    clearGridButton->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum);

    connect(defineGridButton,&QPushButton::clicked,this,&HurricaneSelectionWidget::showGridOnMap);
    connect(selectGridButton,&QPushButton::clicked,this,&HurricaneSelectionWidget::handleGridSelected);
    connect(clearGridButton,&QPushButton::clicked,this,&HurricaneSelectionWidget::clearGridFromMap);

    QLabel* gridLabel = new QLabel("Specify Wind Field Grid:");

    QFrame* gridGroupBox = new QFrame();
    //    gridGroupBox->setObjectName("TopLine");
    //    gridGroupBox->setStyleSheet("#TopLine { border-top: 2px solid black; }");
    gridGroupBox->setContentsMargins(0,0,0,0);

    QGridLayout *gridLayout = new QGridLayout(gridGroupBox);

    QHBoxLayout* gridButtonsLayout = new QHBoxLayout();
    gridButtonsLayout->addWidget(gridLabel);
    gridButtonsLayout->addWidget(defineGridButton);
    gridButtonsLayout->addWidget(selectGridButton);
    gridButtonsLayout->addWidget(clearGridButton);

    gridLayout->addLayout(gridButtonsLayout,0,0,1,3);

    auto divLatLabel = new QLabel("Divisions Latitude");
    divLatSpinBox = new QSpinBox();
    divLatSpinBox->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
    divLatSpinBox->setRange(1, 1000);
    divLatSpinBox->setValue(5);
    divLatSpinBox->setValue(siteGrid->latitude().divisions());

    auto divLonLabel = new QLabel("Divisions Longitude");
    divLonSpinBox = new QSpinBox();
    divLonSpinBox->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
    divLonSpinBox->setRange(1, 1000);
    divLonSpinBox->setValue(5);
    divLonSpinBox->setValue(siteGrid->latitude().divisions());

    connect(divLatSpinBox,
            QOverload<int>::of(&QSpinBox::valueChanged),
            &siteGrid->latitude(),
            &GridDivision::setDivisions);

    connect(&siteGrid->latitude(),
            &GridDivision::divisionsChanged,
            divLatSpinBox,
            &QSpinBox::setValue);

    connect(divLonSpinBox,
            QOverload<int>::of(&QSpinBox::valueChanged),
            &siteGrid->longitude(),
            &GridDivision::setDivisions);

    connect(&siteGrid->longitude(),
            &GridDivision::divisionsChanged,
            divLonSpinBox,
            &QSpinBox::setValue);

    divLonSpinBox->setValue(10);
    divLatSpinBox->setValue(10);

    auto numIMsLabel = new QLabel("Number per site",this);
    numIMsLineEdit = new QLineEdit(this);
    numIMsLineEdit->setText("3");

    auto numImsVal = new QIntValidator(numIMsLineEdit);
    numImsVal->setBottom(1);
    numIMsLineEdit->setValidator(numImsVal);

    gridLayout->addWidget(divLatLabel,0,3);
    gridLayout->addWidget(divLatSpinBox,0,4);
    gridLayout->addWidget(divLonLabel,0,5);
    gridLayout->addWidget(divLonSpinBox,0,6);
    gridLayout->addWidget(numIMsLabel,0,7);
    gridLayout->addWidget(numIMsLineEdit,0,8);


    auto scenarioTypeLabel = new QLabel("Hurricane Definition");

    auto topLayout = new QHBoxLayout();
    topLayout->addWidget(scenarioTypeLabel);
    topLayout->addWidget(simulationTypeComboBox);


    QFrame* bottomWidget = new QFrame();
    bottomWidget->setContentsMargins(0,0,0,0);

    // Functionality to truncate the track
    truncTrackSelectButton = new QPushButton(tr("&Select Area on Map"),this);
    truncTrackApplyButton = new QPushButton(tr("&Apply"),this);
    truncTrackClearButton = new QPushButton(tr("&Clear"),this);

    auto truncLabel = new QLabel("Truncate Hurricane Track:");

    auto bottomLayout = new QHBoxLayout(bottomWidget);

    bottomLayout->addWidget(truncLabel);
    bottomLayout->addWidget(truncTrackSelectButton);
    bottomLayout->addWidget(truncTrackApplyButton);
    bottomLayout->addWidget(truncTrackClearButton);

    QFrame* line1 = new QFrame();
    line1->setFrameShape(QFrame::VLine);
    line1->setFrameShadow(QFrame::Sunken);
    bottomLayout->addWidget(line1);

    // Landfall layout
    QPushButton *defineLandfall = new QPushButton("Define Landfall on Map");
    QPushButton *clearLandfall = new QPushButton("Clear Landfall");
    QPushButton *selectLandfall = new QPushButton("Select Landfall");

    connect(defineLandfall,&QPushButton::clicked,this,&HurricaneSelectionWidget::showPointOnMap);
    connect(selectLandfall,&QPushButton::clicked,this,&HurricaneSelectionWidget::handleLandfallPointSelected);
    connect(clearLandfall,&QPushButton::clicked,this,&HurricaneSelectionWidget::clearLandfallFromMap);

    QLabel* landfallLabel = new QLabel("Specify Hurricane Landfall:",this);

    bottomLayout->addWidget(landfallLabel);
    bottomLayout->addWidget(defineLandfall);
    bottomLayout->addWidget(selectLandfall);
    bottomLayout->addWidget(clearLandfall);

    QFrame* line2 = new QFrame();
    line2->setFrameShape(QFrame::VLine);
    line2->setFrameShadow(QFrame::Sunken);
    bottomLayout->addWidget(line2);

    QLabel* runLabel = new QLabel("Run Hurricane Simulation:");

    runButton = new QPushButton(tr("&Run"));
    connect(runButton,&QPushButton::clicked,this,&HurricaneSelectionWidget::runHazardSimulation);

    bottomLayout->addWidget(runLabel);
    bottomLayout->addWidget(runButton);


    mainLayout->addLayout(topLayout, 0,0);
    mainLayout->addWidget(hurricaneParamsWidget, 0,1,2,1);
    mainLayout->addWidget(typeOfScenarioWidget, 1,0,1,1);
    mainLayout->addWidget(bottomWidget, 2,0,1,2);
    mainLayout->addWidget(gridGroupBox, 3,0,1,2);
    mainLayout->addWidget(mapViewSubWidget.get(), 4,0,1,2);
    mainLayout->setRowStretch(4,1);

    //
    // progress bar
    //

    progressBarWidget = new QWidget();
    auto progressBarLayout = new QVBoxLayout(progressBarWidget);
    progressBarWidget->setLayout(progressBarLayout);

    auto progressText = new QLabel("Loading hurricane database. This may take a while.",progressBarWidget);
    progressLabel =  new QLabel("");
    progressBar = new QProgressBar();

    auto vspacer = new QSpacerItem(0,0,QSizePolicy::Minimum, QSizePolicy::Expanding);
    auto vspacer2 = new QSpacerItem(0,0,QSizePolicy::Minimum, QSizePolicy::Expanding);
    progressBarLayout->addItem(vspacer);
    progressBarLayout->addWidget(progressText,1, Qt::AlignCenter);
    progressBarLayout->addWidget(progressLabel,1, Qt::AlignCenter);
    progressBarLayout->addWidget(progressBar);
    progressBarLayout->addItem(vspacer2);
    //    progressBarLayout->addStretch(1);

    //
    // add file and progress widgets to stacked widgets, then set defaults
    //

    theStackedWidget->addWidget(fileInputWidget);
    theStackedWidget->addWidget(progressBarWidget);

    theStackedWidget->setCurrentWidget(fileInputWidget);

    theStackedWidget->setWindowTitle("Hurricane track selection");

    return theStackedWidget;
}


void HurricaneSelectionWidget::showEvent(QShowEvent *e)
{
    auto mainCanvas = mapViewSubWidget->getMainCanvas();

    auto mainExtent = mainCanvas->extent();

    mapViewSubWidget->mapCanvas()->zoomToFeatureExtent(mainExtent);
    SimCenterAppWidget::showEvent(e);
}


void HurricaneSelectionWidget::showHurricaneSelectDialog(void)
{

    if (!theStackedWidget)
    {
        this->getHurricaneSelectionWidget();
    }

    theStackedWidget->show();
    theStackedWidget->raise();
    theStackedWidget->activateWindow();
}


void HurricaneSelectionWidget::loadHurricaneTrackData(void)
{
    theStackedWidget->setCurrentWidget(progressBarWidget);
    progressBarWidget->setVisible(true);

    QString errMsg;
    auto res = this->importHurricaneTrackData(eventDatabaseFile,errMsg);

    progressLabel->setVisible(false);

    // Reset the widget back to the input pane and close
    theStackedWidget->setCurrentWidget(fileInputWidget);
    fileInputWidget->setVisible(true);

    if(theStackedWidget->isModal())
        theStackedWidget->close();

    emit loadingComplete(true);

    if(res != 0)
        this->errorMessage(errMsg);

    return;
}


#ifdef ARC_GIS
void HurricaneSelectionWidget::setCurrentlyViewable(bool status){

    if (status == true)
        mapViewSubWidget->setCurrentlyViewable(status);
    else
        mapViewSubWidget->removeGridFromScene();
}
#endif


void HurricaneSelectionWidget::loadHurricaneButtonClicked(void)
{

    auto pathToHurricaneDb = QCoreApplication::applicationDirPath() +
            QDir::separator() + "Databases" + QDir::separator() + "ibtracs.last3years.list.v04r00.csv";

    QFile file(pathToHurricaneDb);

    // Return if database does not exist
    if(!file.exists())
    {
        this->errorMessage("The hurricane database does not exist");
        return;
    }

    // Set database if it exists
    eventDatabaseFile = pathToHurricaneDb;

    this->loadHurricaneTrackData();

    loadDbButton->setText("Database Loaded");
    loadDbButton->setEnabled(false);

    return;
}


void HurricaneSelectionWidget::clear(void)
{
    eventDatabaseFile.clear();

    selectedHurricaneName->setText("None");
    selectedHurricaneSID->setText("None");
    selectedHurricaneSeason->setText("None");

    loadDbButton->setText("Load Hurricane Database");
    loadDbButton->setEnabled(true);

    hurricaneParamsWidget->clear();

    this->clearGridFromMap();
    this->clearLandfallFromMap();

    numIMsLineEdit->setText("3");

    trackLineEdit->clear();
    terrainLineEdit->clear();

    divLatSpinBox->setValue(10);
    divLonSpinBox->setValue(10);

    selectedHurricaneObj.clear();
}


void HurricaneSelectionWidget::showGridOnMap(void)
{
#ifdef ARC_GIS
    mapViewSubWidget->addGridToScene();
#endif

#ifdef Q_GIS
    // Also important to get events from QGIS
    mapViewSubWidget->setMapTool(userGrid.get());
    userGrid->show();
#endif

}


void HurricaneSelectionWidget::showPointOnMap(void)
{
#ifdef ARC_GIS
    mapViewSubWidget->addPointToScene();
#endif

#ifdef Q_GIS
    mapViewSubWidget->setMapTool(userGrid.get());
    userPoint->show();
#endif
}


void HurricaneSelectionWidget::runHazardSimulation(void)
{

    QString workingDir = SimCenterPreferences::getInstance()->getLocalWorkDir();

    if(workingDir.isEmpty())
    {
        QString errorMessage = QString("Set the Local Jobs Directory location in preferences.");

        this->errorMessage(errorMessage);

        return;
    }

    // Make a hurricanes folder under hazard simulation
    workingDir += QDir::separator() + QString("HazardSimulation") + QDir::separator() + QString("Hurricanes");

    QDir dirWork(workingDir);

    if (!dirWork.exists())
        if (!dirWork.mkpath(workingDir))
        {
            QString errorMessage = QString("Could not load the Working Directory: ") + workingDir
                    + QString(". Change the Local Jobs Directory location in preferences.");

            this->errorMessage(errorMessage);

            return;
        }


    if(gridData.size()<2)
    {
        QString msg = "Specify a site grid before continuing";
        this->statusMessage(msg);
        return;
    }

    QJsonObject configFile;

    // Get the output directory path
    QString outputDir = workingDir + QDir::separator() + "Output";

    // Get the input directory path
    QString inputDir = workingDir + QDir::separator() + "Input";

    // Delete and create a new output directory
    QDir dirOutput(outputDir);

    if(dirOutput.exists())
        dirOutput.removeRecursively();

    if (!dirOutput.mkpath(outputDir))
    {
        QString errorMessage = QString("Could not create the output Directory: ") + outputDir;
        this->errorMessage(errorMessage);
        return;
    }

    // Delete and create a new input directory
    QDir dirInput(inputDir);

    if(dirInput.exists())
        dirInput.removeRecursively();

    if (!dirInput.mkpath(inputDir))
    {
        QString errorMessage = QString("Could not create the input Directory: ") + inputDir;
        this->errorMessage(errorMessage);
        return;
    }

    // Directory json object
    QJsonObject dirObj;
    dirObj.insert("Work",workingDir);
    dirObj.insert("Input",inputDir);
    dirObj.insert("Output",outputDir);

    configFile.insert("Directory",dirObj);

    int maxID = siteConfig->siteGrid().getNumSites();

    QJsonObject siteObj;
    siteObj.insert("Type", "From_CSV");
    siteObj.insert("input_file", "SiteFile.csv");
    siteObj.insert("min_ID", 1);
    siteObj.insert("max_ID", maxID);

    configFile.insert("Site",siteObj);

    QJsonObject scenarioObj;
    scenarioObj.insert("Type", "Wind");
    scenarioObj.insert("Number", 1);
    scenarioObj.insert("ModelType", "LinearAnalytical");

    // Check if a hurricane exists
    if(selectedHurricaneObj.empty())
    {
        this->statusMessage("Select or specify a hurricane before running");
        return;
    }

    // The path to the track file
    auto pathTrackFile = inputDir + QDir::separator() + "R2DHurricaneTrack.csv";

    QVector<QStringList> trackData;

    // Get the headers
    auto paramLabels = selectedHurricaneObj.parameterLabels;

    // Get the index to the lat and lon
    auto indexLat = paramLabels.indexOf("LAT");
    auto indexLon = paramLabels.indexOf("LON");

    if(indexLat == -1 || indexLon == -1)
    {
        this->errorMessage("Could not get the lat/lon indexes to populate the track");
        return;
    }

    auto fullTrackData = selectedHurricaneObj.getHurricaneData();

    for(auto&& it : fullTrackData)
    {
        QStringList latLonVals = {it.at(indexLat),it.at(indexLon)};

        trackData.push_back(latLonVals);
    }

    // Save the track
    CSVReaderWriter csvTool;
    QString err;
    auto res = csvTool.saveCSVFile(trackData, pathTrackFile, err);

    if(res != 0)
    {
        this->statusMessage("Failed to save the CSV track file with error:"+err);
        return;
    }

    QJsonObject stormObj;

    auto currHurrType = typeOfScenarioWidget->currentWidget();
    if(currHurrType == selectHurricaneWidget)
    {
        scenarioObj.insert("Generator", "SimulationHist");

        auto hurrName = selectedHurricaneName->text();
        auto hurrSeason = selectedHurricaneSeason->text().toInt();

        stormObj.insert("Name",hurrName);
        stormObj.insert("Year",hurrSeason);

        stormObj.insert("Track", pathTrackFile);
    }
    else if(currHurrType == specifyHurricaneWidget)
    {
        scenarioObj.insert("Generator", "Simulation");

        stormObj.insert("Track", pathTrackFile);

        QJsonObject landfallObj = hurricaneParamsWidget->getLandfallParamsJson();

        if(landfallObj.empty())
        {
            this->statusMessage("Some landfall parameters are not specified");
            return;
        }

        stormObj.insert("Landfall",landfallObj);

        auto pathTerrainFile = terrainLineEdit->text();

        if(pathTerrainFile.isEmpty())
        {
            this->statusMessage("No terrain.geojson file provided. Using default values");
            scenarioObj.insert("Terrain", pathTerrainFile);
        }
    }

    // stormObject.insert("TrackSimu",);

    scenarioObj.insert("Storm",stormObj);

    QJsonObject meshObj;
    meshObj.insert("DivRad", 10000.0);
    meshObj.insert("DivDeg", 1.0);

    scenarioObj.insert("Mesh",meshObj);

    configFile.insert("Scenario",scenarioObj);

    QJsonObject eventObj = hurricaneParamsWidget->getEventJson();

    int numPerSite = numIMsLineEdit->text().toInt();
    eventObj.insert("NumberPerSite",numPerSite);

    configFile.insert("Event",eventObj);


    // Now save the site grid .csv file
    QString pathToSiteLocationFile = inputDir + QDir::separator() + "SiteFile.csv";

    QString err2;
    auto res2 = csvTool.saveCSVFile(gridData, pathToSiteLocationFile, err2);
    if(res2 != 0)
    {
        this->errorMessage(err2);
        return;
    }

    // Hazard sim config file
    QString strFromObj = QJsonDocument(configFile).toJson(QJsonDocument::Indented);

    QString pathToConfigFile = inputDir + QDir::separator() + "HurricaneHazardConfiguration.json";

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

    // Get the path to the Python
    auto pythonPath = SimCenterPreferences::getInstance()->getPython();

    // auto pathToHazardSimScript = "/Users/steve/Desktop/SimCenter/SimCenterBackEndKuanshi/applications/performRegionalEventSimulation/regionalWindField/HurricaneSimulation.py";
    QString pathToHazardSimScript = SimCenterPreferences::getInstance()->getAppDir() + QDir::separator()
            + "applications" + QDir::separator() + "performRegionalEventSimulation" + QDir::separator()
            + "regionalWindField" + QDir::separator() + "HurricaneSimulation.py";

    QFileInfo hazardFileInfo(pathToHazardSimScript);
    if (!hazardFileInfo.exists()) {
        QString errorMessage = QString("ERROR - hazardApp does not exist") + pathToHazardSimScript;
        this->errorMessage(errorMessage);
        qDebug() << errorMessage;
        return;
    }
    QStringList args = {pathToHazardSimScript,"--hazard_config",pathToConfigFile};

    qDebug()<<"Hazard Simulation Command:"<<args[0]<<" "<<args[1]<<" "<<args[2];

    process->start(pythonPath, args);
    process->waitForStarted();
}


void HurricaneSelectionWidget::handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    this->runButton->setEnabled(true);
    this->getProgressDialog()->hideProgressBar();

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

    this->statusMessage("Hazard Simulation Complete!");

    // ##
    QString outputDir = SimCenterPreferences::getInstance()->getLocalWorkDir();

    // Make a hurricanes folder under hazard simulation
    outputDir += QDir::separator() + QString("HazardSimulation") + QDir::separator() + QString("Hurricanes") + QDir::separator() + "Output";

    this->loadResults(outputDir);
}


void HurricaneSelectionWidget::handleProcessStarted(void)
{
    this->statusMessage("Running script in the background");
    this->runButton->setEnabled(false);

    this->getProgressDialog()->showProgressBar();
}


void HurricaneSelectionWidget::handleProcessTextOutput(void)
{
    QByteArray output = process->readAllStandardOutput();

    this->statusMessage(QString(output));
}


int HurricaneSelectionWidget::loadResults(const QString& outputDir)
{
    this->statusMessage("Loading windfield results");

    // Check if output directory exists
    QDir dirOutput(outputDir);

    if(!dirOutput.exists())
    {
        this->errorMessage("Output directory: " + outputDir + " - does not exist");
        return -1;
    }

    QString resultsPath = outputDir + QDir::separator() + "EventGrid.csv";

    QFile resultsFile(resultsPath);

    if(!resultsFile.exists())
    {
        this->errorMessage("The results file does not exist: " + resultsPath);
        return -1;
    }

    QStringList acceptableFileExtensions = {"*.CSV", "*.csv"};

    QStringList inputFiles = dirOutput.entryList(acceptableFileExtensions,QDir::Files);

    if(inputFiles.empty())
    {
        this->errorMessage("No files with .csv extensions were found at the path: "+outputDir);
        return -1;
    }

    CSVReaderWriter csvTool;

    QString err;
    QVector<QStringList> data = csvTool.parseCSVFile(resultsPath,err);

    if(!err.isEmpty())
    {
        this->errorMessage(err);
        return -1;
    }

    if(data.empty())
        return -1;

    auto headerFields = data.front();

    auto stationIndex = headerFields.indexOf("GP_file");
    auto latIndex = headerFields.indexOf("Latitude");
    auto lonIndex = headerFields.indexOf("Longitude");

    if(stationIndex == -1 || latIndex == -1 || lonIndex == -1)
    {
        this->errorMessage("Could not find the requires indexes for station name, lat, and lon");
        return -1;
    }

    // Pop off the row that contains the header information
    data.pop_front();

    auto numRows = data.size();

#ifdef Q_GIS
    QgsFeatureList featList;
    featList.reserve(numRows);
#endif
    // Get the data
    for(int i = 0; i<numRows; ++i)
    {

        auto vecValues = data.at(i);

        if(vecValues.size() != 3)
        {
            this->errorMessage("Error in importing wind field");
            return -1;
        }

        auto stationName = vecValues[stationIndex].remove(".csv");

        auto stationPath = outputDir + QDir::separator() + stationName + ".csv";

        // Find the station in the map
        auto station = stationMap.find(stationName);

        if(station->isNull())
        {
            this->errorMessage("Error, could not find the station in the map");
            return -1;
        }

        station->setStationFilePath(stationPath);

        try {
            station->importWindFieldStation();
        } catch (const QString& err) {
            this->errorMessage(err);
            return -1;
        }

        auto pws = station->getStationData();

        if(pws.empty())
        {
            this->errorMessage("Error getting the peak wind speeds from the results");
            return -1;
        }

        auto headings =  station->getStationDataHeaders();

        auto idxPWS = headings.indexOf("PWS");

        if(idxPWS == -1)
        {
            this->errorMessage("Error, PWS index not found in headers");
            return -1;
        }

        QString pwsStr;

        for(int i = 0; i<pws.size()-1; ++i)
        {
            pwsStr += pws[i].at(idxPWS) + ", ";
        }

        pwsStr += pws.back().at(idxPWS);

        QString attribute = "Peak Wind Speeds";

#ifdef ARC_GIS
        station->updateFeatureAttribute(attribute,QVariant(pwsStr));
#endif

#ifdef Q_GIS
        auto feat = station->getStationFeature();

        if(!feat.isValid())
        {
            this->errorMessage("Feature is not valid");
        }

        auto atrb = feat.attribute(attribute);

        if(!atrb.isValid())
        {
            this->errorMessage("Could not find attribute in feature");
        }

        auto res = feat.setAttribute(attribute,pwsStr);
        if(res == false)
        {
            this->errorMessage("Failed to update feature");
        }
        featList.push_back(feat);
#endif
    }

#ifdef Q_GIS
    this->updateGridLayerFeatures(featList);
#endif

    emit outputDirectoryPathChanged(outputDir, resultsPath);

    this->statusMessage("Done loading results");

    return 0;
}


void HurricaneSelectionWidget::handleHurricaneTrackImport(void)
{
    QFileDialog dialog(this);
    QString trackFilePath = QFileDialog::getOpenFileName(this,tr("Hurricane track file (.csv)"),QString(),QString("*.csv"));
    dialog.close();

    auto oldTrackFilePath = trackLineEdit->text();

    // Return if the user cancels or enters same file
    if(trackFilePath.isEmpty() || trackFilePath == oldTrackFilePath)
    {
        return;
    }

    trackLineEdit->setText(trackFilePath);

    CSVReaderWriter csvTool;

    QString err;
    QVector<QStringList> data = csvTool.parseCSVFile(trackFilePath,err);

    if(!err.isEmpty())
    {
        this->errorMessage(err);
        return;
    }

    if(data.empty())
        return;

    selectedHurricaneObj.clear();

    selectedHurricaneObj.name = "USER SPECIFIED";

    QStringList parameterLabels = {"LAT","LON"};

    selectedHurricaneObj.parameterLabels = parameterLabels;

    for(auto&& it : data)
    {
        if(it.size() != 2)
        {
            return;
        }

        selectedHurricaneObj.push_back(it);
    }

    this->createHurricaneVisuals(&selectedHurricaneObj);
}


QJsonArray HurricaneSelectionWidget::getTerrainData(void)
{
    auto terrainPath = this->getTerrainGeojsonPath();

    // open file
    QFile file(terrainPath);
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        QString msg = QString("Cannot open the file:") + terrainPath;
        this->errorMessage(msg);
        return QJsonArray();
    }

    // Place contents of file into json object
    QString val;
    val=file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(val.toUtf8());
    QJsonObject jsonObj = doc.object();

    // Extract the features
    auto featureArray = jsonObj["features"].toArray();

    auto numFeat = featureArray.size();

    if(numFeat == 0)
    {
        QString msg = "Could find any features in: " + terrainPath;
        this->errorMessage(msg);
    }

    return featureArray;
}

QString HurricaneSelectionWidget::getTerrainGeojsonPath()
{
    QFileDialog dialog(this);
    QString terrainPath = QFileDialog::getOpenFileName(this,tr("Terrain file (.geojson)"),QString(),QString("*.geojson"));
    dialog.close();

    auto oldTerrainPath = terrainLineEdit->text();

    // Return if the user cancels or enters same file
    if(terrainPath.isEmpty() || terrainPath == oldTerrainPath)
    {
        return QString();
    }

    // check file exists & set apps current dir of it does
    QFileInfo fileInfo(terrainPath);
    if (!fileInfo.exists()){
        QString msg = QString("File does not exist: ") + terrainPath;
        this->errorMessage(msg);
        return QString();
    }

    terrainLineEdit->setText(terrainPath);

    return terrainPath;
}




