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
#include "VisualizationWidget.h"
#include "CustomListWidget.h"
#include "XMLAdaptor.h"
#include "CSVReaderWriter.h"
#include "TreeItem.h"
#include "Utils/FileOperations.h"


#ifdef OpenSRA
#include "OpenSRAPreferences.h"
#else
#include "SimCenterPreferences.h"
#endif

#include <QDirIterator>
#include <QApplication>
#include <QListWidget>
#include <QDialog>
#include <QJsonArray>
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
#include <QStandardPaths>
#include <QVBoxLayout>

// GIS Stuff
#ifdef ARC_GIS
#include "ArcGISVisualizationWidget.h"

#include "FeatureCollectionLayer.h"
#include "GroupLayer.h"
#include "KmlLayer.h"
#include "Layer.h"
#include "LayerTreeView.h"
#include "LayerListModel.h"
#include "SimpleRenderer.h"

using namespace Esri::ArcGISRuntime;
#endif

#ifdef Q_GIS
#include "QGISVisualizationWidget.h"

#include <qgslinesymbol.h>
#include <qgsvectorlayer.h>
#include <qgsfillsymbol.h>
#endif

ShakeMapWidget::ShakeMapWidget(VisualizationWidget* visWidget, QWidget *parent) : SimCenterAppWidget(parent), theVisualizationWidget(visWidget)
{
    shakeMapStackedWidget = nullptr;
    progressBar = nullptr;
    directoryInputWidget = nullptr;
    progressBarWidget = nullptr;
    progressLabel = nullptr;
    listWidget = nullptr;
    pathToShakeMapDirectory = "NULL";

    auto mainLayout = new QVBoxLayout(this);
    auto mainWidget = this->getShakeMapWidget();

    mainLayout->addWidget(mainWidget);
}


ShakeMapWidget::~ShakeMapWidget()
{
}


QWidget* ShakeMapWidget::getShakeMapWidget(void)
{
    QSplitter *splitter = new QSplitter(Qt::Horizontal);

    listWidget = new CustomListWidget("List of Imported ShakeMaps");

    splitter->addWidget(this->getStackedWidget());
    splitter->addWidget(listWidget);

    splitter->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    return splitter;
}


QStackedWidget* ShakeMapWidget::getStackedWidget(void)
{
    if (shakeMapStackedWidget)
        return shakeMapStackedWidget;

    shakeMapStackedWidget = new QStackedWidget();
    shakeMapStackedWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    progressBarWidget = new QWidget();
    auto progressBarLayout = new QVBoxLayout();
    progressBarWidget->setLayout(progressBarLayout);

    auto progressText = new QLabel("Loading ShakeMap data. This may take a while.");
    progressLabel =  new QLabel(" ");
    progressBar = new QProgressBar();

    auto vspacer = new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto vspacer2 = new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Expanding);
    progressBarLayout->addItem(vspacer);
    progressBarLayout->addWidget(progressText,1, Qt::AlignCenter);
    progressBarLayout->addWidget(progressLabel,1, Qt::AlignCenter);
    progressBarLayout->addWidget(progressBar);
    progressBarLayout->addItem(vspacer2);
    progressBarLayout->addStretch(1);

    QLabel* selectComponentsText = new QLabel("To import ShakeMap files, please download the files from the ShakeMap website and place them in the folder specified below:");
    selectComponentsText->setWordWrap(true);

    shakeMapDirectoryLineEdit = new QLineEdit();
    shakeMapDirectoryLineEdit->setMinimumWidth(400);
    shakeMapDirectoryLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QPushButton *browseFileButton = new QPushButton();
    browseFileButton->setText(tr("Browse"));
    browseFileButton->setMaximumWidth(150);

    QPushButton *loadButton = new QPushButton();
    loadButton->setText(tr("Load"));
    loadButton->setMaximumWidth(150);

    connect(browseFileButton,SIGNAL(clicked()),this,SLOT(chooseShakeMapDirectoryDialog()));

    connect(loadButton,SIGNAL(clicked()),this,SLOT(loadShakeMapData()));

    QLabel* shakeMapText1 = new QLabel("At a minimum, the folder must contain the 'grid.xml' file.");
    shakeMapText1->setWordWrap(true);
    QLabel* shakeMapText2 = new QLabel("Click 'Load' to load the ShakeMap. Multiple ShakeMaps can be added by selecting another folder containing a ShakeMap, and clicking 'Load' again.");
    shakeMapText2->setWordWrap(true);
    QLabel* shakeMapText3 = new QLabel("The list of loaded ShakeMaps will appear on the right.");
    shakeMapText3->setWordWrap(true);

    directoryInputWidget = new QWidget();
    auto inputLayout = new QGridLayout(directoryInputWidget);

    // The combobox to select the IM
    IMListWidget = new QListWidget();

    QListWidgetItem* PGAItem= new QListWidgetItem("PGA");
    PGAItem->setFlags(PGAItem->flags() & (~Qt::ItemIsSelectable));
    PGAItem->setCheckState(Qt::Checked);
    IMListWidget->addItem(PGAItem);

    QListWidgetItem* PGVItem= new QListWidgetItem("PGV");
    PGVItem->setFlags(PGVItem->flags() & (~Qt::ItemIsSelectable));
    PGVItem->setCheckState(Qt::Unchecked);
    IMListWidget->addItem(PGVItem);

    IMListWidget->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum);

    IMListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    IMListWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    IMListWidget->setFixedSize(100,50);

    QLabel* IMLabel = new QLabel("Select the type of Intensity Measure (IM) to visualize");

    QHBoxLayout* IMLayout = new QHBoxLayout();

    IMLayout->addWidget(IMLabel);
    IMLayout->addWidget(IMListWidget);
    IMLayout->addStretch();

    auto vspacer3 = new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Expanding);
    inputLayout->addWidget(selectComponentsText,0,0,1,3);
    inputLayout->addWidget(shakeMapDirectoryLineEdit,1,0);
    inputLayout->addWidget(browseFileButton,1,1);
    inputLayout->addWidget(loadButton,1,2);
    inputLayout->addWidget(shakeMapText1,2,0,1,3);
    inputLayout->addWidget(shakeMapText2,3,0,1,3);
    inputLayout->addWidget(shakeMapText3,4,0,1,3);
    inputLayout->addLayout(IMLayout,5,0,1,3);
    inputLayout->addItem(vspacer3,6,0);

    shakeMapStackedWidget->addWidget(directoryInputWidget);
    shakeMapStackedWidget->addWidget(progressBarWidget);
    shakeMapStackedWidget->setCurrentWidget(directoryInputWidget);

    //    pathToShakeMapDirectory="/Users/steve/Desktop/SimCenter/Examples/ShakeMaps/SanAndreas/";
    //    this->loadShakeMapData();

    return shakeMapStackedWidget;
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


