#include "UserInputGMWidget.h"
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

UserInputGMWidget::UserInputGMWidget(VisualizationWidget* visWidget, QWidget *parent) : SimCenterAppWidget(parent), theVisualizationWidget(visWidget)
{
    progressBar = nullptr;
    fileInputWidget = nullptr;
    progressBarWidget = nullptr;
    userGMStackedWidget = nullptr;
    progressLabel = nullptr;
    pathToUserGMFile = "NULL";


    //    this->loadComponentData();
}

UserInputGMWidget::~UserInputGMWidget()
{

}


void UserInputGMWidget::showUserGMLayers(bool state)
{
    auto layersTreeView = theVisualizationWidget->getLayersTree();


    if(state == false)
    {
        layersTreeView->removeItemFromTree("User Ground Motions");

        return;
    }


    // Check if there is a 'Shake Map' root item in the tree
    auto shakeMapTreeItem = layersTreeView->getTreeItem("User Ground Motions",nullptr);

    // If there is no item, create one
    if(shakeMapTreeItem == nullptr)
        shakeMapTreeItem = layersTreeView->addItemToTree("User Ground Motions",QString());


//    for(auto&& it : shakeMapContainer)
//    {
//        auto eventName = it->eventLayer->name();
//        auto eventID = it->eventLayer->layerId();
//        auto eventItem = layersTreeView->addItemToTree(eventName, eventID, shakeMapTreeItem);

//        auto layers = it->getAllActiveSubLayers();
//        for(auto&& layer : layers)
//        {
//            auto layerName = layer->name();
//            auto layerID = layer->layerId();

//            layersTreeView->addItemToTree(layerName, layerID, eventItem);
//        }
//    }
}


QStackedWidget* UserInputGMWidget::getUserInputGMWidget(void)
{
    if (userGMStackedWidget)
        return userGMStackedWidget.get();

    userGMStackedWidget = std::make_unique<QStackedWidget>();

    fileInputWidget = new QWidget(this);
    auto inputLayout = new QHBoxLayout(fileInputWidget);
    fileInputWidget->setLayout(inputLayout);

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

    userGMStackedWidget->addWidget(fileInputWidget);
    userGMStackedWidget->addWidget(progressBarWidget);

    userGMStackedWidget->setCurrentWidget(fileInputWidget);

    QLabel* selectComponentsText = new QLabel();
    selectComponentsText->setText("Select a file containing earthquake ground motions");

    filePathLineEdit = new QLineEdit();
    filePathLineEdit->setMaximumWidth(750);
    filePathLineEdit->setMinimumWidth(400);
    filePathLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QPushButton *browseFileButton = new QPushButton();
    browseFileButton->setText(tr("Browse"));
    browseFileButton->setMaximumWidth(150);

    connect(browseFileButton,SIGNAL(clicked()),this,SLOT(chooseUserFileDialog()));

    inputLayout->addStretch(0);
    inputLayout->addWidget(selectComponentsText);
    inputLayout->addWidget(filePathLineEdit);
    inputLayout->addWidget(browseFileButton);
    inputLayout->addStretch(0);

    userGMStackedWidget->setWindowTitle("Select file containing earthquake ground motions");
    userGMStackedWidget->setMinimumWidth(400);
    userGMStackedWidget->setMinimumHeight(150);

    return userGMStackedWidget.get();
}


void UserInputGMWidget::showUserGMSelectDialog(void)
{

    if (!userGMStackedWidget)
    {
        this->getUserInputGMWidget();
    }

    userGMStackedWidget->show();
    userGMStackedWidget->raise();
    userGMStackedWidget->activateWindow();
}


void UserInputGMWidget::loadUserGMData(void)
{
    // Set file name & entry in line edit
    filePathLineEdit->setText(pathToUserGMFile);

    // Return if the user cancels
    if(pathToUserGMFile.isEmpty() || pathToUserGMFile == QDir::currentPath())
    {
        pathToUserGMFile = "NULL";
        return;
    }

    const QFileInfo inputDir(pathToUserGMFile);

    if (!inputDir.exists() || !inputDir.isFile())
    {
        QString errMsg ="A file does not exist at the path: "+pathToUserGMFile;
        this->userMessageDialog(errMsg);
        return;
    }

    QStringList acceptableFileExtensions = {"*.json", "*.csv"};

    QStringList inputFiles = inputDir.dir().entryList(acceptableFileExtensions,QDir::Files);

    if(inputFiles.empty())
    {
        QString errMsg ="No files with .json or .csv extensions were found at the path: "+pathToUserGMFile;
        this->userMessageDialog(errMsg);
        return;
    }

    QString fileName = inputDir.fileName();

    // Create a new shakemap
    auto layersTreeView = theVisualizationWidget->getLayersTree();

    // Check if there is a 'Shake Map' root item in the tree
    auto userInputTreeItem = layersTreeView->getTreeItem("User Ground Motions", nullptr);

    // If there is no item, create one
    if(userInputTreeItem == nullptr)
        userInputTreeItem = layersTreeView->addItemToTree("User Ground Motions", QString());


    // Add the event layer to the layer tree
    auto eventItem = layersTreeView->addItemToTree(fileName, QString(), userInputTreeItem);

    // Create the root event group layer
//    inputShakeMap->eventLayer = new GroupLayer(QList<Layer*>{});

//    auto eventLayer = inputShakeMap->eventLayer;

//    eventLayer->setName(eventName);

//    userGMStackedWidget->setCurrentWidget(progressBarWidget);

//    progressBarWidget->setVisible(true);

//    QApplication::processEvents();

//    progressBar->setRange(0,inputFiles.size());

//    progressBar->setValue(0);

    int count = 0;
    foreach(QString filename, inputFiles)
    {
        auto inFilePath = pathToUserGMFile + filename;



        ++count;
        progressLabel->clear();
        progressBar->setValue(count);

        QApplication::processEvents();
    }

    progressLabel->setVisible(false);



    // Add the event layer to the map
//    theVisualizationWidget->addLayerToMap(eventLayer,eventItem);

    // Reset the widget back to the input pane and close
    userGMStackedWidget->setCurrentWidget(fileInputWidget);
    fileInputWidget->setVisible(true);

    if(userGMStackedWidget->isModal())
        userGMStackedWidget->close();

    emit loadingComplete(true);

    return;
}


void UserInputGMWidget::chooseUserFileDialog(void)
{

    QFileDialog dialog(this);

    dialog.setFileMode(QFileDialog::Directory);

    pathToUserGMFile = dialog.getExistingDirectory(this, tr("File with user specified ground motions"));

    pathToUserGMFile.append("/");

    dialog.close();

    this->loadUserGMData();

    return;
}
