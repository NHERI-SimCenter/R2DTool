#include "JSONTransportNetworkInputWidget.h"
#include "QGISVisualizationWidget.h"
#include "LineAssetInputWidget.h"
#include "PointAssetInputWidget.h"
#include "ComponentDatabaseManager.h"
#include "ComponentTableView.h"
#include "ComponentTableModel.h"

#include "AssetFilterDelegate.h"
//#include "JSONReaderWriter.h"

#include <qgsfeature.h>
#include <qgslinesymbol.h>
#include <qgsmarkersymbol.h>

#include <QHeaderView>
#include <QFileDialog>
#include <QSplitter>
#include <QJsonArray>
#include <QColor>

JSONTransportNetworkInputWidget::JSONTransportNetworkInputWidget(QWidget *parent, VisualizationWidget* visWidget) : AssetInputJSONWidget(parent, visWidget, QString("TransportNetwork"), QString("JSON_to_TRANSPORTNETWORK"))
{
    theVisualizationWidget = static_cast<QGISVisualizationWidget*>(visWidget);
    assert(theVisualizationWidget);

////    theNodesDb = ComponentDatabaseManager::getInstance()->getTransportNetworkNodeComponentDb();
//    theRoadsDb = ComponentDatabaseManager::getInstance()->createAssetDb("Transportation Roadways");

//    //    theNodesWidget = new NonselectableAssetInputWidget(this, theNodesDb, theVisualizationWidget, "Transport Network Nodes");
//    //QWidget *parent, VisualizationWidget* visWidget, QString componentType, QString appType = QString()
//    theNodesWidget = new PointAssetInputWidget(this, theVisualizationWidget, "Transport Network Nodes", "CSV_to_AIM");

//    theNodesWidget->setLabel1("Load Transport network inventory from a JSON file");


//    connect(theNodesWidget,&LineAssetInputWidget::doneLoadingComponents,this,&JSONTransportNetworkInputWidget::handleAssetsLoaded);
//    connect(theLinksWidget,&LineAssetInputWidget::doneLoadingComponents,this,&JSONTransportNetworkInputWidget::handleAssetsLoaded);

//    theLinksWidget->setTheNodesWidget(theNodesWidget);

//    QVBoxLayout* mainLayout = new QVBoxLayout(this);

//    QSplitter* verticalSplitter = new QSplitter(Qt::Vertical);

//    verticalSplitter->addWidget(theNodesWidget);
//    verticalSplitter->addWidget(theLinksWidget);

//    mainLayout->addWidget(verticalSplitter);

}

//int JSONTransportNetworkInputWidget::loadAssetVisualization(){
//    return 0;
//}


JSONTransportNetworkInputWidget::~JSONTransportNetworkInputWidget()
{

}