int ShakeMapWidget::loadShakeMapData(void)
{

    // Return if the user cancels
    if(pathToShakeMapDirectory.isEmpty() || pathToShakeMapDirectory == QDir::currentPath())
    {
        pathToShakeMapDirectory = "NULL";
        return -1;
    }


    QFileInfo inputDirInfo(pathToShakeMapDirectory);

    if(!inputDirInfo.exists())
    {
        auto relPathToDir = QCoreApplication::applicationDirPath() + QDir::separator() + pathToShakeMapDirectory;

        if (!QFileInfo::exists(relPathToDir))
        {
            QString errMsg = "The directory "+ pathToShakeMapDirectory+" does not exist check your directory and try again.";
            errorMessage(errMsg);
            return -1;
        }
        else
        {
            pathToShakeMapDirectory = relPathToDir;
            shakeMapDirectoryLineEdit->setText(pathToShakeMapDirectory);
        }

    }


    inputDirInfo = QFileInfo(pathToShakeMapDirectory);

    auto inputDir = inputDirInfo.absoluteFilePath();

    // First load files from the current directory
    this->loadDataFromDirectory(inputDir);

    // Then check to see if an array of events was provided
    if(!shakeMapList.empty())
    {
        for(auto&& event : shakeMapList)
        {
            auto dir = pathToShakeMapDirectory + QDir::separator() + event;
            this->loadDataFromDirectory(dir);
        }
    }
    else // If no list provided iterate through sub-dirs to try to find additional shakemap dirs
    {
        QDirIterator iter(inputDir, QDir::Dirs | QDir::NoDotAndDotDot/*, QDirIterator::Subdirectories*/);

        while(iter.hasNext() )
        {
            auto dir = iter.next();

            this->loadDataFromDirectory(dir);
        }
    }

    emit loadingComplete(true);

    if(this->getNumShakeMapsLoaded() == 0)
    {
        this->errorMessage("Failed to load the ShakeMaps. Check the folder and try again.");
        return -1;
    }

    return 0;
}

