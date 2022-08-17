#include "CSVWaterNetworkInputWidget.h"
#include "QGISVisualizationWidget.h"
#include "LineAssetInputWidget.h"
#include "PointAssetInputWidget.h"
#include "ComponentDatabaseManager.h"
#include "ComponentTableView.h"
#include "ComponentTableModel.h"

#include "AssetFilterDelegate.h"
#include "CSVReaderWriter.h"

#include <qgsfeature.h>
#include <qgslinesymbol.h>
#include <qgsmarkersymbol.h>

#include <QHeaderView>
#include <QFileDialog>
#include <QSplitter>


CSVWaterNetworkInputWidget::CSVWaterNetworkInputWidget(QWidget *parent, VisualizationWidget* visWidget) : SimCenterAppWidget(parent)
{
    theVisualizationWidget = static_cast<QGISVisualizationWidget*>(visWidget);
    assert(theVisualizationWidget);

//    theNodesDb = ComponentDatabaseManager::getInstance()->getWaterNetworkNodeComponentDb();
    thePipelinesDb = ComponentDatabaseManager::getInstance()->createAssetDb("Water Network Pipelines");

    //    theNodesWidget = new NonselectableAssetInputWidget(this, theNodesDb, theVisualizationWidget, "Water Network Nodes");
    //QWidget *parent, VisualizationWidget* visWidget, QString componentType, QString appType = QString()
    theNodesWidget = new PointAssetInputWidget(this, theVisualizationWidget, "Water Network Nodes", "CSV_to_AIM");

    theNodesWidget->setLabel1("Load water network node information from a CSV file");

    //    thePipelinesWidget = new NonselectableAssetInputWidget(this, thePipelinesDb, theVisualizationWidget, "Water Network Pipelines");
    thePipelinesWidget = new LineAssetInputWidget(this, theVisualizationWidget, "Water Network Pipelines", "CSV_to_AIM");

    thePipelinesWidget->setLabel1("Load water network pipeline information from a CSV file");

    //    connect(theNodesWidget,&NonselectableAssetInputWidget::doneLoadingComponents,this,&CSVWaterNetworkInputWidget::handleAssetsLoaded);
    //    connect(thePipelinesWidget,&NonselectableAssetInputWidget::doneLoadingComponents,this,&CSVWaterNetworkInputWidget::handleAssetsLoaded);

    connect(theNodesWidget,&LineAssetInputWidget::doneLoadingComponents,this,&CSVWaterNetworkInputWidget::handleAssetsLoaded);
    connect(thePipelinesWidget,&LineAssetInputWidget::doneLoadingComponents,this,&CSVWaterNetworkInputWidget::handleAssetsLoaded);

    thePipelinesWidget->setTheNodesWidget(theNodesWidget);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QSplitter* verticalSplitter = new QSplitter(Qt::Vertical);

    verticalSplitter->addWidget(theNodesWidget);
    verticalSplitter->addWidget(thePipelinesWidget);

    mainLayout->addWidget(verticalSplitter);

    // Testing to remove
    //    theNodesWidget->setPathToComponentInputFile("/Users/steve/Desktop/SogaExample/node_information.csv");
    //    theNodesWidget->loadComponentData();

    //    thePipelinesWidget->setPathToComponentInputFile("/Users/steve/Desktop/SogaExample/pipe_information.csv");
    //    thePipelinesWidget->loadComponentData();

}


CSVWaterNetworkInputWidget::~CSVWaterNetworkInputWidget()
{

}


bool CSVWaterNetworkInputWidget::copyFiles(QString &destName)
{
    auto res = theNodesWidget->copyFiles(destName);

    if(!res)
        return res;

    return thePipelinesWidget->copyFiles(destName);
}



bool CSVWaterNetworkInputWidget::outputAppDataToJSON(QJsonObject &jsonObject)
{
    jsonObject["Application"]="CSV_to_WATERNETWORK";

    QJsonObject data;

    QJsonObject nodeData;
    // The file containing the network nodes
    auto res = theNodesWidget->outputAppDataToJSON(nodeData);

    if(!res)
    {
        this->errorMessage("Error, could not get the .json output from the nodes widget in CSV_to_WATERNETWORK");
        return false;
    }

    data["WaterNetworkNodes"] = nodeData;

    QJsonObject pipelineData;
    // The file containing the network pipelines
    res = thePipelinesWidget->outputAppDataToJSON(pipelineData);

    if(!res)
    {
        this->errorMessage("Error, could not get the .json output from the pipelines widget in CSV_to_WATERNETWORK");
        return false;
    }

    data["WaterNetworkPipelines"] = pipelineData;

    jsonObject["ApplicationData"] = data;

    return true;
}


