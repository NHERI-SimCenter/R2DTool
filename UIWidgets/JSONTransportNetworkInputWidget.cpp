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

    roadsfilterWidget = new QWidget();
    QHBoxLayout* roadsSelectComponentsLayout = new QHBoxLayout(roadsfilterWidget);
    QLabel* roadsFilterText = new QLabel();
    roadsFilterText->setText(QString("Enter the indices of one or more roadways to analyze.")
                             +QString("\nDefine a range of roadways with a dash and separate multiple roadways with a comma."));
    roadsSelectComponentsLayout->addWidget(roadsFilterText);
    roadsSelectComponentsLineEdit = new AssetInputDelegate();
//    roadsSelectComponentsLineEdit->setMaximumWidth(1000);
//    roadsSelectComponentsLineEdit->setMinimumWidth(400);
//    roadsSelectComponentsLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    connect(roadsSelectComponentsLineEdit,&AssetInputDelegate::componentSelectionComplete,this,&JSONTransportNetworkInputWidget::handleRoadsSelection);
    connect(roadsSelectComponentsLineEdit,&QLineEdit::editingFinished,this,&JSONTransportNetworkInputWidget::selectRoads);
    QPushButton *roadsClearSelectionButton = new QPushButton();
    roadsClearSelectionButton->setText(tr("Clear Selection"));
    roadsClearSelectionButton->setMaximumWidth(150);
    roadsSelectComponentsLayout->addWidget(roadsFilterText);
    roadsSelectComponentsLayout->addWidget(roadsSelectComponentsLineEdit);
    roadsSelectComponentsLayout->addWidget(roadsClearSelectionButton);
    connect(roadsClearSelectionButton,SIGNAL(clicked()),this,SLOT(clearRoadsComponentSelection()));
//    roadsfilterWidget->setMaximumWidth(1000);
//    roadsfilterWidget->setMinimumWidth(400);
//    roadsfilterWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    mainWidgetLayout->addWidget(roadsfilterWidget);

    roadLengthWidget = new QWidget();
    QHBoxLayout* roadLengthLayout = new QHBoxLayout(roadLengthWidget);
    QLabel* roadLengthLabel = new QLabel("Maximum roadway length (m) per AIM",this);
    roadLengthLineEdit = new QLineEdit(this);
    roadLengthLineEdit->setText("100.0");
    QDoubleValidator *validator = new QDoubleValidator(this);
    validator->setBottom(0.0);
    roadLengthLayout->addWidget(roadLengthLabel);
    roadLengthLayout->addWidget(roadLengthLineEdit);
//    roadLengthWidget->setMaximumWidth(200);
//    roadLengthWidget->setMinimumWidth(100);
//    roadLengthWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    mainWidgetLayout->addWidget(roadLengthWidget);
    connect(roadLengthLineEdit, &QLineEdit::editingFinished, this, &JSONTransportNetworkInputWidget::printRoadLengthInput);

    bridgesfilterWidget = new QWidget();
    QHBoxLayout* bridgesSelectComponentsLayout = new QHBoxLayout(bridgesfilterWidget);
    QLabel* bridgesFilterText = new QLabel();
    bridgesFilterText->setText(QString("Enter the indices of one or more bridges to analyze.")
                             +QString("\nDefine a range of bridges with a dash and separate multiple bridges with a comma."));
    bridgesSelectComponentsLayout->addWidget(bridgesFilterText);
    bridgesSelectComponentsLineEdit = new AssetInputDelegate();
    connect(bridgesSelectComponentsLineEdit,&AssetInputDelegate::componentSelectionComplete,this,&JSONTransportNetworkInputWidget::handleBridgesSelection);
    connect(bridgesSelectComponentsLineEdit,&QLineEdit::editingFinished,this,&JSONTransportNetworkInputWidget::selectBridges);
    QPushButton *bridgesClearSelectionButton = new QPushButton();
    bridgesClearSelectionButton->setText(tr("Clear Selection"));
    bridgesClearSelectionButton->setMaximumWidth(150);
    bridgesSelectComponentsLayout->addWidget(bridgesFilterText);
    bridgesSelectComponentsLayout->addWidget(bridgesSelectComponentsLineEdit);
    bridgesSelectComponentsLayout->addWidget(bridgesClearSelectionButton);
    connect(bridgesClearSelectionButton,SIGNAL(clicked()),this,SLOT(clearBridgesComponentSelection()));
