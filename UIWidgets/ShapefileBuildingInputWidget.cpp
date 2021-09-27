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

// Written by: Stevan Gavrilovic

#include "AssetInputDelegate.h"
#include "ShapefileBuildingInputWidget.h"
#include "VisualizationWidget.h"
#include "CSVReaderWriter.h"
#include "ComponentTableView.h"
#include "ComponentTableModel.h"
#include "ComponentDatabaseManager.h"

#include <QCoreApplication>
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
#include <qgsfield.h>
#include <qgsfields.h>
#include <qgsvectorlayer.h>

// Std library headers
#include <string>
#include <algorithm>



ShapefileBuildingInputWidget::ShapefileBuildingInputWidget(QWidget *parent, VisualizationWidget* visWidget, QString componentType, QString appType) : SimCenterAppWidget(parent), appType(appType), componentType(componentType)
{    
    theComponentDb = ComponentDatabaseManager::getInstance()->getBuildingComponentDb();

    theVisualizationWidget = dynamic_cast<QGISVisualizationWidget*>(visWidget);

    this->setContentsMargins(0,0,0,0);

    label1 = "Load buildings from a Shapefile (.shp)";
    label2 = "Enter the IDs of one or more " + componentType.toLower() + " to analyze."
    "Define a range of " + componentType.toLower() + " with a dash and separate multiple " + componentType.toLower() + " with a comma.";

    label3 = QStringRef(&componentType, 0, componentType.length()-1) + " Information";

    pathToComponentInputFile = "NULL";
    componentGroupBox = nullptr;
    this->createComponentsBox();

//    pathToComponentInputFile = "/Users/steve/Desktop/GalvestonTestbed/GalvestonBuildings/galveston-bldg-v7.shp";
//    componentFileLineEdit->setText(pathToComponentInputFile);
//    this->loadComponentData();
}


ShapefileBuildingInputWidget::~ShapefileBuildingInputWidget()
{

}


void ShapefileBuildingInputWidget::loadComponentData(void)
{
    // Ask for the file path if the file path has not yet been set, and return if it is still null
    if(pathToComponentInputFile.compare("NULL") == 0)
        this->chooseComponentInfoFileDialog();

    if(pathToComponentInputFile.compare("NULL") == 0)
        return;

    // Check if the directory exists
    QFileInfo file(pathToComponentInputFile);

    if (!file.exists())
    {
        auto relPathToComponentFile = QCoreApplication::applicationDirPath() + QDir::separator() + pathToComponentInputFile;

        if (!QFile(relPathToComponentFile).exists())
        {
            QString errMsg = "Cannot find the file: "+ pathToComponentInputFile + "\n" +"Check your directory and try again.";
            this->errorMessage(errMsg);
            return;
        }
        else
        {
            pathToComponentInputFile = relPathToComponentFile;
            componentFileLineEdit->setText(pathToComponentInputFile);
        }
    }

    // Name the layer according to the filename
    auto fName = file.fileName();

    auto layer = theVisualizationWidget->addVectorLayer(pathToComponentInputFile, fName, "ogr");

    auto numFeat = layer->featureCount();

    if(numFeat == 0)
    {
        this->errorMessage("Input Shapefile does not have any features");
        return;
    }
    else{
        this->statusMessage("Loading information for " + QString::number(numFeat)+ " assets");
        QApplication::processEvents();
    }

    auto features = layer->getFeatures();

    auto fields = layer->fields();

    if(fields.size() == 0)
    {
        this->errorMessage("Input Shapefile layer does not have any fields. Will not show table");
        return;
    }

    QStringList fieldsStrList;
    for(int i = 0; i<fields.size(); ++i)
    {
        auto field = fields[i];
        auto fieldName = field.name();

        fieldsStrList.push_back(fieldName);
    }

    tableHorizontalHeadings = fieldsStrList;

    // Add 1 for the header row
    QVector<QStringList> data(numFeat);

    QgsFeature feat;
    size_t i = 0;
    while (features.nextFeature(feat))
    {
        QStringList attributeStrList;
        auto attributes = feat.attributes();
        for(int i = 0; i<attributes.size(); ++i)
        {
            auto attribute = attributes[i];
            auto attributeStr = attribute.toString();

            attributeStrList.push_back(attributeStr);
        }

        data[i] = attributeStrList;

        ++i;
    }

    componentTableWidget->clear();
    componentTableWidget->getTableModel()->populateData(data, tableHorizontalHeadings);

    componentInfoText->show();
    componentTableWidget->show();
    componentTableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);

    this->statusMessage("Done loading assets");
    QApplication::processEvents();

    return;
}


