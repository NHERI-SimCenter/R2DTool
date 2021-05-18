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
#include "PolygonBoundary.h"
#include "CSVReaderWriter.h"
#include "Utils/PythonProgressDialog.h"

#include "GroupLayer.h"
#include "Feature.h"
#include "FeatureCollection.h"
#include "FeatureCollectionLayer.h"
#include "SimpleRenderer.h"
#include "SimpleFillSymbol.h"
#include "SimpleMarkerSymbol.h"

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

using namespace Esri::ArcGISRuntime;

HurricaneSelectionWidget::HurricaneSelectionWidget(VisualizationWidget* visWidget, QWidget *parent) : SimCenterAppWidget(parent), theVisualizationWidget(visWidget)
{
    fileInputWidget = nullptr;
    gridLayer = nullptr;
    landfallItem = nullptr;
    hurricaneTrackItem = nullptr;
    hurricaneTrackPointsItem = nullptr;
    hurricaneImportTool = nullptr;
    hurricaneParamsWidget = nullptr;
    loadDbButton = nullptr;
    numIMsLineEdit = nullptr;
    progressBar = nullptr;
    progressBarWidget = nullptr;
    progressLabel = nullptr;
    selectHurricaneWidget = nullptr;
    selectedHurricaneFeature = nullptr;
    selectedHurricaneLayer = nullptr;
    selectedHurricaneItem = nullptr;
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

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(0,0,0,0);

    layout->addWidget(this->getHurricaneSelectionWidget());
    layout->addStretch();
    this->setLayout(layout);

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


bool HurricaneSelectionWidget::outputToJSON(QJsonObject &/*jsonObj*/)
{
    // qDebug() << "USER GM outputPLAIN";

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
        return theStackedWidget.get();

    theStackedWidget = std::make_unique<QStackedWidget>();
    theStackedWidget->setContentsMargins(0,0,0,0);

    //
    // file and dir input
    //

    fileInputWidget = new QWidget(this);
    fileInputWidget->setContentsMargins(0,0,0,0);

    QGridLayout *mainLayout = new QGridLayout(fileInputWidget);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);

    mapViewSubWidget = std::make_unique<EmbeddedMapViewWidget>(nullptr);
    siteConfig = new SiteConfig();
    siteGrid = &siteConfig->siteGrid();

    auto userGrid = mapViewSubWidget->getGrid();
    userGrid->createGrid();
    userGrid->setSiteGridConfig(siteConfig);
    userGrid->setVisualizationWidget(theVisualizationWidget);


    QComboBox* simulationTypeComboBox = new QComboBox(this);
    simulationTypeComboBox->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);

    simulationTypeComboBox->addItem("Specify Hurricane Track");
    simulationTypeComboBox->addItem("Select Hurricane Track from Database");
    typeOfScenarioWidget = new QStackedWidget(this);
    typeOfScenarioWidget->setContentsMargins(5,0,5,0);

    // Widget to select hurricane from database
    selectHurricaneWidget = new QWidget(this);

    selectHurricaneWidget->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum);

    QGridLayout *selectHurricaneLayout = new QGridLayout(selectHurricaneWidget);
    selectHurricaneLayout->setContentsMargins(0,5,0,0);

    loadDbButton = new QPushButton("Load Hurricane Database",this);
    loadDbButton->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum);

    connect(loadDbButton,SIGNAL(clicked()),this,SLOT(loadHurricaneButtonClicked()));

    QPushButton *selectHurricaneButton = new QPushButton("Select Hurricane",this);

    connect(selectHurricaneButton,&QPushButton::clicked,this,&HurricaneSelectionWidget::handleHurricaneSelect);

    QLabel* selectedHurricaneLabel = new QLabel("Hurricane Selected: ",this);
    QLabel* SIDLabel = new QLabel("SID: ",this);
    QLabel* seasonLabel = new QLabel("Season: ",this);

    selectedHurricaneName = new QLabel("None",this);
    selectedHurricaneSID = new QLabel("None",this);
    selectedHurricaneSeason = new QLabel("None",this);

    hurricaneParamsWidget = new HurricaneParameterWidget(this);

    selectHurricaneLayout->addWidget(loadDbButton,0,0);
    selectHurricaneLayout->addWidget(selectHurricaneButton,0,1);
    selectHurricaneLayout->addWidget(selectedHurricaneLabel,1,0);
    selectHurricaneLayout->addWidget(selectedHurricaneName,1,1);
    selectHurricaneLayout->addWidget(seasonLabel,2,0);
    selectHurricaneLayout->addWidget(selectedHurricaneSeason,2,1);
    selectHurricaneLayout->addWidget(SIDLabel,3,0);
    selectHurricaneLayout->addWidget(selectedHurricaneSID,3,1);
    selectHurricaneLayout->rowStretch(4);

    // Widget to specify hurricane track
    specifyHurricaneWidget = new QWidget(this);
    QGridLayout* specifyHurricaneLayout = new QGridLayout(specifyHurricaneWidget);
    specifyHurricaneLayout->setContentsMargins(0,0,0,0);
    //    specifyHurricaneLayout->setSpacing(8);

    QLabel* trackLabel = new QLabel("Hurricane Track (.csv)");
    trackLineEdit = new QLineEdit();
    QPushButton *browseTrackButton = new QPushButton("Browse");

    connect(browseTrackButton,&QPushButton::clicked,this,&HurricaneSelectionWidget::handleHurricaneTrackImport);

    QLabel* terrainLabel = new QLabel("Terrain (.geojson)");
    terrainLineEdit = new QLineEdit();
    QPushButton *browseTerrainButton = new QPushButton("Browse");

    connect(browseTerrainButton,&QPushButton::clicked,this,&HurricaneSelectionWidget::handleTerrainImport);

    QPushButton *defineLandfall = new QPushButton("Define Landfall on Map");
    QPushButton *clearLandfall = new QPushButton("Clear Landfall");
    QPushButton *selectLandfall = new QPushButton("Select Landfall");

    connect(defineLandfall,&QPushButton::clicked,this,&HurricaneSelectionWidget::showPointOnMap);
    connect(selectLandfall,&QPushButton::clicked,this,&HurricaneSelectionWidget::handleLandfallPointSelected);
    connect(clearLandfall,&QPushButton::clicked,this,&HurricaneSelectionWidget::clearLandfallFromMap);

    QLabel* landfallLabel = new QLabel("Specify Hurricane Landfall:",this);

    QHBoxLayout* selectLandfallLayout = new QHBoxLayout();
    selectLandfallLayout->addWidget(landfallLabel);
    selectLandfallLayout->addWidget(defineLandfall);
    selectLandfallLayout->addWidget(selectLandfall);
    selectLandfallLayout->addWidget(clearLandfall);

    specifyHurricaneLayout->addWidget(trackLabel,0,0);
    specifyHurricaneLayout->addWidget(trackLineEdit,0,1);
    specifyHurricaneLayout->addWidget(browseTrackButton,0,2);
    specifyHurricaneLayout->addWidget(terrainLabel,1,0);
    specifyHurricaneLayout->addWidget(terrainLineEdit,1,1);
    specifyHurricaneLayout->addWidget(browseTerrainButton,1,2);
    specifyHurricaneLayout->addLayout(selectLandfallLayout,2,0,1,3);

    //    connect(browseFileButton,SIGNAL(clicked()),this,SLOT(chooseEventFileDialog()));

    typeOfScenarioWidget->addWidget(specifyHurricaneWidget);
    typeOfScenarioWidget->addWidget(selectHurricaneWidget);

    connect(simulationTypeComboBox,SIGNAL(currentIndexChanged(int)),typeOfScenarioWidget,SLOT(setCurrentIndex(int)));


    // Grid selection
    auto defineGridButton = new QPushButton(tr("&Define Grid on Map"),this);
    auto selectGridButton = new QPushButton(tr("&Select Grid"),this);
    auto clearGridButton = new QPushButton(tr("&Clear Grid"),this);

    //    defineGridButton->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum);
    //    selectGridButton->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum);
    //    clearGridButton->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum);

    connect(defineGridButton,&QPushButton::clicked,this,&HurricaneSelectionWidget::showGridOnMap);
    connect(selectGridButton,&QPushButton::clicked,this,&HurricaneSelectionWidget::handleGridSelected);
    connect(clearGridButton,&QPushButton::clicked,this,&HurricaneSelectionWidget::clearGridFromMap);

    QLabel* gridLabel = new QLabel("Specify Windfield Grid:",this);

    QFrame* gridGroupBox = new QFrame(this);
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

    auto divLatLabel = new QLabel("Divisions Latitude",this);
    divLatSpinBox = new QSpinBox(this);
    divLatSpinBox->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
    divLatSpinBox->setRange(1, 1000);
    divLatSpinBox->setValue(5);
    divLatSpinBox->setValue(siteGrid->latitude().divisions());

    auto divLonLabel = new QLabel("Divisions Longitude",this);
    divLonSpinBox = new QSpinBox(this);
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

    runButton = new QPushButton(tr("&Run Hurricane Simulation"), this);
    connect(runButton,&QPushButton::clicked,this,&HurricaneSelectionWidget::runHazardSimulation);

    gridLayout->addWidget(runButton,0,9);

    QFrame* runFrame = new QFrame(this);
    //    runFrame->setObjectName("TopLine");
    //    runFrame->setStyleSheet("#TopLine { border-top: 2px solid black; }");
    runFrame->setContentsMargins(0,0,0,0);

    // Functionality to truncate the track
    auto truncTrackSelectButton = new QPushButton(tr("&Select Area on Map"),this);
    auto truncTrackApplyButton = new QPushButton(tr("&Apply"),this);
    auto truncTrackClearButton = new QPushButton(tr("&Clear"),this);

    auto thePolygonBoundaryTool = theVisualizationWidget->getThePolygonBoundaryTool();

    connect(truncTrackSelectButton,&QPushButton::clicked,this,&HurricaneSelectionWidget::handleSelectAreaMap);
    connect(truncTrackClearButton,&QPushButton::clicked,this,&HurricaneSelectionWidget::handleClearSelectAreaMap);
    connect(truncTrackApplyButton,SIGNAL(clicked()),thePolygonBoundaryTool,SLOT(getItemsInPolygonBoundary())); // Asynchronous task

    auto truncLabel = new QLabel("Truncate Hurricane Track:");

    auto runLayout = new QHBoxLayout(runFrame);\

    runLayout->addWidget(truncLabel);
    runLayout->addWidget(truncTrackSelectButton);
    runLayout->addWidget(truncTrackApplyButton);
    runLayout->addWidget(truncTrackClearButton);

    auto scenarioTypeLabel = new QLabel("Hurricane Scenario Type",this);

    auto topLayout = new QHBoxLayout();
    topLayout->addWidget(scenarioTypeLabel);
    topLayout->addWidget(simulationTypeComboBox);

    mainLayout->addLayout(topLayout, 0,0);

    mainLayout->addWidget(hurricaneParamsWidget, 0,1,2,1);

    mainLayout->addWidget(typeOfScenarioWidget, 1,0,2,1);
    mainLayout->addWidget(runFrame, 2,1);

    mainLayout->addWidget(gridGroupBox, 3,0,1,4);

    mainLayout->addWidget(mapViewSubWidget.get(), 4,0,1,4);

    //
    // progress bar
    //

    progressBarWidget = new QWidget(this);
    auto progressBarLayout = new QVBoxLayout(progressBarWidget);
    progressBarWidget->setLayout(progressBarLayout);

    auto progressText = new QLabel("Loading hurricane database. This may take a while.",progressBarWidget);
    progressLabel =  new QLabel("",this);
    progressBar = new QProgressBar(progressBarWidget);

    hurricaneImportTool = std::make_unique<HurricanePreprocessor>(progressBar, theVisualizationWidget, this);

    auto vspacer = new QSpacerItem(0,0,QSizePolicy::Minimum, QSizePolicy::Expanding);
    progressBarLayout->addItem(vspacer);
    progressBarLayout->addWidget(progressText,1, Qt::AlignCenter);
    progressBarLayout->addWidget(progressLabel,1, Qt::AlignCenter);
    progressBarLayout->addWidget(progressBar);
    progressBarLayout->addItem(vspacer);
    progressBarLayout->addStretch(1);

    //
    // add file and progress widgets to stacked widgets, then set defaults
    //

    theStackedWidget->addWidget(fileInputWidget);
    theStackedWidget->addWidget(progressBarWidget);

    theStackedWidget->setCurrentWidget(fileInputWidget);

    theStackedWidget->setWindowTitle("Hurricane track selection");

    return theStackedWidget.get();
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
    auto res = hurricaneImportTool->loadHurricaneTrackData(eventDatabaseFile,errMsg);

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


