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

// Written by: Jinyan Zhao

#include "AssetInputDelegate.h"
#include "AssetFilterDelegate.h"
#include "AssetInputJSONWidget.h"
#include "VisualizationWidget.h"
#include "CSVReaderWriter.h"
#include "ComponentTableView.h"
#include "ComponentTableModel.h"
#include "ComponentDatabaseManager.h"

// Test to remove
//#include <chrono>
//using namespace std::chrono;

#include <QMessageBox>
#include <QCoreApplication>
#include <QMessageBox>
#include <QApplication>
#include <QFileDialog>
#include <QLineEdit>
#include <QLabel>
#include <QGroupBox>
#include <QGridLayout>
#include <QPushButton>
#include <QFileInfo>
#include <QJsonObject>
#include <QHeaderView>

#include "QGISVisualizationWidget.h"
#include <qgsmarkersymbol.h>
#include <qgslinesymbol.h>


// Std library headers
#include <string>
#include <algorithm>

AssetInputJSONWidget::AssetInputJSONWidget(QWidget *parent, VisualizationWidget* visWidget, QString assetType, QString appType) : SimCenterAppWidget(parent), appType(appType), assetType(assetType)
{

    offset = 0;

    theVisualizationWidget = static_cast<QGISVisualizationWidget*>(visWidget);
    assert(theVisualizationWidget);

    this->setContentsMargins(0,0,0,0);

    pathToComponentInputFile = "NULL";

    AssetInputJSONWidget::createComponentsBox();

    auto txt1 = "Load information from a JSON file";
    label1->setText(txt1);
//    label2->setText(txt2);
//    label3->setText(txt3);

    theComponentDb = ComponentDatabaseManager::getInstance()->createAssetDb(assetType);

    if(theComponentDb == nullptr)
        this->errorMessage("Could not find the component database of the type "+assetType);
}


AssetInputJSONWidget::~AssetInputJSONWidget()
{
    
}


void AssetInputJSONWidget::chooseComponentInfoFileDialog(void)
{
    auto newPathToComponentInputFile = QFileDialog::getOpenFileName(this,tr("Component Information File"));
    
    // Return if the user cancels
    if(newPathToComponentInputFile.isEmpty())
    {
        pathToComponentInputFile = "NULL";
        return;
    }

    this->clearLoadedData();

    pathToComponentInputFile = newPathToComponentInputFile;

    // Set file name & entry in qLine edit
    componentFileLineEdit->setText(pathToComponentInputFile);
    
    this->loadAssetData();
    
    return;
}




ComponentTableView *AssetInputJSONWidget::getTableWidget() const
{
    return componentTableWidget;
}


void AssetInputJSONWidget::createComponentsBox(void)
{
    componentGroupBox = new QGroupBox(assetType);
    componentGroupBox->setFlat(true);
    componentGroupBox->setContentsMargins(0,0,0,0);
    
    mainWidgetLayout = new QVBoxLayout();
    mainWidgetLayout->setMargin(0);
    mainWidgetLayout->setSpacing(5);
    mainWidgetLayout->setContentsMargins(10,0,0,0);
    
    componentGroupBox->setLayout(mainWidgetLayout);
    
    label1 = new QLabel();
    
    QLabel* pathText = new QLabel();
    pathText->setText("Path to file:");
    
    componentFileLineEdit = new QLineEdit();
//        componentFileLineEdit->setMaximumWidth(750);
    componentFileLineEdit->setMinimumWidth(400);
    componentFileLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);


    browseFileButton = new QPushButton();
    browseFileButton->setText(tr("Browse"));
    browseFileButton->setMaximumWidth(150);
    
    connect(browseFileButton,SIGNAL(clicked()),this,SLOT(chooseComponentInfoFileDialog()));
    
    // Add a horizontal spacer after the browse and load buttons
    //    auto hspacer = new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Minimum);
    
    
    QHBoxLayout* pathLayout = new QHBoxLayout();
    pathLayout->addWidget(pathText);
    pathLayout->addWidget(componentFileLineEdit);
    pathLayout->addWidget(browseFileButton);
    
//    // Add a vertical spacer at the bottom to push everything up
    mainWidgetLayout->addWidget(label1);
    mainWidgetLayout->addLayout(pathLayout);

    mainWidgetLayout->addStretch();

    this->setLayout(mainWidgetLayout);
}