#if Q_GIS
int ShakeMapWidget::loadDataFromDirectory(const QString& dir)
{

    auto qGsVisWidget = static_cast<QGISVisualizationWidget*>(theVisualizationWidget);

    if(qGsVisWidget == nullptr)
    {
        qDebug()<<"Failed to cast to ArcGISVisualizationWidget";
        return -1;
    }


    const QFileInfo inputDirInfo(dir);

    // Return if the directory does not exist
    if (!inputDirInfo.exists() || !inputDirInfo.isDir())
    {
        QString errMsg ="A directory does not exist at the path: " + dir;
        this->errorMessage(errMsg);
        return -1;
    }

    QStringList acceptableFileExtensions = {"*.kmz", "*.xml", "*.shp", "*.json"};

    QDir inputDir(dir);

    QStringList inputFiles = inputDir.entryList(acceptableFileExtensions, QDir::Files);

    if(inputFiles.empty())
        return 0;

    QString eventName = inputDir.dirName();

    // Check if the shake map already exists
    if(shakeMapContainer.contains(eventName))
        return 0;

    this->statusMessage("Loading ShakeMap "+eventName);

    eventsVec.push_back(eventName);

    // Create a new shakemap
    auto inputShakeMap = new ShakeMap();

    inputShakeMap->eventName = eventName;

    shakeMapStackedWidget->setCurrentWidget(progressBarWidget);

    progressBarWidget->setVisible(true);

    QApplication::processEvents();

    progressBar->setRange(0,inputFiles.size());

    progressBar->setValue(0);

    QVector<QgsMapLayer*> layerGroup;

    int count = 0;
    foreach(QString filename, inputFiles)
    {
        auto inFilePath = dir + QDir::separator() + filename;

        // Create the XML grid
#ifndef OpenSRA
        if(filename.compare("grid.xml") == 0) // XML grid
        {
            progressLabel->setText("Loading Grid Layer");
            this->statusMessage("Loading Grid Layer");
            progressLabel->setVisible(true);
            QApplication::processEvents();

            XMLAdaptor XMLImportAdaptor;

            QString errMess;
            auto XMLlayer = XMLImportAdaptor.parseXMLFile(inFilePath, errMess, qGsVisWidget);

            if(XMLlayer == nullptr)
            {
                this->errorMessage(errMess);
                return -1;
            }

            XMLlayer->setName("Grid");

            inputShakeMap->stationList = XMLImportAdaptor.getStationList();

            inputShakeMap->gridLayer = XMLlayer;
            layerGroup.push_back(XMLlayer);
        }
#endif

        if(filename.compare("cont_pga.json") == 0) // PGA contours layer
        {
            progressLabel->setText("Loading PGA Contour Layer");
            this->statusMessage("Loading PGA Contour Layer");

            QApplication::processEvents();
            auto contPGAlayer = qGsVisWidget->addVectorLayer(inFilePath, "PGA Contours", "ogr");

            if(contPGAlayer == nullptr)
            {
                errorMessage("Failed to create the PGA contour layer");
                continue;
            }

            // Set the category renderer
            QgsLineSymbol* lineSymbol = new QgsLineSymbol();
            lineSymbol->setWidth(1.0);
            qGsVisWidget->createCategoryRenderer("value",contPGAlayer,lineSymbol);

            inputShakeMap->pgaContourLayer = contPGAlayer;
            layerGroup.push_back(contPGAlayer);
        }
        else if(filename.compare("rupture.json") == 0) // Rupture layer
        {
            progressLabel->setText("Loading Rupture Layer");
            this->statusMessage("Loading Rupture Layer");

            QApplication::processEvents();

            auto rupLayer = qGsVisWidget->addVectorLayer(inFilePath, "Rupture Plane", "ogr");

            if(rupLayer == nullptr)
            {
                errorMessage("Failed to create the rupture layer");
                continue;
            }

            inputShakeMap->faultLayer = rupLayer;

            // Set the simple fill renderer
            QgsFillSymbol* fillSymbol = new QgsFillSymbol();
            QColor color(0,0,255,40);
            fillSymbol->setColor(color);
            qGsVisWidget->createSimpleRenderer(fillSymbol,rupLayer);

            layerGroup.push_back(rupLayer);
        }
        //        else if(filename.contains("_se.kmz")) // Event layer
        //        {
        //            progressLabel->setText("Loading Event Layer");
        //            QApplication::processEvents();
        //            inputShakeMap->eventKMZLayer = theVisualizationWidget->createAndAddKMLLayer(inFilePath, "Event", eventItem);
        //            eventLayer->layers()->append(inputShakeMap->eventKMZLayer);
        //        }
        //        else if(filename.compare("polygons_mi.kmz") == 0)
        //        {
        //            progressLabel->setText("Loading PGA Polygon Layer");
        //            progressLabel->setVisible(true);
        //            QApplication::processEvents();

        //            inputShakeMap->pgaPolygonLayer = theVisualizationWidget->createAndAddKMLLayer(inFilePath, "PGA Polygons", eventItem, 0.3);
        //            eventLayer->layers()->append(inputShakeMap->pgaPolygonLayer);
        //        }
        //        else if(filename.compare("epicenter.kmz") == 0)
        //        {
        //            progressLabel->setText("Loading Epicenter Layer");
        //            QApplication::processEvents();
        //            inputShakeMap->epicenterLayer = theVisualizationWidget->createAndAddKMLLayer(inFilePath, "Epicenter", eventItem);
        //            eventLayer->layers()->append(inputShakeMap->epicenterLayer);
        //        }
        //        else if(filename.compare("cont_pga.kmz") == 0)
        //        {
        //            progressLabel->setText("Loading PGA Contour Layer");
        //            QApplication::processEvents();
        //            inputShakeMap->pgaContourLayer = theVisualizationWidget->createAndAddKMLLayer(inFilePath, "PGA Contours", eventItem);
        //            eventLayer->layers()->append(inputShakeMap->pgaContourLayer);
        //        }
        //        else if(filename.compare("overlay.kmz") == 0)
        //        {
        //            progressLabel->setText("Loading PGA Overlay Layer");
        //            QApplication::processEvents();
        //            inputShakeMap->pgaOverlayLayer = theVisualizationWidget->createAndAddKMLLayer(inFilePath, "PGA Intensity Overlay", eventItem, 0.3);
        //            eventLayer->layers()->append(inputShakeMap->pgaOverlayLayer);
        //        }
        //        else if(filename.contains("_se.kmz")) // Fault layer
        //        {
        //            progressLabel->setText("Loading Fault Layer");
        //            QApplication::processEvents();
        //            inputShakeMap->faultLayer = theVisualizationWidget->createAndAddKMLLayer(inFilePath, "Fault", eventItem);
        //            eventLayer->layers()->append(inputShakeMap->faultLayer);
        //        }
        else
        {
            continue;
        }

        ++count;
        progressLabel->clear();
        progressBar->setValue(count);

        QApplication::processEvents();
    }

    qGsVisWidget->createLayerGroup(layerGroup,eventName);

    progressLabel->setVisible(false);

    // Insert the ShakeMap into its container
    shakeMapContainer.insert(eventName,inputShakeMap);

    auto addedItem = listWidget->addItem(eventName);

    if(addedItem == nullptr)
        return -1;

    auto itemID = addedItem->getItemID();

    listWidget->setCurrentItem(itemID);

    // Reset the widget back to the input pane and close
    shakeMapStackedWidget->setCurrentWidget(directoryInputWidget);
    directoryInputWidget->setVisible(true);

    if(shakeMapStackedWidget->isModal())
        shakeMapStackedWidget->close();

    this->statusMessage("ShakeMap "+eventName+" loading complete.");

    return 0;
}
#endif