//    bridgesfilterWidget->setMaximumWidth(1000);
//    bridgesfilterWidget->setMinimumWidth(400);
//    bridgesfilterWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    mainWidgetLayout->addWidget(bridgesfilterWidget);

    tunnelsfilterWidget = new QWidget();
    QHBoxLayout* tunnelsSelectComponentsLayout = new QHBoxLayout(tunnelsfilterWidget);
    QLabel* tunnelsFilterText = new QLabel();
    tunnelsFilterText->setText(QString("Enter the indices of one or more tunnels to analyze.")
                               +QString("\nDefine a range of tunnels with a dash and separate multiple tunnels with a comma."));
    tunnelsSelectComponentsLayout->addWidget(tunnelsFilterText);
    tunnelsSelectComponentsLineEdit = new AssetInputDelegate();
    connect(tunnelsSelectComponentsLineEdit,&AssetInputDelegate::componentSelectionComplete,this,&JSONTransportNetworkInputWidget::handleTunnelsSelection);
    connect(tunnelsSelectComponentsLineEdit,&QLineEdit::editingFinished,this,&JSONTransportNetworkInputWidget::selectTunnels);
    QPushButton *tunnelsClearSelectionButton = new QPushButton();
    tunnelsClearSelectionButton->setText(tr("Clear Selection"));
    tunnelsClearSelectionButton->setMaximumWidth(150);
    tunnelsSelectComponentsLayout->addWidget(tunnelsFilterText);
    tunnelsSelectComponentsLayout->addWidget(tunnelsSelectComponentsLineEdit);
    tunnelsSelectComponentsLayout->addWidget(tunnelsClearSelectionButton);
    connect(tunnelsClearSelectionButton,SIGNAL(clicked()),this,SLOT(clearTunnelsComponentSelection()));
//    tunnelsfilterWidget->setMaximumWidth(1000);
//    tunnelsfilterWidget->setMinimumWidth(400);
//    tunnelsfilterWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    mainWidgetLayout->addWidget(tunnelsfilterWidget);

    mainWidgetLayout->addStretch();
}


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




bool JSONTransportNetworkInputWidget::outputAppDataToJSON(QJsonObject &jsonObject)
{
    jsonObject["Application"]=appType;

    QJsonObject data;
    QFileInfo componentFile(componentFileLineEdit->text());
    if (componentFile.exists()) {
        // if componentFile is a dir, then it is likely a folder containing shapefile and its dependencies
        if (componentFile.isDir())
        {
            // look for shp, gdb, gpkg
            QDir gisDir = componentFile.absoluteFilePath();
            QStringList acceptableFileExtensions = {"*.shp", "*.gdb", "*.gpkg"};
            QStringList inputFiles = gisDir.entryList(acceptableFileExtensions, QDir::Files);
            if(inputFiles.empty())
            {
                this->errorMessage("Cannot find GIS file in input site data directory.");
                return 0;
            }
            data["assetSourceFile"]=componentFile.absoluteFilePath() + QDir::separator() + inputFiles.value(0);
            data["pathToSource"]=componentFile.absoluteFilePath();
            data["inputIsGIS"]=true;
        }
        else
        {
            data["assetSourceFile"]=componentFile.fileName();
            data["pathToSource"]=componentFile.absoluteDir().path();
            data["inputIsGIS"]=false;
        }

        QString roadsFilterData = this->getRoadsFilterString();
        QString bridgesFilterData = this->getBridgesFilterString();
        QString tunnelsFilterData = this->getTunnelsFilterString();

        if(roadsFilterData.isEmpty())
        {
            auto msgBox =  std::make_unique<QMessageBox>();
            msgBox->setText("No IDs are selected for analysis in ASD roadways of "+assetType.toLower()+". Really run with all components?");
            msgBox->setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
            auto res = msgBox->exec();
            if(res != QMessageBox::Yes)
                return false;
        } else {
            data["roadsFilter"] = roadsFilterData;
        }
        if(bridgesFilterData.isEmpty())
        {
            auto msgBox =  std::make_unique<QMessageBox>();
            msgBox->setText("No IDs are selected for analysis in ASD bridges of "+assetType.toLower()+". Really run with all components?");
            msgBox->setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
            auto res = msgBox->exec();
            if(res != QMessageBox::Yes)
                return false;
        } else {
            data["bridgesFilter"] = bridgesFilterData;
        }
        if(tunnelsFilterData.isEmpty())
        {
            auto msgBox =  std::make_unique<QMessageBox>();
            msgBox->setText("No IDs are selected for analysis in ASD tunnels of "+assetType.toLower()+". Really run with all components?");
            msgBox->setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
            auto res = msgBox->exec();
            if(res != QMessageBox::Yes)
                return false;
        } else {
            data["tunnelsFilter"] = tunnelsFilterData;
        }

        data["roadSegLength"] = roadLengthLineEdit->text().toDouble();

    }
    else
    {
        data["sourceFile"]=QString("None");
        data["pathToSource"]=QString("");
        return false;
    }

    jsonObject["ApplicationData"] = data;

    return true;
}