QgsVectorLayer *AssetInputJSONWidget::getMainLayer() const
{
    return mainLayer;
}


void AssetInputJSONWidget::selectComponents(void)
{
    try
    {
        selectComponentsLineEdit->selectComponents();
    }
    catch (const QString msg)
    {
        this->errorMessage(msg);
    }
}


void AssetInputJSONWidget::handleComponentSelection(void)
{

    auto nRows = componentTableWidget->rowCount();

    if(nRows == 0)
        return;

    // Get the ID of the first and last component
    bool OK;
    auto firstID = componentTableWidget->item(0,0).toInt(&OK);

    if(!OK)
    {
        QString msg = "Error in getting the component ID in " + QString(__FUNCTION__);
        this->errorMessage(msg);
        return;
    }

    auto lastID = componentTableWidget->item(nRows-1,0).toInt(&OK);

    if(!OK)
    {
        QString msg = "Error in getting the component ID in " + QString(__FUNCTION__);
        this->errorMessage(msg);
        return;
    }

    auto selectedComponentIDs = selectComponentsLineEdit->getSelectedComponentIDs();

    // First check that all of the selected IDs are within range
    for(auto&& it : selectedComponentIDs)
    {
        if(it<firstID || it>lastID)
        {
            QString msg = "The component ID " + QString::number(it) + " is out of range of the components provided";
            this->errorMessage(msg);
            selectComponentsLineEdit->clear();
            return;
        }
    }

    theComponentDb->startEditing();

    // Test to remove
    //    auto start = high_resolution_clock::now();

    auto res = theComponentDb->addFeaturesToSelectedLayer(selectedComponentIDs);
    if(res == false)
    {
        this->errorMessage("Error adding features to selected layer");
        return;
    }

    auto numAssets = selectedComponentIDs.size();
    QString msg = "A total of "+ QString::number(numAssets) + " " + assetType.toLower() + " are selected for analysis";
    this->statusMessage(msg);


    // Test to remove
    //    auto stop = high_resolution_clock::now();
    //    auto duration = duration_cast<milliseconds>(stop - start);
    //    this->statusMessage("Done ALL "+QString::number(duration.count()));

    theComponentDb->commitChanges();

    // Hide all of the rows that are not selecetd. Takes a long time!
    //    // Hide all rows in the table
    //    for(int i = 0; i<nRows; ++i)
    //        componentTableWidget->setRowHidden(i,true);

    //    // Unhide the selected rows
    //    for(auto&& it : selectedComponentIDs)
    //        componentTableWidget->setRowHidden(it - firstID,false);


}



QStringList AssetInputJSONWidget::getTableHorizontalHeadings()
{
    return tableHorizontalHeadings;
}


void AssetInputJSONWidget::clearComponentSelection(void)
{
    //    auto nRows = componentTableWidget->rowCount();

    // Hide all rows in the table
    //    for(int i = 0; i<nRows; ++i)
    //    {
    //        componentTableWidget->setRowHidden(i,false);
    //    }

    selectComponentsLineEdit->clear();

    theComponentDb->clearSelectedLayer();

    theComponentDb->getSelectedLayer()->updateExtents();
}


void AssetInputJSONWidget::setLabel1(const QString &value)
{
    label1->setText(value);
}


void AssetInputJSONWidget::setLabel2(const QString &value)
{
    label2->setText(value);
}


void AssetInputJSONWidget::setLabel3(const QString &value)
{
    label3->setText(value);
}


void AssetInputJSONWidget::setGroupBoxText(const QString &value)
{
    componentGroupBox->setTitle(value);
}


void AssetInputJSONWidget::setComponentType(const QString &value)
{
    assetType = value;
}


int AssetInputJSONWidget::numberComponentsSelected(void)
{
    return selectComponentsLineEdit->size();
}


QString AssetInputJSONWidget::getPathToComponentFile(void) const
{
    return pathToComponentInputFile;
}


void AssetInputJSONWidget::setPathToComponentFile(const QString& path)
{
    pathToComponentInputFile = path;

    componentFileLineEdit->setText(pathToComponentInputFile);
}


bool AssetInputJSONWidget::outputAppDataToJSON(QJsonObject &jsonObject)
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

        QString filterData = this->getFilterString();
#ifdef OpenSRA
        filterData = "1";