bool JSONTransportNetworkInputWidget::loadAssetData(){
    // Ask for the file path if the file path has not yet been set, and return if it is still null
    if(pathToComponentInputFile.compare("NULL") == 0)
        this->chooseComponentInfoFileDialog();

    if(pathToComponentInputFile.compare("NULL") == 0)
        return false;

    // Check if the directory exists
    QFileInfo file(pathToComponentInputFile);

    if (!file.exists())
    {
        auto relPathToComponentFile = QCoreApplication::applicationDirPath() + QDir::separator() + pathToComponentInputFile;

        if (!QFile(relPathToComponentFile).exists())
        {
            QString errMsg = "Cannot find the file: "+ pathToComponentInputFile + "\n" +"Check your directory and try again.";
            this->errorMessage(errMsg);
            return false;
        }
        else
        {
            pathToComponentInputFile = relPathToComponentFile;
            componentFileLineEdit->setText(pathToComponentInputFile);
        }
    }
    QFile qfile(pathToComponentInputFile);
    if (!qfile.open(QFile::ReadOnly | QFile::Text)) {
        this->errorMessage(QString("Could Not Open File: ") + pathToComponentInputFile);
        return -1;
    }
    QString val;
    val=qfile.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(val.toUtf8());
    QJsonObject jsonObj = doc.object();
    // close file
    qfile.close();
    QJsonObject nodesDict;
    if(jsonObj.contains("nodes")){
        nodesDict = jsonObj["nodes"].toObject();
    }else{
        this->errorMessage(pathToComponentInputFile+QString(" does not contain a key 'nodes'"));
        return -1;
    }
    // Add the bridges layer
    QgsVectorLayer* bridgeLayer = nullptr;
    if (jsonObj.contains("hwy_bridges") && jsonObj["hwy_bridges"].isArray()){
//        QJsonValue bridgeArray = jsonObj["hwy_bridges"];
        QJsonArray bridgeArray = jsonObj["hwy_bridges"].toArray();
        bridgeLayer = this->loadPointAssetVisualization(bridgeArray, nodesDict, QString("hwy_bridge"), Qt::red);
    }else{
        this->errorMessage(pathToComponentInputFile+QString(" does not contain an array with key 'hwy_bridges'"));
        return -1;
    }
    // Add the tunnel layer
    QgsVectorLayer* tunnelLayer = nullptr;
    if (jsonObj.contains("hwy_tunnels") && jsonObj["hwy_tunnels"].isArray()){
        QJsonArray tunnelArray = jsonObj["hwy_tunnels"].toArray();
        tunnelLayer = this->loadPointAssetVisualization(tunnelArray, nodesDict, QString("hwy_tunnel"), Qt::yellow);
    }else{
        this->errorMessage(pathToComponentInputFile+QString(" does not contain an array with key 'hwy_tunnels'"));
        return -1;
    }
    // Add the roadways layer
    QgsVectorLayer* roadLayer = nullptr;
    if (jsonObj.contains("roadways") && jsonObj["roadways"].isArray()){
        QJsonArray roadArray = jsonObj["roadways"].toArray();
        roadLayer = this->loadLineAssetVisualization(roadArray, nodesDict, QString("roadway"), Qt::blue);
    }else{
        this->errorMessage(pathToComponentInputFile+QString(" does not contain an array with key 'hwy_tunnels'"));
        return -1;
    }
    // Add a group of transportation network
    QVector<QgsMapLayer*> mapLayers;
    mapLayers.push_back(bridgeLayer);
    mapLayers.push_back(tunnelLayer);
    mapLayers.push_back(roadLayer);
    theVisualizationWidget->createLayerGroup(mapLayers,QString("highway transportation network"));
    theVisualizationWidget->zoomToLayer(roadLayer);
    return true;
}

//bool JSONTransportNetworkInputWidget::copyFiles(QString &destName)
//{
////    auto res = theNodesWidget->copyFiles(destName);

////    if(!res)
////        return res;

////    return theLinksWidget->copyFiles(destName);
//}



//bool JSONTransportNetworkInputWidget::outputAppDataToJSON(QJsonObject &jsonObject)
//{
////    jsonObject["Application"]="JSON_to_TransportNETWORK";

////    QJsonObject data;

////    QJsonObject nodeData;
////    // The file containing the network nodes
////    auto res = theNodesWidget->outputAppDataToJSON(nodeData);

////    if(!res)
////    {
////        this->errorMessage("Error, could not get the .json output from the nodes widget in JSON_to_TransportNETWORK");
////        return false;
////    }

////    data["TransportNetworkNodes"] = nodeData;

////    QJsonObject pipelineData;
////    // The file containing the network pipelines
////    res = theLinksWidget->outputAppDataToJSON(pipelineData);

////    if(!res)
////    {
////        this->errorMessage("Error, could not get the .json output from the pipelines widget in JSON_to_TransportNETWORK");
////        return false;
////    }

////    data["TransportNetworkPipelines"] = pipelineData;

////    jsonObject["ApplicationData"] = data;

////    return false;
//}


bool JSONTransportNetworkInputWidget::inputAppDataFromJSON(QJsonObject &jsonObject)
{
    bool res = AssetInputJSONWidget::inputAppDataFromJSON(jsonObject);
    return res;
}