bool CSVWaterNetworkInputWidget::inputAppDataFromJSON(QJsonObject &jsonObject)
{

    // Check the app type
    if (jsonObject.contains("Application")) {
        if ("CSV_to_WATERNETWORK" != jsonObject["Application"].toString()) {
            this->errorMessage("CSVWaterNetworkInputWidget::inputFRommJSON app name conflict");
            return false;
        }
    }


    if (jsonObject.contains("ApplicationData")) {
        QJsonObject appData = jsonObject["ApplicationData"].toObject();


        if(appData.contains("WaterNetworkNodes"))
        {
            auto assetObj = appData["WaterNetworkNodes"].toObject();

            // Input the nodes
            auto res = theNodesWidget->inputAppDataFromJSON(assetObj);

            if(!res)
                return res;
        }
        else
        {
            this->errorMessage("Could not find the 'WaterNetworkNodes' key in 'ApplicationData'");
            return false;
        }

        if(appData.contains("WaterNetworkPipelines"))
        {

            auto assetObj = appData["WaterNetworkPipelines"].toObject();


            // Input the pipes
            auto res = thePipelinesWidget->inputAppDataFromJSON(assetObj);

            if(!res)
                return res;
        }
        else
        {
            this->errorMessage("Could not find the 'WaterNetworkPipelines' key in 'ApplicationData'");
            return false;
        }

    }
    else
    {
        this->errorMessage("Could not find the 'ApplicationData' key in 'CSV_to_WATERNETWORK' input");
        return false;
    }

    return true;
}




int CSVWaterNetworkInputWidget::loadPipelinesVisualization()
{

    if(nodePointsMap.isEmpty())
    {
        this->errorMessage("The node map is empty in WaterNetworkInputWidget");
        return -1;
    }

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

        // Create a unique ID for the pipeline
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

    auto layerId = pipelinesMainLayer->id();

    theVisualizationWidget->registerLayerForSelection(layerId,thePipelinesWidget);

    // Create the selected pipeline layer
    pipelinesSelectedLayer = theVisualizationWidget->addVectorLayer("linestring","Selected Pipelines");

    if(pipelinesSelectedLayer == nullptr)
    {
        this->errorMessage("Error adding the selected assets vector layer");
        return -1;
    }

    QgsLineSymbol* selectedLayerMarkerSymbol = new QgsLineSymbol();

    selectedLayerMarkerSymbol->setWidth(2.0);
    selectedLayerMarkerSymbol->setColor(Qt::darkBlue);
    theVisualizationWidget->createSimpleRenderer(selectedLayerMarkerSymbol,pipelinesSelectedLayer);

    auto pr2 = pipelinesSelectedLayer->dataProvider();

    auto res2 = pr2->addAttributes(attribFields);

    if(!res2)
        this->errorMessage("Error adding attributes to the layer");

    pipelinesSelectedLayer->updateFields(); // tell the vector layer to fetch changes from the provider

    thePipelinesDb->setSelectedLayer(pipelinesSelectedLayer);

    QVector<QgsMapLayer*> mapLayers;
    mapLayers.push_back(pipelinesSelectedLayer);
    mapLayers.push_back(pipelinesMainLayer);

    theVisualizationWidget->createLayerGroup(mapLayers,"Pipelines");

//    QVector<QgsMapLayer*> mapLayers;
//    mapLayers.push_back(nodesMainLayer);
//    mapLayers.push_back(pipelinesMainLayer);

//    theVisualizationWidget->createLayerGroup(mapLayers,"Water Network");

    return 0;
}



int CSVWaterNetworkInputWidget::getNodeMap()
{

    if(theNodesWidget->isEmpty())
    {
        this->errorMessage("Error, the nodes table is empty in WaterNetworkInputWidget");
        return -1;
    }

    auto theNodesTableWidget = theNodesWidget->getTableWidget();

    // Get the number of rows
    auto nRows = theNodesTableWidget->rowCount();

    auto headers = theNodesWidget->getTableHorizontalHeadings();

    // First check if a footprint was provided
    auto indexLatitude = theVisualizationWidget->getIndexOfVal(headers, "latitude");
    auto indexLongitude = theVisualizationWidget->getIndexOfVal(headers, "longitude");

    if(indexLongitude == -1 || indexLatitude == -1)
    {
        this->errorMessage("Could not find latitude and longitude in the header columns");
        return -1;
    }


    for(int i = 0; i<nRows; ++i)
    {

        // Create a new node
        QString nodeIDStr = theNodesTableWidget->item(i,0).toString();

        int nodeID = nodeIDStr.toInt();

        auto latitude = theNodesTableWidget->item(i,indexLatitude).toDouble();
        auto longitude = theNodesTableWidget->item(i,indexLongitude).toDouble();

        QgsPointXY point(longitude,latitude);
        auto geom = QgsGeometry::fromPointXY(point);
        if(geom.isEmpty())
        {
            this->errorMessage("Error creating the water network node geometry");
            return -1;
        }

        nodePointsMap.insert(nodeID,point);
    }

    return 0;
}


void CSVWaterNetworkInputWidget::clear()
{
//    theNodesDb->clear();
    thePipelinesDb->clear();
    nodePointsMap.clear();
    theNodesWidget->clear();
    thePipelinesWidget->clear();
}


void CSVWaterNetworkInputWidget::handleAssetsLoaded()
{
    if(theNodesWidget->isEmpty() || thePipelinesWidget->isEmpty())
        return;

    auto res = this->getNodeMap();

    if(res != 0)
    {
        this->errorMessage("Error, failed to load the water network nodes mapped");
        return;
    }

    res = this->loadPipelinesVisualization();

    if(res != 0)
    {
        this->errorMessage("Error, failed to load the water network pipelines visualization");
        return;
    }

}
