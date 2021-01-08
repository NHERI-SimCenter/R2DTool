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

// Written by: Stevan Gavrilovic

#include "ShakeMapWidget.h"
#include "TreeView.h"
#include "VisualizationWidget.h"
#include "CustomListWidget.h"

// GIS Layers
#include "FeatureCollectionLayer.h"
#include "GroupLayer.h"
#include "KmlLayer.h"
#include "Layer.h"
#include "LayerListModel.h"

#include <QApplication>
#include <QDialog>
#include <QFile>
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QSpacerItem>
#include <QSplitter>
#include <QStackedWidget>
#include <QVBoxLayout>

using namespace Esri::ArcGISRuntime;

ShakeMapWidget::ShakeMapWidget(VisualizationWidget* visWidget, QWidget *parent) : SimCenterAppWidget(parent), theVisualizationWidget(visWidget)
{
    progressBar = nullptr;
    directoryInputWidget = nullptr;
    progressBarWidget = nullptr;
    shakeMapStackedWidget = nullptr;
    progressLabel = nullptr;
    pathToShakeMapDirectory = "NULL";

}


ShakeMapWidget::~ShakeMapWidget()
{
    qDeleteAll(shakeMapContainer);
}


void ShakeMapWidget::showShakeMapLayers(bool state)
{
    auto layersTreeView = theVisualizationWidget->getLayersTree();


    if(state == false)
    {
        layersTreeView->removeItemFromTree("Shake Map");

        for(auto&& it : shakeMapContainer)
        {
            auto eventName = it->eventLayer->name();
            //            it->eventKMZLayer->name();
            layersTreeView->removeItemFromTree(eventName);
        }

        return;
    }


    // Check if there is a 'Shake Map' root item in the tree
    auto shakeMapTreeItem = layersTreeView->getTreeItem("Shake Map",nullptr);

    // If there is no item, create one
    if(shakeMapTreeItem == nullptr)
        shakeMapTreeItem = layersTreeView->addItemToTree("Shake Map",QString());


    for(auto&& it : shakeMapContainer)
    {
        auto eventName = it->eventLayer->name();
        auto eventID = it->eventLayer->layerId();
        auto eventItem = layersTreeView->addItemToTree(eventName, eventID, shakeMapTreeItem);

        auto layers = it->getAllActiveSubLayers();
        for(auto&& layer : layers)
        {
            auto layerName = layer->name();
            auto layerID = layer->layerId();

            layersTreeView->addItemToTree(layerName, layerID, eventItem);
        }
    }
}


QWidget* ShakeMapWidget::getShakeMapWidget(void)
{
    QSplitter *splitter = new QSplitter(this);

    listWidget = new CustomListWidget(this, "List of Imported ShakeMaps");

    splitter->addWidget(this->getStackedWidget());
    splitter->addWidget(listWidget);

    return splitter;
}