#ifdef ARC_GIS
int ShakeMapWidget::loadDataFromDirectory(const QString& dir)
{
    auto arcVizWidget = static_cast<ArcGISVisualizationWidget*>(theVisualizationWidget);

    if(arcVizWidget == nullptr)
    {
        qDebug()<<"Failed to cast to ArcGISVisualizationWidget";
        return -1;
    }

    const QFileInfo inputDirInfo(dir);

    // Return if the directory does not exist
    if (!inputDirInfo.exists() || !inputDirInfo.isDir())
    {
        QString errMsg ="A directory does not exist at the path: " + dir;
        this->errorMessage(errMsg);
        return -1;
    }

    QStringList acceptableFileExtensions = {"*.kmz", "*.xml", "*.shp", "*.json"};

    QDir inputDir(dir);

    QStringList inputFiles = inputDir.entryList(acceptableFileExtensions, QDir::Files);

    if(inputFiles.empty())
        return 0;

    QString eventName = inputDir.dirName();

    // Check if the shake map already exists
    if(shakeMapContainer.contains(eventName))
        return 0;

    this->statusMessage("Loading ShakeMap "+eventName);

    eventsVec.push_back(eventName);

    // Create a new shakemap
    auto inputShakeMap = new ShakeMap();

    inputShakeMap->eventName = eventName;

    auto layersTreeView = arcVizWidget->getLayersTree();

    // Check if there is a 'Shake Map' root item in the tree
    auto shakeMapLayerTreeItem = layersTreeView->getTreeItem("Shake Map", nullptr);

    // If there is no item, create one
    if(shakeMapLayerTreeItem == nullptr)
        shakeMapLayerTreeItem = layersTreeView->addItemToTree("Shake Map", theVisualizationWidget->createUniqueID());

    // Add the event layer to the layer tree
    auto eventItem = layersTreeView->addItemToTree(eventName, theVisualizationWidget->createUniqueID(), shakeMapLayerTreeItem);

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
        auto inFilePath = dir + QDir::separator() + filename;

#ifndef OpenSRA
        // Create the XML grid
        if(filename.compare("grid.xml") == 0) // XML grid
        {
            progressLabel->setText("Loading Grid Layer");
            this->statusMessage("Loading Grid Layer");
            progressLabel->setVisible(true);
            QApplication::processEvents();

            XMLAdaptor XMLImportAdaptor;

            QString errMess;
            auto XMLlayer = XMLImportAdaptor.parseXMLFile(inFilePath, errMess, this);

            if(XMLlayer == nullptr)
            {
                this->errorMessage(errMess);
                return -1;
            }

            XMLlayer->setName("Grid");

            XMLlayer->setAutoFetchLegendInfos(true);

            arcVizWidget->addLayerToMap(XMLlayer,eventItem);

            inputShakeMap->gridLayer = XMLlayer;
            eventLayer->layers()->append(inputShakeMap->gridLayer);

            inputShakeMap->stationList = XMLImportAdaptor.getStationList();
        }
#endif
        if(filename.compare("cont_pga.json") == 0) // PGA contours layer
        {
            progressLabel->setText("Loading PGA Contour Layer");
            this->statusMessage("Loading PGA Contour Layer");

            QApplication::processEvents();
            auto layer = arcVizWidget->createAndAddJsonLayer(inFilePath, "PGA Contours", eventItem);

            if(layer == nullptr)
            {
                errorMessage("Failed to create the PGA contour layer");
                continue;
            }

            auto featCollection = layer->featureCollection();

            auto tables = featCollection->tables();

            for(int i = 0;i<tables->size(); ++i)
            {
                auto table = tables->at(i);

                // Get the renderer
                Renderer* tableRenderer = table->renderer();

                auto simpleRenderer = dynamic_cast<SimpleRenderer*>(tableRenderer);

                if(simpleRenderer == nullptr)
                    continue;

                auto labelVal = simpleRenderer->label();

                QString labelStr = "PGA (%g) " + labelVal;
                simpleRenderer->setLabel(labelStr);
            }

            inputShakeMap->pgaContourLayer = layer;
            eventLayer->layers()->append(inputShakeMap->pgaContourLayer);
        }
        else if(filename.compare("rupture.json") == 0) // Rupture layer
        {
            progressLabel->setText("Loading Rupture Layer");
            this->statusMessage("Loading Rupture Layer");

            QApplication::processEvents();

            QColor color(0,0,255,40);
            auto layer = arcVizWidget->createAndAddJsonLayer(inFilePath, "Rupture", eventItem,color);

            if(layer == nullptr)
            {
                errorMessage("Failed to create the rupture layer");
                continue;
            }

            auto featCollection = layer->featureCollection();

            auto tables = featCollection->tables();

            for(int i = 0;i<tables->size(); ++i)
            {
                auto table = tables->at(i);

                // Get the renderer
                Renderer* tableRenderer = table->renderer();

                auto simpleRenderer = dynamic_cast<SimpleRenderer*>(tableRenderer);

                if(simpleRenderer == nullptr)
                    continue;

                auto labelStr = "Rupture";
                simpleRenderer->setLabel(labelStr);
            }

            inputShakeMap->faultLayer = layer;
            eventLayer->layers()->append(inputShakeMap->faultLayer);
        }
        //        else if(filename.contains("_se.kmz")) // Event layer
        //        {
        //            progressLabel->setText("Loading Event Layer");
        //            QApplication::processEvents();
        //            inputShakeMap->eventKMZLayer = theVisualizationWidget->createAndAddKMLLayer(inFilePath, "Event", eventItem);
        //            eventLayer->layers()->append(inputShakeMap->eventKMZLayer);
        //        }
        //        else if(filename.compare("polygons_mi.kmz") == 0)
        //        {
        //            progressLabel->setText("Loading PGA Polygon Layer");
        //            progressLabel->setVisible(true);
        //            QApplication::processEvents();

        //            inputShakeMap->pgaPolygonLayer = theVisualizationWidget->createAndAddKMLLayer(inFilePath, "PGA Polygons", eventItem, 0.3);
        //            eventLayer->layers()->append(inputShakeMap->pgaPolygonLayer);
        //        }
        //        else if(filename.compare("epicenter.kmz") == 0)
        //        {
        //            progressLabel->setText("Loading Epicenter Layer");
        //            QApplication::processEvents();
        //            inputShakeMap->epicenterLayer = theVisualizationWidget->createAndAddKMLLayer(inFilePath, "Epicenter", eventItem);
        //            eventLayer->layers()->append(inputShakeMap->epicenterLayer);
        //        }
        //        else if(filename.compare("cont_pga.kmz") == 0)
        //        {
        //            progressLabel->setText("Loading PGA Contour Layer");
        //            QApplication::processEvents();
        //            inputShakeMap->pgaContourLayer = theVisualizationWidget->createAndAddKMLLayer(inFilePath, "PGA Contours", eventItem);
        //            eventLayer->layers()->append(inputShakeMap->pgaContourLayer);
        //        }
        //        else if(filename.compare("overlay.kmz") == 0)
        //        {
        //            progressLabel->setText("Loading PGA Overlay Layer");
        //            QApplication::processEvents();
        //            inputShakeMap->pgaOverlayLayer = theVisualizationWidget->createAndAddKMLLayer(inFilePath, "PGA Intensity Overlay", eventItem, 0.3);
        //            eventLayer->layers()->append(inputShakeMap->pgaOverlayLayer);
        //        }
        //        else if(filename.contains("_se.kmz")) // Fault layer
        //        {
        //            progressLabel->setText("Loading Fault Layer");
        //            QApplication::processEvents();
        //            inputShakeMap->faultLayer = theVisualizationWidget->createAndAddKMLLayer(inFilePath, "Fault", eventItem);
        //            eventLayer->layers()->append(inputShakeMap->faultLayer);
        //        }
        else
        {
            continue;
        }


        ++count;
        progressLabel->clear();
        progressBar->setValue(count);

        QApplication::processEvents();
    }

    progressLabel->setVisible(false);


    // Insert the ShakeMap into its container
    shakeMapContainer.insert(eventName,inputShakeMap);

    auto addedItem = listWidget->addItem(eventName);

    if(addedItem == nullptr)
        return -1;

    auto itemID = addedItem->getItemID();

    listWidget->setCurrentItem(itemID);

    // Add the event layer to the map
    //    arcVizWidget->addLayerToMap(eventLayer,eventItem);

    // Reset the widget back to the input pane and close
    shakeMapStackedWidget->setCurrentWidget(directoryInputWidget);
    directoryInputWidget->setVisible(true);

    if(shakeMapStackedWidget->isModal())
        shakeMapStackedWidget->close();

    this->statusMessage("ShakeMap "+eventName+" loading complete.");

    return 0;
}
#endif