void HurricaneSelectionWidget::setCurrentlyViewable(bool status){

    if (status == true)
        mapViewSubWidget->setCurrentlyViewable(status);
    else
        mapViewSubWidget->removeGridFromScene();
}


void HurricaneSelectionWidget::loadHurricaneButtonClicked(void)
{

    auto pathToHurricaneDb = QCoreApplication::applicationDirPath() +
            QDir::separator() + "Databases" + QDir::separator() + "ibtracs.last3years.list.v04r00.csv";

    QFile file(pathToHurricaneDb);

    // Return if database does not exist
    if(!file.exists())
        return;

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

    hurricaneImportTool->clear();
    hurricaneParamsWidget->clear();

    this->clearGridFromMap();
    this->clearLandfallFromMap();

    delete selectedHurricaneLayer;
    selectedHurricaneLayer = nullptr;
    selectedHurricaneItem = nullptr;
    hurricaneTrackPointsItem = nullptr;
    hurricaneTrackItem = nullptr;

    numIMsLineEdit->setText("3");

    trackLineEdit->clear();
    terrainLineEdit->clear();

    divLatSpinBox->setValue(10);
    divLonSpinBox->setValue(10);

    selectedHurricaneObj.clear();
}


void HurricaneSelectionWidget::handleHurricaneSelect(void)
{
    auto selectedFeatures = theVisualizationWidget->getSelectedFeaturesList();

    if(selectedFeatures.empty())
        return;

    QString hurricaneSID;

    // Only select the first hurricane, if there is more than one selected
    for(auto&& it : selectedFeatures)
    {
        auto attrbList = it->attributes();

        auto featType = attrbList->attributeValue("AssetType");

        if(featType.toString() != "HURRICANE")
            continue;

        if(selectedHurricaneFeature == it)
            return;
        else
            selectedHurricaneFeature = it;

        auto hurricaneName = attrbList->attributeValue("NAME").toString();
        hurricaneSID = attrbList->attributeValue("SID").toString();
        auto hurricaneSeason = attrbList->attributeValue("SEASON").toString();

        selectedHurricaneName->setText(hurricaneName);
        selectedHurricaneSID->setText(hurricaneSID);
        selectedHurricaneSeason->setText(hurricaneSeason);

        break;
    }

    // Get the selected hurricane from the preprocessor
    auto importedHurricane = hurricaneImportTool->getHurricane(hurricaneSID);

    if(importedHurricane == nullptr)
    {
        QString err = "Could not find the hurricane with the SID " + hurricaneSID;
        qDebug()<<err;
        return;
    }

    selectedHurricaneObj = *importedHurricane;

    // Populate the landfall parameters
    auto pressure = selectedHurricaneObj.getPressureAtLandfall();
    auto lat = selectedHurricaneObj.getLatitudeAtLandfall();
    auto lon = selectedHurricaneObj.getLongitudeAtLandfall();
    auto stormDir = selectedHurricaneObj.getLandingAngle();
    auto stormSpeed = selectedHurricaneObj.getStormSpeedAtLandfall();
    auto radius = selectedHurricaneObj.getRadiusAtLandfall();

    if(lat == 0.0 || lon == 0.0)
    {
        QString err = "The latitude and/or longitude should not be 0";
        qDebug()<<err;
    }

    if(radius == 0.0)
    {
        QString warn = "Warning: The storm radius is 0.0 from the database, assuming a radius of 50 nmile";
        qDebug()<<warn;
        radius = 50.0;
    }

    hurricaneParamsWidget->setLandfallLat(lat);
    hurricaneParamsWidget->setLandfallLon(lon);
    hurricaneParamsWidget->setLandfallPress(pressure);
    hurricaneParamsWidget->setLandingAngle(stormDir);
    hurricaneParamsWidget->setLandfallSpeed(stormSpeed);
    hurricaneParamsWidget->setLandfallRadius(radius);

    this->createHurricaneVisuals(&selectedHurricaneObj);

    // Set the all hurricanes layer to off
    auto allHurricanesLayer = hurricaneImportTool->getAllHurricanesLayer();
    theVisualizationWidget->setLayerVisibility(allHurricanesLayer->layerId(), false);
    theVisualizationWidget->clearSelection();
}