QStackedWidget* ShakeMapWidget::getStackedWidget(void)
{
    if (shakeMapStackedWidget)
        return shakeMapStackedWidget.get();

    shakeMapStackedWidget = std::make_unique<QStackedWidget>();

    directoryInputWidget = new QWidget(this);    
    auto inputLayout = new QGridLayout(directoryInputWidget);

    progressBarWidget = new QWidget(this);
    auto progressBarLayout = new QVBoxLayout(progressBarWidget);
    progressBarWidget->setLayout(progressBarLayout);

    auto progressText = new QLabel("Loading ShakeMap data. This may take a while.",progressBarWidget);
    progressLabel =  new QLabel(" ",this);
    progressBar = new QProgressBar(progressBarWidget);

    auto vspacer = new QSpacerItem(0,0,QSizePolicy::Minimum, QSizePolicy::Expanding);
    progressBarLayout->addItem(vspacer);
    progressBarLayout->addWidget(progressText,1, Qt::AlignCenter);
    progressBarLayout->addWidget(progressLabel,1, Qt::AlignCenter);
    progressBarLayout->addWidget(progressBar);
    progressBarLayout->addItem(vspacer);
    progressBarLayout->addStretch(1);

    shakeMapStackedWidget->addWidget(directoryInputWidget);
    shakeMapStackedWidget->addWidget(progressBarWidget);

    shakeMapStackedWidget->setCurrentWidget(directoryInputWidget);

    QLabel* selectComponentsText = new QLabel("To import ShakeMap files, please download the files from the ShakeMap website and place them in the folder specified below:", this);
    selectComponentsText->setWordWrap(true);

    shakeMapDirectoryLineEdit = new QLineEdit(this);
    shakeMapDirectoryLineEdit->setMaximumWidth(750);
    shakeMapDirectoryLineEdit->setMinimumWidth(400);
    shakeMapDirectoryLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QPushButton *browseFileButton = new QPushButton(this);
    browseFileButton->setText(tr("Browse"));
    browseFileButton->setMaximumWidth(150);

    QPushButton *loadButton = new QPushButton(this);
    loadButton->setText(tr("Load"));
    loadButton->setMaximumWidth(150);

    connect(browseFileButton,SIGNAL(clicked()),this,SLOT(chooseShakeMapDirectoryDialog()));

    connect(loadButton,SIGNAL(clicked()),this,SLOT(loadShakeMapData()));

    QLabel* shakeMapText1 = new QLabel("At a minimum, the folder must contain the following files: 1) grid.xml, 2) uncertainty.xml, and 3) rupture.json", this);
    shakeMapText1->setWordWrap(true);
    QLabel* shakeMapText2 = new QLabel("Click 'Load' to load the ShakeMap. Multiple ShakeMaps can be added by selecting another folder containing a ShakeMap, and clicking 'Load' again.", this);
    shakeMapText2->setWordWrap(true);
    QLabel* shakeMapText3 = new QLabel("The list of loaded ShakeMaps will appear on the right.", this);
    shakeMapText3->setWordWrap(true);

    inputLayout->addItem(vspacer,0,0);
    inputLayout->addWidget(selectComponentsText,1,0,1,3);
    inputLayout->addWidget(shakeMapDirectoryLineEdit,2,0);
    inputLayout->addWidget(browseFileButton,2,1);
    inputLayout->addWidget(loadButton,2,2);
    inputLayout->addWidget(shakeMapText1,3,0,1,3);
    inputLayout->addWidget(shakeMapText2,4,0,1,3);
    inputLayout->addWidget(shakeMapText3,5,0,1,3);
    inputLayout->addItem(vspacer,6,0);

    //    pathToShakeMapDirectory="/Users/steve/Desktop/SimCenter/Examples/ShakeMaps/SanAndreas/";
    //    this->loadShakeMapData();

    return shakeMapStackedWidget.get();
}


void ShakeMapWidget::showLoadShakeMapDialog(void)
{

    if (!shakeMapStackedWidget)
    {
        this->getShakeMapWidget();
    }

    shakeMapStackedWidget->show();
    shakeMapStackedWidget->raise();
    shakeMapStackedWidget->activateWindow();
}


