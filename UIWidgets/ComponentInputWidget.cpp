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
#include "ComponentInputWidget.h"
#include "VisualizationWidget.h"
#include "CSVReaderWriter.h"
#include "ComponentTableView.h"
#include "ComponentTableModel.h"

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

#ifdef ARC_GIS
#include "ArcGISVisualizationWidget.h"
#include <FeatureCollectionLayer.h>
#endif

#ifdef Q_GIS
#include "QGISVisualizationWidget.h"
#endif

// Std library headers
#include <string>
#include <algorithm>

#ifdef OpenSRA
#include "WorkflowAppOpenSRA.h"
#include "WidgetFactory.h"
#include "JsonGroupBoxWidget.h"
#endif

ComponentInputWidget::ComponentInputWidget(QWidget *parent, VisualizationWidget* visWidget, QString componentType, QString appType) : SimCenterAppWidget(parent), appType(appType), componentType(componentType)
{
#ifdef ARC_GIS
    theVisualizationWidget = static_cast<ArcGISVisualizationWidget*>(visWidget);

    if(theVisualizationWidget == nullptr)
    {
        this->errorMessage("Failed to cast to QISVisualizationWidget");
        return;
    }
#endif

    offset = 0;

    theVisualizationWidget = static_cast<QGISVisualizationWidget*>(visWidget);
    assert(theVisualizationWidget);

    this->setContentsMargins(0,0,0,0);

    pathToComponentInputFile = "NULL";
    componentGroupBox = nullptr;
    componentFileLineEdit = nullptr;

    this->createComponentsBox();

    auto txt1 = "Load information from a CSV file";
    auto txt2  = "Enter the IDs of one or more " + componentType.toLower() + " to analyze."
    "Define a range of " + componentType.toLower() + " with a dash and separate multiple " + componentType.toLower() + " with a comma.";

    auto txt3 = QStringRef(&componentType, 0, componentType.length()-1) + " Information";

    label1->setText(txt1);
    label2->setText(txt2);
    label3->setText(txt3);

}


ComponentInputWidget::~ComponentInputWidget()
{
    
}


#ifdef OpenSRA
bool ComponentInputWidget::loadFileFromPath(const QString& filePath)
{
    QFileInfo fileInfo;
    if (!fileInfo.exists(filePath))
        return false;

    pathToComponentInputFile = filePath;
    componentFileLineEdit->setText(filePath);

    this->loadComponentData();

    return true;
}
#endif


bool ComponentInputWidget::loadComponentData(void)
{
    // Ask for the file path if the file path has not yet been set, and return if it is still null
    if(pathToComponentInputFile.compare("NULL") == 0)
        this->chooseComponentInfoFileDialog();
    
    if(pathToComponentInputFile.compare("NULL") == 0)
        return false;
    
    // Check if the directory exists
    QFile file(pathToComponentInputFile);
    
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
    
    // Test to remove
    // auto start = high_resolution_clock::now();
    
    CSVReaderWriter csvTool;
    
    QString err;
    QVector<QStringList> data = csvTool.parseCSVFile(pathToComponentInputFile,err);
    
    if(!err.isEmpty())
    {
        this->errorMessage(err);
        return false;
    }
    
    if(data.empty())
    {
        this->errorMessage("Input file is empty");
        return false;
    }
    
    // Get the header file
    QStringList tableHeadings = data.first();
    
    tableHorizontalHeadings = tableHeadings;
    
    tableHeadings.push_front("N/A");
    
    emit headingValuesChanged(tableHeadings);
    
    // Pop off the row that contains the header information
    data.pop_front();
    
    auto numRows = data.size();
    
    if(numRows == 0)
    {
        this->errorMessage("Input file is empty");
        return false;
    }
    else{
        this->statusMessage("Loading visualization for " + QString::number(numRows)+ " assets");
        QApplication::processEvents();
    }
    
    auto firstRow = data.first();
    
    if(firstRow.empty())
    {
        this->errorMessage("First row is empty");
        return false;
    }
    
    componentTableWidget->getTableModel()->populateData(data, tableHorizontalHeadings);
    
    label3->show();
    componentTableWidget->show();
    componentTableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
    
    this->loadComponentVisualization();

    // Get the ID of the first and last component
    bool OK;
    auto firstID = componentTableWidget->item(0,0).toInt(&OK);

    if(!OK)
    {
        QString msg = "Error in getting the component ID in " + QString(__FUNCTION__);
        this->errorMessage(msg);
        return false;
    }


    offset = 1-firstID;

    theComponentDb->setOffset(offset);
    
    // Test to remove
    //    auto stop = high_resolution_clock::now();
    //    auto duration = duration_cast<milliseconds>(stop - start);
    //    this->statusMessage("Done ALL "+QString::number(duration.count()));
    
    this->statusMessage("Done loading assets");
    QApplication::processEvents();
    
    return true;
}