#endif

        if(filterData.isEmpty())
        {

            auto msgBox =  std::make_unique<QMessageBox>();

            msgBox->setText("No IDs are selected for analysis in ASD "+assetType.toLower()+". Really run with all components?");
            msgBox->setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);

            auto res = msgBox->exec();

            if(res != QMessageBox::Yes)
                return false;

            this->selectAllComponents();

            statusMessage("Selecting all components for analysis");
            filterData = this->getFilterString();
//This need to be restored after selection is implemented
//            if(filterData.isEmpty())
//            {
//                errorMessage("Error selecting components for analysis");
//                return false;
//            }
        }

        data["filter"] = filterData;
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


bool AssetInputJSONWidget::inputAppDataFromJSON(QJsonObject &jsonObject)
{

    //jsonObject["Application"]=appType;
    if (jsonObject.contains("Application")) {
        if (appType != jsonObject["Application"].toString()) {
            this->errorMessage("AssetInputJSONWidget::inputFRommJSON app name conflict");
            return false;
        }
    }


    if (jsonObject.contains("ApplicationData")) {
        QJsonObject appData = jsonObject["ApplicationData"].toObject();

        QFileInfo fileInfo;
        QString fileName;
        QString pathToFile;
        bool foundFile = false;

        if (appData.contains("assetSourceFile"))
            fileName = appData["assetSourceFile"].toString();
        else
        {
            this->errorMessage("The input file " + fileName+ " is missing the 'assetSourceFile' field");
            return false;
        }

        if (fileInfo.exists(fileName)) {

            pathToComponentInputFile = fileName;
            componentFileLineEdit->setText(fileName);

            this->loadAssetData();
            foundFile = true;

        } else {

            if (appData.contains("pathToSource"))
                pathToFile = appData["pathToSource"].toString();
            else
                pathToFile=QDir::currentPath();

            pathToComponentInputFile = pathToFile + QDir::separator() + fileName;

            if (fileInfo.exists(pathToComponentInputFile)) {
                componentFileLineEdit->setText(pathToComponentInputFile);
                foundFile = true;
                this->loadAssetData();

            } else {
                // adam .. adam .. adam
                pathToComponentInputFile = pathToFile + QDir::separator()
                        + "input_data" + QDir::separator() + fileName;
                if (fileInfo.exists(pathToComponentInputFile)) {
                    componentFileLineEdit->setText(pathToComponentInputFile);
                    foundFile = true;
                    this->loadAssetData();
                }
                else
                {
                    QString errMessage = appType + " no file found at: " + fileName;
                    this->errorMessage(errMessage);
                    return false;
                }
            }
        }

        if(foundFile == false)
        {
            QString errMessage = appType + " no file found: " + fileName;
            this->errorMessage(errMessage);
            return false;
        }

        if (appData.contains("filter"))
            this->setFilterString(appData["filter"].toString());

    }
    else
    {
        this->errorMessage("Could not find app data in Component input widget");
        return false;
    }

    return true;
}

