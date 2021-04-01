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
#include "SiteConfig.h"
#include "GridNode.h"

#include "Feature.h"
#include "FeatureCollection.h"
#include "FeatureCollectionLayer.h"
#include "SimpleRenderer.h"
#include "SimpleMarkerSymbol.h"

#include <QApplication>
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
    progressBar = nullptr;
    fileInputWidget = nullptr;
    progressBarWidget = nullptr;
    theStackedWidget = nullptr;
    progressLabel = nullptr;
    selectedHurricaneName = nullptr;
    selectedHurricaneSeason = nullptr;
    selectedHurricaneSID = nullptr;
    siteConfig = nullptr;
    typeOfScenarioWidget = nullptr;
    selectHurricaneWidget = nullptr;
    specifyHurricaneWidget = nullptr;
    loadDbButton = nullptr;
    siteGrid = nullptr;
    hurricaneImportTool = nullptr;
    hurricaneParamsWidget = nullptr;

    eventDatabaseFile = "";

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(0,0,0,0);

    layout->addWidget(this->getHurricaneSelectionWidget());
    layout->addStretch();
    this->setLayout(layout);

    //    this->loadUserHurricaneData();
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


bool HurricaneSelectionWidget::outputToJSON(QJsonObject &jsonObj)
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


void HurricaneSelectionWidget::showUserGMLayers(bool state)
{
    auto layersTreeView = theVisualizationWidget->getLayersTree();


    if(state == false)
    {
        layersTreeView->removeItemFromTree("User Ground Motions");

        return;
    }


    // Check if there is a 'User Ground Motions' root item in the tree
    auto shakeMapTreeItem = layersTreeView->getTreeItem("User Ground Motions",nullptr);

    // If there is no item, create one
    if(shakeMapTreeItem == nullptr)
        shakeMapTreeItem = layersTreeView->addItemToTree("User Ground Motions",QString());

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
    QWidget* selectHurricaneWidget = new QWidget(this);

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
    QWidget* specifyHurricaneWidget = new QWidget(this);
    QGridLayout* specifyHurricaneLayout = new QGridLayout(specifyHurricaneWidget);
    specifyHurricaneLayout->setContentsMargins(0,0,0,0);
    //    specifyHurricaneLayout->setSpacing(8);

    QLabel* trackLabel = new QLabel("Hurricane Track");
    QLineEdit* trackLineEdit = new QLineEdit();
    QPushButton *browseTrackButton = new QPushButton("Browse");

    QLabel* terrainLabel = new QLabel("Terrain");
    QLineEdit* terrainLineEdit = new QLineEdit();
    QPushButton *browseTerrainButton = new QPushButton("Browse");

    QPushButton *selectLandfall = new QPushButton("Select Landfall");

    specifyHurricaneLayout->addWidget(trackLabel,0,0);
    specifyHurricaneLayout->addWidget(trackLineEdit,0,1);
    specifyHurricaneLayout->addWidget(browseTrackButton,0,2);
    specifyHurricaneLayout->addWidget(terrainLabel,1,0);
    specifyHurricaneLayout->addWidget(terrainLineEdit,1,1);
    specifyHurricaneLayout->addWidget(browseTerrainButton,1,2);
    specifyHurricaneLayout->addWidget(selectLandfall,2,0,1,3);

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

    QFrame* gridGroupBox = new QFrame(this);
    //    gridGroupBox->setObjectName("TopLine");
    //    gridGroupBox->setStyleSheet("#TopLine { border-top: 2px solid black; }");
    gridGroupBox->setContentsMargins(0,0,0,0);

    QGridLayout *gridLayout = new QGridLayout(gridGroupBox);

    QHBoxLayout* gridButtonsLayout = new QHBoxLayout();
    gridButtonsLayout->addWidget(defineGridButton);
    gridButtonsLayout->addWidget(selectGridButton);
    gridButtonsLayout->addWidget(clearGridButton);

    gridLayout->addLayout(gridButtonsLayout,0,0,1,3);

    auto divLatLabel = new QLabel("Divisions Latitude",this);
    auto divLatSpinBox = new QSpinBox(this);
    divLatSpinBox->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);
    divLatSpinBox->setRange(1, 60);
    divLatSpinBox->setValue(5);
    divLatSpinBox->setValue(siteGrid->latitude().divisions());

    auto divLonLabel = new QLabel("Divisions Longitude",this);
    auto divLonSpinBox = new QSpinBox(this);
    divLonSpinBox->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);
    divLonSpinBox->setRange(1, 60);
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

    auto numIMsLabel = new QLabel("Number per site",this);
    auto numIMsLineEdit = new QLineEdit(this);
    numIMsLineEdit->setText("1");

    gridLayout->addWidget(divLatLabel,0,3);
    gridLayout->addWidget(divLatSpinBox,0,4);
    gridLayout->addWidget(divLonLabel,0,5);
    gridLayout->addWidget(divLonSpinBox,0,6);
    gridLayout->addWidget(numIMsLabel,0,7);
    gridLayout->addWidget(numIMsLineEdit,0,8);

    auto runButton = new QPushButton(tr("&Run"));
    auto settingButton = new QPushButton(tr("&Settings"));

    QFrame* runFrame = new QFrame(this);
    //    runFrame->setObjectName("TopLine");
    //    runFrame->setStyleSheet("#TopLine { border-top: 2px solid black; }");
    runFrame->setContentsMargins(0,0,0,0);

    auto runLayout = new QHBoxLayout(runFrame);
    runLayout->addWidget(settingButton,0,Qt::AlignLeft);
    runLayout->addWidget(runButton,1);

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

    if(res != 0)
        this->userMessageDialog(errMsg);

    progressLabel->setVisible(false);

    // Reset the widget back to the input pane and close
    theStackedWidget->setCurrentWidget(fileInputWidget);
    fileInputWidget->setVisible(true);

    if(theStackedWidget->isModal())
        theStackedWidget->close();

    emit loadingComplete(true);

    return;
}