void ComponentInputWidget::chooseComponentInfoFileDialog(void)
{
    this->clear();

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


ComponentTableView *ComponentInputWidget::getTableWidget() const
{
    return componentTableWidget;
}


void ComponentInputWidget::createComponentsBox(void)
{
    componentGroupBox = new QGroupBox(componentType);
    componentGroupBox->setFlat(true);
    componentGroupBox->setContentsMargins(0,0,0,0);
    
    QVBoxLayout* gridLayout = new QVBoxLayout();
    gridLayout->setMargin(0);
    gridLayout->setSpacing(5);
    gridLayout->setContentsMargins(10,0,0,0);
    
    componentGroupBox->setLayout(gridLayout);
    
    label1 = new QLabel();
    
    QLabel* pathText = new QLabel();
    pathText->setText("Path to file:");
    
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
    
    label2 = new QLabel();
    
    selectComponentsLineEdit = new AssetInputDelegate();
    connect(selectComponentsLineEdit,&AssetInputDelegate::componentSelectionComplete,this,&ComponentInputWidget::handleComponentSelection);
    
    QPushButton *selectComponentsButton = new QPushButton();
    selectComponentsButton->setText(tr("Select"));
    selectComponentsButton->setMaximumWidth(150);
    
    connect(selectComponentsButton,SIGNAL(clicked()),this,SLOT(selectComponents()));
    
    QPushButton *clearSelectionButton = new QPushButton();
    clearSelectionButton->setText(tr("Clear Selection"));
    clearSelectionButton->setMaximumWidth(150);
    
    connect(clearSelectionButton,SIGNAL(clicked()),this,SLOT(clearComponentSelection()));
    
    // Text label for Component information
    label3 = new QLabel();
    label3->setStyleSheet("font-weight: bold; color: black");
    label3->hide();
    
    // Create the table that will show the Component information
    componentTableWidget = new ComponentTableView();
    
    connect(componentTableWidget->getTableModel(), &ComponentTableModel::handleCellChanged, this, &ComponentInputWidget::handleCellChanged);
    
#ifdef OpenSRA
    auto methodsAndParams = WorkflowAppOpenSRA::getInstance()->getMethodsAndParamsObj();
    
    QJsonObject thisObj = methodsAndParams["Infrastructure"].toObject()["SiteLocationParams"].toObject();
    
    if(thisObj.isEmpty())
    {
        this->errorMessage("Json object is empty in " + QString(__FUNCTION__));
        return;
    }
    
    auto theWidgetFactory = new WidgetFactory(this);

    WorkflowAppOpenSRA::getInstance()->setTheWidgetFactory(theWidgetFactory);

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
    gridLayout->addWidget(label1);
    gridLayout->addLayout(pathLayout);
    gridLayout->addWidget(label2);
        
    QHBoxLayout* selectComponentsLayout = new QHBoxLayout();
    selectComponentsLayout->addWidget(selectComponentsLineEdit);
    selectComponentsLayout->addWidget(selectComponentsButton);
    selectComponentsLayout->addWidget(clearSelectionButton);
    
    gridLayout->addLayout(selectComponentsLayout);
    gridLayout->addWidget(locationWidget);
    gridLayout->addWidget(label3,0,Qt::AlignCenter);
    gridLayout->addWidget(componentTableWidget,0,Qt::AlignCenter);

    gridLayout->addStretch();
    
#else
    QHBoxLayout* pathLayout = new QHBoxLayout();
    pathLayout->addWidget(pathText);
    pathLayout->addWidget(componentFileLineEdit);
    pathLayout->addWidget(browseFileButton);
    
    // Add a vertical spacer at the bottom to push everything up
    gridLayout->addWidget(label1);
    gridLayout->addLayout(pathLayout);
    gridLayout->addWidget(label2);

    QHBoxLayout* selectComponentsLayout = new QHBoxLayout();
    selectComponentsLayout->addWidget(selectComponentsLineEdit);
    selectComponentsLayout->addWidget(selectComponentsButton);
    selectComponentsLayout->addWidget(clearSelectionButton);
    
    gridLayout->addLayout(selectComponentsLayout);
    gridLayout->addWidget(label3,0,Qt::AlignCenter);
    gridLayout->addWidget(componentTableWidget,0,Qt::AlignCenter);
    
    gridLayout->addStretch();
#endif
    
    this->setLayout(gridLayout);
}


void ComponentInputWidget::selectComponents(void)
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


#ifdef ARC_GIS
void ComponentInputWidget::handleComponentSelection(void)
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
    
    // Hide all rows in the table
    for(int i = 0; i<nRows; ++i)
        componentTableWidget->setRowHidden(i,true);
    
    // Unhide the selected rows
    for(auto&& it : selectedComponentIDs)
        componentTableWidget->setRowHidden(it - firstID,false);
    
    auto numAssets = selectedComponentIDs.size();
    QString msg = "A total of "+ QString::number(numAssets) + " " + componentType.toLower() + " are selected for analysis";
    this->statusMessage(msg);
    
    for(auto&& it : selectedComponentIDs)
    {
        auto component = theComponentDb->getComponent(it);
        
        auto feature = component.ComponentFeature;
        
        if(feature == nullptr)
            continue;
        
        QMap<QString, QVariant> featureAttributes;
        auto atrb = feature->attributes()->attributesMap();
        
        auto id = atrb.value("UID").toString();
        
        if(selectedFeaturesForAnalysis.contains(id))
            continue;
        
        auto atrVals = atrb.values();
        auto atrKeys = atrb.keys();
        
        // qDebug()<<"Num atributes: "<<atrb.size();
        
        for(int i = 0; i<atrb.size();++i)
        {
            auto key = atrKeys.at(i);
            auto val = atrVals.at(i);
            
            // Including the ObjectID causes a crash!!! Do not include it when creating an object
            if(key == "ObjectID")
                continue;
            
            // qDebug()<< nid<<"-key:"<<key<<"-value:"<<atrVals.at(i).toString();
            
            featureAttributes[key] = val;
        }

        
        //        auto geom = feature->geometry();
        
        auto res = this->addFeatureToSelectedLayer(*feature);

        if(res == false)
            this->errorMessage("Error adding feature to selected feature layer");
        else
            selectedFeaturesForAnalysis.insert(id,feature);
    }

    auto selecFeatLayer = this->getSelectedFeatureLayer();

    if(selecFeatLayer == nullptr)
    {
        QString err = "Error in getting the selected feature layer";
        qDebug()<<err;
        return;
    }

    // Add the layer to the map if it does not already exist
    auto layerExists = theVisualizationWidget->getLayer(selecFeatLayer->layerId());

    if(layerExists == nullptr)
        theVisualizationWidget->addSelectedFeatureLayerToMap(selecFeatLayer);

}
#endif


