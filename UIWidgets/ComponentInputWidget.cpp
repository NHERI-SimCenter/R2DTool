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

#include <QCoreApplication>
#include <QFileDialog>
#include <QLineEdit>
#include <QTableWidget>
#include <QLabel>
#include <QGroupBox>
#include <QGridLayout>
#include <QPushButton>
#include <QHeaderView>
#include <QFileInfo>
#include <QJsonObject>

#include "FeatureCollectionLayer.h"

// Std library headers
#include <string>
#include <algorithm>

using namespace Esri::ArcGISRuntime;

ComponentInputWidget::ComponentInputWidget(QWidget *parent, QString componentType, QString appType) : SimCenterAppWidget(parent), componentType(componentType), appType(appType)
{
    label1 = "Load information from a CSV file";
    label2 = "Enter the IDs of one or more " + componentType.toLower() + " to analyze."
    "Define a range of " + componentType.toLower() + " with a dash and separate multiple " + componentType.toLower() + " with a comma.";

    label3 = QStringRef(&componentType, 0, componentType.length()-1) + " Information";

    pathToComponentInfoFile = "NULL";
    componentGroupBox = nullptr;
    theVisualizationWidget = nullptr;
    this->createComponentsBox();
}


ComponentInputWidget::~ComponentInputWidget()
{

}


void ComponentInputWidget::loadComponentData(void)
{
    // Ask for the file path if the file path has not yet been set, and return if it is still null
    if(pathToComponentInfoFile.compare("NULL") == 0)
        this->chooseComponentInfoFileDialog();

    if(pathToComponentInfoFile.compare("NULL") == 0)
        return;

    // Check if the directory exists
    QFile file(pathToComponentInfoFile);

    if (!file.exists())
    {
        auto relPathToComponentFile = QCoreApplication::applicationDirPath() + QDir::separator() + pathToComponentInfoFile;

        if (!QFile(relPathToComponentFile).exists())
        {
            QString errMsg = "Cannot find the file: "+ pathToComponentInfoFile + "\n" +"Check your directory and try again.";
            emit sendErrorMessage(errMsg);
            return;
        }
        else
        {
            pathToComponentInfoFile = relPathToComponentFile;
            componentFileLineEdit->setText(pathToComponentInfoFile);
        }
    }

    CSVReaderWriter csvTool;

    QString err;
    QVector<QStringList> data = csvTool.parseCSVFile(pathToComponentInfoFile,err);

    if(!err.isEmpty())
    {
        emit sendErrorMessage(err);
        return;
    }

    if(data.empty())
    {
        emit sendErrorMessage("Input file is empty");
        return;
    }

    // Get the header file
    QStringList tableHeadings = data.first();

    // Pop off the row that contains the header information
    data.pop_front();

    auto numRows = data.size();
    auto numCols = tableHeadings.size();

    if(numRows == 0)
    {
        emit sendErrorMessage("Input file is empty");
        return;
    }

    auto firstRow = data.first();

    if(firstRow.empty())
    {
        emit sendErrorMessage("First row is empty");
        return;
    }

    auto initialID = firstRow.first().toInt();

    componentTableWidget->clear();
    componentTableWidget->setColumnCount(numCols);
    componentTableWidget->setRowCount(numRows);
    componentTableWidget->setHorizontalHeaderLabels(tableHeadings);

    // Fill in the cells
    for(int i = 0; i<numRows; ++i)
    {
        auto rowStringList = data[i];

        if(rowStringList.size() != numCols)
        {
            this->userMessageDialog("Error, the number of items in row " + QString::number(i+1) + " does not equal number of headings in the file");
            return;
        }

        auto currID = rowStringList.first().toInt();

        if(initialID+i != currID)
        {
            this->userMessageDialog("Error, the asset IDs must be sequential");
            return;
        }

        for(int j = 0; j<numCols; ++j)
        {
            auto cellData = rowStringList[j];

            auto item = new QTableWidgetItem(cellData);

            // Make the first three columns (ID, lat, lon) uneditable
            if(j < 3)
                item->setFlags(item->flags() ^ (Qt::ItemIsEditable | Qt::ItemIsEnabled));

            componentTableWidget->setItem(i,j, item);
        }
    }

    componentInfoText->show();
    componentTableWidget->show();

    emit componentDataLoaded();

    this->loadComponentVisualization();

    return;
}


void ComponentInputWidget::chooseComponentInfoFileDialog(void)
{
    pathToComponentInfoFile = QFileDialog::getOpenFileName(this,tr("Component Information File"));

    // Return if the user cancels
    if(pathToComponentInfoFile.isEmpty())
    {
        pathToComponentInfoFile = "NULL";
        return;
    }

    // Set file name & entry in qLine edit
    componentFileLineEdit->setText(pathToComponentInfoFile);

    this->loadComponentData();

    return;
}


