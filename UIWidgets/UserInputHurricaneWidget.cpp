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

// Written by: Stevan Gavrilovic, Frank McKenna


#include "CSVReaderWriter.h"
#include "LayerTreeView.h"
#include "UserInputHurricaneWidget.h"
#include "VisualizationWidget.h"
#include "WorkflowAppR2D.h"
#include "WindFieldStation.h"
#include "SimCenterUnitsWidget.h"

#ifdef ARC_GIS
#include "ArcGISHurricanePreprocessor.h"
#include "ArcGISVisualizationWidget.h"

// GIS Layers
#include <FeatureCollectionLayer.h>
#include <GroupLayer.h>
#include <Layer.h>
#include <LayerListModel.h>
#include <SimpleMarkerSymbol.h>
#include <SimpleRenderer.h>

using namespace Esri::ArcGISRuntime;
#endif

#ifdef Q_GIS
#include "QGISHurricanePreprocessor.h"
#include "QGISVisualizationWidget.h"

#include <qgsvectorlayer.h>
#endif

#include <QApplication>
#include <QDialog>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QProgressBar>
#include <QJsonObject>
#include <QComboBox>
#include <QPushButton>
#include <QSpacerItem>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QDir>

UserInputHurricaneWidget::UserInputHurricaneWidget(VisualizationWidget* visWidget, QWidget *parent) : SimCenterAppWidget(parent), theVisualizationWidget(visWidget)
{
    progressBar = nullptr;
    fileInputWidget = nullptr;
    progressBarWidget = nullptr;
    theStackedWidget = nullptr;
    progressLabel = nullptr;
    unitsWidget = nullptr;
    eventFile = "";

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(this->getUserInputHurricaneWidget());
    layout->addStretch();
    this->setLayout(layout);

}


UserInputHurricaneWidget::~UserInputHurricaneWidget()
{

}


bool UserInputHurricaneWidget::outputAppDataToJSON(QJsonObject &jsonObject) {

    emit eventTypeChangedSignal("Hurricane");

    jsonObject["Application"] = "UserInputHurricane";

    QJsonObject appData;
    QFileInfo theFile(eventFile);
    if (theFile.exists()) {
        appData["eventFile"]=theFile.fileName();
        appData["eventFileDir"]=theFile.path();
    } else {
        appData["eventFile"]=eventFile; // may be valid on others computer
        appData["eventFileDir"]=QString("");
    }
    QFileInfo theDir(eventDir);
    if (theDir.exists()) {
        appData["eventDir"]=theDir.absoluteFilePath();
    } else {
        appData["eventDir"]=QString("None");
    }

    unitsWidget->outputToJSON(appData);

    jsonObject["ApplicationData"]=appData;

    return true;
}


bool UserInputHurricaneWidget::outputToJSON(QJsonObject &jsonObj)
{


    return true;
}


bool UserInputHurricaneWidget::inputAppDataFromJSON(QJsonObject &jsonObj)
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
            pathToFile = QDir::currentPath();

        QString fullFilePath= pathToFile + QDir::separator() + fileName;

        // adam .. adam .. adam
        if (!QFileInfo::exists(fullFilePath)){
            fullFilePath = pathToFile + QDir::separator()
                    + "input_data" + QDir::separator() + fileName;

            if (!QFile::exists(fullFilePath)) {
                this->errorMessage("UserInputWF - could not find EventGrid.csv file");
                return false;
            }
        }

        eventFileLineEdit->setText(fullFilePath);
        eventFile = fullFilePath;

        if (appData.contains("eventDir"))
        {
            eventDir = appData["eventDir"].toString();

            QDir evtD(eventDir);

            if (!evtD.exists()){
                QString trialDir = QDir::currentPath() +
                        QDir::separator() + "input_data" + eventDir;
                if (evtD.exists(trialDir)) {
                    eventDir = trialDir;
                    eventDirLineEdit->setText(trialDir);
                } else {
                    this->errorMessage("UserInputHurricane - could not find hurricane field dir" + eventDir + " " + trialDir);
                    return false;
                }
            } else {
                eventDirLineEdit->setText(eventDir);
            }
        }

        this->loadUserWFData();

        auto res = unitsWidget->inputFromJSON(appData);

        if(!res)
        {
            auto paramNames = unitsWidget->getParameterNames();

            this->infoMessage("Warning \\!/: Failed to find/import the units in 'User Specified Hurricane' widget. Setting default units for the following parameters:");

            for(auto&& it : paramNames)
            {
                auto res = unitsWidget->setUnit(it,"mph");

                if(res == 0)
                    this->infoMessage("For parameter "+it+" setting default unit as: mph");
                else
                    this->errorMessage("Failed to set default units for parameter "+it);
            }
        }

        return true;
    }

    return false;
}


