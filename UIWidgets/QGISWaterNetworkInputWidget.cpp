#include "QGISWaterNetworkInputWidget.h"
#include "QGISVisualizationWidget.h"
#include "ComponentDatabaseManager.h"
#include "ComponentTableView.h"
#include "ComponentTableModel.h"
#include "NonselectableComponentInputWidget.h"

#include "AssetFilterDelegate.h"
#include "CSVReaderWriter.h"

#include <qgsfeature.h>
#include <qgslinesymbol.h>
#include <qgsmarkersymbol.h>

#include <QHeaderView>
#include <QFileDialog>
#include <QSplitter>


QGISWaterNetworkInputWidget::QGISWaterNetworkInputWidget(QWidget *parent, VisualizationWidget* visWidget) : SimCenterAppWidget(parent)
{
    theVisualizationWidget = static_cast<QGISVisualizationWidget*>(visWidget);
    assert(theVisualizationWidget);

    theNodesDb = ComponentDatabaseManager::getInstance()->getWaterNetworkNodeComponentDb();
    thePipelinesDb = ComponentDatabaseManager::getInstance()->getWaterNetworkPipeComponentDb();

    theNodesWidget = new NonselectableComponentInputWidget(this, theNodesDb, theVisualizationWidget, "Water Network Nodes");

    theNodesWidget->setLabel1("Load water network node information from a CSV file");

    thePipelinesWidget = new NonselectableComponentInputWidget(this, thePipelinesDb, theVisualizationWidget, "Water Network Pipelines");

    thePipelinesWidget->setLabel1("Load water network pipeline information from a CSV file");

    connect(theNodesWidget,&NonselectableComponentInputWidget::doneLoadingComponents,this,&QGISWaterNetworkInputWidget::handleAssetsLoaded);
    connect(thePipelinesWidget,&NonselectableComponentInputWidget::doneLoadingComponents,this,&QGISWaterNetworkInputWidget::handleAssetsLoaded);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QLabel* pathText = new QLabel();
    pathText->setText("Path to .inp file:");

    inpFileLineEdit = new QLineEdit();
    //    componentFileLineEdit->setMaximumWidth(750);
    inpFileLineEdit->setMinimumWidth(400);
    inpFileLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    QPushButton *browseFileButton = new QPushButton();
    browseFileButton->setText(tr("Browse"));
    browseFileButton->setMaximumWidth(150);

    connect(browseFileButton,SIGNAL(clicked()),this,SLOT(chooseFileDialog()));

    QHBoxLayout* inpPathLayout = new QHBoxLayout;

    inpPathLayout->addWidget(pathText);
    inpPathLayout->addWidget(inpFileLineEdit);
    inpPathLayout->addWidget(browseFileButton);

    mainLayout->addLayout(inpPathLayout);

    QSplitter* verticalSplitter = new QSplitter(Qt::Vertical);

    verticalSplitter->addWidget(theNodesWidget);
    verticalSplitter->addWidget(thePipelinesWidget);

    mainLayout->addWidget(verticalSplitter);

    // Testing to remove
//    theNodesWidget->setPathToComponentInputFile("/Users/steve/Desktop/SogaExample/node_information.csv");
//    theNodesWidget->loadComponentData();

//    thePipelinesWidget->setPathToComponentInputFile("/Users/steve/Desktop/SogaExample/pipe_information.csv");
//    thePipelinesWidget->loadComponentData();

//    inpFileLineEdit->setText("/Users/steve/Desktop/SogaExample/central.inp");
}


bool QGISWaterNetworkInputWidget::copyFiles(QString &destName)
{
    Q_UNUSED(destName);

    //    auto compLineEditText = componentFileLineEdit->text();

    //    QFileInfo componentFile(compLineEditText);

    //    if (!componentFile.exists())
    //        return false;

    //    // Do not copy the file, output a new csv which will have the changes that the user makes in the table
    //    //        if (componentFile.exists()) {
    //    //            return this->copyFile(componentFileLineEdit->text(), destName);
    //    //        }

    //    auto pathToSaveFile = destName + QDir::separator() + componentFile.fileName();

    //    auto nRows = componentTableWidget->rowCount();

    //    if(nRows == 0)
    //        return false;

    //    auto data = componentTableWidget->getTableModel()->getTableData();

    //    auto headerValues = componentTableWidget->getTableModel()->getHeaderStringList();

    //    data.push_front(headerValues);

    //    CSVReaderWriter csvTool;

    //    QString err;
    //    csvTool.saveCSVFile(data,pathToSaveFile,err);

    //    if(!err.isEmpty())
    //        return false;

    return true;
}