void HurricaneSelectionWidget::createHurricaneVisuals(HurricaneObject* hurricane)
{
    // Get the hurricane description
    auto name = hurricane->name;
    auto landfallData = hurricane->landfallData;

    if(selectedHurricaneLayer == nullptr)
    {
        selectedHurricaneLayer = new GroupLayer(QList<Layer*>{},this);
        selectedHurricaneLayer->setName("Hurricane " + name);
        selectedHurricaneLayer->setAutoFetchLegendInfos(true);

        selectedHurricaneItem = theVisualizationWidget->addSelectedFeatureLayerToMap(selectedHurricaneLayer);
    }

    // Track
    QString err;
    hurricaneTrackItem = hurricaneImportTool->createTrackVisualization(hurricane,selectedHurricaneItem,selectedHurricaneLayer, err);
    if(hurricaneTrackItem == nullptr)
    {
        this->errorMessage(err);
        return;
    }

    // Track points
    hurricaneTrackPointsItem = hurricaneImportTool->createTrackPointsVisualization(hurricane,selectedHurricaneItem, selectedHurricaneLayer,err);
    if(hurricaneTrackPointsItem == nullptr)
    {
        this->errorMessage(err);
        return;
    }

    auto lat = hurricane->getLatitudeAtLandfall();
    auto lon = hurricane->getLongitudeAtLandfall();

    // Landfall
    if(!landfallData.empty())
    {
        auto lfParams = hurricane->parameterLabels;

        QMap<QString, QVariant> featureAttributes;
        for(int i = 0; i < landfallData.size(); ++i)
        {
            auto dataVal = landfallData.at(i);

            if(!dataVal.isEmpty())
                featureAttributes.insert(lfParams.at(i), landfallData.at(i));
        }

        featureAttributes.insert("Station Name", "Landfall");
        featureAttributes.insert("AssetType", "HURRICANE_LANDFALL");
        featureAttributes.insert("TabName", "Landfall Point");
        featureAttributes.insert("Latitude", lat);
        featureAttributes.insert("Longitude", lon);

        landfallItem = hurricaneImportTool->createLandfallVisualization(lat,lon,featureAttributes, selectedHurricaneItem, selectedHurricaneLayer);
    }
}