int JSONTransportNetworkInputWidget::loadAssetVisualization()
{

//    if(nodePointsMap.isEmpty())
//    {
//        this->errorMessage("The node map is empty in TransportNetworkInputWidget");
//        return -1;
//    }

//    if(transportNetworkMainLayer != nullptr)
//        theVisualizationWidget->removeLayer(transportNetworkMainLayer);

//    if(transportNetworkSelectedLayer != nullptr)
//        theVisualizationWidget->removeLayer(transportNetworkSelectedLayer);

//    auto pipelinesTableWidget = theLinksWidget->getTableWidget();

//    QgsFields featFields;
//    featFields.append(QgsField("ID", QVariant::Int));
//    featFields.append(QgsField("AssetType", QVariant::String));
//    featFields.append(QgsField("TabName", QVariant::String));

//    // Set the table headers as fields in the table
//    for(int i = 1; i<pipelinesTableWidget->columnCount(); ++i)
//    {
//        auto fieldText = pipelinesTableWidget->horizontalHeaderItemVariant(i);
//        featFields.append(QgsField(fieldText.toString(),fieldText.type()));
//    }

//    auto attribFields = featFields.toList();


//    auto horzHeaders = theLinksWidget->getTableHorizontalHeadings();

//    auto indexNodeTag1 = horzHeaders.indexOf("node1");

//    if(indexNodeTag1 == -1)
//    {
//        this->errorMessage("Error, cannot find column header 'node1' that specifies the starting node of a pipe");
//        return -1;
//    }

//    auto indexNodeTag2 = horzHeaders.indexOf("node2");

//    if(indexNodeTag2 == -1)
//    {
//        this->errorMessage("Error, cannot find column header 'node2' that specifies the ending node of a pipe");
//        return -1;
//    }

//    // Create the pipelines layer
//    transportNetworkMainLayer = theVisualizationWidget->addVectorLayer("linestring","All Transport Network Links");

//    if(transportNetworkMainLayer == nullptr)
//    {
//        this->errorMessage("Error adding a vector layer");
//        return -1;
//    }

//    auto pr = transportNetworkMainLayer->dataProvider();

//    transportNetworkMainLayer->startEditing();

//    auto res = pr->addAttributes(attribFields);

//    if(!res)
//        this->errorMessage("Error adding attributes to the layer" + transportNetworkMainLayer->name());

//    transportNetworkMainLayer->updateFields(); // tell the vector layer to fetch changes from the provider

//    theRoadsDb->setMainLayer(transportNetworkMainLayer);

//    // Get the number of rows
//    auto nRows = pipelinesTableWidget->rowCount();

//    auto numAtrb = attribFields.size();

//    for(int i = 0; i<nRows; ++i)
//    {

//        // create the feature attributes
//        QgsAttributes featureAttributes(numAtrb);

//        // Create a new pipeline
//        QString pipelineIDStr = pipelinesTableWidget->item(i,0).toString();

//        int pipelineID =  pipelineIDStr.toInt();

//        // Create a unique ID for the pipeline
//        //        auto uid = theVisualizationWidget->createUniqueID();

//        // "ID"
//        // "AssetType"
//        // "Tabname"

//        featureAttributes[0] = QVariant(pipelineID);
//        featureAttributes[1] = QVariant("TransportPIPELINES");
//        featureAttributes[2] = QVariant("ID: "+QString::number(pipelineID));

//        // The feature attributes are the columns from the table
//        for(int j = 1; j<pipelinesTableWidget->columnCount(); ++j)
//        {
//            auto attrbVal = pipelinesTableWidget->item(i,j);
//            featureAttributes[2+j] = attrbVal;
//        }

//        QgsFeature feature;
//        feature.setFields(featFields);

//        auto nodeTag1 = pipelinesTableWidget->item(i,indexNodeTag1).toInt();

//        auto nodeTag2 = pipelinesTableWidget->item(i,indexNodeTag2).toInt();

//        // Start and end point of the pipe
//        QgsPointXY point1 = nodePointsMap.value(nodeTag1);

//        if(point1.isEmpty())
//        {
//            this->errorMessage("Error, could not find node with ID "+ QString::number(nodeTag1)+ " in the node table");
//            return -1;
//        }

//        QgsPointXY point2 = nodePointsMap.value(nodeTag2);

//        if(point2.isEmpty())
//        {
//            this->errorMessage("Error, could not find node with ID "+ QString::number(nodeTag2)+ " in the node table");
//            return -1;
//        }

//        QgsPolylineXY pipeSegment(2);
//        pipeSegment[0]=point1;
//        pipeSegment[1]=point2;

//        feature.setGeometry(QgsGeometry::fromPolylineXY(pipeSegment));

//        feature.setAttributes(featureAttributes);

//        if(!feature.isValid())
//            return -1;

//        auto res = pr->addFeature(feature, QgsFeatureSink::FastInsert);
//        if(!res)
//        {
//            this->errorMessage("Error adding the feature to the layer");
//            return -1;
//        }
//    }

//    transportNetworkMainLayer->commitChanges(true);
//    transportNetworkMainLayer->updateExtents();

//    QgsLineSymbol* markerSymbol = new QgsLineSymbol();

//    markerSymbol->setWidth(0.8);
//    markerSymbol->setColor(Qt::darkBlue);
//    theVisualizationWidget->createSimpleRenderer(markerSymbol,transportNetworkMainLayer);

//    //    auto numFeat = mainLayer->featureCount();

//    theVisualizationWidget->zoomToLayer(transportNetworkMainLayer);

//    auto layerId = transportNetworkMainLayer->id();

//    theVisualizationWidget->registerLayerForSelection(layerId,theLinksWidget);

//    // Create the selected pipeline layer
//    transportNetworkSelectedLayer = theVisualizationWidget->addVectorLayer("linestring","Selected Transport Network Links");

//    if(transportNetworkSelectedLayer == nullptr)
//    {
//        this->errorMessage("Error adding the selected assets vector layer");
//        return -1;
//    }

//    QgsLineSymbol* selectedLayerMarkerSymbol = new QgsLineSymbol();

//    selectedLayerMarkerSymbol->setWidth(2.0);
//    selectedLayerMarkerSymbol->setColor(Qt::darkBlue);
//    theVisualizationWidget->createSimpleRenderer(selectedLayerMarkerSymbol,transportNetworkSelectedLayer);

//    auto pr2 = transportNetworkSelectedLayer->dataProvider();

//    auto res2 = pr2->addAttributes(attribFields);

//    if(!res2)
//        this->errorMessage("Error adding attributes to the layer");

//    transportNetworkSelectedLayer->updateFields(); // tell the vector layer to fetch changes from the provider

//    theRoadsDb->setSelectedLayer(transportNetworkSelectedLayer);

//    QVector<QgsMapLayer*> mapLayers;
//    mapLayers.push_back(transportNetworkSelectedLayer);
//    mapLayers.push_back(transportNetworkMainLayer);

//    theVisualizationWidget->createLayerGroup(mapLayers,"Transport Network Links");

//    return 0;
}