void HurricaneSelectionWidget::setCurrentlyViewable(bool status){

    if (status == true)
        mapViewSubWidget->setCurrentlyViewable(status);
    else
        this->clearGridFromMap();
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
}


void HurricaneSelectionWidget::handleHurricaneSelect(void)
{
    auto selectedFeatures = theVisualizationWidget->getSelectedFeaturesList();

    if(selectedFeatures.empty())
        return;

    QString hurricaneSID;

    // Only select the first hurricane
    for(auto&& it : selectedFeatures)
    {
        auto attrbList = it->attributes();

        auto featType = attrbList->attributeValue("AssetType");

        if(featType.toString() != "HURRICANE")
            continue;

        auto hurricaneName = attrbList->attributeValue("NAME").toString();
        hurricaneSID = attrbList->attributeValue("SID").toString();
        auto hurricaneSeason = attrbList->attributeValue("SEASON").toString();

        selectedHurricaneName->setText(hurricaneName);
        selectedHurricaneSID->setText(hurricaneSID);
        selectedHurricaneSeason->setText(hurricaneSeason);
    }

    auto selectedHurricane = hurricaneImportTool->getHurricane(hurricaneSID);

    if(selectedHurricane == nullptr)
    {
        QString err = "Could not find the hurricane with the SID " + hurricaneSID;
        qDebug()<<err;
        return;
    }

    auto pressure = selectedHurricane->getPressureAtLandfall();
    auto lat = selectedHurricane->getLatitudeAtLandfall();
    auto lon = selectedHurricane->getLongitudeAtLandfall();
    auto stormDir = selectedHurricane->getLandingAngle();
    auto stormSpeed = selectedHurricane->getStormSpeedAtLandfall();
    auto radius = selectedHurricane->getRadiusAtLandfall();

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
}


void HurricaneSelectionWidget::showGridOnMap(void)
{
    mapViewSubWidget->addGridToScene();
}


void HurricaneSelectionWidget::handleGridSelected(void)
{

    // Create the objects needed to visualize the grid in the GIS
    auto siteGrid = mapViewSubWidget->getGrid();

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

//    QStringList headerRow = {"Station", "Latitude", "Longitude"};
//    gridData.push_back(headerRow);

    for(int i = 0; i<gridNodeVec.size(); ++i)
    {
        auto gridNode = gridNodeVec.at(i);

        // The station id
        auto stationName = QString::number(i);

        auto screenPoint = gridNode->getPoint();

        // The latitude and longitude
        auto longitude = theVisualizationWidget->getLongFromScreenPoint(screenPoint);
        auto latitude = theVisualizationWidget->getLatFromScreenPoint(screenPoint);

        // create the feature attributes
        QMap<QString, QVariant> featureAttributes;
        featureAttributes.insert("Station Name", stationName);
        featureAttributes.insert("AssetType", "WindfieldGridPoint");
        featureAttributes.insert("TabName", "Wind Field Grid Point");
        featureAttributes.insert("Latitude", latitude);
        featureAttributes.insert("Longitude", longitude);

        // Create the point and add it to the feature table
        Point point(longitude,latitude);
        Feature* feature = gridFeatureCollectionTable->createFeature(featureAttributes, point, this);

        gridFeatureCollectionTable->addFeature(feature);
    }

    // Create a new layer
    LayerTreeView *layersTreeView = theVisualizationWidget->getLayersTree();

    // Check if there is a 'User Ground Motions' root item in the tree
    auto userInputTreeItem = layersTreeView->getTreeItem("Hurricane Simulation Grid", nullptr);

    // If there is no item, create one
    if(userInputTreeItem == nullptr)
        userInputTreeItem = layersTreeView->addItemToTree("Hurricane Simulation Grid", QString());

    // Add the event layer to the layer tree
    auto eventItem = layersTreeView->addItemToTree("Windfield", QString(), userInputTreeItem);

    // Add the event layer to the map
    theVisualizationWidget->addLayerToMap(gridLayer,eventItem);

    this->clearGridFromMap();
}


void HurricaneSelectionWidget::clearGridFromMap(void)
{
    mapViewSubWidget->removeGridFromScene();

}