void ShapefileBuildingInputWidget::chooseComponentInfoFileDialog(void)
{
    pathToComponentInputFile = QFileDialog::getOpenFileName(this,tr("Component Information File"));

    // Return if the user cancels
    if(pathToComponentInputFile.isEmpty())
    {
        pathToComponentInputFile = "NULL";
        return;
    }

    // Set file name & entry in qLine edit
    componentFileLineEdit->setText(pathToComponentInputFile);

    this->loadComponentData();

    return;
}



ComponentTableView *ShapefileBuildingInputWidget::getTableWidget() const
{
    return componentTableWidget;
}


void ShapefileBuildingInputWidget::createComponentsBox(void)
{
    componentGroupBox = new QGroupBox(componentType);
    componentGroupBox->setFlat(true);
    componentGroupBox->setContentsMargins(0,0,0,0);

    QVBoxLayout* gridLayout = new QVBoxLayout();
    gridLayout->setMargin(0);
    gridLayout->setSpacing(5);
    gridLayout->setContentsMargins(10,0,0,0);

    componentGroupBox->setLayout(gridLayout);

    QLabel* topText = new QLabel();
    topText->setText(label1);

    QLabel* pathText = new QLabel();
    pathText->setText("Import Path:");

    componentFileLineEdit = new QLineEdit();
    //    componentFileLineEdit->setMaximumWidth(750);
    componentFileLineEdit->setMinimumWidth(400);
    componentFileLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    QPushButton *browseFileButton = new QPushButton();
    browseFileButton->setText(tr("Browse"));
    browseFileButton->setMaximumWidth(150);

    connect(browseFileButton,SIGNAL(clicked()),this,SLOT(chooseComponentInfoFileDialog()));

    // Add a horizontal spacer after the browse and load buttons
    //    auto hspacer = new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Minimum);

    QLabel* selectComponentsText = new QLabel();
    selectComponentsText->setText(label2);

    selectComponentsLineEdit = new AssetInputDelegate();
    connect(selectComponentsLineEdit,&AssetInputDelegate::componentSelectionComplete,this,&ShapefileBuildingInputWidget::handleComponentSelection);

    QPushButton *selectComponentsButton = new QPushButton();
    selectComponentsButton->setText(tr("Select"));
    selectComponentsButton->setMaximumWidth(150);

    connect(selectComponentsButton,SIGNAL(clicked()),this,SLOT(selectComponents()));

    QPushButton *clearSelectionButton = new QPushButton();
    clearSelectionButton->setText(tr("Clear Selection"));
    clearSelectionButton->setMaximumWidth(150);

    connect(clearSelectionButton,SIGNAL(clicked()),this,SLOT(clearComponentSelection()));

    // Text label for Component information
    componentInfoText = new QLabel(label3);
    componentInfoText->setStyleSheet("font-weight: bold; color: black");
    componentInfoText->hide();

    // Create the table that will show the Component information
    componentTableWidget = new ComponentTableView(this);

    connect(componentTableWidget->getTableModel(), &ComponentTableModel::handleCellChanged, this, &ShapefileBuildingInputWidget::handleCellChanged);

#ifdef OpenSRA
    auto methodsAndParams = WorkflowAppOpenSRA::getInstance()->getMethodsAndParamsObj();

    QJsonObject thisObj = methodsAndParams["Infrastructure"].toObject()["SiteLocationParams"].toObject();

    if(thisObj.isEmpty())
    {
        this->errorMessage("Json object is empty in " + QString(__FUNCTION__));
        return;
    }

    auto theWidgetFactory = std::make_unique<WidgetFactory>(this);

    QJsonObject paramsObj = thisObj["Params"].toObject();

    // The string given in the Methods and params json file
    QString nameStr = "SiteLocationParams";

    auto widgetLabelText = thisObj["NameToDisplay"].toString();

    if(widgetLabelText.isEmpty())
    {
        this->errorMessage("Could not find the *NameToDisplay* key in object json for " + nameStr);
        return;
    }

    locationWidget = new JsonGroupBoxWidget(this);
    locationWidget->setObjectName(nameStr);

    locationWidget->setTitle(widgetLabelText);

    QJsonObject paramsLat;
    paramsLat["LatBegin"] = paramsObj.value("LatBegin");
    paramsLat["LatMid"] = paramsObj.value("LatMid");
    paramsLat["LatEnd"] = paramsObj.value("LatEnd");

    QJsonObject paramsLon;
    paramsLon["LonBegin"] = paramsObj.value("LonBegin");
    paramsLon["LonMid"] = paramsObj.value("LonMid");
    paramsLon["LonEnd"] = paramsObj.value("LonEnd");
    paramsLon["Length"] = paramsObj.value("Length");

    auto latLayout = theWidgetFactory->getLayoutFromParams(paramsLat,nameStr,locationWidget, Qt::Horizontal);
    auto lonLayout = theWidgetFactory->getLayoutFromParams(paramsLon,nameStr,locationWidget, Qt::Horizontal);

    QVBoxLayout* latLonLayout = new QVBoxLayout();
    latLonLayout->addLayout(latLayout);
    latLonLayout->addLayout(lonLayout);

    locationWidget->setLayout(latLonLayout);

    QHBoxLayout* pathLayout = new QHBoxLayout();
    pathLayout->addWidget(pathText);
    pathLayout->addWidget(componentFileLineEdit);
    pathLayout->addWidget(browseFileButton);

    // Add a vertical spacer at the bottom to push everything up
    gridLayout->addWidget(topText);
    gridLayout->addLayout(pathLayout);
    gridLayout->addWidget(selectComponentsText);

    gridLayout->addWidget(selectComponentsText);

    QHBoxLayout* selectComponentsLayout = new QHBoxLayout();
    selectComponentsLayout->addWidget(selectComponentsLineEdit);
    selectComponentsLayout->addWidget(selectComponentsButton);
    selectComponentsLayout->addWidget(clearSelectionButton);

    gridLayout->addLayout(selectComponentsLayout);
    gridLayout->addWidget(locationWidget);
    gridLayout->addWidget(componentInfoText,0,Qt::AlignCenter);
    gridLayout->addWidget(componentTableWidget,0,Qt::AlignCenter);

#else
    QHBoxLayout* pathLayout = new QHBoxLayout();
    pathLayout->addWidget(pathText);
    pathLayout->addWidget(componentFileLineEdit);
    pathLayout->addWidget(browseFileButton);

    // Add a vertical spacer at the bottom to push everything up
    gridLayout->addWidget(topText);
    gridLayout->addLayout(pathLayout);
    gridLayout->addWidget(selectComponentsText);

    gridLayout->addWidget(selectComponentsText);

    QHBoxLayout* selectComponentsLayout = new QHBoxLayout();
    selectComponentsLayout->addWidget(selectComponentsLineEdit);
    selectComponentsLayout->addWidget(selectComponentsButton);
    selectComponentsLayout->addWidget(clearSelectionButton);

    gridLayout->addLayout(selectComponentsLayout);
    gridLayout->addWidget(componentInfoText,0,Qt::AlignCenter);
    gridLayout->addWidget(componentTableWidget,0,Qt::AlignCenter);

    gridLayout->addStretch();
#endif

    this->setLayout(gridLayout);
}


