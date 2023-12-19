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
#include "AssetFilterDelegate.h"
#include "AssetInputWidget.h"
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


// Std library headers
#include <string>
#include <algorithm>

AssetInputWidget::AssetInputWidget(QWidget *parent, VisualizationWidget* visWidget, QString assetType, QString appType) : SimCenterAppWidget(parent), appType(appType), assetType(assetType)
{

    offset = 0;

    theVisualizationWidget = static_cast<QGISVisualizationWidget*>(visWidget);
    assert(theVisualizationWidget);

    this->setContentsMargins(0,0,0,0);

    pathToComponentInputFile = "NULL";

    AssetInputWidget::createComponentsBox();

    auto txt1 = "Load information from a CSV file";
    auto txt2  = "Enter the IDs of one or more " + assetType.toLower() + " to analyze."
                                                                         "\nDefine a range of " + assetType.toLower() + " with a dash and separate multiple " + assetType.toLower() + " with a comma.";

    auto txt3 = QStringRef(&assetType, 0, assetType.length()-1) + " Information";

#ifdef OpenSRA
    label1->setText(txt1);
    label2->setText(txt2);
    label3->setText(txt3);
#endif

    theComponentDb = ComponentDatabaseManager::getInstance()->createAssetDb(assetType);

    if(theComponentDb == nullptr)
        this->errorMessage("Could not find the component database of the type "+assetType);
}


AssetInputWidget::~AssetInputWidget()
{
    
}


bool AssetInputWidget::loadAssetData(bool message)
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

    // Clear the old layers if any
    if(mainLayer != nullptr)
        theVisualizationWidget->removeLayer(mainLayer);

    if(selectedFeaturesLayer != nullptr)
        theVisualizationWidget->removeLayer(selectedFeaturesLayer);
    
    auto res = this->loadAssetVisualization();

    if(res != 0)
        return false;

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

    emit doneLoadingComponents();
    
    return true;
}


void AssetInputWidget::chooseComponentInfoFileDialog(void)
{
    auto newPathToComponentInputFile = QFileDialog::getOpenFileName(this,tr("Component Information File"));
    
    // Return if the user cancels
    if(newPathToComponentInputFile.isEmpty())
    {
        pathToComponentInputFile = "NULL";
        return;
    }

    this->clearTableData();

    pathToComponentInputFile = newPathToComponentInputFile;

    // Set file name & entry in qLine edit
    componentFileLineEdit->setText(pathToComponentInputFile);
    
    this->loadAssetData();
    
    return;
}


ComponentTableView *AssetInputWidget::getTableWidget() const
{
    return componentTableWidget;
}