QStackedWidget* UserInputHurricaneWidget::getUserInputHurricaneWidget(void)
{
    if (theStackedWidget)
        return theStackedWidget;

    theStackedWidget = new QStackedWidget();

    //
    // file and dir input
    //

    fileInputWidget = new QWidget();
    QGridLayout *fileLayout = new QGridLayout(fileInputWidget);

    QLabel* selectComponentsText = new QLabel("Event File Listing Hurricane Grid");
    eventFileLineEdit = new QLineEdit();
    QPushButton *browseFileButton = new QPushButton("Browse");

    connect(browseFileButton,SIGNAL(clicked()),this,SLOT(chooseEventFileDialog()));

    fileLayout->addWidget(selectComponentsText, 0,0);
    fileLayout->addWidget(eventFileLineEdit,    0,1);
    fileLayout->addWidget(browseFileButton,     0,2);

    QLabel* selectFolderText = new QLabel("Folder Containing Hurricane Stations");
    eventDirLineEdit = new QLineEdit();
    QPushButton *browseFolderButton = new QPushButton("Browse");

    connect(browseFolderButton,SIGNAL(clicked()),this,SLOT(chooseEventDirDialog()));


    unitsWidget = new SimCenterUnitsWidget();


    fileLayout->addWidget(selectFolderText,   1,0);
    fileLayout->addWidget(eventDirLineEdit, 1,1);
    fileLayout->addWidget(browseFolderButton, 1,2);

    fileLayout->addWidget(unitsWidget,2,0,1,3);

    fileLayout->setRowStretch(3,1);

    //
    // progress bar
    //

    progressBarWidget = new QWidget();
    auto progressBarLayout = new QVBoxLayout(progressBarWidget);
    progressBarWidget->setLayout(progressBarLayout);

    auto progressText = new QLabel("Loading user hurricane data.  This may take a while.",progressBarWidget);
    progressLabel =  new QLabel(" ",this);
    progressBar = new QProgressBar(progressBarWidget);

    auto vspacer = new QSpacerItem(0,0,QSizePolicy::Minimum, QSizePolicy::Expanding);
    auto vspacer2 = new QSpacerItem(0,0,QSizePolicy::Minimum, QSizePolicy::Expanding);
    progressBarLayout->addItem(vspacer);
    progressBarLayout->addWidget(progressText,1, Qt::AlignCenter);
    progressBarLayout->addWidget(progressLabel,1, Qt::AlignCenter);
    progressBarLayout->addWidget(progressBar);
    progressBarLayout->addItem(vspacer2);
    progressBarLayout->addStretch(1);

    //
    // add file and progress widgets to stacked widgets, then set defaults
    //

    theStackedWidget->addWidget(fileInputWidget);
    theStackedWidget->addWidget(progressBarWidget);
    theStackedWidget->setCurrentWidget(fileInputWidget);
    theStackedWidget->setWindowTitle("Select folder containing hurricane track");

    return theStackedWidget;
}


void UserInputHurricaneWidget::showEventSelectDialog(void)
{

    if (!theStackedWidget)
    {
        this->getUserInputHurricaneWidget();
    }

    theStackedWidget->show();
    theStackedWidget->raise();
    theStackedWidget->activateWindow();
}