int JSONTransportNetworkInputWidget::getNodeMap()
{

//    if(theNodesWidget->isEmpty())
//    {
//        this->errorMessage("Error, the nodes table is empty in TransportNetworkInputWidget");
//        return -1;
//    }

//    auto theNodesTableWidget = theNodesWidget->getTableWidget();

//    // Get the number of rows
//    auto nRows = theNodesTableWidget->rowCount();

//    auto headers = theNodesWidget->getTableHorizontalHeadings();

//    // First check if a footprint was provided
//    auto indexLatitude = theVisualizationWidget->getIndexOfVal(headers, "latitude");
//    auto indexLongitude = theVisualizationWidget->getIndexOfVal(headers, "longitude");



//    if(indexLongitude == -1 || indexLatitude == -1)
//    {
//        indexLatitude = theVisualizationWidget->getIndexOfVal(headers, "lat");
//        indexLongitude = theVisualizationWidget->getIndexOfVal(headers, "lon");
//        if(indexLongitude == -1 || indexLatitude == -1){
//            this->errorMessage("Could not find latitude and longitude in the header columns");
//            return -1;
//        }
//    }


//    for(int i = 0; i<nRows; ++i)
//    {

//        // Create a new node
//        QString nodeIDStr = theNodesTableWidget->item(i,0).toString();

//        int nodeID = nodeIDStr.toInt();

//        auto latitude = theNodesTableWidget->item(i,indexLatitude).toDouble();
//        auto longitude = theNodesTableWidget->item(i,indexLongitude).toDouble();

//        QgsPointXY point(longitude,latitude);
//        auto geom = QgsGeometry::fromPointXY(point);
//        if(geom.isEmpty())
//        {
//            this->errorMessage("Error creating the Transport network node geometry");
//            return -1;
//        }

//        nodePointsMap.insert(nodeID,point);
//    }

//    return 0;
}


void JSONTransportNetworkInputWidget::clear()
{
//    theRoadsDb->clear();
//    nodePointsMap.clear();
//    theNodesWidget->clear();
//    theLinksWidget->clear();

//    transportNetworkMainLayer = nullptr;
//    transportNetworkSelectedLayer = nullptr;
}


void JSONTransportNetworkInputWidget::handleAssetsLoaded()
{
//    if(theNodesWidget->isEmpty() || theLinksWidget->isEmpty())
//        return;

//    auto res = this->getNodeMap();

//    if(res != 0)
//    {
//        this->errorMessage("Error, failed to load the Transport network nodes mapped");
//        return;
//    }

//    res = this->loadPipelinesVisualization();

//    if(res != 0)
//    {
//        this->errorMessage("Error, failed to load the Transport network links visualization");
//        return;
//    }

}