void AssetInputWidget::createComponentsBox(void)
{
  
#ifdef OpenSRA 
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
    pathText->setText("Assets to Analyze:");
    
    componentFileLineEdit = new QLineEdit();
    // componentFileLineEdit->setMaximumWidth(750);
    // componentFileLineEdit->setMinimumWidth(400);
    // componentFileLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);


    browseFileButton = new QPushButton();
    browseFileButton->setText(tr("Browse"));
    browseFileButton->setMaximumWidth(150);
    
    connect(browseFileButton,SIGNAL(clicked()),this,SLOT(chooseComponentInfoFileDialog()));
    
    // Add a horizontal spacer after the browse and load buttons
    //    auto hspacer = new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Minimum);
    
    label2 = new QLabel();
    
    selectComponentsLineEdit = new AssetInputDelegate();
    connect(selectComponentsLineEdit,&AssetInputDelegate::componentSelectionComplete,this,&AssetInputWidget::handleComponentSelection);
    connect(selectComponentsLineEdit,&QLineEdit::editingFinished,this,&AssetInputWidget::selectComponents);
    
    QPushButton *clearSelectionButton = new QPushButton();
    clearSelectionButton->setText(tr("Clear Selection"));
    clearSelectionButton->setMaximumWidth(150);
    
    connect(clearSelectionButton,SIGNAL(clicked()),this,SLOT(clearComponentSelection()));

    QPushButton *filterExpressionButton = new QPushButton();
    filterExpressionButton->setText(tr("Advanced Filter"));
    filterExpressionButton->setMaximumWidth(150);
    connect(filterExpressionButton,SIGNAL(clicked()),this,SLOT(handleComponentFilter()));
    
    // Text label for Component information
    label3 = new QLabel();
    label3->setStyleSheet("font-weight: bold; color: black");
    label3->hide();
    
    // Create the table that will show the Component information
    componentTableWidget = new ComponentTableView();
    
    connect(componentTableWidget->getTableModel(), &ComponentTableModel::handleCellChanged, this, &AssetInputWidget::handleCellChanged);
    
    pathLayout = new QHBoxLayout();
    pathLayout->addWidget(pathText);
    pathLayout->addWidget(componentFileLineEdit);
    pathLayout->addWidget(browseFileButton);
    
    // Add a vertical spacer at the bottom to push everything up
    mainWidgetLayout->addWidget(label1);
    mainWidgetLayout->addLayout(pathLayout);

    filterWidget = new QWidget();
    QHBoxLayout* selectComponentsLayout = new QHBoxLayout(filterWidget);
    selectComponentsLayout->addWidget(label2);
    selectComponentsLayout->addWidget(selectComponentsLineEdit);
    selectComponentsLayout->addWidget(filterExpressionButton);
    selectComponentsLayout->addWidget(clearSelectionButton);

    // hide selection part
    selectComponentsLineEdit->setText("1");
    selectComponentsLineEdit->hide();
    clearSelectionButton->hide();
    filterExpressionButton->hide();

    mainWidgetLayout->addWidget(label3,0,Qt::AlignCenter);
    mainWidgetLayout->addWidget(componentTableWidget,0,Qt::AlignCenter);
    
    mainWidgetLayout->addStretch();

#else // using GridLayout
    
    componentGroupBox = new QGroupBox(assetType);
    componentGroupBox->setFlat(true);
    componentGroupBox->setContentsMargins(0,0,0,0);
    
    mainWidgetLayout = new QGridLayout();
    
    // mainWidgetLayout->setMargin(0);
    // mainWidgetLayout->setSpacing(5);
    // mainWidgetLayout->setContentsMargins(10,0,0,0);
    
    componentGroupBox->setLayout(mainWidgetLayout);
    
    label1 = new QLabel();
    label1->setText("Asset File:");
    
    // QLabel* pathText = new QLabel();
    // pathText->setText("Path to file:");
    
    componentFileLineEdit = new QLineEdit();
    // componentFileLineEdit->setMaximumWidth(750);
    // componentFileLineEdit->setMinimumWidth(400);
    // componentFileLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);


    browseFileButton = new QPushButton("Browse");
    //    browseFileButton->setText(tr("Browse"));
    browseFileButton->setMaximumWidth(150);
    
    connect(browseFileButton,SIGNAL(clicked()),this,SLOT(chooseComponentInfoFileDialog()));
    
    // Add a horizontal spacer after the browse and load buttons
    //    auto hspacer = new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Minimum);
    
    label2 = new QLabel();
    label2->setText("Assets to Analyze:");
    selectComponentsLineEdit = new AssetInputDelegate();
    connect(selectComponentsLineEdit,&AssetInputDelegate::componentSelectionComplete,this,&AssetInputWidget::handleComponentSelection);
    connect(selectComponentsLineEdit,&QLineEdit::editingFinished,this,&AssetInputWidget::selectComponents);
    
    QPushButton *clearSelectionButton = new QPushButton();
    clearSelectionButton->setText(tr("Clear Selection"));
    clearSelectionButton->setMaximumWidth(150);
    
    connect(clearSelectionButton,SIGNAL(clicked()),this,SLOT(clearComponentSelection()));

    QPushButton *filterExpressionButton = new QPushButton();
    filterExpressionButton->setText(tr("Advanced Filter"));
    filterExpressionButton->setMaximumWidth(150);
    connect(filterExpressionButton,SIGNAL(clicked()),this,SLOT(handleComponentFilter()));
    
    // Text label for Component information
    label3 = new QLabel();
    label3->setStyleSheet("font-weight: bold; color: black");
    label3->hide();
    
    // Create the table that will show the Component information
    componentTableWidget = new ComponentTableView();
    
    connect(componentTableWidget->getTableModel(), &ComponentTableModel::handleCellChanged, this, &AssetInputWidget::handleCellChanged);

    mainWidgetLayout->addWidget(label1, 1,0);
    mainWidgetLayout->addWidget(componentFileLineEdit, 1,1);
    mainWidgetLayout->addWidget(browseFileButton, 1,2);    
    
    //pathLayout = new QHBoxLayout();
    //pathLayout->addWidget(pathText);
    //pathLayout->addWidget(componentFileLineEdit);
    //pathLayout->addWidget(browseFileButton);
    
    // Add a vertical spacer at the bottom to push everything up
    // mainWidgetLayout->addWidget(label1);
    //    mainWidgetLayout->addLayout(pathLayout);

    //    filterWidget = new QWidget();
    //QHBoxLayout* selectComponentsLayout = new QHBoxLayout(filterWidget);
    //selectComponentsLayout->addWidget(label2);
    // selectComponentsLayout->addWidget(selectComponentsLineEdit);
    // selectComponentsLayout->addWidget(filterExpressionButton);
    //selectComponentsLayout->addWidget(clearSelectionButton);
    mainWidgetLayout->addWidget(label2, 2,0);
    mainWidgetLayout->addWidget(selectComponentsLineEdit, 2,1);
    mainWidgetLayout->addWidget(filterExpressionButton, 2,2);
    mainWidgetLayout->addWidget(clearSelectionButton, 2,3);
    // mainWidgetLayout->addWidget(label3,0,Qt::AlignCenter);
    
    mainWidgetLayout->addWidget(componentTableWidget,3, 0, 1,4);
    mainWidgetLayout->setRowStretch(4,1);
    
    //    mainWidgetLayout->addStretch();

#endif

    this->setLayout(mainWidgetLayout);    
}


