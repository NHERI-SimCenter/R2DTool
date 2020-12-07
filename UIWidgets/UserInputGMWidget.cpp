#include "UserInputGMWidget.h"
#include "VisualizationWidget.h"
#include "TreeView.h"
#include "RegionalMappingWidget.h"
#include "WorkflowAppRDT.h"
#include "CSVReaderWriter.h"

// GIS Layers
#include "GroupLayer.h"
#include "LayerListModel.h"
#include "FeatureCollectionLayer.h"
#include "Layer.h"
#include "SimpleMarkerSymbol.h"
#include "SimpleRenderer.h"

#include <QLabel>
#include <QDialog>
#include <QGridLayout>
#include <QFile>
#include <QFileInfo>
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

}

UserInputGMWidget::~UserInputGMWidget()
{

}


bool UserInputGMWidget::outputToJSON(QJsonObject &jsonObj)
{

    auto pathToEventGrid = filePathLineEdit->text();

    if(pathToEventGrid.isEmpty())
    {
        QString msg = "Please specify the location of the ground motion input file";
        this->userMessageDialog(msg);
        return false;
    }


    const QFileInfo inputFile(pathToEventGrid);

    if (!inputFile.exists() )
    {
        QString errMsg ="A File does not exist at the path: "+pathToEventGrid;
        this->userMessageDialog(errMsg);
        return false;
    }

    jsonObj.insert("pathEventData", inputFile.dir().absolutePath());

    return true;
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


    auto regMapWidget = WorkflowAppRDT::getInstance()->getTheRegionalMappingWidget();
    connect(this,&UserInputGMWidget::outputDirectoryPathChanged,regMapWidget,&RegionalMappingWidget::handleFileNameChanged);

    pathToUserGMFile = "/Users/steve/Desktop/SimCenter/Examples/rWhaleExample/input_data_rdt/records/EventGrid.csv";
    this->loadUserGMData();

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
        pathToUserGMFile.clear();
        filePathLineEdit->clear();
        return;
    }

    const QFileInfo inputFile(pathToUserGMFile);

    if (!inputFile.exists() || !inputFile.isFile())
    {
        QString errMsg ="A file does not exist at the path: "+pathToUserGMFile;
        this->userMessageDialog(errMsg);
        return;
    }

    QStringList acceptableFileExtensions = {"*.json", "*.csv"};

    QStringList inputFiles = inputFile.dir().entryList(acceptableFileExtensions,QDir::Files);

    if(inputFiles.empty())
    {
        QString errMsg ="No files with .csv extensions were found at the path: "+pathToUserGMFile;
        this->userMessageDialog(errMsg);
        return;
    }

    QString fileName = inputFile.fileName();


    CSVReaderWriter csvTool;

    QString err;
    QVector<QStringList> data = csvTool.parseCSVFile(pathToUserGMFile,err);

    if(!err.isEmpty())
    {
        this->userMessageDialog(err);
        return;
    }

    if(data.empty())
        return;

    userGMStackedWidget->setCurrentWidget(progressBarWidget);

    progressBarWidget->setVisible(true);

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
    //    gridLayer->setMinScale(80000);

    // Pop off the row that contains the header information
    data.pop_front();

    auto numRows = data.size();

    int count = 0;

    // Get the data
    for(int i = 0; i<numRows; ++i)
    {
        auto rowStr = data.at(i);

        // Split the string at the spaces
        auto vecValues = rowStr.at(0).split(QRegExp("\\s+"), Qt::SkipEmptyParts);

        if(vecValues.size() != 3)
        {
            qDebug()<<"Error in importing user ground motions";
            return;
        }

        auto stationName = vecValues[0];

        auto stationPath = inputFile.dir().absolutePath() +"/"+ stationName;

        bool ok;
        auto lon = vecValues[1].toDouble(&ok);

        if(!ok)
        {
            QString errMsg = "Error longitude to a double, check the value";
            this->userMessageDialog(errMsg);
            return;
        }

        auto lat = vecValues[2].toDouble(&ok);

        if(!ok)
        {
            QString errMsg = "Error latitude to a double, check the value";
            this->userMessageDialog(errMsg);
            return;
        }

        GroundMotionStation GMStation(stationPath,lat,lon);

        auto res = GMStation.importGroundMotions();

        if(res == 0)
        {
            stationList.push_back(GMStation);

            // create the feature attributes
            QMap<QString, QVariant> featureAttributes;

            //            auto attrbText = GMStation.
            //            auto attrbVal = pointData[i];
            //            featureAttributes.insert(attrbText,attrbVal);

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
            QString errMsg = "Error importing ground motion " + stationName;
            this->userMessageDialog(errMsg);
            return;
        }

        ++count;
        progressLabel->clear();
        progressBar->setValue(count);

        QApplication::processEvents();
    }

    // Create a new layer
    auto layersTreeView = theVisualizationWidget->getLayersTree();

    // Check if there is a 'User Ground Motions' root item in the tree
    auto userInputTreeItem = layersTreeView->getTreeItem("User Ground Motions", nullptr);

    // If there is no item, create one
    if(userInputTreeItem == nullptr)
        userInputTreeItem = layersTreeView->addItemToTree("User Ground Motions", QString());


    // Add the event layer to the layer tree
    auto eventItem = layersTreeView->addItemToTree(fileName, QString(), userInputTreeItem);


    progressLabel->setVisible(false);

    // Add the event layer to the map
    theVisualizationWidget->addLayerToMap(gridLayer,eventItem);

    // Reset the widget back to the input pane and close
    userGMStackedWidget->setCurrentWidget(fileInputWidget);
    fileInputWidget->setVisible(true);

    if(userGMStackedWidget->isModal())
        userGMStackedWidget->close();

    emit loadingComplete(true);

    emit outputDirectoryPathChanged(pathToUserGMFile);

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