void ShapefileBuildingInputWidget::selectComponents(void)
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


void ShapefileBuildingInputWidget::handleComponentSelection(void)
{
//    qDebug()<<"Implement me in ShapefileBuildingInputWidget::handleComponentSelection";

//    return;

//    auto nRows = componentTableWidget->rowCount();

//    if(nRows == 0)
//        return;

//    // Get the ID of the first and last component
//    bool OK;
//    auto firstID = componentTableWidget->item(0,0).toInt(&OK);

//    if(!OK)
//    {
//        QString msg = "Error in getting the component ID in " + QString(__FUNCTION__);
//        this->errorMessage(msg);
//        return;
//    }

//    auto lastID = componentTableWidget->item(nRows-1,0).toInt(&OK);

//    if(!OK)
//    {
//        QString msg = "Error in getting the component ID in " + QString(__FUNCTION__);
//        this->errorMessage(msg);
//        return;
//    }

//    auto selectedComponentIDs = selectComponentsLineEdit->getSelectedComponentIDs();

//    // First check that all of the selected IDs are within range
//    for(auto&& it : selectedComponentIDs)
//    {
//        if(it<firstID || it>lastID)
//        {
//            QString msg = "The component ID " + QString::number(it) + " is out of range of the components provided";
//            this->errorMessage(msg);
//            selectComponentsLineEdit->clear();
//            return;
//        }
//    }

//    // Hide all rows in the table
//    for(int i = 0; i<nRows; ++i)
//        componentTableWidget->setRowHidden(i,true);

//    // Unhide the selected rows
//    for(auto&& it : selectedComponentIDs)
//        componentTableWidget->setRowHidden(it - firstID,false);

//    auto numAssets = selectedComponentIDs.size();
//    QString msg = "A total of "+ QString::number(numAssets) + " " + componentType.toLower() + " are selected for analysis";
//    this->statusMessage(msg);

//    for(auto&& it : selectedComponentIDs)
//    {
//        auto component = theComponentDb->getComponent(it);

//        auto feature = component.ComponentFeature;

//        if(feature.isValid())
//            continue;

//        QMap<QString, QVariant> featureAttributes;

//        auto id = feature.id();

////        if(selectedFeaturesForAnalysis.contains(id))
////            continue;

////        auto geom = feature->geometry();

////        auto feat = this->addFeatureToSelectedLayer(featureAttributes,geom);

////        if(feat)
////            selectedFeaturesForAnalysis.insert(id,feat);
//    }

//    auto selecFeatLayer = this->getSelectedFeatureLayer();

//    if(selecFeatLayer == nullptr)
//    {
//        QString err = "Error in getting the selected feature layer";
//        qDebug()<<err;
//        return;
//    }
}