void HurricaneSelectionWidget::showGridOnMap(void)
{
    mapViewSubWidget->addGridToScene();
}


void HurricaneSelectionWidget::showPointOnMap(void)
{
    mapViewSubWidget->addPointToScene();
}


void HurricaneSelectionWidget::handleGridSelected(void)
{

    // Create the objects needed to visualize the grid in the GIS
    auto siteGrid = mapViewSubWidget->getGrid();

    if(!siteGrid->isVisible())
        return;

    // Get the vector of grid nodes
    auto gridNodeVec = siteGrid->getGridNodeVec();

    if(gridNodeVec.isEmpty())
        return;

    // Create the table to store the fields
    QList<Field> tableFields;
    tableFields.append(Field::createText("AssetType", "NULL",4));
    tableFields.append(Field::createText("TabName", "NULL",4));
    tableFields.append(Field::createText("Station Name", "NULL",4));
    tableFields.append(Field::createText("Latitude", "NULL",8));
    tableFields.append(Field::createText("Longitude", "NULL",9));
    tableFields.append(Field::createText("Peak Wind Speeds", "NULL",9));

    auto gridFeatureCollection = new FeatureCollection(this);

    // Create the feature collection table/layers
    auto gridFeatureCollectionTable = new FeatureCollectionTable(tableFields, GeometryType::Point, SpatialReference::wgs84(), this);
    gridFeatureCollection->tables()->append(gridFeatureCollectionTable);

    gridLayer = new FeatureCollectionLayer(gridFeatureCollection,this);
    gridLayer->setAutoFetchLegendInfos(true);
    gridLayer->setName("Wind Field Grid");

    // Create red cross SimpleMarkerSymbol
    SimpleMarkerSymbol* crossSymbol = new SimpleMarkerSymbol(SimpleMarkerSymbolStyle::Cross, QColor("black"), 6, this);

    // Create renderer and set symbol to crossSymbol
    SimpleRenderer* renderer = new SimpleRenderer(crossSymbol, this);
    renderer->setLabel("Windfield Grid Point");

    // Set the renderer for the feature layer
    gridFeatureCollectionTable->setRenderer(renderer);

    QStringList headerRow = {"Station", "Latitude", "Longitude"};
    gridData.push_back(headerRow);

    for(int i = 0; i<gridNodeVec.size(); ++i)
    {
        auto gridNode = gridNodeVec.at(i);

        // The station id
        auto stationName = QString::number(i+1);

        auto screenPoint = gridNode->getPoint();

        // The latitude and longitude
        auto longitude = theVisualizationWidget->getLongFromScreenPoint(screenPoint);
        auto latitude = theVisualizationWidget->getLatFromScreenPoint(screenPoint);

        WindFieldStation station(stationName,latitude,longitude);

        // create the feature attributes
        QMap<QString, QVariant> featureAttributes;
        featureAttributes.insert("Station Name", stationName);
        featureAttributes.insert("AssetType", "WindfieldGridPoint");
        featureAttributes.insert("TabName", "Wind Field Grid Point");
        featureAttributes.insert("Latitude", latitude);
        featureAttributes.insert("Longitude", longitude);
        featureAttributes.insert("Peak Wind Speeds", "N/A");

        // Create the point and add it to the feature table
        Point point(longitude,latitude);
        Feature* feature = gridFeatureCollectionTable->createFeature(featureAttributes, point, this);

        station.setStationFeature(feature);

        gridFeatureCollectionTable->addFeature(feature);

        QStringList stationRow;
        stationRow.push_back(stationName);
        stationRow.push_back(QString::number(latitude));
        stationRow.push_back(QString::number(longitude));

        stationMap.insert(stationName,station);

        gridData.push_back(stationRow);
    }


    // Create a new layer
    LayerTreeView *layersTreeView = theVisualizationWidget->getLayersTree();

    // Check if there is a 'User Ground Motions' root item in the tree
    auto hurricaneMainItem = layersTreeView->getTreeItem("Hurricanes", nullptr);

    // If there is no item, create one
    if(hurricaneMainItem == nullptr)
    {
        auto gridID = theVisualizationWidget->createUniqueID();
        hurricaneMainItem = layersTreeView->addItemToTree("Hurricanes", gridID);
    }

    // Add the event layer to the layer tree
    //    auto eventID = theVisualizationWidget->createUniqueID();
    //    auto eventItem = layersTreeView->addItemToTree("Windfield Grid", eventID, hurricaneGridItem);

    // Add the event layer to the map
    theVisualizationWidget->addLayerToMap(gridLayer,hurricaneMainItem);

    mapViewSubWidget->removeGridFromScene();
}


