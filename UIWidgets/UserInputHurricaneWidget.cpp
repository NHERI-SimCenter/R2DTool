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

// GIS Layers
#include "FeatureCollectionLayer.h"
#include "GroupLayer.h"
#include "Layer.h"
#include "LayerListModel.h"
#include "SimpleMarkerSymbol.h"
#include "SimpleRenderer.h"
#include "PolygonBuilder.h"

#include <QApplication>
#include <QDialog>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QSpacerItem>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QDir>

using namespace Esri::ArcGISRuntime;

UserInputHurricaneWidget::UserInputHurricaneWidget(VisualizationWidget* visWidget, QWidget *parent) : SimCenterAppWidget(parent), theVisualizationWidget(visWidget)
{
    progressBar = nullptr;
    fileInputWidget = nullptr;
    progressBarWidget = nullptr;
    theStackedWidget = nullptr;
    progressLabel = nullptr;
    selectedHurricaneName = nullptr;
    selectedHurricaneSID = nullptr;

    eventFile = "";

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(this->getUserInputHurricaneWidget());
    layout->addStretch();
    this->setLayout(layout);

//    eventFile = "/Users/steve/Desktop/ibtracs.last3years.list.v04r00.csv";
//    this->loadUserHurricaneData();

}


UserInputHurricaneWidget::~UserInputHurricaneWidget()
{

}