void ShakeMapWidget::chooseShakeMapDirectoryDialog(void)
{

    QFileDialog dialog(this);

    dialog.setFileMode(QFileDialog::Directory);

    QString oldPath;

    if(!shakeMapDirectoryLineEdit->text().isEmpty())
        oldPath = shakeMapDirectoryLineEdit->text();
    else
        oldPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

    pathToShakeMapDirectory = dialog.getExistingDirectory(this, tr("Folder with ShakeMap files"), oldPath) + QDir::separator();

    dialog.close();

    // Set file name & entry in line edit
    shakeMapDirectoryLineEdit->setText(pathToShakeMapDirectory);

    return;
}


bool ShakeMapWidget::outputToJSON(QJsonObject &jsonObject)
{

#ifdef OpenSRA
    jsonObject["Directory"] = pathToShakeMapDirectory;

    QJsonArray eventsArray;

    for(auto&& it : eventsVec)
        eventsArray.append(it);

    jsonObject["Events"] = eventsArray;
#else

    QFileInfo theFile(pathToEventFile);
    if (theFile.exists()) {

      QString fileName = theFile.fileName();
      QString dirPath = theFile.path();            
      QDir filePath = theFile.dir();
      QString dirName = filePath.dirName();
      filePath.cdUp();
      QString dirdirPath = filePath.absolutePath();      
      
      jsonObject["eventFile"]=dirName + QString("/") + fileName;
      jsonObject["eventFilePath"]=dirdirPath;
    } else {
        jsonObject["eventFile"]=pathToEventFile; // may be valid on others computer
        jsonObject["eventFilePath"]=QString("");
    }


    QJsonObject unitsObj;

    for(int i = 0; i < IMListWidget->count(); ++i)
    {
        auto item = IMListWidget->item(i);

        if(item->checkState() == Qt::Unchecked)
            continue;

        auto IMtag = item->text();

        if(IMtag.compare("PGA") == 0)
        {
            unitsObj["PGA"] = "g";
        }
        else if(IMtag.compare("PGV") == 0)
        {
            unitsObj["PGV"] = "cmps";
        }
        else
        {
            this->errorMessage("Could not recognize the provided intensity measure "+IMtag);
        }
    }


    jsonObject["units"] = unitsObj;

#endif

    return true;
}