void HurricaneSelectionWidget::handleLandfallPointSelected(void)
{

    // Create the objects needed to visualize the grid in the GIS
    auto landfallPoint = mapViewSubWidget->getPoint();

    if(!landfallPoint->isVisible())
        return;

    // Get the vector of grid nodes
    auto posNodeVec = landfallPoint->pos();

    if(posNodeVec.isNull())
        return;

    // The latitude and longitude
    auto longitude = theVisualizationWidget->getLongFromScreenPoint(posNodeVec);
    auto latitude = theVisualizationWidget->getLatFromScreenPoint(posNodeVec);

    hurricaneParamsWidget->setLandfallLat(latitude);
    hurricaneParamsWidget->setLandfallLon(longitude);

    // create the feature attributes
    QMap<QString, QVariant> featureAttributes;
    featureAttributes.insert("Station Name", "Landfall");
    featureAttributes.insert("AssetType", "HURRICANE_LANDFALL");
    featureAttributes.insert("TabName", "Landfall Point");
    featureAttributes.insert("Latitude", latitude);
    featureAttributes.insert("Longitude", longitude);


    // Create a new hurricane layer
    if(selectedHurricaneLayer == nullptr)
    {
        auto name =selectedHurricaneObj.name;

        if(name.isEmpty())
            name = "USER SPECIFIED";

        selectedHurricaneLayer = new GroupLayer(QList<Layer*>{},this);
        selectedHurricaneLayer->setName("Hurricane " + name);
        selectedHurricaneLayer->setAutoFetchLegendInfos(true);

        selectedHurricaneItem = theVisualizationWidget->addSelectedFeatureLayerToMap(selectedHurricaneLayer);
    }


    landfallItem = hurricaneImportTool->createLandfallVisualization(latitude,longitude,featureAttributes, selectedHurricaneItem,selectedHurricaneLayer);

    mapViewSubWidget->removePointFromScene();

    theVisualizationWidget->hideLegend();
}