void UserInputHurricaneWidget::loadHurricaneTrackData(void)
{

#ifdef ARC_GIS
    auto arcVizWidget = static_cast<ArcGISVisualizationWidget*>(theVisualizationWidget);

    if(arcVizWidget == nullptr)
    {
        qDebug()<<"Failed to cast to ArcGISVisualizationWidget";
        return;
    }

    ArcGISHurricanePreprocessor hurricaneImportTool(progressBar, arcVizWidget, this);
#endif


#ifdef Q_GIS
    auto qgisVizWidget = static_cast<QGISVisualizationWidget*>(theVisualizationWidget);

    if(qgisVizWidget == nullptr)
    {
        qDebug()<<"Failed to cast to ArcGISVisualizationWidget";
        return;
    }

    QGISHurricanePreprocessor hurricaneImportTool(progressBar, qgisVizWidget, this);
#endif

    theStackedWidget->setCurrentWidget(progressBarWidget);
    progressBarWidget->setVisible(true);

    QString errMsg;
    auto res = hurricaneImportTool.loadHurricaneDatabaseData(eventFile,errMsg);

    if(res != 0)
        this->statusMessage(errMsg);

    // Reset the widget back to the input pane and close
    theStackedWidget->setCurrentWidget(fileInputWidget);
    fileInputWidget->setVisible(true);

    if(theStackedWidget->isModal())
        theStackedWidget->close();

    emit loadingComplete(true);

    return;
}


void UserInputHurricaneWidget::chooseEventDirDialog(void)
{

    QFileDialog dialog(this);

    dialog.setFileMode(QFileDialog::Directory);
    QString newPath = dialog.getExistingDirectory(this, tr("Dir containing specified motions"));
    dialog.close();

    // Return if the user cancels or enters same dir
    if(newPath.isEmpty() || newPath == eventDir)
    {
        return;
    }

    eventDir = newPath;
    eventDirLineEdit->setText(eventDir);

    // check if dir contains EventGrid.csv file, if it does set the file
    QFileInfo eventFileInfo(newPath, "EventGrid.csv");
    if (eventFileInfo.exists()) {
        eventFile = newPath + "/EventGrid.csv";
        eventFileLineEdit->setText(eventFile);

        this->loadUserWFData();
    }

    return;
}


void UserInputHurricaneWidget::chooseEventFileDialog(void)
{

    QFileDialog dialog(this);
    QString newEventFile = QFileDialog::getOpenFileName(this,tr("Event Grid File"));
    dialog.close();

    // Return if the user cancels or enters same file
    if(newEventFile.isEmpty() || newEventFile == eventFile)
    {
        return;
    }

    // Set file name & entry in qLine edit

    // if file
    //    check valid
    //    set motionDir if file in dir that contains all the motions
    //    invoke loadUserGMData

    CSVReaderWriter csvTool;

    QString err;
    QVector<QStringList> data = csvTool.parseCSVFile(newEventFile, err);

    if(!err.isEmpty())
    {
        this->errorMessage(err);
        return;
    }

    if(data.empty())
        return;

    eventFile = newEventFile;
    eventFileLineEdit->setText(eventFile);

    // check if file in dir with all motions, if so set motionDir
    // Pop off the row that contains the header information
    data.pop_front();
    auto numRows = data.size();
    int count = 0;
    QFileInfo eventFileInfo(eventFile);
    QDir fileDir(eventFileInfo.absolutePath());
    QStringList filesInDir = fileDir.entryList(QStringList() << "*", QDir::Files);

    // check all files are there
    bool allThere = true;
    for(int i = 0; i<numRows; ++i) {
        auto rowStr = data.at(i);
        auto stationName = rowStr[0];
        if (!filesInDir.contains(stationName)) {
            allThere = false;
            i=numRows;
        }
    }

    if (allThere == true) {
        eventDir = fileDir.path();
        eventDirLineEdit->setText(fileDir.path());
        this->loadUserWFData();
    } else {
        QDir motionDirDir(eventDir);
        if (motionDirDir.exists()) {
            QStringList filesInDir = motionDirDir.entryList(QStringList() << "*", QDir::Files);
            bool allThere = true;
            for(int i = 0; i<numRows; ++i) {
                auto rowStr = data.at(i);
                auto stationName = rowStr[0];
                if (!filesInDir.contains(stationName)) {
                    allThere = false;
                    i=numRows;
                }
            }
            if (allThere == true)
                this->loadUserWFData();
        }
    }

    return;
}


void UserInputHurricaneWidget::clear(void)
{
    eventFile.clear();
    eventDir.clear();

    eventFileLineEdit->clear();
    eventDirLineEdit->clear();

    this->hideProgressBar();
    unitsWidget->clear();
}