bool ShakeMapWidget::outputAppDataToJSON(QJsonObject &jsonObject)
{
    emit eventTypeChangedSignal("Earthquake");

    jsonObject["Application"] = "UserInputShakeMap";

    QJsonObject appData;

    appData["Directory"] = pathToShakeMapDirectory;


    QJsonArray IMType;

    for(int i = 0; i < IMListWidget->count(); ++i)
    {
        auto item = IMListWidget->item(i);

        if(item->checkState() == Qt::Unchecked)
            continue;

        auto IMtag = item->text();

        IMType.append(IMtag);
    }

    appData["IntensityMeasureType"] = IMType;

    jsonObject["ApplicationData"]=appData;

    return true;
}


bool ShakeMapWidget::inputAppDataFromJSON(QJsonObject &jsonObject)
{
    auto appData = jsonObject.value("ApplicationData").toObject();

    pathToShakeMapDirectory = appData.value("Directory").toString();

    shakeMapDirectoryLineEdit->setText(pathToShakeMapDirectory);

    auto IMType = appData.value("IntensityMeasureType").toArray();

    if(!IMType.isEmpty())
    {

        for (auto&& v : IMType)
        {
            QString IMVal = v.toString();

            bool found = false;
            for(int i = 0; i < IMListWidget->count(); ++i)
            {
                auto item = IMListWidget->item(i);

                auto IMtag = item->text();

                if(IMVal.compare(IMtag) == 0)
                {
                    item->setCheckState(Qt::Checked);
                    found = true;
                    break;
                }

            }

            if(!found)
                this->errorMessage("Error, the intensity measure provided "+IMVal+" is currently not supported");

        }
    }

    auto res = this->loadShakeMapData();

    if(res != 0)
        return false;
    else
        return true;
}