void HurricaneSelectionWidget::clearGridFromMap(void)
{
    if(gridLayer)
    {
        LayerTreeView *layersTreeView = theVisualizationWidget->getLayersTree();
        layersTreeView->removeItemFromTree(gridLayer->layerId());

        delete gridLayer;
        gridLayer = nullptr;
    }

    stationMap.clear();
    gridData.clear();
    mapViewSubWidget->removeGridFromScene();
}


void HurricaneSelectionWidget::clearLandfallFromMap(void)
{
    if(landfallItem)
    {
        LayerTreeView *layersTreeView = theVisualizationWidget->getLayersTree();
        auto res = layersTreeView->removeItemFromTree(landfallItem->getItemID());

        if(res == false)
            qDebug()<<"Error removing landfall item from tree";

        landfallItem = nullptr;

        hurricaneParamsWidget->setLandfallLat(0.0);
        hurricaneParamsWidget->setLandfallLon(0.0);

        // Check if the selected hurricane item is still there...
        if(selectedHurricaneLayer)
            selectedHurricaneItem = theVisualizationWidget->getLayersTree()->getTreeItem(selectedHurricaneLayer->layerId());

        if(selectedHurricaneItem == nullptr)
        {
            delete selectedHurricaneLayer;
            selectedHurricaneLayer = nullptr;
        }
    }

    mapViewSubWidget->removePointFromScene();
}