// Implement in subclass
int ComponentInputWidget::loadComponentVisualization()
{

    return 0;
}


QTableWidget *ComponentInputWidget::getTableWidget() const
{
    return componentTableWidget;
}


QGroupBox* ComponentInputWidget::getComponentsWidget(void)
{
    if(componentGroupBox == nullptr)
        this->createComponentsBox();

    return componentGroupBox;
}


void ComponentInputWidget::createComponentsBox(void)
{
    componentGroupBox = new QGroupBox(componentType);
    componentGroupBox->setFlat(true);

    QGridLayout* gridLayout = new QGridLayout();
    componentGroupBox->setLayout(gridLayout);

    auto smallVSpacer = new QSpacerItem(0,10);

    QLabel* topText = new QLabel();
    topText->setText(label1);

    QLabel* pathText = new QLabel();
    pathText->setText("Import Path:");

    componentFileLineEdit = new QLineEdit();
    componentFileLineEdit->setMaximumWidth(750);
    componentFileLineEdit->setMinimumWidth(400);
    componentFileLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    QPushButton *browseFileButton = new QPushButton();
    browseFileButton->setText(tr("Browse"));
    browseFileButton->setMaximumWidth(150);

    connect(browseFileButton,SIGNAL(clicked()),this,SLOT(chooseComponentInfoFileDialog()));

    // Add a horizontal spacer after the browse and load buttons
    auto hspacer = new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Minimum);

    QLabel* selectComponentsText = new QLabel();
    selectComponentsText->setText(label2);

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
    componentInfoText = new QLabel(label3);
    componentInfoText->setStyleSheet("font-weight: bold; color: black");
    componentInfoText->hide();

    // Create the table that will show the Component information
    componentTableWidget = new QTableWidget();
    componentTableWidget->hide();
    componentTableWidget->setToolTip("Component details");
    componentTableWidget->verticalHeader()->setVisible(false);
    componentTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    componentTableWidget->setSizeAdjustPolicy(QAbstractScrollArea::SizeAdjustPolicy::AdjustToContents);
    componentTableWidget->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Expanding);

    connect(componentTableWidget, &QTableWidget::cellChanged, this, &ComponentInputWidget::handleCellChanged);

    // Add a vertical spacer at the bottom to push everything up
    gridLayout->addItem(smallVSpacer,0,0,1,5);
    gridLayout->addWidget(topText,1,0,1,5);
    gridLayout->addWidget(pathText,2,0);
    gridLayout->addWidget(componentFileLineEdit,2,1);
    gridLayout->addWidget(browseFileButton,2,2);
    gridLayout->addItem(hspacer, 2, 4);
    gridLayout->addWidget(selectComponentsText, 3, 0, 1, 4);
    gridLayout->addWidget(selectComponentsLineEdit, 4, 0, 1, 2);
    gridLayout->addWidget(selectComponentsButton, 4, 2);
    gridLayout->addWidget(clearSelectionButton, 4, 3);
    gridLayout->addItem(smallVSpacer,5,0,1,5);
    gridLayout->addWidget(componentInfoText,6,0,1,5,Qt::AlignCenter);
    gridLayout->addWidget(componentTableWidget, 7, 0, 1, 5,Qt::AlignCenter);
    gridLayout->setRowStretch(8, 1);
    this->setLayout(gridLayout);
}


void ComponentInputWidget::setTheVisualizationWidget(VisualizationWidget *value)
{
    theVisualizationWidget = value;
}


void ComponentInputWidget::selectComponents(void)
{
    try
    {
        selectComponentsLineEdit->selectComponents();
    }
    catch (const QString msg)
    {
        this->userMessageDialog(msg);
    }
}