#ifdef Q_GIS
void ComponentInputWidget::handleComponentSelection(void)
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

    auto numAssets = selectedComponentIDs.size();
    QString msg = "A total of "+ QString::number(numAssets) + " " + componentType.toLower() + " are selected for analysis";
    this->statusMessage(msg);


    theComponentDb->startEditing();

    // Test to remove
    //    auto start = high_resolution_clock::now();

    theComponentDb->addFeaturesToSelectedLayer(selectedComponentIDs);

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
#endif



QStringList ComponentInputWidget::getTableHorizontalHeadings()
{
    return tableHorizontalHeadings;
}


void ComponentInputWidget::clearComponentSelection(void)
{
    auto nRows = componentTableWidget->rowCount();

    // Hide all rows in the table
    for(int i = 0; i<nRows; ++i)
    {
        componentTableWidget->setRowHidden(i,false);
    }

    selectComponentsLineEdit->clear();

    theComponentDb->clearSelectedLayer();

    theComponentDb->getSelectedLayer()->updateExtents();
}


void ComponentInputWidget::setLabel1(const QString &value)
{
    label1->setText(value);
}


void ComponentInputWidget::setLabel2(const QString &value)
{
    label2->setText(value);
}


void ComponentInputWidget::setLabel3(const QString &value)
{
    label3->setText(value);
}


void ComponentInputWidget::setGroupBoxText(const QString &value)
{
    componentGroupBox->setTitle(value);
}


void ComponentInputWidget::setComponentType(const QString &value)
{
    componentType = value;
}


int ComponentInputWidget::numberComponentsSelected(void)
{
    return selectComponentsLineEdit->size();
}


QString ComponentInputWidget::getPathToComponentFile(void) const
{
    return pathToComponentInputFile;
}


bool ComponentInputWidget::outputAppDataToJSON(QJsonObject &jsonObject)
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

            auto msgBox =  std::make_unique<QMessageBox>();

            msgBox->setText("No IDs are selected for analysis in ASD "+componentType.toLower()+". Really run with all components?");
            msgBox->setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);

            auto res = msgBox->exec();

            if(res != QMessageBox::Yes)
                return false;

            this->selectAllComponents();

            statusMessage("Selecting all components for analysis");
            filterData = this->getFilterString();

            if(filterData.isEmpty())
            {
                errorMessage("Error selecting components for analysis");
                return false;
            }
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