bool QGISWaterNetworkInputWidget::outputAppDataToJSON(QJsonObject &jsonObject)
{
    jsonObject["Application"]="CSV_to_WATERNETWORK";

    QJsonObject data;

    // First the inp file (in EPANET format)
    QFileInfo inpFile(inpFileLineEdit->text());

    if (inpFile.exists()) {

        data["WaterNetworkSourceFile"]=inpFile.fileName();
        data["pathToWaterNetworkSource"]=inpFile.path();
    }
    else
    {
        data["WaterNetworkSourceFile"]=QString("None");
        data["pathToWaterNetworkSource"]=QString("");
        return false;
    }

    // The file containing the network nodes
    theNodesWidget->outputAppDataToJSON(data);

    // The file containing the network pipelines
    thePipelinesWidget->outputAppDataToJSON(data);

    jsonObject["ApplicationData"] = data;

    return true;
}


bool QGISWaterNetworkInputWidget::inputAppDataFromJSON(QJsonObject &jsonObject)
{

    // Check the app type
    if (jsonObject.contains("Application")) {
        if ("CSV_to_WATERNETWORK" != jsonObject["Application"].toString()) {
            this->errorMessage("QGISWaterNetworkInputWidget::inputFRommJSON app name conflict");
            return false;
        }
    }


    if (jsonObject.contains("ApplicationData")) {
        QJsonObject appData = jsonObject["ApplicationData"].toObject();

        // First load the .inp file
        QFileInfo inpFileInfo;
        QString inpFileName;
        QString pathToInpFile;
        bool foundFile = false;
        if (appData.contains("WaterNetworkSourceFile"))
            inpFileName = appData["WaterNetworkSourceFile"].toString();

        if (inpFileInfo.exists(inpFileName)) {

            pathToINPInputFile = inpFileName;
            inpFileLineEdit->setText(inpFileName);

            foundFile = true;

        } else {

            if (appData.contains("pathToWaterNetworkSource"))
                pathToInpFile = appData["pathToWaterNetworkSource"].toString();
            else
                pathToInpFile=QDir::currentPath();

            pathToINPInputFile = pathToInpFile + QDir::separator() + inpFileName;

            if (inpFileInfo.exists(pathToINPInputFile)) {
                inpFileLineEdit->setText(pathToINPInputFile);
                foundFile = true;

            } else {
                // adam .. adam .. adam
                pathToINPInputFile = pathToInpFile + QDir::separator()
                        + "input_data" + QDir::separator() + inpFileName;
                if (inpFileInfo.exists(pathToINPInputFile)) {
                    inpFileLineEdit->setText(pathToINPInputFile);
                    foundFile = true;
                }
                else
                {
                    QString errMessage = "CSV_to_WATERNETWORK no file found at: " + inpFileName;
                    this->errorMessage(errMessage);
                    return false;
                }
            }
        }

        if(foundFile == false)
        {
            QString errMessage = "CSV_to_WATERNETWORK no file found: " + inpFileName;
            this->errorMessage(errMessage);
            return false;
        }

        // Input the nodes
        auto res = theNodesWidget->inputAppDataFromJSON(appData);

        if(!res)
            return res;

        // Input the pipes
        res = thePipelinesWidget->inputAppDataFromJSON(appData);

        if(!res)
            return res;

    }

    return true;
}