QgsVectorLayer *AssetInputWidget::getMainLayer() const
{
    return mainLayer;
}

QgsVectorLayer *AssetInputWidget::getSelectedLayer() const
{
    return selectedFeaturesLayer;
}


void AssetInputWidget::selectComponents(void)
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


void AssetInputWidget::handleComponentSelection(void)
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



QStringList AssetInputWidget::getTableHorizontalHeadings()
{
    return tableHorizontalHeadings;
}


void AssetInputWidget::clearComponentSelection(void)
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


void AssetInputWidget::setLabel1(const QString &value)
{
#ifdef OpenSRA
  label1->setText(value);
#endif
}


void AssetInputWidget::setLabel2(const QString &value)
{
#ifdef OpenSRA  
  label2->setText(value);
#endif  
}


void AssetInputWidget::setLabel3(const QString &value)
{
#ifdef OpenSRA    
  label3->setText(value);
#endif  
}


void AssetInputWidget::setGroupBoxText(const QString &value)
{
    componentGroupBox->setTitle(value);
}


void AssetInputWidget::setComponentType(const QString &value)
{
    assetType = value;
}


int AssetInputWidget::numberComponentsSelected(void)
{
    return selectComponentsLineEdit->size();
}


QString AssetInputWidget::getPathToComponentFile(void) const
{
    return pathToComponentInputFile;
}


void AssetInputWidget::setPathToComponentFile(const QString& path)
{
    pathToComponentInputFile = path;

    componentFileLineEdit->setText(pathToComponentInputFile);
}


bool AssetInputWidget::outputAppDataToJSON(QJsonObject &jsonObject)
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