void ShapefileBuildingInputWidget::clearLayerSelectedForAnalysis(void)
{
//    if(selectedFeaturesForAnalysis.empty())
//        return;

//    for(auto&& it : selectedFeaturesForAnalysis)
//    {
//        this->removeFeatureFromSelectedLayer(it);
//    }

//    selectedFeaturesForAnalysis.clear();
}


QStringList ShapefileBuildingInputWidget::getTableHorizontalHeadings()
{
    return tableHorizontalHeadings;
}


void ShapefileBuildingInputWidget::clearComponentSelection(void)
{

    this->clearLayerSelectedForAnalysis();

    auto nRows = componentTableWidget->rowCount();

    // Hide all rows in the table
    for(int i = 0; i<nRows; ++i)
    {
        componentTableWidget->setRowHidden(i,false);
    }

    selectComponentsLineEdit->clear();
}


void ShapefileBuildingInputWidget::setLabel1(const QString &value)
{
    label1 = value;
}


void ShapefileBuildingInputWidget::setLabel2(const QString &value)
{
    label2 = value;
}


void ShapefileBuildingInputWidget::setLabel3(const QString &value)
{
    label3 = value;
}


void ShapefileBuildingInputWidget::setGroupBoxText(const QString &value)
{
    componentGroupBox->setTitle(value);
}


void ShapefileBuildingInputWidget::setComponentType(const QString &value)
{
    componentType = value;
}


void ShapefileBuildingInputWidget::insertSelectedComponent(const int ComponentID)
{
    selectComponentsLineEdit->insertSelectedComponent(ComponentID);
}