QgsVectorLayer* AssetInputJSONWidget::loadPointAssetVisualization(QJsonArray &assetArray, QJsonObject &nodesDict, QString assetType, QColor color){
    //Create the building attributes that are fixed
    QgsFields featFields;
    featFields.append(QgsField("OID", QVariant::Int));
    featFields.append(QgsField("AssetType", QVariant::String));
    QJsonObject asset0 = assetArray[0].toObject();
    foreach(const QString& key, asset0.keys()){
        featFields.append(QgsField(key, asset0[key].toVariant().type()));
    }
    QList<QgsField> attribFields;
    for(int i = 0; i<featFields.size(); ++i)
        attribFields.push_back(featFields[i]);

    // First check if a footprint was provided
    auto indexLocation = asset0.keys().indexOf("location");
    if(indexLocation == -1){
        this->errorMessage("Could not find 'location' in point assest");
        return nullptr;
    }
    // Get the number of rows
    auto nRows = assetArray.size();
    QString layerType = "point";
    mainLayer  = theVisualizationWidget->addVectorLayer(layerType, assetType+QString("s"));
    if(mainLayer == nullptr)
    {
        this->errorMessage("Error adding a vector layer");
        return nullptr;
    }
    auto pr = mainLayer->dataProvider();
    mainLayer->startEditing();
    auto res = pr->addAttributes(attribFields);
    if(!res)
        this->errorMessage("Error adding attributes to the layer" + mainLayer->name());
    mainLayer->updateFields(); // tell the vector layer to fetch changes from the provider
    theComponentDb->setMainLayer(mainLayer);
    auto numAtrb = attribFields.size();
    for(int i = 0; i<nRows; ++i)
    {
        // create the feature attributes
        QgsAttributes featureAttributes(numAtrb);
        featureAttributes[0] = QVariant(i);
        featureAttributes[1] = QVariant(assetType);
        // The feature attributes are the columns from the table
        for(int j = 0; j<assetArray[i].toObject().keys().size(); ++j)
        {
            auto key = assetArray[i].toObject().keys()[j];
            auto attrbVal = assetArray[i].toObject()[key].toVariant();
            featureAttributes[2+j] = attrbVal;
        }
        QgsFeature feature;
        feature.setFields(featFields);
        QString nodeID = QString::number(assetArray[i].toObject()["location"].toInt(), 10);
        double longitude; double latitude;
        if (nodesDict.contains(nodeID)){
            longitude = nodesDict[nodeID].toObject()["lon"].toDouble();
            latitude = nodesDict[nodeID].toObject()["lat"].toDouble();
        }else{
            this->errorMessage(QString("node ") + nodeID + QString(" is missing in inventory file"));
            return nullptr;
        }
        QgsPointXY(longitude,latitude);
        auto geom = QgsGeometry::fromPointXY(QgsPointXY(longitude,latitude));
        if(geom.isEmpty())
        {
            this->errorMessage("Error getting the point asset geometry");
            return nullptr;
        }
        feature.setGeometry(geom);
        feature.setAttributes(featureAttributes);
        if(!feature.isValid())
            return nullptr;
        auto res = pr->addFeature(feature, QgsFeatureSink::FastInsert);
        if(!res)
        {
            this->errorMessage("Error adding the feature to the layer");
            return nullptr;
        }
        mainLayer->commitChanges(true);
        mainLayer->updateExtents();
        QgsMarkerSymbol* markerSymbol = new QgsMarkerSymbol();
        markerSymbol->setColor(color);
        markerSymbol->setSize(4.0);
        theVisualizationWidget->createSimpleRenderer(markerSymbol,mainLayer);
        theVisualizationWidget->zoomToLayer(mainLayer);


    }
    return mainLayer;
}