bool AssetInputWidget::inputAppDataFromJSON(QJsonObject &jsonObject)
{

    //jsonObject["Application"]=appType;
    if (jsonObject.contains("Application")) {
        if (appType != jsonObject["Application"].toString()) {
            this->errorMessage("AssetInputWidget::inputFRommJSON app name conflict");
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


void AssetInputWidget::setFilterString(const QString& filter)
{
    selectComponentsLineEdit->setText(filter);
    selectComponentsLineEdit->selectComponents();
}


QString AssetInputWidget::getFilterString(void)
{
    QString filterData = selectComponentsLineEdit->getComponentAnalysisList();

    return filterData;
}


void AssetInputWidget::selectAllComponents(void)
{
    // Get the ID of the first and last component
    auto firstID = componentTableWidget->item(0,0).toString();

    auto nRows = componentTableWidget->rowCount();
    auto lastID = componentTableWidget->item(nRows-1,0).toString();

    QString filter = firstID + "-" + lastID;

    selectComponentsLineEdit->setText(filter);
    selectComponentsLineEdit->selectComponents();
}


bool AssetInputWidget::outputToJSON(QJsonObject &rvObject)
{
    Q_UNUSED(rvObject);

    return true;
}


bool AssetInputWidget::inputFromJSON(QJsonObject &rvObject)
{
    Q_UNUSED(rvObject);

    return true;
}


bool AssetInputWidget::copyFiles(QString &destName)
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

    pathToSaveFile += ".csv";

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

    // Put this here because copy files gets called first and we need to select the components before we can create the input file
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

        if(filterData.isEmpty())
        {
            errorMessage("Error selecting components for analysis");
            return false;
        }
    }

    //     For testing, creates a csv file of only the selected components
    //        qDebug()<<"Saving selected components to .csv";
    //        auto selectedIDs = selectComponentsLineEdit->getSelectedComponentIDs();

    //        QVector<QStringList> selectedData(selectedIDs.size()+1);

    //        selectedData[0] = headerValues;

    //        auto nCols = componentTableWidget->columnCount();

    //        int i = 0;
    //        for(auto&& rowID : selectedIDs)
    //        {
    //            QStringList rowData;
    //            rowData.reserve(nCols);

    //            for(int j = 0; j<nCols; ++j)
    //            {
    //                auto item = componentTableWidget->item(rowID-1,j).toString();

    //                rowData<<item;
    //            }
    //            selectedData[i+1] = rowData;

    //            ++i;
    //        }

    //        csvTool.saveCSVFile(selectedData,"/Users/steve/Desktop/Selected.csv",err);
    //     For testing end

    return true;
}


void AssetInputWidget::clearTableData(void)
{
    theComponentDb->clear();
    pathToComponentInputFile.clear();
    componentFileLineEdit->clear();
    selectComponentsLineEdit->clear();
    filterDelegateWidget->clear();
    componentTableWidget->clear();
    componentTableWidget->hide();
    tableHorizontalHeadings.clear();
}



void AssetInputWidget::clear(void)
{
    this->clearTableData();

    mainLayer = nullptr;
    selectedFeaturesLayer = nullptr;

    emit headingValuesChanged(QStringList{"N/A"});
}


void AssetInputWidget::handleCellChanged(const int row, const int col)
{
    auto ID = componentTableWidget->item(row,0).toInt();

    auto attrib = componentTableWidget->horizontalHeaderItem(col);

    auto attribVal = componentTableWidget->item(row,col);


    auto res = theComponentDb->updateComponentAttribute(ID,attrib,attribVal);
    if(res == false)
        this->errorMessage("Error could not update asset "+QString::number(ID)+" after cell change");

}


void AssetInputWidget::insertSelectedAssets(QgsFeatureIds& featureIds)
{

    QVector<int> assetIds;
    assetIds.reserve(featureIds.size());

    for(auto&& it : featureIds)
        assetIds.push_back(it-offset);

    selectComponentsLineEdit->insertSelectedComponents(assetIds);

    this->selectComponents();
}


void AssetInputWidget::clearSelectedAssets(void)
{
    this->clearComponentSelection();
}


void AssetInputWidget::handleComponentFilter(void)
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


void AssetInputWidget::setPathToComponentInputFile(const QString &newPathToComponentInputFile)
{
    pathToComponentInputFile = newPathToComponentInputFile;
}


QLabel *AssetInputWidget::getLabel1() const
{
    return label1;
}


QHBoxLayout *AssetInputWidget::getAssetFilePathLayout() const
{
    return pathLayout;
}


QString AssetInputWidget::getAssetType() const
{
    return assetType;
}


int AssetInputWidget::applyFilterString(const QString& filter)
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


bool AssetInputWidget::isEmpty()
{
    if(componentTableWidget->rowCount() == 0)
        return true;

    return false;
}


int AssetInputWidget::getNumberOfAseets(void)
{
    return componentTableWidget->rowCount();
}


void AssetInputWidget::setFilterVisibility(const bool value)
{
    filterWidget->setVisible(value);
}

void AssetInputWidget::hideCRS_Selection() const
{
    int rowAssetPath = 0; // I don't like this .. crs row should be defined and hidden .. the inut takes a layout
    for (int i=0; i<mainWidgetLayout->columnCount(); i++) {
        QLayoutItem *item = mainWidgetLayout->itemAtPosition(rowAssetPath,i);
        if (item != nullptr)
            item->widget()->hide();
    }
}

void AssetInputWidget::hideAssetFilePath() const
{
    int rowAssetPath = 1; // again i don't like this solution
    for (int i=0; i<mainWidgetLayout->columnCount(); i++) {
        QLayoutItem *item = mainWidgetLayout->itemAtPosition(rowAssetPath,i);
        if (item != nullptr)
            item->widget()->hide();
    }
}