int QGISWaterNetworkInputWidget::loadPipelinesVisualization()
{

    if(nodePointsMap.isEmpty())
        return -1;


    auto pipelinesTableWidget = thePipelinesWidget->getTableWidget();

    QgsFields featFields;
    featFields.append(QgsField("ID", QVariant::Int));
    featFields.append(QgsField("AssetType", QVariant::String));
    featFields.append(QgsField("TabName", QVariant::String));

    // Set the table headers as fields in the table
    for(int i = 1; i<pipelinesTableWidget->columnCount(); ++i)
    {
        auto fieldText = pipelinesTableWidget->horizontalHeaderItemVariant(i);
        featFields.append(QgsField(fieldText.toString(),fieldText.type()));
    }

    auto attribFields = featFields.toList();


    auto horzHeaders = thePipelinesWidget->getTableHorizontalHeadings();

    auto indexNodeTag1 = horzHeaders.indexOf("node1");

    if(indexNodeTag1 == -1)
    {
        this->errorMessage("Error, cannot find column header 'node1' that specifies the starting node of a pipe");
        return -1;
    }

    auto indexNodeTag2 = horzHeaders.indexOf("node2");

    if(indexNodeTag2 == -1)
    {
        this->errorMessage("Error, cannot find column header 'node2' that specifies the ending node of a pipe");
        return -1;
    }

    // Create the pipelines layer
    pipelinesMainLayer = theVisualizationWidget->addVectorLayer("linestring","Water Network Pipelines");

    if(pipelinesMainLayer == nullptr)
    {
        this->errorMessage("Error adding a vector layer");
        return -1;
    }

    auto pr = pipelinesMainLayer->dataProvider();

    pipelinesMainLayer->startEditing();

    auto res = pr->addAttributes(attribFields);

    if(!res)
        this->errorMessage("Error adding attributes to the layer" + pipelinesMainLayer->name());

    pipelinesMainLayer->updateFields(); // tell the vector layer to fetch changes from the provider

    thePipelinesDb->setMainLayer(pipelinesMainLayer);

    // Get the number of rows
    auto nRows = pipelinesTableWidget->rowCount();

    auto numAtrb = attribFields.size();

    for(int i = 0; i<nRows; ++i)
    {

        // create the feature attributes
        QgsAttributes featureAttributes(numAtrb);

        // Create a new pipeline
        QString pipelineIDStr = pipelinesTableWidget->item(i,0).toString();

        int pipelineID =  pipelineIDStr.toInt();

        // Create a unique ID for the building
        //        auto uid = theVisualizationWidget->createUniqueID();

        // "ID"
        // "AssetType"
        // "Tabname"

        featureAttributes[0] = QVariant(pipelineID);
        featureAttributes[1] = QVariant("WATERPIPELINES");
        featureAttributes[2] = QVariant("ID: "+QString::number(pipelineID));

        // The feature attributes are the columns from the table
        for(int j = 1; j<pipelinesTableWidget->columnCount(); ++j)
        {
            auto attrbVal = pipelinesTableWidget->item(i,j);
            featureAttributes[2+j] = attrbVal;
        }

        QgsFeature feature;
        feature.setFields(featFields);

        auto nodeTag1 = pipelinesTableWidget->item(i,indexNodeTag1).toInt();

        auto nodeTag2 = pipelinesTableWidget->item(i,indexNodeTag2).toInt();

        // Start and end point of the pipe
        QgsPointXY point1 = nodePointsMap.value(nodeTag1);

        if(point1.isEmpty())
        {
            this->errorMessage("Error, could not find node with ID "+ QString::number(nodeTag1)+ " in the node table");
            return -1;
        }

        QgsPointXY point2 = nodePointsMap.value(nodeTag2);

        if(point2.isEmpty())
        {
            this->errorMessage("Error, could not find node with ID "+ QString::number(nodeTag2)+ " in the node table");
            return -1;
        }

        QgsPolylineXY pipeSegment(2);
        pipeSegment[0]=point1;
        pipeSegment[1]=point2;

        feature.setGeometry(QgsGeometry::fromPolylineXY(pipeSegment));

        feature.setAttributes(featureAttributes);

        if(!feature.isValid())
            return -1;

        auto res = pr->addFeature(feature, QgsFeatureSink::FastInsert);
        if(!res)
        {
            this->errorMessage("Error adding the feature to the layer");
            return -1;
        }
    }

    pipelinesMainLayer->commitChanges(true);
    pipelinesMainLayer->updateExtents();

    QgsLineSymbol* markerSymbol = new QgsLineSymbol();

    markerSymbol->setWidth(0.8);
    markerSymbol->setColor(Qt::darkBlue);
    theVisualizationWidget->createSimpleRenderer(markerSymbol,pipelinesMainLayer);

    //    auto numFeat = mainLayer->featureCount();

    theVisualizationWidget->zoomToLayer(pipelinesMainLayer);

    QVector<QgsMapLayer*> mapLayers;
    mapLayers.push_back(nodesMainLayer);
    mapLayers.push_back(pipelinesMainLayer);

    theVisualizationWidget->createLayerGroup(mapLayers,"Water Network");

    return 0;
}