bool ShakeMapWidget::inputFromJSON(QJsonObject &jsonObject)
{

#ifdef OpenSRA
    auto thisObject = jsonObject.value("ShakeMap").toObject();

    pathToShakeMapDirectory = thisObject.value("Directory").toString();


    QFileInfo inputDirInfo(pathToShakeMapDirectory);

    if(!inputDirInfo.exists())
    {
        auto relPathToDir = QCoreApplication::applicationDirPath() + QDir::separator() + pathToShakeMapDirectory;

        if (!QFileInfo::exists(relPathToDir))
        {
            QString errMsg = "The directory "+ pathToShakeMapDirectory+" does not exist check your directory and try again.";
            errorMessage(errMsg);
            return -1;
        }
        else
        {
            pathToShakeMapDirectory = relPathToDir;
            shakeMapDirectoryLineEdit->setText(pathToShakeMapDirectory);
        }

    }

    pathToShakeMapDirectory = inputDirInfo.absoluteFilePath();
    shakeMapDirectoryLineEdit->setText(pathToShakeMapDirectory);

    auto eventsArray = thisObject.value("Events").toArray().toVariantList();

    for(auto&& event : eventsArray)
        shakeMapList<<event.toString();

    auto res = this->loadShakeMapData();

    if(res != 0)
        return false;
    else
        return true;
#else
    Q_UNUSED(jsonObject);
    return true;
#endif

}