void ComponentInputWidget::handleComponentSelection(void)
{

    auto nRows = componentTableWidget->rowCount();

    if(nRows == 0)
        return;

    // Get the ID of the first and last component
    bool OK;
    auto firstID = componentTableWidget->item(0,0)->data(0).toInt(&OK);

    if(!OK)
    {
        QString msg = "Error in getting the component ID in " + QString(__FUNCTION__);
        this->userMessageDialog(msg);
        return;
    }

    auto lastID = componentTableWidget->item(nRows-1,0)->data(0).toInt(&OK);

    if(!OK)
    {
        QString msg = "Error in getting the component ID in " + QString(__FUNCTION__);
        this->userMessageDialog(msg);
        return;
    }

    auto selectedComponentIDs = selectComponentsLineEdit->getSelectedComponentIDs();

    // First check that all of the selected IDs are within range
    for(auto&& it : selectedComponentIDs)
    {
        if(it<firstID || it>lastID)
        {
            QString msg = "The component ID " + QString::number(it) + " is out of range of the components provided";
            this->userMessageDialog(msg);
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
    sendStatusMessage(msg);

    for(auto&& it : selectedComponentIDs)
    {
        auto component = theComponentDb.getComponent(it);

        auto feature = component.ComponentFeature;

        if(feature == nullptr)
            continue;

        auto atrb = feature->attributes()->attributesMap();
        auto id = atrb.value("UID").toString();

        if(selectedFeaturesForAnalysis.contains(id))
            continue;

        auto atrVals = atrb.values();
        auto atrKeys = atrb.keys();

        // qDebug()<<"Num atributes: "<<atrb.size();

        QMap<QString, QVariant> featureAttributes;
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

        // featureAttributes.insert("ID", "99");
        // featureAttributes.insert("LossRatio", 0.0);
        // featureAttributes.insert("AssetType", "BUILDING");
        // featureAttributes.insert("TabName", "99");
        // featureAttributes.insert("UID", "99");

        auto geom = feature->geometry();

        auto feat = this->addFeatureToSelectedLayer(featureAttributes,geom);

        if(feat)
            selectedFeaturesForAnalysis.insert(id,feat);
    }

    auto selecFeatLayer = this->getSelectedFeatureLayer();

    theVisualizationWidget->addSelectedFeatureLayerToMap(selecFeatLayer);

    //this->userMessageDialog(msg);
}


void ComponentInputWidget::clearLayerSelectedForAnalysis(void)
{
    if(selectedFeaturesForAnalysis.empty())
        return;

    for(auto&& it : selectedFeaturesForAnalysis)
    {
        this->removeFeatureFromSelectedLayer(it);
    }

    selectedFeaturesForAnalysis.clear();
}


void ComponentInputWidget::clearComponentSelection(void)
{
    auto selectedComponentIDs = selectComponentsLineEdit->getSelectedComponentIDs();

    QList<Esri::ArcGISRuntime::Feature*> selectedFeatures;
    for(auto&& it : selectedComponentIDs)
    {
        auto component = theComponentDb.getComponent(it);

        auto feature = component.ComponentFeature;

        selectedFeatures<<feature;
    }

    this->clearLayerSelectedForAnalysis();


    auto nRows = componentTableWidget->rowCount();

    // Hide all rows in the table
    for(int i = 0; i<nRows; ++i)
    {
        componentTableWidget->setRowHidden(i,false);
    }

    selectComponentsLineEdit->clear();

}


void ComponentInputWidget::setLabel1(const QString &value)
{
    label1 = value;
}


void ComponentInputWidget::setLabel2(const QString &value)
{
    label2 = value;
}


void ComponentInputWidget::setLabel3(const QString &value)
{
    label3 = value;
}


void ComponentInputWidget::setGroupBoxText(const QString &value)
{
    componentGroupBox->setTitle(value);
}


void ComponentInputWidget::setComponentType(const QString &value)
{
    componentType = value;
}


void ComponentInputWidget::insertSelectedComponent(const int ComponentID)
{
    selectComponentsLineEdit->insertSelectedCompoonent(ComponentID);
}


int ComponentInputWidget::numberComponentsSelected(void)
{
    return selectComponentsLineEdit->size();
}


QString ComponentInputWidget::getPathToComponentFile(void) const
{
    return pathToComponentInfoFile;
}


void ComponentInputWidget::loadFileFromPath(QString& path)
{
    this->clear();
    pathToComponentInfoFile = path;
    componentFileLineEdit->setText(path);
    this->loadComponentData();
}


bool ComponentInputWidget::outputAppDataToJSON(QJsonObject &jsonObject)
{
    jsonObject["Application"]=appType;

    QJsonObject data;
    QFileInfo componentFile(componentFileLineEdit->text());
    if (componentFile.exists()) {
        data["buildingSourceFile"]=componentFile.fileName();
        data["pathToSource"]=componentFile.path();

        QString filterData = selectComponentsLineEdit->getComponentAnalysisList();

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
            emit sendErrorMessage("ComponentINputWidget::inputFRommJSON app name conflict");
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

            pathToComponentInfoFile = pathToFile + QDir::separator() + fileName;

            if (fileInfo.exists(pathToComponentInfoFile)) {
                componentFileLineEdit->setText(pathToComponentInfoFile);
                foundFile = true;
                this->loadComponentData();

            } else {
                // adam .. adam .. adam
                pathToComponentInfoFile = pathToFile + QDir::separator()
                        + "input_data" + QDir::separator() + fileName;
                if (fileInfo.exists(pathToComponentInfoFile)) {
                    componentFileLineEdit->setText(pathToComponentInfoFile);
                    foundFile = true;
                    this->loadComponentData();
                }
            }
        }

        if (appData.contains("filter"))
            selectComponentsLineEdit->setText(appData["filter"].toString());

        if (foundFile == true)
            selectComponentsLineEdit->selectComponents();
        else {
            QString errMessage = appType + "no file found" + fileName;
            emit sendErrorMessage(errMessage);
            return false;
        }
    }

    QString errMessage = appType + "no ApplicationDta found";
    emit sendErrorMessage(errMessage);
    return false;

}


bool ComponentInputWidget::outputToJSON(QJsonObject &rvObject)
{
    Q_UNUSED(rvObject);
    return true;
}


bool ComponentInputWidget::inputFromJSON(QJsonObject &rvObject)
{
    Q_UNUSED(rvObject);
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
    auto nCols = componentTableWidget->columnCount();

    if(nRows == 0)
        return false;


    QVector<QStringList> data(nRows+1);

    QStringList headerInfo;

    for(int i = 0; i<nCols; ++i)
    {
        auto headerText = componentTableWidget->horizontalHeaderItem(i)->data(0).toString();
        headerInfo << headerText;
    }

    data[0] = headerInfo;

    for(int i = 0; i<nRows; ++i)
    {
        QStringList rowData;
        rowData.reserve(nCols);

        for(int j = 0; j<nCols; ++j)
        {
            auto item = componentTableWidget->item(i,j)->data(0).toString();

            rowData<<item;
        }
        data[i+1] = rowData;
    }

    CSVReaderWriter csvTool;

    QString err;
    csvTool.saveCSVFile(data,pathToSaveFile,err);

    if(!err.isEmpty())
        return false;


    // Creates a csv file of only the selected components
    //     auto selectedIDs = selectComponentsLineEdit->getSelectedComponentIDs();

    //     QVector<QStringList> selectedData(selectedIDs.size()+1);

    //     selectedData[0] = headerInfo;

    //     int i = 0;
    //     for(auto&& rowID : selectedIDs)
    //     {
    //         QStringList rowData;
    //         rowData.reserve(nCols);

    //         for(int j = 0; j<nCols; ++j)
    //         {
    //             auto item = componentTableWidget->item(rowID,j)->data(0).toString();

    //             rowData<<item;
    //         }
    //         selectedData[i+1] = rowData;

    //         ++i;
    //     }

    //     csvTool.saveCSVFile(selectedData,"/Users/steve/Desktop/Selected.csv",err);


    return true;
}


void ComponentInputWidget::clear(void)
{
    theComponentDb.clear();
    pathToComponentInfoFile.clear();
    componentFileLineEdit->clear();
    selectComponentsLineEdit->clear();
    componentTableWidget->clear();
    componentTableWidget->hide();
}


ComponentDatabase* ComponentInputWidget::getComponentDatabase()
{
    return &theComponentDb;
}


void ComponentInputWidget::handleCellChanged(int row, int column)
{
    // Cannot change the ID, lat, or long
    if(column < 3)
        return;

    auto item = componentTableWidget->item(row,column);

    // Only update when the user selects the cell and changes it, inefficient to update programatically
    if(!item->isSelected())
        return;

    auto ID = componentTableWidget->item(row,0)->data(0).toInt();

    auto attrib = componentTableWidget->horizontalHeaderItem(column)->data(0).toString();

    auto attribVal = item->data(0);

    theComponentDb.updateComponentAttribute(ID,attrib,attribVal);

    auto component = theComponentDb.getComponent(ID);

    if(!component.isValid())
        return;

    auto uid = component.UID;
    this->updateSelectedComponentAttribute(uid,attrib,attribVal);

}


Feature* ComponentInputWidget::addFeatureToSelectedLayer(QMap<QString, QVariant>& /*featureAttributes*/, Geometry& /*geom*/)
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


void ComponentInputWidget::updateComponentAttribute(const int uid, const QString& attribute, const QVariant& value)
{
    theComponentDb.updateComponentAttribute(uid,attribute,value);
}


void ComponentInputWidget::updateSelectedComponentAttribute(const QString&  uid, const QString& attribute, const QVariant& value)
{
    if(selectedFeaturesForAnalysis.empty())
    {
        qDebug()<<"Selected features map is empty";
        return;
    }

    if(!selectedFeaturesForAnalysis.contains(uid))
    {
        qDebug()<<"Feature not found in selected components map";
        return;
    }

    // Get the feature
    Feature* feat = selectedFeaturesForAnalysis[uid];

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