QgsVectorLayer* AssetInputJSONWidget::loadLineAssetVisualization(QJsonArray &assetArray, QJsonObject &nodesDict, QString assetType, QColor color){
    //Create the building attributes that are fixed
    QgsFields featFields;
    featFields.append(QgsField("OID", QVariant::Int));
    featFields.append(QgsField("AssetType", QVariant::String));
    QJsonObject asset0 = assetArray[0].toObject();
    foreach(const QString& key, asset0.keys()){
        featFields.append(QgsField(key, asset0[key].toVariant().type()));
    }
    QList<QgsField> attribFields;
    for(int i = 0; i<featFields.size(); ++i)
        attribFields.push_back(featFields[i]);

    // First check if a footprint was provided
    auto indexStartNode = asset0.keys().indexOf("start_node");
    if(indexStartNode == -1){
        this->errorMessage("Could not find 'start_node' in line assest");
        return nullptr;
    }
    // First check if a footprint was provided
    auto indexEndNode = asset0.keys().indexOf("end_node");
    if(indexEndNode == -1){
        this->errorMessage("Could not find 'end_node' in line assest");
        return nullptr;
    }
    // Get the number of rows
    auto nRows = assetArray.size();
    QString layerType = "linestring";
    mainLayer  = theVisualizationWidget->addVectorLayer(layerType, assetType+QString("s"));
    if(mainLayer == nullptr)
    {
        this->errorMessage("Error adding a vector layer");
        return nullptr;
    }
    auto pr = mainLayer->dataProvider();
    mainLayer->startEditing();
    auto res = pr->addAttributes(attribFields);
    if(!res)
        this->errorMessage("Error adding attributes to the layer" + mainLayer->name());
    mainLayer->updateFields(); // tell the vector layer to fetch changes from the provider
    theComponentDb->setMainLayer(mainLayer);
    auto numAtrb = attribFields.size();
    for(int i = 0; i<nRows; ++i)
    {
        // create the feature attributes
        QgsAttributes featureAttributes(numAtrb);
        featureAttributes[0] = QVariant(i);
        featureAttributes[1] = QVariant(assetType);
        // The feature attributes are the columns from the table
        for(int j = 0; j<assetArray[i].toObject().keys().size(); ++j)
        {
            auto key = assetArray[i].toObject().keys()[j];
            auto attrbVal = assetArray[i].toObject()[key].toVariant();
            featureAttributes[2+j] = attrbVal;
        }
        QgsFeature feature;
        feature.setFields(featFields);
        QString startNodeID = QString::number(assetArray[i].toObject()["start_node"].toInt(), 10);
        QString endNodeID = QString::number(assetArray[i].toObject()["end_node"].toInt(), 10);
        double longitudeStart; double latitudeStart;
        double longitudeEnd; double latitudeEnd;
        if (nodesDict.contains(startNodeID) && nodesDict.contains(endNodeID)){
            longitudeStart = nodesDict[startNodeID].toObject()["lon"].toDouble();
            latitudeStart = nodesDict[startNodeID].toObject()["lat"].toDouble();
            longitudeEnd = nodesDict[endNodeID].toObject()["lon"].toDouble();
            latitudeEnd = nodesDict[endNodeID].toObject()["lat"].toDouble();
        }else{
            this->errorMessage(QString("node ") + startNodeID + QString(" or ") + endNodeID + QString(" is missing in inventory file"));
            return nullptr;
        }
        // Start and end point of the pipe
        QgsPointXY point1(longitudeStart,latitudeStart);
        QgsPointXY point2(longitudeEnd,latitudeEnd);
        QgsPolylineXY lineGeom(2);
        lineGeom[0]=point1;
        lineGeom[1]=point2;
        if(lineGeom.isEmpty())
        {
            this->errorMessage("Error getting the line asset geometry");
            return nullptr;
        }
        feature.setGeometry(QgsGeometry::fromPolylineXY(lineGeom));
        feature.setAttributes(featureAttributes);
        if(!feature.isValid())
            return nullptr;
        auto res = pr->addFeature(feature, QgsFeatureSink::FastInsert);
        if(!res)
        {
            this->errorMessage("Error adding the feature to the layer");
            return nullptr;
        }
        mainLayer->commitChanges(true);
        mainLayer->updateExtents();
        QgsLineSymbol* markerSymbol = new QgsLineSymbol();
        markerSymbol->setColor(color);
        markerSymbol->setWidth(0.8);
        theVisualizationWidget->createSimpleRenderer(markerSymbol,mainLayer);
        theVisualizationWidget->zoomToLayer(mainLayer);


    }
    return mainLayer;
}

void AssetInputJSONWidget::setFilterString(const QString& filter)
{
    selectComponentsLineEdit->setText(filter);
    selectComponentsLineEdit->selectComponents();
}


QString AssetInputJSONWidget::getFilterString(void)
{
//    QString filterData = selectComponentsLineEdit->getComponentAnalysisList();
    QString filterData = "";

    return filterData;
}


void AssetInputJSONWidget::selectAllComponents(void)
{
    // Get the ID of the first and last component
//    auto firstID = componentTableWidget->item(0,0).toString();

//    auto nRows = componentTableWidget->rowCount();
//    auto lastID = componentTableWidget->item(nRows-1,0).toString();

//    QString filter = firstID + "-" + lastID;

//    selectComponentsLineEdit->setText(filter);
//    selectComponentsLineEdit->selectComponents();
}


bool AssetInputJSONWidget::outputToJSON(QJsonObject &rvObject)
{
    Q_UNUSED(rvObject);

    return true;
}


bool AssetInputJSONWidget::inputFromJSON(QJsonObject &rvObject)
{
    Q_UNUSED(rvObject);

    return true;
}