bool UserInputHurricaneWidget::outputAppDataToJSON(QJsonObject &jsonObject) {

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


bool UserInputHurricaneWidget::outputToJSON(QJsonObject &jsonObj)
{
    // qDebug() << "USER GM outputPLAIN";

    return true;
}


bool UserInputHurricaneWidget::inputAppDataFromJSON(QJsonObject &jsonObj)
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


void UserInputHurricaneWidget::showUserGMLayers(bool state)
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


QStackedWidget* UserInputHurricaneWidget::getUserInputHurricaneWidget(void)
{
    if (theStackedWidget)
        return theStackedWidget.get();

    theStackedWidget = std::make_unique<QStackedWidget>();
    theStackedWidget->setContentsMargins(0,0,0,0);

    //
    // file and dir input
    //

    fileInputWidget = new QWidget(this);
    QGridLayout *fileLayout = new QGridLayout(fileInputWidget);
    fileLayout->setContentsMargins(0,0,0,0);

    QLabel* selectComponentsText = new QLabel("Event File Listing Hurricanes",this);
    eventFileLineEdit = new QLineEdit();
    QPushButton *browseFileButton = new QPushButton("Browse",this);

    connect(browseFileButton,SIGNAL(clicked()),this,SLOT(chooseEventFileDialog()));

    mapViewSubWidget = std::make_unique<ResultsMapViewWidget>(nullptr);

    QHBoxLayout *selectedHurricaneLayout = new QHBoxLayout();

    QPushButton *selectHurricaneButton = new QPushButton("Select Hurricane",this);

    connect(selectHurricaneButton,&QPushButton::clicked,this,&UserInputHurricaneWidget::handleHurricaneSelect);

    QLabel* selectedHurricaneLabel = new QLabel("Hurricane Selected For Analysis: ",this);
    QLabel* SIDLabel = new QLabel(" - SID: ",this);

    selectedHurricaneName = new QLabel("None",this);
    selectedHurricaneSID = new QLabel("None",this);

    selectedHurricaneLayout->addWidget(selectHurricaneButton);
    selectedHurricaneLayout->addWidget(selectedHurricaneLabel);
    selectedHurricaneLayout->addWidget(selectedHurricaneName);
    selectedHurricaneLayout->addWidget(SIDLabel);
    selectedHurricaneLayout->addWidget(selectedHurricaneSID);
    selectedHurricaneLayout->addStretch(1);

    fileLayout->addWidget(selectComponentsText,   0,0);
    fileLayout->addWidget(eventFileLineEdit,      0,1);
    fileLayout->addWidget(browseFileButton,       0,2);
    fileLayout->addLayout(selectedHurricaneLayout, 1,0,1,3);
    fileLayout->addWidget(mapViewSubWidget.get(), 2,0,1,3);

    //
    // progress bar
    //

    progressBarWidget = new QWidget(this);
    auto progressBarLayout = new QVBoxLayout(progressBarWidget);
    progressBarWidget->setLayout(progressBarLayout);

    auto progressText = new QLabel("Loading user hurricane data. This may take a while.",progressBarWidget);
    progressLabel =  new QLabel("",this);
    progressBar = new QProgressBar(progressBarWidget);

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

    theStackedWidget->setWindowTitle("Select folder containing hurricanes");

    return theStackedWidget.get();
}


void UserInputHurricaneWidget::showUserGMSelectDialog(void)
{

    if (!theStackedWidget)
    {
        this->getUserInputHurricaneWidget();
    }

    theStackedWidget->show();
    theStackedWidget->raise();
    theStackedWidget->activateWindow();
}


void UserInputHurricaneWidget::loadUserHurricaneData(void)
{
    CSVReaderWriter csvTool;

    QString err;
    QVector<QStringList> data = csvTool.parseCSVFile(eventFile, err);

    if(!err.isEmpty())
    {
        this->userMessageDialog(err);
        return;
    }

    if(data.empty())
        return;

    theStackedWidget->setCurrentWidget(progressBarWidget);
    progressBarWidget->setVisible(true);

    // Get the header information to populate the fields
    auto topHeaderData = data.at(0);
    auto unitsData = data.at(1);

    auto numCol = topHeaderData.size();

    auto indexName = -1;
    auto indexSID = -1;
    auto indexLat = -1;
    auto indexLon = -1;
    auto indexSeason = -1;

    for(int col = 0; col < numCol; ++col)
    {
        if(topHeaderData.at(col) == "LAT")
            indexLat = col;
        else if(topHeaderData.at(col) == "LON")
            indexLon = col;
        else if(topHeaderData.at(col) == "NAME")
            indexName = col;
        else if(topHeaderData.at(col) == "SID")
            indexSID = col;
        else if(topHeaderData.at(col) == "SEASON")
            indexSeason = col;
    }

    if(indexName == -1 || indexSID == -1 || indexLat == -1 || indexLon == -1 || indexSeason == -1)
    {
        qDebug()<<"Could not find the required column indexes in the data file";
        return;
    }

    // Create the table to store the fields
    QList<Field> tableFields;

    // Common fields
    tableFields.append(Field::createText("AssetType", "NULL",4));
    tableFields.append(Field::createText("TabName", "NULL",4));
    tableFields.append(Field::createText("UID", "NULL",4));

    for(auto&& it : topHeaderData)
    {
        tableFields.append(Field::createText(it, "NULL",4));
    }

    // Pop off the first two rows that contain the header information
    data.pop_front();
    data.pop_front();

    // Create the buildings group layer that will hold the sublayers
    auto allHurricanesLayer = new GroupLayer(QList<Layer*>{},this);
    allHurricanesLayer->setName("Hurricanes");

    auto allHurricanesItem = theVisualizationWidget->addLayerToMap(allHurricanesLayer);

    if(allHurricanesItem == nullptr)
    {
        qDebug()<<"Error adding item to the map";
        return;
    }


    // Split the hurricanes up as they come in one long list
    QVector<QVector<QStringList>> hurricanes;

    QVectorIterator<QStringList> i(data);
    QString SID;

    QVector<QStringList> hurricane;
    while (i.hasNext())
    {
        auto row = i.next();

        if(row.size() != numCol)
            return;

        auto currSID = row.at(indexSID);

        if(SID.compare(currSID) != 0)
        {
            if(!hurricane.isEmpty())
            {
                hurricanes.push_back(hurricane);
                hurricane.clear();
            }

            SID = currSID;
        }
        else
        {
            hurricane.push_back(row);
        }
    }


    auto numHurricanes = hurricanes.size();

    progressBar->setMinimum(0);
    progressBar->setMaximum(numHurricanes);
    progressBar->reset();
    QApplication::processEvents();

    for(int i = 0; i<numHurricanes; ++i)
    {
        progressBar->setValue(i);
        QApplication::processEvents();

        // Get the hurricane
        auto hurricane = hurricanes.at(i);

        auto numPnts = hurricane.size();

        auto name = hurricane.front().at(indexName);
        auto SID = hurricane.front().at(indexSID);
        auto season = hurricane.front().at(indexSeason);

        auto nameID = name+"-"+season;

        auto thisHurricaneLayer = new GroupLayer(QList<Layer*>{},this);
        thisHurricaneLayer->setName(nameID);

        auto thisHurricanesItem = theVisualizationWidget->addLayerToMap(thisHurricaneLayer,allHurricanesItem,allHurricanesLayer);

        //        auto trackPntsFeatureCollection = new FeatureCollection(this);

        //        // Create the feature collection table/layers
        //        auto trackPntsFeatureCollectionTable = new FeatureCollectionTable(tableFields, GeometryType::Point, SpatialReference::wgs84(), this);
        //        trackPntsFeatureCollection->tables()->append(trackPntsFeatureCollectionTable);

        //        auto trackPntsLayer = new FeatureCollectionLayer(trackPntsFeatureCollection,this);

        //        trackPntsLayer->setName(name+" - Points");

        //        // Create cross SimpleMarkerSymbol
        //        SimpleMarkerSymbol* markerSymbol = new SimpleMarkerSymbol(SimpleMarkerSymbolStyle::Circle, QColor("black"), 4, this);

        //        // Create renderer and set symbol to crossSymbol
        //        SimpleRenderer* pointRenderer = new SimpleRenderer(markerSymbol, this);
        //        pointRenderer->setLabel("Hurricane track points");

        //        // Set the renderer for the feature layer
        //        trackPntsFeatureCollectionTable->setRenderer(pointRenderer);

        // Each row is a point on the hurricane track
        PartCollection* trackCollection = new PartCollection(SpatialReference::wgs84(), this);
        double latitude = 0.0;
        double longitude = 0.0;
        for(int j = 0; j<numPnts; ++j)
        {
            auto trackPoint = hurricane.at(j);

            // create the feature attributes
            //  QMap<QString, QVariant> featureAttributes;
            //  for(int k = 0; k<numCol; ++k)
            //  {
            //      featureAttributes.insert(topHeaderData.at(k), trackPoint.at(k));
            //  }

            //  featureAttributes.insert("AssetType", buildingIDStr);
            //  featureAttributes.insert("TabName", buildingIDStr);
            //  featureAttributes.insert("UID", uid);

            Point pointPrev(longitude,latitude);

            // Create the geometry for visualization
            latitude = trackPoint.at(indexLat).toDouble();
            longitude = trackPoint.at(indexLon).toDouble();

            Point point(longitude,latitude);

            if(j != 0)
            {
                Part* partj = new Part(SpatialReference::wgs84(), this);
                partj->addPoint(point);
                partj->addPoint(pointPrev);

                trackCollection->addPart(partj);
            }

            // Create the point feature
            //            auto feature = trackPntsFeatureCollectionTable->createFeature(featureAttributes, point, this);

            //            trackPntsFeatureCollectionTable->addFeature(feature);
        }

        // Add the points layer
        //        theVisualizationWidget->addLayerToMap(trackPntsLayer,thisHurricanesItem, thisHurricaneLayer);

        PolygonBuilder polylineBuilder(SpatialReference::wgs84());
        polylineBuilder.setParts(trackCollection);

        // Add the track polyline layer
        // if(!polylineBuilder.isSketchValid())
        // {
        //     qDebug()<<"Error, cannot create a feature with the latitude and longitude provided";
        //     return;
        // }

        // Create the feature collection table/layers
        QList<Field> trackFields;
        trackFields.append(Field::createText("NAME", "NULL",4));
        trackFields.append(Field::createText("SID", "NULL",4));
        trackFields.append(Field::createText("SEASON", "NULL",4));
        trackFields.append(Field::createText("AssetType", "NULL",4));
        trackFields.append(Field::createText("TabName", "NULL",4));
        trackFields.append(Field::createText("UID", "NULL",4));


        auto trackFeatureCollection = new FeatureCollection(this);
        auto trackFeatureCollectionTable = new FeatureCollectionTable(trackFields, GeometryType::Polyline, SpatialReference::wgs84(), this);
        trackFeatureCollection->tables()->append(trackFeatureCollectionTable);

        auto trackLayer = new FeatureCollectionLayer(trackFeatureCollection,this);

        trackLayer->setName(nameID+" - Track");

        // Create line symbol for the track
        SimpleLineSymbol* lineSymbol = new SimpleLineSymbol(SimpleLineSymbolStyle::Solid,
                                                            QColor(0, 0, 0),
                                                            2.0f /*width*/,
                                                            SimpleLineSymbolMarkerStyle::Arrow,
                                                            SimpleLineSymbolMarkerPlacement::End,
                                                            this);

        // Create renderer and set symbol for the track
        SimpleRenderer* lineRenderer = new SimpleRenderer(lineSymbol, this);
        lineRenderer->setLabel("Hurricane track");

        // Set the renderer for the feature layer
        trackFeatureCollectionTable->setRenderer(lineRenderer);

        auto uid = theVisualizationWidget->createUniqueID();

        QMap<QString, QVariant> featureAttributes;
        featureAttributes.insert("NAME",name);
        featureAttributes.insert("SID",SID);
        featureAttributes.insert("SEASON",season);
        featureAttributes.insert("TabName", nameID);
        featureAttributes.insert("AssetType", "HURRICANE");
        featureAttributes.insert("UID", uid);

        // Create the polyline feature
        auto polyline =  polylineBuilder.toPolyline();

        auto trackFeat = trackFeatureCollectionTable->createFeature(featureAttributes,polyline,this);
        trackFeatureCollectionTable->addFeature(trackFeat);

        theVisualizationWidget->addLayerToMap(trackLayer, thisHurricanesItem, thisHurricaneLayer);
    }


    progressLabel->setVisible(false);


    // Reset the widget back to the input pane and close
    theStackedWidget->setCurrentWidget(fileInputWidget);
    fileInputWidget->setVisible(true);

    if(theStackedWidget->isModal())
        theStackedWidget->close();

    return;
}


void UserInputHurricaneWidget::setCurrentlyViewable(bool status){

    if (status == true)
        mapViewSubWidget->setCurrentlyViewable(status);
}


void UserInputHurricaneWidget::chooseEventFileDialog(void)
{

    QFileDialog dialog(this);
    QString newEventFile = QFileDialog::getOpenFileName(this,tr("List of Hurricanes File"));
    dialog.close();

    // Return if the user cancels or enters same file
    if(newEventFile.isEmpty() || newEventFile == eventFile)
    {
        return;
    }

    // Set file name & entry in qLine edit
    eventFile = newEventFile;
    eventFileLineEdit->setText(eventFile);

    this->loadUserHurricaneData();

    return;
}


void UserInputHurricaneWidget::clear(void)
{
    eventFile.clear();
    eventFileLineEdit->clear();
}


void UserInputHurricaneWidget::handleHurricaneSelect(void)
{
    auto selectedFeatures = theVisualizationWidget->getSelectedFeaturesList();

    if(selectedFeatures.empty())
        return;

    // Only select the first hurricane
    for(auto&& it : selectedFeatures)
    {
        auto attrbList = it->attributes();

        auto featType = attrbList->attributeValue("AssetType");

        if(featType.toString() != "HURRICANE")
            continue;

        auto hurricaneName = attrbList->attributeValue("NAME").toString();
        auto hurricaneSID = attrbList->attributeValue("SID").toString();

        selectedHurricaneName->setText(hurricaneName);
        selectedHurricaneSID->setText(hurricaneSID);

    }

}