void ShakeMapWidget::loadShakeMapData(void)
{

    // Return if the user cancels
    if(pathToShakeMapDirectory.isEmpty() || pathToShakeMapDirectory == QDir::currentPath())
    {
        pathToShakeMapDirectory = "NULL";
        return;
    }

    const QFileInfo inputDir(pathToShakeMapDirectory);

    if (!inputDir.exists() || !inputDir.isDir())
    {
        QString errMsg ="A directory does not exist at the path: "+pathToShakeMapDirectory;
        this->userMessageDialog(errMsg);
        return;
    }

    QStringList acceptableFileExtensions = {"*.kmz", "*.xml", "*.shp"};

    QStringList inputFiles = inputDir.dir().entryList(acceptableFileExtensions,QDir::Files);

    if(inputFiles.empty())
    {
        QString errMsg ="No ShakeMap files were found at the path: "+pathToShakeMapDirectory;
        this->userMessageDialog(errMsg);
        return;
    }

    QString eventName = inputDir.dir().dirName();

    // Check if the shake map already exists
    if(shakeMapContainer.contains(eventName))
        return;

    // Create a new shakemap
    auto inputShakeMap = new ShakeMap();

    inputShakeMap->eventName = eventName;

    auto layersTreeView = theVisualizationWidget->getLayersTree();

    // Check if there is a 'Shake Map' root item in the tree
    auto shakeMapTreeItem = layersTreeView->getTreeItem("Shake Map", nullptr);

    // If there is no item, create one
    if(shakeMapTreeItem == nullptr)
        shakeMapTreeItem = layersTreeView->addItemToTree("Shake Map", QString());


    // Add the event layer to the layer tree
    auto eventItem = layersTreeView->addItemToTree(eventName, QString(), shakeMapTreeItem);

    // Create the root event group layer
    inputShakeMap->eventLayer = new GroupLayer(QList<Layer*>{});

    auto eventLayer = inputShakeMap->eventLayer;

    eventLayer->setName(eventName);

    shakeMapStackedWidget->setCurrentWidget(progressBarWidget);

    progressBarWidget->setVisible(true);

    QApplication::processEvents();

    progressBar->setRange(0,inputFiles.size());

    progressBar->setValue(0);

    int count = 0;
    foreach(QString filename, inputFiles)
    {
        auto inFilePath = pathToShakeMapDirectory + filename;

        // Create the XML grid
        if(filename.compare("grid.xml") == 0) // XML grid
        {
            progressLabel->setText("Loading Grid Layer");
            progressLabel->setVisible(true);
            QApplication::processEvents();

            inputShakeMap->gridLayer = theVisualizationWidget->createAndAddXMLShakeMapLayer(inFilePath, "Grid", eventItem);
            eventLayer->layers()->append(inputShakeMap->gridLayer);
        }
        else if(filename.contains("_se.kmz")) // Event layer
        {
            progressLabel->setText("Loading Event Layer");
            QApplication::processEvents();

            inputShakeMap->eventKMZLayer = theVisualizationWidget->createAndAddKMLLayer(inFilePath, "Event", eventItem);
            eventLayer->layers()->append(inputShakeMap->eventKMZLayer);
        }
        else if(filename.compare("polygons_mi.kmz") == 0)
        {
            progressLabel->setText("Loading PGA Polygon Layer");
            progressLabel->setVisible(true);
            QApplication::processEvents();

            inputShakeMap->pgaPolygonLayer = theVisualizationWidget->createAndAddKMLLayer(inFilePath, "PGA Polygons", eventItem, 0.3);
            eventLayer->layers()->append(inputShakeMap->pgaPolygonLayer);
        }
        else if(filename.compare("epicenter.kmz") == 0)
        {
            progressLabel->setText("Loading Epicenter Layer");
            QApplication::processEvents();

            inputShakeMap->epicenterLayer = theVisualizationWidget->createAndAddKMLLayer(inFilePath, "Epicenter", eventItem);
            eventLayer->layers()->append(inputShakeMap->epicenterLayer);
        }
        else if(filename.compare("cont_pga.kmz") == 0)
        {
            progressLabel->setText("Loading PGA Contour Layer");
            QApplication::processEvents();

            inputShakeMap->pgaContourLayer = theVisualizationWidget->createAndAddKMLLayer(inFilePath, "PGA Contours", eventItem);
            eventLayer->layers()->append(inputShakeMap->pgaContourLayer);
        }
        else if(filename.compare("overlay.kmz") == 0)
        {
            progressLabel->setText("Loading PGA Overlay Layer");
            QApplication::processEvents();

            inputShakeMap->pgaOverlayLayer = theVisualizationWidget->createAndAddKMLLayer(inFilePath, "PGA Intensity Overlay", eventItem, 0.3);
            eventLayer->layers()->append(inputShakeMap->pgaOverlayLayer);
        }
        else if(filename.contains("_se.kmz")) // Fault layer
        {
            progressLabel->setText("Loading Fault Layer");
            QApplication::processEvents();

            inputShakeMap->faultLayer = theVisualizationWidget->createAndAddKMLLayer(inFilePath, "Fault", eventItem);
            eventLayer->layers()->append(inputShakeMap->faultLayer);
        }
        else
        {
            qDebug()<<"Warning could not load the file :"<<filename;
        }


        ++count;
        progressLabel->clear();
        progressBar->setValue(count);

        QApplication::processEvents();
    }

    progressLabel->setVisible(false);


    // Insert the ShakeMap into its container
    shakeMapContainer.insert(eventName,inputShakeMap);

    listWidget->addItem(eventName);

    // Add the event layer to the map
    theVisualizationWidget->addLayerToMap(eventLayer,eventItem);

    // Reset the widget back to the input pane and close
    shakeMapStackedWidget->setCurrentWidget(directoryInputWidget);
    directoryInputWidget->setVisible(true);

    if(shakeMapStackedWidget->isModal())
        shakeMapStackedWidget->close();

    emit loadingComplete(true);

    return;
}


void ShakeMapWidget::chooseShakeMapDirectoryDialog(void)
{

    QFileDialog dialog(this);

    dialog.setFileMode(QFileDialog::Directory);

    pathToShakeMapDirectory = dialog.getExistingDirectory(this, tr("Folder with ShakeMap files"));

    pathToShakeMapDirectory.append("/");

    dialog.close();

    // Set file name & entry in line edit
    shakeMapDirectoryLineEdit->setText(pathToShakeMapDirectory);

    return;
}


bool ShakeMapWidget::outputToJSON(QJsonObject &jsonObject)
{
    jsonObject.insert("SourceForIM","ShakeMap");

    QJsonObject sourceParamObj;

    sourceParamObj.insert("Directory",pathToShakeMapDirectory);

    jsonObject.insert("SourceParameters",sourceParamObj);

    return true;
}


bool ShakeMapWidget::inputFromJSON(QJsonObject &jsonObject)
{
    return false;
}