void HurricaneSelectionWidget::runHazardSimulation(void)
{
    this->getProgressDialog()->setVisibility(true);

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
            this->statusMessage("Please specify a terrain.geojson file to run a simulation");
            return;
        }

        scenarioObj.insert("Terrain", pathTerrainFile);
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
        this->errorMessage(err);
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

    // TODO: make this a relative link once we figure out the folder structure
    auto pathToHazardSimScript = "/Users/steve/Desktop/SimCenter/SimCenterBackEndKuanshi/applications/performRegionalEventSimulation/regionalWindField/HurricaneSimulation.py";
    // auto pathToHazardSimScript = "/Users/fmckenna/release/HazardSimulation/HazardSimulation.py";
    //    QString pathToHazardSimScript = SimCenterPreferences::getInstance()->getAppDir() + QDir::separator()
    //            + "applications" + QDir::separator() + "performRegionalEventSimulation" + QDir::separator()
    //            + "regionalWindField" + QDir::separator() + "HurricaneSimulation.py";

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

    auto stationIndex = headerFields.indexOf("Station");
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

        auto pws = station->getPeakWindSpeeds();

        QString pwsStr;
        for(int i = 0; i<pws.size()-1; ++i)
        {
            pwsStr += QString::number(pws[i]) + ", ";
        }

        pwsStr += QString::number(pws.back());

        auto attribute = "Peak Wind Speeds";

        station->updateFeatureAttribute(attribute,QVariant(pwsStr));
    }

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

    selectedHurricaneLayer->load();

    theVisualizationWidget->zoomToLayer(selectedHurricaneLayer->layerId());
}


void HurricaneSelectionWidget::handleTerrainImport(void)
{
    QFileDialog dialog(this);
    QString terrainPath = QFileDialog::getOpenFileName(this,tr("Terrain file (.geojson)"),QString(),QString("*.geojson"));
    dialog.close();

    auto oldTerrainPath = terrainLineEdit->text();

    // Return if the user cancels or enters same file
    if(terrainPath.isEmpty() || terrainPath == oldTerrainPath)
    {
        return;
    }

    // check file exists & set apps current dir of it does
    QFileInfo fileInfo(terrainPath);
    if (!fileInfo.exists()){
        QString msg = QString("File does not exist: ") + terrainPath;
        this->errorMessage(msg);
        return;
    }

    // open file
    QFile file(terrainPath);
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        QString msg = QString("Cannot open the file:") + terrainPath;
        this->errorMessage(msg);
        return;
    }

    terrainLineEdit->setText(terrainPath);

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
        return;
    }

    std::vector<Esri::ArcGISRuntime::Geometry> geomVec;
    std::vector<QVariantMap> propertiesMapVec;

    for(auto&& it : featureArray)
    {
        auto featObj = it.toObject();

        auto geom = featObj["geometry"].toObject();

        auto coordArray = geom["coordinates"].toArray();

        auto featGeom = theVisualizationWidget->getPolygonGeometryFromJson(coordArray);

        if(featGeom.isEmpty())
        {
            QString msg ="Error getting the hurricane geometry in: " + terrainPath;
            this->errorMessage(msg);
            return;
        }

        geomVec.push_back(featGeom);

        auto properties = featObj["properties"].toObject();
        propertiesMapVec.push_back(properties.toVariantMap());
    }

    QList<Field> tableFields;
    tableFields.append(Field::createText("AssetType", "NULL",4));
    tableFields.append(Field::createText("TabName", "NULL",4));

    // Use the property map from the first object to populate the fields. This assumes that all features have the same properties.

    if(!propertiesMapVec.empty())
    {
        auto keys = propertiesMapVec.begin()->keys();
        for(auto&& it : keys)
            tableFields.append(Field::createText(it, "NULL",4));
    }

    auto featureCollection = new FeatureCollection(this);

    auto featureCollectionTable = new FeatureCollectionTable(tableFields, GeometryType::Polygon, SpatialReference::wgs84(),this);

    featureCollection->tables()->append(featureCollectionTable);

    auto newGeojsonLayer = new FeatureCollectionLayer(featureCollection,this);

    newGeojsonLayer->setName("Terrain Roughness");
    newGeojsonLayer->setAutoFetchLegendInfos(true);

    SimpleFillSymbol* fillSymbol = new SimpleFillSymbol(SimpleFillSymbolStyle::Solid, QColor(0, 255, 0, 75), this);
    SimpleRenderer* lineRenderer = new SimpleRenderer(fillSymbol, this);
    lineRenderer->setLabel("Terrain roughness polygon");

    featureCollectionTable->setRenderer(lineRenderer);

    if(propertiesMapVec.size() != geomVec.size())
    {
        QString msg ="Error, inconsistency in geometry and properties vector size";
        this->errorMessage(msg);
        return;
    }

    for(size_t i = 0;i<propertiesMapVec.size(); ++i)
    {
        auto geom = geomVec.at(i);
        auto prop = propertiesMapVec.at(i);

        QMap<QString, QVariant> featureAttributes;
        featureAttributes.insert("AssetType", "HURRICANE_TERRAIN_GEOJSON");
        featureAttributes.insert("TabName", "Terrain Polygon");

        featureAttributes.insert(prop);

        auto feature = featureCollectionTable->createFeature(featureAttributes, geom, this);

        featureCollectionTable->addFeature(feature);
    }


    // Create a new hurricane layer
    if(selectedHurricaneLayer == nullptr)
    {
        auto name =selectedHurricaneObj.name;

        if(name.isEmpty())
            name = "USER SPECIFIED";

        selectedHurricaneLayer = new GroupLayer(QList<Layer*>{},this);
        selectedHurricaneLayer->setName("Hurricane " + name);
        selectedHurricaneLayer->setAutoFetchLegendInfos(true);

        selectedHurricaneItem = theVisualizationWidget->addSelectedFeatureLayerToMap(selectedHurricaneLayer);
    }

    theVisualizationWidget->addLayerToMap(newGeojsonLayer,selectedHurricaneItem, selectedHurricaneLayer);

}