int QGISWaterNetworkInputWidget::loadNodesVisualization()
{

    auto theNodesTableWidget = theNodesWidget->getTableWidget();

    // Create the building attributes that are fixed
    QgsFields featFields;
    featFields.append(QgsField("ID", QVariant::Int));
    featFields.append(QgsField("AssetType", QVariant::String));
    featFields.append(QgsField("TabName", QVariant::String));

    // Set the table headers as fields in the table
    for(int i = 1; i<theNodesTableWidget->columnCount(); ++i)
    {
        auto fieldText = theNodesTableWidget->horizontalHeaderItemVariant(i);
        featFields.append(QgsField(fieldText.toString(),fieldText.type()));
    }

    QList<QgsField> attribFields;
    for(int i = 0; i<featFields.size(); ++i)
        attribFields.push_back(featFields[i]);

    auto headers = theNodesWidget->getTableHorizontalHeadings();

    // First check if a footprint was provided
    auto indexLatitude = theVisualizationWidget->getIndexOfVal(headers, "latitude");
    auto indexLongitude = theVisualizationWidget->getIndexOfVal(headers, "longitude");

    if(indexLongitude == -1 || indexLatitude == -1)
    {
        this->errorMessage("Could not find latitude and longitude in the header columns");
        return -1;
    }

    // Get the number of rows
    auto nRows = theNodesTableWidget->rowCount();

    QString layerType = "point";

    // Create the buildings layer
    nodesMainLayer = theVisualizationWidget->addVectorLayer(layerType,"Water Network Nodes");

    if(nodesMainLayer == nullptr)
    {
        this->errorMessage("Error adding a vector layer");
        return -1;
    }

    auto pr = nodesMainLayer->dataProvider();

    nodesMainLayer->startEditing();

    auto res = pr->addAttributes(attribFields);

    if(!res)
        this->errorMessage("Error adding attributes to the layer" + nodesMainLayer->name());

    nodesMainLayer->updateFields(); // tell the vector layer to fetch changes from the provider

    theNodesDb->setMainLayer(nodesMainLayer);

    auto numAtrb = attribFields.size();

    for(int i = 0; i<nRows; ++i)
    {
        // create the feature attributes
        QgsAttributes featureAttributes(numAtrb);

        // Create a new node
        QString nodeIDStr = theNodesTableWidget->item(i,0).toString();

        int nodeID = nodeIDStr.toInt();

        // Create a unique ID for the building
        //        auto uid = theVisualizationWidget->createUniqueID();

        //  "ID"
        //  "AssetType"
        //  "TabName"

        featureAttributes[0] = QVariant(nodeID);
        featureAttributes[1] = QVariant("WATERNETWORKNODE");
        featureAttributes[2] = QVariant(nodeIDStr);

        // The feature attributes are the columns from the table
        for(int j = 1; j<theNodesTableWidget->columnCount(); ++j)
        {
            auto attrbVal = theNodesTableWidget->item(i,j);
            featureAttributes[2+j] = attrbVal;
        }

        auto latitude = theNodesTableWidget->item(i,indexLatitude).toDouble();
        auto longitude = theNodesTableWidget->item(i,indexLongitude).toDouble();

        QgsFeature feature;
        feature.setFields(featFields);

        QgsPointXY point(longitude,latitude);
        auto geom = QgsGeometry::fromPointXY(point);
        if(geom.isEmpty())
        {
            this->errorMessage("Error getting the building footprint geometry");
            return -1;
        }

        nodePointsMap.insert(nodeID,point);

        feature.setGeometry(geom);

        feature.setAttributes(featureAttributes);

        if(!feature.isValid())
            return -1;


        auto res = pr->addFeature(feature, QgsFeatureSink::FastInsert);
        if(!res)
        {
            this->errorMessage("Error adding the feature to the layer");
            return -1;
        }

        //        auto id = feature.id();

        //        qDebug()<<id;
    }

    nodesMainLayer->commitChanges(true);
    nodesMainLayer->updateExtents();

    QgsSymbol* markerSymbol = new QgsMarkerSymbol();

    markerSymbol->setColor(Qt::blue);
    theVisualizationWidget->createSimpleRenderer(markerSymbol,nodesMainLayer);

    //    auto numFeat = mainLayer->featureCount();

    theVisualizationWidget->zoomToLayer(nodesMainLayer);

    return 0;
}



void QGISWaterNetworkInputWidget::clear()
{
    theNodesDb->clear();
    thePipelinesDb->clear();
    nodePointsMap.clear();
    theNodesWidget->clear();
    thePipelinesWidget->clear();
    pathToINPInputFile.clear();
    inpFileLineEdit->clear();
}


void QGISWaterNetworkInputWidget::handleAssetsLoaded()
{
    if(theNodesWidget->isEmpty() || thePipelinesWidget->isEmpty())
        return;

    auto res = this->loadNodesVisualization();

    if(res != 0)
    {
        this->errorMessage("Error, failed to load the water network nodes visualization");
        return;
    }

    res = this->loadPipelinesVisualization();

    if(res != 0)
    {
        this->errorMessage("Error, failed to load the water network pipelines visualization");
        return;
    }

}


void QGISWaterNetworkInputWidget::chooseFileDialog(void)
{
    pathToINPInputFile = QFileDialog::getOpenFileName(this,tr("Component Information File"));

    // Return if the user cancels
    if(pathToINPInputFile.isEmpty())
    {
        pathToINPInputFile = "NULL";
        return;
    }

    // Set file name & entry in qLine edit
    inpFileLineEdit->setText(pathToINPInputFile);
}