bool ShakeMapWidget::copyFiles(QString &destDir)
{


    QFileInfo inputDirInfo(pathToShakeMapDirectory);

    if(!inputDirInfo.exists())
    {
        QString errMsg = "The directory "+ pathToShakeMapDirectory+" does not exist check your directory and try again.";
        errorMessage(errMsg);
    }


    auto inputDir = inputDirInfo.absoluteFilePath();

    QDir dirInfo = QDir(inputDir);
    auto sourceDir = dirInfo.dirName();

    auto destPath = destDir + QDir::separator() + sourceDir;

    QDir dirDest(destPath);

    if (!dirDest.exists())
    {
        if (!dirDest.mkpath(destPath))
        {
            QString errMsg = QString("Could not create destination Dir: ") + destPath;
            this->errorMessage(errMsg);

            return false;
        }
    }

    emit outputDirectoryPathChanged(destPath, pathToEventFile);

    motionDir = destPath + QDir::separator();
    pathToEventFile = motionDir + "EventGrid.csv";

#ifdef OpenSRA
    // only copy over events in shakemap list
    for(auto&& event : this->shakeMapList)
    {
        auto currShakeMapInputPath = inputDir + QDir::separator() + event;
        auto currShakeMapDestPath = destPath + QDir::separator() + event;
        auto res = SCUtils::recursiveCopy(currShakeMapInputPath, currShakeMapDestPath);
        if(!res)
        {
            QString msg = "Error copying files over to the directory for event " + event;
            errorMessage(msg);

            return res;
        }
    }
#else
    auto res = SCUtils::recursiveCopy(inputDir, destPath);

    if(!res)
    {
        QString msg = "Error copying ShakeMap files over to the directory " + destPath;
        errorMessage(msg);

        return res;
    }
#endif

#ifndef OpenSRA

    auto currentItem = listWidget->getCurrentItem();

    auto currItemName = currentItem->getName();

    auto selectedShakeMap = shakeMapContainer.value(currItemName,nullptr);

    if(selectedShakeMap == nullptr)
    {
        this->errorMessage("Could not find the ShakeMap "+currItemName);
        return false;
    }

    CSVReaderWriter csvTool;

    auto stationList = selectedShakeMap->stationList;

    if(stationList.empty())
    {
        this->errorMessage("Error, the station list is empty for "+currItemName);
        return false;
    }

    // First create the event grid file
    QVector<QStringList> gridData;

    QStringList headerRow = {"GP_file", "Latitude", "Longitude"};
    gridData.push_back(headerRow);

    QStringList stationHeader;
    for(int i = 0; i < IMListWidget->count(); ++i)
    {
        auto item = IMListWidget->item(i);

        if(item->checkState() == Qt::Unchecked)
            continue;

        auto IMtag = item->text();

        stationHeader.append(IMtag);
    }

    this->statusMessage("Creating ground motion station files from ShakeMap, this may take some time.");

    QApplication::processEvents();

    for(int i = 0; i<stationList.size(); ++i)
    {
        auto stationFile = "Site_"+QString::number(i)+".csv";

        auto station = stationList.at(i);

        auto lat = QString::number(station.getLatitude());
        auto lon = QString::number(station.getLongitude());

        QStringList stationRow = {stationFile, lat, lon};

        gridData.push_back(stationRow);

        QStringList IMstrList;

        for(int i = 0; i < IMListWidget->count(); ++i)
        {
            auto item = IMListWidget->item(i);

            if(item->checkState() == Qt::Unchecked)
                continue;

            auto IMtag = item->text();

            if(IMtag.compare("PGA") == 0)
            {
                auto attribVal = station.getAttributeValue(IMtag);

                if(attribVal.isNull())
                {
                    this->errorMessage("Error getting the desired IM "+IMtag+" from ShakeMap grid data");
                    return false;
                }

                bool Ok = false;
                auto PGAval = attribVal.toDouble(&Ok);

                if(!Ok)
                {
                    this->errorMessage("Error getting the desired IM "+IMtag+" from ShakeMap grid data");
                    return false;
                }

                // Convert from pct g into g
                PGAval /= 100.0;

                auto PGAstr = QString::number(PGAval);
                IMstrList.append(PGAstr);

            }
            else if(IMtag.compare("PGV") == 0)
            {
                auto attribVal = station.getAttributeValue(IMtag);

                if(attribVal.isNull())
                {
                    this->errorMessage("Error getting the desired IM "+IMtag+" from ShakeMap grid data");
                    return false;
                }

                bool Ok = false;
                auto IMVal = attribVal.toDouble(&Ok);

                if(!Ok)
                {
                    this->errorMessage("Error getting the desired IM "+IMtag+" from ShakeMap grid data");
                    return false;
                }

                // Units cmps
                auto IMstr = QString::number(IMVal);
                IMstrList.append(IMstr);
            }
            else
            {
                this->errorMessage("Could not recognize the provided intensity measure "+IMtag);
            }
        }

        QVector<QStringList> stationData = {stationHeader,IMstrList};

        QString pathToStationFile = motionDir + QDir::separator() + stationFile;

        QString err;
        auto res2 = csvTool.saveCSVFile(stationData, pathToStationFile, err);
        if(res2 != 0)
        {
            this->errorMessage(err);
            return false;
        }

    }

    // Now save the site grid .csv file
    QString err2;
    auto res2 = csvTool.saveCSVFile(gridData, pathToEventFile, err2);
    if(res2 != 0)
    {
        this->errorMessage(err2);
        return false;
    }
#endif

    return true;
}


int ShakeMapWidget::getNumShakeMapsLoaded()
{
    return listWidget->getNumberOfItems();
}


void ShakeMapWidget::clear()
{
    listWidget->clear();
    shakeMapDirectoryLineEdit->clear();
    pathToShakeMapDirectory = "NULL";
    qDeleteAll(shakeMapContainer);
    shakeMapContainer.clear();
    eventsVec.clear();
    motionDir.clear();
    pathToEventFile.clear();
    shakeMapList.clear();
}
