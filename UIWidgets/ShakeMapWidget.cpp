#include "ShakeMapWidget.h"
#include "VisualizationWidget.h"
#include "TreeView.h"

// GIS Layers
#include "GroupLayer.h"
#include "LayerListModel.h"
#include "FeatureCollectionLayer.h"
#include "KmlLayer.h"
#include "Layer.h"

#include <QLabel>
#include <QDialog>
#include <QGridLayout>
#include <QFile>
#include <QFileDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QProgressBar>
#include <QSpacerItem>
#include <QApplication>

using namespace Esri::ArcGISRuntime;

ShakeMapWidget::ShakeMapWidget(VisualizationWidget* visWidget) : SimCenterAppWidget(nullptr), theVisualizationWidget(visWidget)
{
    progressBar = nullptr;
    directoryInputWidget = nullptr;
    progressBarWidget = nullptr;
    loadShakeMapStackedWidget = nullptr;
    progressLabel = nullptr;
    pathToShakeMapDirectory = "NULL";

    //    this->loadComponentData();
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
        shakeMapTreeItem = layersTreeView->addItemToTree("Shake Map");


    for(auto&& it : shakeMapContainer)
    {
        auto eventName = it->eventLayer->name();
        auto eventItem = layersTreeView->addItemToTree(eventName, shakeMapTreeItem);

        auto layers = it->getAllActiveSubLayers();
        for(auto&& layer : layers)
        {
            auto layerName = layer->name();

            layersTreeView->addItemToTree(layerName, eventItem);
        }
    }


}


void ShakeMapWidget::showLoadShakeMapDialog(void)
{

    if (!loadShakeMapStackedWidget)
    {
        loadShakeMapStackedWidget = std::make_unique<QStackedWidget>();

        directoryInputWidget = new QWidget(this);
        auto inputLayout = new QGridLayout(directoryInputWidget);
        directoryInputWidget->setLayout(inputLayout);

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

        loadShakeMapStackedWidget->addWidget(directoryInputWidget);
        loadShakeMapStackedWidget->addWidget(progressBarWidget);

        loadShakeMapStackedWidget->setCurrentWidget(directoryInputWidget);

        QLabel* selectComponentsText = new QLabel();
        selectComponentsText->setText("Select a folder with ShakeMap files");

        shakeMapDirectoryLineEdit = new QLineEdit();
        shakeMapDirectoryLineEdit->setMaximumWidth(750);
        shakeMapDirectoryLineEdit->setMinimumWidth(400);
        shakeMapDirectoryLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

        QPushButton *browseFileButton = new QPushButton();
        browseFileButton->setText(tr("Browse"));
        browseFileButton->setMaximumWidth(150);

        //        QPushButton *loadFileButton = new QPushButton();
        //        loadFileButton->setText(tr("Load File"));
        //        loadFileButton->setMaximumWidth(150);

        connect(browseFileButton,SIGNAL(clicked()),this,SLOT(chooseShakeMapDirectoryDialog()));
        //        connect(loadFileButton,SIGNAL(clicked()),this,SLOT(loadComponentData()));

        inputLayout->addWidget(selectComponentsText, 0, 0);
        inputLayout->addWidget(shakeMapDirectoryLineEdit, 0, 1);
        inputLayout->addWidget(browseFileButton, 0, 2);

        loadShakeMapStackedWidget->setWindowTitle("Select folder containing ShakeMap");

        loadShakeMapStackedWidget->setMinimumWidth(400);
        loadShakeMapStackedWidget->setMinimumHeight(150);
    }

    loadShakeMapStackedWidget->show();
    loadShakeMapStackedWidget->raise();
    loadShakeMapStackedWidget->activateWindow();
}


void ShakeMapWidget::loadShakeMapData(void)
{
    // Set file name & entry in line edit
    shakeMapDirectoryLineEdit->setText(pathToShakeMapDirectory);

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
        shakeMapTreeItem = layersTreeView->addItemToTree("Shake Map");


    // Add the event layer to the layer tree
    auto eventItem = layersTreeView->addItemToTree(eventName, shakeMapTreeItem);

    // Create the root event group layer
    inputShakeMap->eventLayer = new GroupLayer(QList<Layer*>{});

    auto eventLayer = inputShakeMap->eventLayer;

    eventLayer->setName(eventName);

    loadShakeMapStackedWidget->setCurrentWidget(progressBarWidget);

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

    // Add the event layer to the map
    theVisualizationWidget->addLayerToMap(eventLayer,eventItem);

    // Reset the widget back to the input pane and close
    loadShakeMapStackedWidget->setCurrentWidget(directoryInputWidget);
    directoryInputWidget->setVisible(true);
    loadShakeMapStackedWidget->close();

    return;
}


void ShakeMapWidget::chooseShakeMapDirectoryDialog(void)
{

    QFileDialog dialog(this);

    dialog.setFileMode(QFileDialog::Directory);

    pathToShakeMapDirectory = dialog.getExistingDirectory(this, tr("Folder with ShakeMap files"));

    pathToShakeMapDirectory.append("/");

    dialog.close();

    this->loadShakeMapData();

    return;
}