bool AssetInputJSONWidget::copyFiles(QString &destName)
{
    auto compLineEditText = componentFileLineEdit->text();

    QFileInfo componentFile(compLineEditText);

    if (!componentFile.exists())
    {
        this->errorMessage("The asset file path does not exist. Did you load any assets?");
        return false;
    }

    // Do not copy the file, output a new csv which will have the changes that the user makes in the table
    //        if (componentFile.exists()) {
    //            return this->copyFile(componentFileLineEdit->text(), destName);
    //        }

    auto pathToSaveFile = destName + QDir::separator() + componentFile.baseName();

    pathToSaveFile += ".json";

    // Copy the entire file, need to add selection function later
    bool res = QFile::copy(componentFile.absoluteFilePath(), pathToSaveFile);
    // Use the SimCenterAppWidge.copyFile function to prevent overwrite same file with itself
//    bool res = this->copyFile(componentFile.absoluteFilePath(), destName);
    if (!res){
        this->errorMessage(QString("Filed copy asset file: ")+componentFile.absoluteFilePath());
        return false;
    }

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

//    // Put this here because copy files gets called first and we need to select the components before we can create the input file
//    QString filterData = this->getFilterString();

//#ifdef OpenSRA
//    filterData = "1";
//#endif

//    if(filterData.isEmpty())
//    {

//        auto msgBox =  std::make_unique<QMessageBox>();

//        msgBox->setText("No IDs are selected for analysis in ASD "+assetType.toLower()+". Really run with all components?");
//        msgBox->setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);

//        auto res = msgBox->exec();

//        if(res != QMessageBox::Yes)
//            return false;

//        this->selectAllComponents();

//        statusMessage("Selecting all components for analysis");
//        filterData = this->getFilterString();

//        if(filterData.isEmpty())
//        {
//            errorMessage("Error selecting components for analysis");
//            return false;
//        }
//    }


    statusMessage("Asset files copied");
    return true;
}


void AssetInputJSONWidget::clearLoadedData(void)
{
    theComponentDb->clear();
    pathToComponentInputFile.clear();
    componentFileLineEdit->clear();
}



void AssetInputJSONWidget::clear(void)
{
    this->clearLoadedData();

    mainLayer = nullptr;
    selectedFeaturesLayer = nullptr;

    emit headingValuesChanged(QStringList{"N/A"});
}


void AssetInputJSONWidget::handleCellChanged(const int row, const int col)
{
    auto ID = componentTableWidget->item(row,0).toInt();

    auto attrib = componentTableWidget->horizontalHeaderItem(col);

    auto attribVal = componentTableWidget->item(row,col);


    auto res = theComponentDb->updateComponentAttribute(ID,attrib,attribVal);
    if(res == false)
        this->errorMessage("Error could not update asset "+QString::number(ID)+" after cell change");

}


void AssetInputJSONWidget::insertSelectedAssets(QgsFeatureIds& featureIds)
{

    QVector<int> assetIds;
    assetIds.reserve(featureIds.size());

    for(auto&& it : featureIds)
        assetIds.push_back(it-offset);

    selectComponentsLineEdit->insertSelectedComponents(assetIds);

    this->selectComponents();
}


void AssetInputJSONWidget::clearSelectedAssets(void)
{
    this->clearComponentSelection();
}


void AssetInputJSONWidget::handleComponentFilter(void)
{
    auto mainAssetLayer = theComponentDb->getMainLayer();

    if(mainAssetLayer == nullptr)
    {
        this->statusMessage("Please import assets to create filter");
        return;
    }

    QVector<int> filterIds;
    auto res = filterDelegateWidget->openQueryBuilderDialog(filterIds);

    if(res == -1)
    {
        this->errorMessage("Error creating the filter");
        return;
    }

    if(!filterIds.isEmpty())
    {
        selectComponentsLineEdit->insertSelectedComponents(filterIds);
        selectComponentsLineEdit->selectComponents();
    }
}


int AssetInputJSONWidget::applyFilterString(const QString& filter)
{
    QVector<int> filterIds;
    auto res = filterDelegateWidget->setFilterString(filter,filterIds);

    if(res == -1)
    {
        this->errorMessage("Error setting the filter string");
        return -1;
    }

    if(!filterIds.isEmpty())
    {
        selectComponentsLineEdit->insertSelectedComponents(filterIds);
        selectComponentsLineEdit->selectComponents();
    }

    return 0;
}


bool AssetInputJSONWidget::isEmpty()
{
    if(componentTableWidget->rowCount() == 0)
        return true;

    return false;
}


int AssetInputJSONWidget::getNumberOfAseets(void)
{
    return componentTableWidget->rowCount();
}


void AssetInputJSONWidget::setFilterVisibility(const bool value)
{
    filterWidget->setVisible(value);
}