bool ComponentInputWidget::inputAppDataFromJSON(QJsonObject &jsonObject)
{

    //jsonObject["Application"]=appType;
    if (jsonObject.contains("Application")) {
        if (appType != jsonObject["Application"].toString()) {
            this->errorMessage("ComponentINputWidget::inputFRommJSON app name conflict");
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

            pathToComponentInputFile = fileName;
            componentFileLineEdit->setText(fileName);

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


void ComponentInputWidget::setFilterString(const QString& filter)
{
    selectComponentsLineEdit->setText(filter);
    selectComponentsLineEdit->selectComponents();
}


QString ComponentInputWidget::getFilterString(void)
{
    QString filterData = selectComponentsLineEdit->getComponentAnalysisList();

    return filterData;
}


void ComponentInputWidget::selectAllComponents(void)
{
    // Get the ID of the first and last component
    auto firstID = componentTableWidget->item(0,0).toString();

    auto nRows = componentTableWidget->rowCount();
    auto lastID = componentTableWidget->item(nRows-1,0).toString();

    QString filter = firstID + "-" + lastID;

    selectComponentsLineEdit->setText(filter);
    selectComponentsLineEdit->selectComponents();
}


bool ComponentInputWidget::outputToJSON(QJsonObject &rvObject)
{
#ifdef OpenSRA
    locationWidget->outputToJSON(rvObject);
#else
    Q_UNUSED(rvObject);
#endif
    return true;
}


bool ComponentInputWidget::inputFromJSON(QJsonObject &rvObject)
{
#ifdef OpenSRA
    locationWidget->inputFromJSON(rvObject);
#else
    Q_UNUSED(rvObject);
#endif

    return true;
}


bool ComponentInputWidget::copyFiles(QString &destName)
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


void ComponentInputWidget::clear(void)
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


void ComponentInputWidget::handleCellChanged(const int row, const int col)
{
    auto ID = componentTableWidget->item(row,0).toInt();

    auto attrib = componentTableWidget->horizontalHeaderItem(col);

    auto attribVal = componentTableWidget->item(row,col);

#ifdef ARC_GIS
    auto uid = component.UID;
    this->updateSelectedComponentAttribute(uid,attrib,attribVal);
#endif

#ifdef Q_GIS
    theComponentDb->updateComponentAttribute(ID,attrib,attribVal);
#endif

}

#ifdef ARC_GIS
Esri::ArcGISRuntime::Feature* ComponentInputWidget::addFeatureToSelectedLayer(QMap<QString, QVariant>& /*featureAttributes*/, Esri::ArcGISRuntime::Geometry& /*geom*/)
{
    return nullptr;
}


int ComponentInputWidget::removeFeatureFromSelectedLayer(Esri::ArcGISRuntime::Feature* /*feat*/)
{
    return -1;
}


Esri::ArcGISRuntime::FeatureCollectionLayer* ComponentInputWidget::getSelectedFeatureLayer(void)
{
    return nullptr;
}
#endif


#ifdef ARC_GIS
void ComponentInputWidget::updateSelectedComponentAttribute(const QString&  uid, const QString& attribute, const QVariant& value)
{

    if(selectedFeaturesForAnalysis.empty())
    {
        this->statusMessage("Selected features map is empty, nothing to update");
        return;
    }

    if(!selectedFeaturesForAnalysis.contains(id))
    {
        this->statusMessage("Feature not found in selected components map");
        return;
    }

    // Get the feature
    Esri::ArcGISRuntime::Feature* feat = selectedFeaturesForAnalysis[uid];

    if(feat == nullptr)
    {
        qDebug()<<"Feature is a nullptr";
        return;
    }

    feat->attributes()->replaceAttribute(attribute,value);
    feat->featureTable()->updateFeature(feat);

    if(feat->attributes()->attributeValue(attribute).isNull())
    {
        qDebug()<<"Failed to update feature "<<feat->attributes()->attributeValue("ID").toString();
        return;
    }
}
#endif


void ComponentInputWidget::insertSelectedAssets(QgsFeatureIds& featureIds)
{

    QVector<int> assetIds;
    assetIds.reserve(featureIds.size());

    for(auto&& it : featureIds)
        assetIds.push_back(it-offset);

    selectComponentsLineEdit->insertSelectedComponents(assetIds);

    this->selectComponents();
}


void ComponentInputWidget::clearSelectedAssets(void)
{
    this->clearComponentSelection();
}