int ShapefileBuildingInputWidget::numberComponentsSelected(void)
{
    return selectComponentsLineEdit->size();
}


QString ShapefileBuildingInputWidget::getPathToComponentFile(void) const
{
    return pathToComponentInputFile;
}


void ShapefileBuildingInputWidget::loadFileFromPath(QString& path)
{
    this->clear();
    pathToComponentInputFile = path;
    componentFileLineEdit->setText(path);
    this->loadComponentData();
}


bool ShapefileBuildingInputWidget::outputAppDataToJSON(QJsonObject &jsonObject)
{
    jsonObject["Application"]=appType;

    QJsonObject data;
    QFileInfo componentFile(componentFileLineEdit->text());
    if (componentFile.exists()) {
        data["buildingSourceFile"]=componentFile.fileName();
        data["pathToSource"]=componentFile.path();

        QString filterData = this->getFilterString();

        if(filterData.isEmpty())
        {
            errorMessage("Please select components for analysis");
            return false;
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


bool ShapefileBuildingInputWidget::inputAppDataFromJSON(QJsonObject &jsonObject)
{

    //jsonObject["Application"]=appType;
    if (jsonObject.contains("Application")) {
        if (appType != jsonObject["Application"].toString()) {
            this->errorMessage("ShapefileBuildingInputWidget::inputFRommJSON app name conflict");
            return false;
        }
    }


    if (jsonObject.contains("ApplicationData")) {
        QJsonObject appData = jsonObject["ApplicationData"].toObject();

        QFileInfo fileInfo;
        QString fileName;
        QString pathToFile;
        bool foundFile = false;
        if (appData.contains("buildingSourceFile"))
            fileName = appData["buildingSourceFile"].toString();

        if (fileInfo.exists(fileName)) {

            selectComponentsLineEdit->setText(fileName);

            this->loadComponentData();
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
                this->loadComponentData();

            } else {
                // adam .. adam .. adam
                pathToComponentInputFile = pathToFile + QDir::separator()
                        + "input_data" + QDir::separator() + fileName;
                if (fileInfo.exists(pathToComponentInputFile)) {
                    componentFileLineEdit->setText(pathToComponentInputFile);
                    foundFile = true;
                    this->loadComponentData();
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

    return true;
}


void ShapefileBuildingInputWidget::setFilterString(const QString& filter)
{
    selectComponentsLineEdit->setText(filter);
    selectComponentsLineEdit->selectComponents();
}


QString ShapefileBuildingInputWidget::getFilterString(void)
{
    QString filterData = selectComponentsLineEdit->getComponentAnalysisList();

    return filterData;
}


void ShapefileBuildingInputWidget::selectAllComponents(void)
{
    // Get the ID of the first and last component
    auto firstID = componentTableWidget->item(0,0).toString();

    auto nRows = componentTableWidget->rowCount();
    auto lastID = componentTableWidget->item(nRows-1,0).toString();

    QString filter = firstID + "-" + lastID;

    selectComponentsLineEdit->setText(filter);
    selectComponentsLineEdit->selectComponents();
}


bool ShapefileBuildingInputWidget::outputToJSON(QJsonObject &rvObject)
{
#ifdef OpenSRA
    locationWidget->outputToJSON(rvObject);
#else
    Q_UNUSED(rvObject);
#endif
    return true;
}


bool ShapefileBuildingInputWidget::inputFromJSON(QJsonObject &rvObject)
{
#ifdef OpenSRA
    locationWidget->inputFromJSON(rvObject);
#else
    Q_UNUSED(rvObject);
#endif

    return true;
}


bool ShapefileBuildingInputWidget::copyFiles(QString &destName)
{
    auto compLineEditText = componentFileLineEdit->text();

    QFileInfo componentFile(compLineEditText);

    if (!componentFile.exists())
        return false;

    // Do not copy the file, output a new csv which will have the changes that the user makes in the table
    //        if (componentFile.exists()) {
    //            return this->copyFile(componentFileLineEdit->text(), destName);
    //        }

    auto pathToSaveFile = destName + QDir::separator() + componentFile.fileName();

    auto nRows = componentTableWidget->rowCount();

    if(nRows == 0)
        return false;

    auto data = componentTableWidget->getTableModel()->getTableData();

    auto headerValues = componentTableWidget->getTableModel()->getHeaderStringList();

    data.push_front(headerValues);

    CSVReaderWriter csvTool;

    QString err;
    csvTool.saveCSVFile(data,pathToSaveFile,err);

    if(!err.isEmpty())
        return false;


    // For testing, creates a csv file of only the selected components
    //    qDebug()<<"Saving selected components to .csv";
    //    auto selectedIDs = selectComponentsLineEdit->getSelectedComponentIDs();

    //    QVector<QStringList> selectedData(selectedIDs.size()+1);

    //    selectedData[0] = headerInfo;

    //    int i = 0;
    //    for(auto&& rowID : selectedIDs)
    //    {
    //        QStringList rowData;
    //        rowData.reserve(nCols);

    //        for(int j = 0; j<nCols; ++j)
    //        {
    //            auto item = componentTableWidget->item(rowID-1,j)->data(0).toString();

    //            rowData<<item;
    //        }
    //        selectedData[i+1] = rowData;

    //        ++i;
    //    }

    //    csvTool.saveCSVFile(selectedData,"/Users/steve/Desktop/Selected.csv",err);

    return true;
}


void ShapefileBuildingInputWidget::clear(void)
{
    theComponentDb->clear();
    pathToComponentInputFile.clear();
    componentFileLineEdit->clear();
    selectComponentsLineEdit->clear();
    componentTableWidget->clear();
    componentTableWidget->hide();
    tableHorizontalHeadings.clear();

    emit headingValuesChanged(QStringList{"N/A"});
}


void ShapefileBuildingInputWidget::handleCellChanged(const int row, const int col)
{

//    auto ID = componentTableWidget->item(row,0).toInt();

//    auto attrib = componentTableWidget->horizontalHeaderItem(col);

//    auto attribVal = componentTableWidget->item(row,col);

//    theComponentDb->updateComponentAttribute(ID,attrib,attribVal);

//    auto component = theComponentDb->getComponent(ID);

//    if(!component.isValid())
//        return;

//    auto uid = component.UID;
//    this->updateSelectedComponentAttribute(uid,attrib,attribVal);

}


void ShapefileBuildingInputWidget::updateComponentAttribute(const int uid, const QString& attribute, const QVariant& value)
{
    theComponentDb->updateComponentAttribute(uid,attribute,value);
}


QgsFeature*  ShapefileBuildingInputWidget::addFeatureToSelectedLayer(QMap<QString, QVariant>& featureAttributes, QgsGeometry& geom)
{
    Q_UNUSED(featureAttributes);
    Q_UNUSED(geom);

    return nullptr;
}


int ShapefileBuildingInputWidget::removeFeatureFromSelectedLayer(QgsFeature* feat)
{
    Q_UNUSED(feat);
    return -1;
}


QgsVectorLayer* ShapefileBuildingInputWidget::getSelectedFeatureLayer(void)
{
    return nullptr;
}


void ShapefileBuildingInputWidget::updateSelectedComponentAttribute(const QString&  uid, const QString& attribute, const QVariant& value)
{

    if(selectedFeaturesForAnalysis.empty())
    {
        qDebug()<<"Selected features map is empty";
        return;
    }

//    if(!selectedFeaturesForAnalysis.contains(uid))
//    {
//        qDebug()<<"Feature not found in selected components map";
//        return;
//    }


//#ifdef Q_GIS
//    // Get the feature
//    QgsFeature feat = selectedFeaturesForAnalysis[uid];

//    if(feat == nullptr)
//    {
//        qDebug()<<"Feature is a nullptr";
//        return;
//    }

//    auto res = feat->setAttribute(attribute,value);

//    if(res == false)
//    {
//        qDebug()<<"Failed to update feature "<<feat->attribute("ID").toString();
//        return;
//    }
//#endif

}