bool JSONTransportNetworkInputWidget::inputAppDataFromJSON(QJsonObject &jsonObject)
{
    bool res = AssetInputJSONWidget::inputAppDataFromJSON(jsonObject);
    if (jsonObject.contains("ApplicationData")) {
        QJsonObject appData = jsonObject["ApplicationData"].toObject();
        if (appData.contains("roadsFilter")) {
            roadsSelectComponentsLineEdit->setText(appData["roadsFilter"].toString());
            this->selectRoads();
        }
        if (appData.contains("bridgesFilter")) {
            bridgesSelectComponentsLineEdit->setText(appData["bridgesFilter"].toString());
            this->selectBridges();
        }
        if (appData.contains("tunnelsFilter")) {
            tunnelsSelectComponentsLineEdit->setText(appData["tunnelsFilter"].toString());
            this->selectTunnels();
        }
        if (appData.contains("roadSegLength")) {
            roadLengthLineEdit->setText(QString::number(appData["roadSegLength"].toDouble()));
        }
    }
    return res;
}

void JSONTransportNetworkInputWidget::printRoadLengthInput(void){
    QString msg = "Roadway length per AIM is set as "+ roadLengthLineEdit->text() + " meters";
    this->statusMessage(msg);
}


int JSONTransportNetworkInputWidget::loadAssetVisualization()
{

}



int JSONTransportNetworkInputWidget::getNodeMap()
{

}


void JSONTransportNetworkInputWidget::clear()
{
    this->clearLoadedData();
    roadsSelectComponentsLineEdit->clear();
    bridgesSelectComponentsLineEdit->clear();
    tunnelsSelectComponentsLineEdit->clear();
    roadLengthLineEdit->clear();
//    theRoadsDb->clear();
//    nodePointsMap.clear();
//    theNodesWidget->clear();
//    theLinksWidget->clear();

//    transportNetworkMainLayer = nullptr;
//    transportNetworkSelectedLayer = nullptr;
}


void JSONTransportNetworkInputWidget::handleAssetsLoaded()
{

}


void JSONTransportNetworkInputWidget::selectRoads(void)
{
    try
    {
        roadsSelectComponentsLineEdit->selectComponents();
    }
    catch (const QString msg)
    {
        this->errorMessage(msg);
    }
}

void JSONTransportNetworkInputWidget::selectBridges(void)
{
    try
    {
        bridgesSelectComponentsLineEdit->selectComponents();
    }
    catch (const QString msg)
    {
        this->errorMessage(msg);
    }
}

void JSONTransportNetworkInputWidget::selectTunnels(void)
{
    try
    {
        tunnelsSelectComponentsLineEdit->selectComponents();
    }
    catch (const QString msg)
    {
        this->errorMessage(msg);
    }
}

void::JSONTransportNetworkInputWidget::handleRoadsSelection(){
    auto selectedComponentIDs = roadsSelectComponentsLineEdit->getSelectedComponentIDs();
    auto numAssets = selectedComponentIDs.size();
    QString msg = "A total of "+ QString::number(numAssets) + " roadways are selected for analysis";
    this->statusMessage(msg);
    //Do nothing. Can be used to check if roads filter input is valid
}

void::JSONTransportNetworkInputWidget::handleBridgesSelection(){
    auto selectedComponentIDs = bridgesSelectComponentsLineEdit->getSelectedComponentIDs();
    auto numAssets = selectedComponentIDs.size();
    QString msg = "A total of "+ QString::number(numAssets) + " bridges are selected for analysis";
    this->statusMessage(msg);
    //Do nothing. Can be used to check if roads filter input is valid
}

void::JSONTransportNetworkInputWidget::handleTunnelsSelection(){
    auto selectedComponentIDs = tunnelsSelectComponentsLineEdit->getSelectedComponentIDs();
    auto numAssets = selectedComponentIDs.size();
    QString msg = "A total of "+ QString::number(numAssets) + " tunnels are selected for analysis";
    this->statusMessage(msg);
    //Do nothing. Can be used to check if roads filter input is valid
}

void JSONTransportNetworkInputWidget::clearRoadsComponentSelection()
{
    roadsSelectComponentsLineEdit->clear();
    //    theComponentDb->clearSelectedLayer();
    //    theComponentDb->getSelectedLayer()->updateExtents();
}

void JSONTransportNetworkInputWidget::clearBridgesComponentSelection()
{
    bridgesSelectComponentsLineEdit->clear();
    //    theComponentDb->clearSelectedLayer();
    //    theComponentDb->getSelectedLayer()->updateExtents();
}

void JSONTransportNetworkInputWidget::clearTunnelsComponentSelection()
{
    tunnelsSelectComponentsLineEdit->clear();
    //    theComponentDb->clearSelectedLayer();
    //    theComponentDb->getSelectedLayer()->updateExtents();
}


QString JSONTransportNetworkInputWidget::getRoadsFilterString(void){
    QString filterData = roadsSelectComponentsLineEdit->getComponentAnalysisList();
    return filterData;
}
QString JSONTransportNetworkInputWidget::getBridgesFilterString(void){
    QString filterData = bridgesSelectComponentsLineEdit->getComponentAnalysisList();
    return filterData;
}
QString JSONTransportNetworkInputWidget::getTunnelsFilterString(void){
    QString filterData = tunnelsSelectComponentsLineEdit->getComponentAnalysisList();
    return filterData;
}