void HurricaneSelectionWidget::handleClearSelectAreaMap(void)
{
    auto thePolygonBoundaryTool = theVisualizationWidget->getThePolygonBoundaryTool();
    thePolygonBoundaryTool->resetPolygonBoundary();
    disconnect(theVisualizationWidget,&VisualizationWidget::taskSelectionComplete,this,&HurricaneSelectionWidget::handleAreaSelected);
}


void HurricaneSelectionWidget::handleSelectAreaMap(void)
{
    connect(theVisualizationWidget,&VisualizationWidget::taskSelectionComplete,this,&HurricaneSelectionWidget::handleAreaSelected);

    auto thePolygonBoundaryTool = theVisualizationWidget->getThePolygonBoundaryTool();

    thePolygonBoundaryTool->getPolygonBoundaryInputs();
}


void HurricaneSelectionWidget::handleAreaSelected(void)
{
    // Get the features from the selection query
    auto selectedFeatures = theVisualizationWidget->getFeaturesFromQueryList();

    HurricaneObject newHurricaneObj = selectedHurricaneObj;

    QVector<QStringList>& hurricaneData = newHurricaneObj.getHurricaneData();

    hurricaneData.clear();

    // Save only the features that are track points
    QList<Feature*> featureList;
    for(auto&& it : selectedFeatures)
    {
        FeatureIterator iter = it->iterator();
        while (iter.hasNext())
        {
            Feature* feature = iter.next();

            auto atrbList = feature->attributes();

            auto artbMap = atrbList->attributesMap();

            auto assetType = artbMap.value("AssetType").toString();

            if(assetType.compare("HURRICANE_TRACK_POINT") == 0)
                featureList.push_back(feature);

        }
    }

    if(featureList.empty())
        return;

    for(auto&& it : featureList)
    {
        auto atrbList = it->attributes();

        auto artbMap = atrbList->attributesMap();

        auto lat = artbMap.value("LAT").toDouble();
        auto lon = artbMap.value("LON").toDouble();

        QStringList trackPoint = selectedHurricaneObj.trackPointAtLatLon(lat,lon);

        if(trackPoint.isEmpty())
        {
            this->errorMessage("Could not get the track point");
            return;
        }

        hurricaneData.push_back(trackPoint);
    }

    // Delete the old hurricane layer
    if(hurricaneTrackItem != nullptr)
    {
        theVisualizationWidget->removeLayerFromMapAndTree(hurricaneTrackItem->getItemID());
        hurricaneTrackItem = nullptr;

        theVisualizationWidget->removeLayerFromMapAndTree(hurricaneTrackPointsItem->getItemID());
        hurricaneTrackPointsItem = nullptr;
    }

    // Check if the selected hurricane item is still there...
    if(selectedHurricaneLayer)
        selectedHurricaneItem = theVisualizationWidget->getLayersTree()->getTreeItem(selectedHurricaneLayer->layerId());

    if(selectedHurricaneItem == nullptr)
    {
        delete selectedHurricaneLayer;
        selectedHurricaneLayer = nullptr;
    }

    // Create the new hurricane layer
    this->createHurricaneVisuals(&newHurricaneObj);

    disconnect(theVisualizationWidget,&VisualizationWidget::taskSelectionComplete,this,&HurricaneSelectionWidget::handleAreaSelected);

    selectedHurricaneObj = newHurricaneObj;
}