#ifdef Q_GIS
void UserInputHurricaneWidget::loadUserWFData(void)
{
    auto QGsVisWidget = static_cast<QGISVisualizationWidget*>(theVisualizationWidget);

    if(QGsVisWidget == nullptr)
    {
        qDebug()<<"Failed to cast to QGISVisualizationWidget";
        return;
    }

    this->statusMessage("Loading wind field data");
    CSVReaderWriter csvTool;

    QString err;
    QVector<QStringList> data = csvTool.parseCSVFile(eventFile, err);

    if(!err.isEmpty())
    {
        this->errorMessage(err);
        return;
    }

    if(data.empty())
        return;

    this->showProgressBar();

    QApplication::processEvents();

    //progressBar->setRange(0,inputFiles.size());
    progressBar->setRange(0, data.count());

    progressBar->setValue(0);

    // Get the headers in the first station file - assume that the rest will be the same
    auto rowStr = data.at(1);
    auto stationName = rowStr[0];

    // Path to station files, e.g., site0.csv
    auto stationFilePath = eventDir + QDir::separator() + stationName;

    QString err2;
    QVector<QStringList> sampleStationData = csvTool.parseCSVFile(stationFilePath,err);

    // Return if there is an error or the station data is empty
    if(!err2.isEmpty())
    {
        this->errorMessage("Could not parse the first station with the following error: "+err2);
        return;
    }

    if(sampleStationData.size() < 2)
    {
        this->errorMessage("The file " + stationFilePath + " is empty");
        return;
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
    {
        attribFields.push_back(QgsField(it, QVariant::String));
        unitsWidget->addNewUnitItem(it);
    }


    auto headerInfo = data.front();

    auto latIndex = headerInfo.indexOf("Latitude");
    auto lonIndex = headerInfo.indexOf("Longitude");

    if(latIndex == -1 || lonIndex == -1)
    {
        this->errorMessage("Could not find the Latitude and Longitude headsers in the EventGrid.csv file");
        this->hideProgressBar();
        return;
    }

    // Pop off the row that contains the header information
    data.pop_front();

    auto numRows = data.size();

    int count = 0;

    QgsFeatureList featureList;
    // Get the data
    for(int i = 0; i<numRows; ++i)
    {
        auto rowStr = data.at(i);

        auto stationName = rowStr[0];

        // Path to station files, e.g., site0.csv
        auto stationPath = eventDir + QDir::separator() + stationName;

        bool ok;
        auto longitude = rowStr[lonIndex].toDouble(&ok);

        if(!ok)
        {
            QString errMsg = "Error casting longitude to a double, check the input value";
            this->errorMessage(errMsg);

            this->hideProgressBar();

            return;
        }

        auto latitude = rowStr[latIndex].toDouble(&ok);

        if(!ok)
        {
            QString errMsg = "Error casting latitude to a double, check the input value";
            this->errorMessage(errMsg);

            this->hideProgressBar();

            return;
        }

        WindFieldStation WFStation(stationName,latitude,longitude);

        WFStation.setStationFilePath(stationPath);

        try
        {
            WFStation.importWindFieldStation();
        }
        catch(QString msg)
        {
            auto errorMessage = "Error importing wind field file: " + stationName+"\n"+msg;

            this->errorMessage(errorMessage);

            this->hideProgressBar();

            return;
        }

        auto stationData = WFStation.getStationData();

        // create the feature attributes
        QgsAttributes featAttributes(attribFields.size());

        featAttributes[0] = "HurricaneGridPoint"; // AssetType
        featAttributes[1] = "Hurricane Grid Point"; // TabName
        featAttributes[2] = stationName; // Station Name
        featAttributes[3] = latitude; // Latitude
        featAttributes[4] = longitude; // Longitude

        // The number of headings in the file
        auto numParams =stationData.front().size();

        QVector<QString> dataStrs(numParams);

        for(int i = 0; i<stationData.size(); ++i)
        {
            auto stationParams = stationData[i];

            for(int j = 0; j<numParams; ++j)
            {
                dataStrs[j] += stationParams[j] + " ";
            }
        }

        for(int i = 0; i<numParams; ++i)
        {
            featAttributes[5+i] = dataStrs[i];
        }

        // Create the point and add it to the feature table
        QgsFeature feature;
        feature.setGeometry(QgsGeometry::fromPointXY(QgsPointXY(longitude,latitude)));
        feature.setAttributes(featAttributes);
        featureList.append(feature);

        WFStation.setStationFeature(feature);

        ++count;
        progressLabel->clear();
        progressBar->setValue(count);

        QApplication::processEvents();
    }


    auto vectorLayer = QGsVisWidget->addVectorLayer("Point", "Hurricane Grid");

    if(vectorLayer == nullptr)
    {
        this->errorMessage("Error creating a layer");
        this->hideProgressBar();
        return;
    }


    auto dProvider = vectorLayer->dataProvider();
    auto res = dProvider->addAttributes(attribFields);

    if(!res)
    {
        this->errorMessage("Error adding attribute fields to layer");
        QGsVisWidget->removeLayer(vectorLayer);
        this->hideProgressBar();
        return;
    }

    vectorLayer->updateFields(); // tell the vector layer to fetch changes from the provider

    dProvider->addFeatures(featureList);
    vectorLayer->updateExtents();

    QGsVisWidget->createSymbolRenderer(QgsSimpleMarkerSymbolLayerBase::Cross,Qt::black,2.0,vectorLayer);

    progressLabel->setVisible(false);

    // Reset the widget back to the input pane and close
    this->hideProgressBar();

    if(theStackedWidget->isModal())
        theStackedWidget->close();

    emit loadingComplete(true);

    emit outputDirectoryPathChanged(eventDir, eventFile);

    return;
}
#endif


#ifdef ARC_GIS
void UserInputHurricaneWidget::loadUserWFData(void)
{
    auto arcVizWidget = static_cast<ArcGISVisualizationWidget*>(theVisualizationWidget);

    if(arcVizWidget == nullptr)
    {
        qDebug()<<"Failed to cast to ArcGISVisualizationWidget";
        return;
    }

    this->statusMessage("Loading wind field data");
    CSVReaderWriter csvTool;

    QString err;
    QVector<QStringList> data = csvTool.parseCSVFile(eventFile, err);

    if(!err.isEmpty())
    {
        this->errorMessage(err);
        return;
    }

    if(data.empty())
        return;

    theStackedWidget->setCurrentWidget(progressBarWidget);
    progressBarWidget->setVisible(true);

    QApplication::processEvents();

    //progressBar->setRange(0,inputFiles.size());
    progressBar->setRange(0, data.count());

    progressBar->setValue(0);

    // Create the table to store the fields
    QList<Field> tableFields;
    tableFields.append(Field::createText("AssetType", "NULL",4));
    tableFields.append(Field::createText("TabName", "NULL",4));
    tableFields.append(Field::createText("Station Name", "NULL",4));
    tableFields.append(Field::createText("Latitude", "NULL",8));
    tableFields.append(Field::createText("Longitude", "NULL",9));
    tableFields.append(Field::createText("Peak Wind Speeds", "NULL",9));
    tableFields.append(Field::createText("Peak Inundation Heights", "N/A",9));

    auto gridFeatureCollection = new FeatureCollection(this);

    // Create the feature collection table/layers
    auto gridFeatureCollectionTable = new FeatureCollectionTable(tableFields, GeometryType::Point, SpatialReference::wgs84(), this);
    gridFeatureCollection->tables()->append(gridFeatureCollectionTable);

    auto gridLayer = new FeatureCollectionLayer(gridFeatureCollection,this);

    gridLayer->setName("Wind Field Grid");
    gridLayer->setAutoFetchLegendInfos(true);

    // Create red cross SimpleMarkerSymbol
    SimpleMarkerSymbol* crossSymbol = new SimpleMarkerSymbol(SimpleMarkerSymbolStyle::Cross, QColor("black"), 6, this);

    // Create renderer and set symbol to crossSymbol
    SimpleRenderer* renderer = new SimpleRenderer(crossSymbol, this);
    renderer->setLabel("Wind Field Grid Points");

    // Set the renderer for the feature layer
    gridFeatureCollectionTable->setRenderer(renderer);

    // Set the scale at which the layer will become visible - if scale is too high, then the entire view will be filled with symbols
    // gridLayer->setMinScale(80000);

    auto headerInfo = data.front();

    auto latIndex = headerInfo.indexOf("Latitude");
    auto lonIndex = headerInfo.indexOf("Longitude");

    if(latIndex == -1 || lonIndex == -1)
    {
        this->errorMessage("Could not find the Latitude and Longitude headsers in the EventGrid.csv file");
        return;
    }

    // Pop off the row that contains the header information
    data.pop_front();

    auto numRows = data.size();

    int count = 0;

    // Get the data
    for(int i = 0; i<numRows; ++i)
    {
        auto rowStr = data.at(i);

        auto stationName = rowStr[0];

        // Path to station files, e.g., site0.csv
        auto stationPath = eventDir + QDir::separator() + stationName;

        bool ok;
        auto longitude = rowStr[lonIndex].toDouble(&ok);

        if(!ok)
        {
            QString errMsg = "Error longitude to a double, check the value";
            this->errorMessage(errMsg);

            theStackedWidget->setCurrentWidget(fileInputWidget);
            progressBarWidget->setVisible(false);

            return;
        }

        auto latitude = rowStr[latIndex].toDouble(&ok);

        if(!ok)
        {
            QString errMsg = "Error latitude to a double, check the value";
            this->errorMessage(errMsg);

            theStackedWidget->setCurrentWidget(fileInputWidget);
            progressBarWidget->setVisible(false);

            return;
        }

        WindFieldStation WFStation(stationName,latitude,longitude);

        WFStation.setStationFilePath(stationPath);

        try
        {
            WFStation.importWindFieldStation();
        }
        catch(QString msg)
        {
            auto errorMessage = "Error importing wind field file: " + stationName+"\n"+msg;

            this->errorMessage(errorMessage);

            theStackedWidget->setCurrentWidget(fileInputWidget);
            progressBarWidget->setVisible(false);

            return;
        }

        auto pws = WFStation.getPeakWindSpeeds();

        QString pwsStr;
        for(int i = 0; i<pws.size()-1; ++i)
        {
            pwsStr += QString::number(pws[i]) + ", ";
        }

        pwsStr += QString::number(pws.back());


        auto pih = WFStation.getPeakInundationHeights();

        QString pihStr;
        for(int i = 0; i<pih.size()-1; ++i)
        {
            pihStr += QString::number(pih[i]) + ", ";
        }

        pihStr += QString::number(pih.back());


        // create the feature attributes
        QMap<QString, QVariant> featureAttributes;
        featureAttributes.insert("Station Name", stationName);
        featureAttributes.insert("AssetType", "WindfieldGridPoint");
        featureAttributes.insert("TabName", "Wind Field Grid Point");
        featureAttributes.insert("Latitude", latitude);
        featureAttributes.insert("Longitude", longitude);
        featureAttributes.insert("Peak Wind Speeds", pwsStr);

        if(!pihStr.isEmpty())
            featureAttributes.insert("Peak Inundation Heights", pihStr);

        // Create the point and add it to the feature table
        Point point(longitude,latitude);
        Feature* feature = gridFeatureCollectionTable->createFeature(featureAttributes, point, this);

        gridFeatureCollectionTable->addFeature(feature);
        WFStation.setStationFeature(feature);

        ++count;
        progressLabel->clear();
        progressBar->setValue(count);

        QApplication::processEvents();
    }

    // Create a new layer
    auto layersTreeView = arcVizWidget->getLayersTree();

    // Check if there is a 'User Ground Motions' root item in the tree
    auto userInputTreeItem = layersTreeView->getTreeItem("User Wind Field", nullptr);

    // If there is no item, create one
    if(userInputTreeItem == nullptr)
    {
        auto itemUID = theVisualizationWidget->createUniqueID();
        userInputTreeItem = layersTreeView->addItemToTree("User Wind Field", itemUID);
    }


    // Add the event layer to the layer tree
    //    auto eventItem = layersTreeView->addItemToTree(eventFile, QString(), userInputTreeItem);

    progressLabel->setVisible(false);

    // Add the event layer to the map
    arcVizWidget->addLayerToMap(gridLayer,userInputTreeItem);

    // Reset the widget back to the input pane and close
    theStackedWidget->setCurrentWidget(fileInputWidget);
    fileInputWidget->setVisible(true);

    if(theStackedWidget->isModal())
        theStackedWidget->close();

    emit loadingComplete(true);

    emit outputDirectoryPathChanged(eventDir, eventFile);

    return;
}
#endif


void UserInputHurricaneWidget::showProgressBar(void)
{
    theStackedWidget->setCurrentWidget(progressBarWidget);
    fileInputWidget->setVisible(false);
    progressBarWidget->setVisible(true);
}


void UserInputHurricaneWidget::hideProgressBar(void)
{
    theStackedWidget->setCurrentWidget(fileInputWidget);
    progressBarWidget->setVisible(false);
    fileInputWidget->setVisible(true);
}
