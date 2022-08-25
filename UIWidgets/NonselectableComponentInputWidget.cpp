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
#include "NonselectableComponentInputWidget.h"
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

#include "QGISVisualizationWidget.h"

// Std library headers
#include <string>
#include <algorithm>


NonselectableComponentInputWidget::NonselectableComponentInputWidget(QWidget *parent, ComponentDatabase* compDB, QGISVisualizationWidget* visWidget, QString componentType) : SimCenterAppWidget(parent), theVisualizationWidget(visWidget), theComponentDb(compDB), componentType(componentType)
{
    offset = 0;

    appType = componentType.simplified();
    appType.replace( " ", "" );

    this->setContentsMargins(0,0,0,0);

    pathToComponentInputFile = "NULL";
    componentGroupBox = nullptr;
    componentFileLineEdit = nullptr;

    this->createComponentsBox();

    auto txt1 = "Load information from a CSV file";

    auto txt2 = QStringRef(&componentType, 0, componentType.length()-1) + " Information";

    label1->setText(txt1);
    label2->setText(txt2);

}


NonselectableComponentInputWidget::~NonselectableComponentInputWidget()
{
    
}


bool NonselectableComponentInputWidget::isEmpty()
{
    if(componentTableWidget->rowCount() == 0)
        return true;

    return false;
}

bool NonselectableComponentInputWidget::loadComponentData(void)
{
    // Ask for the file path if the file path has not yet been set, and return if it is still null
    if(pathToComponentInputFile.compare("NULL") == 0)
        this->chooseComponentInfoFileDialog();
    
    if(pathToComponentInputFile.compare("NULL") == 0)
        return false;
    
    // Check if the directory exists
    QFileInfo file(pathToComponentInputFile);
    
    if (!file.exists() || !file.isFile())
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
    
    label2->show();
    componentTableWidget->show();
    componentTableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
    
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


void NonselectableComponentInputWidget::chooseComponentInfoFileDialog(void)
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


void NonselectableComponentInputWidget::setPathToComponentInputFile(const QString &newPathToComponentInputFile)
{
    pathToComponentInputFile = newPathToComponentInputFile;
    componentFileLineEdit->setText(pathToComponentInputFile);
}


ComponentTableView *NonselectableComponentInputWidget::getTableWidget() const
{
    return componentTableWidget;
}


void NonselectableComponentInputWidget::createComponentsBox(void)
{
    componentGroupBox = new QGroupBox(componentType);
    componentGroupBox->setFlat(true);
    componentGroupBox->setContentsMargins(0,0,0,0);
    
    QVBoxLayout* gridLayout = new QVBoxLayout();
    gridLayout->setMargin(0);
    gridLayout->setSpacing(5);
    gridLayout->setContentsMargins(10,10,0,0);
    
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
    

    // Text label for Component information
    label2 = new QLabel();
    label2->setStyleSheet("font-weight: bold; color: black");
    label2->hide();
    
    // Create the table that will show the Component information
    componentTableWidget = new ComponentTableView();
    
    connect(componentTableWidget->getTableModel(), &ComponentTableModel::handleCellChanged, this, &NonselectableComponentInputWidget::handleCellChanged);

    QHBoxLayout* pathLayout = new QHBoxLayout();
    pathLayout->addWidget(pathText);
    pathLayout->addWidget(componentFileLineEdit);
    pathLayout->addWidget(browseFileButton);
    
    // Add a vertical spacer at the bottom to push everything up
    gridLayout->addWidget(label1);
    gridLayout->addLayout(pathLayout);

    gridLayout->addWidget(label2,0,Qt::AlignCenter);
    gridLayout->addWidget(componentTableWidget,0,Qt::AlignCenter);
    
    gridLayout->addStretch();

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    mainLayout->addWidget(componentGroupBox);
}



QStringList NonselectableComponentInputWidget::getTableHorizontalHeadings()
{
    return tableHorizontalHeadings;
}



void NonselectableComponentInputWidget::setLabel1(const QString &value)
{
    label1->setText(value);
}


void NonselectableComponentInputWidget::setLabel2(const QString &value)
{
    label2->setText(value);
}


void NonselectableComponentInputWidget::setGroupBoxText(const QString &value)
{
    componentGroupBox->setTitle(value);
    componentGroupBox->setVisible(true);
}


void NonselectableComponentInputWidget::setComponentType(const QString &value)
{
    componentType = value;
}



QString NonselectableComponentInputWidget::getPathToComponentFile(void) const
{
    return pathToComponentInputFile;
}


bool NonselectableComponentInputWidget::outputAppDataToJSON(QJsonObject &jsonObject)
{

    QFileInfo componentFile(componentFileLineEdit->text());
    if (componentFile.exists()) {
        jsonObject[appType+"SourceFile"]=componentFile.fileName();
        jsonObject["pathTo"+appType+"Source"]=componentFile.path();

    }
    else
    {
        jsonObject[appType+"SourceFile"]=QString("None");
        jsonObject["pathTo"+appType+"Source"]=QString("");
        return false;
    }

    return true;
}


bool NonselectableComponentInputWidget::inputAppDataFromJSON(QJsonObject &jsonObject)
{

    QFileInfo fileInfo;
    QString fileName;
    QString pathToFile;
    bool foundFile = false;
    if (jsonObject.contains(appType+"SourceFile"))
        fileName = jsonObject[appType+"SourceFile"].toString();

    if (fileInfo.exists(fileName)) {

        pathToComponentInputFile = fileName;
        componentFileLineEdit->setText(fileName);

        this->loadComponentData();
        foundFile = true;

    } else {

        if (jsonObject.contains("pathTo"+appType+"Source"))
            pathToFile = jsonObject["pathTo"+appType+"Source"].toString();
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


    return true;
}


bool NonselectableComponentInputWidget::outputToJSON(QJsonObject &rvObject)
{
#ifdef OpenSRA
    locationWidget->outputToJSON(rvObject);
#else
    Q_UNUSED(rvObject);
#endif
    return true;
}


bool NonselectableComponentInputWidget::inputFromJSON(QJsonObject &rvObject)
{
#ifdef OpenSRA
    locationWidget->inputFromJSON(rvObject);
#else
    Q_UNUSED(rvObject);
#endif

    return true;
}


bool NonselectableComponentInputWidget::copyFiles(QString &destName)
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

    //    selectedData[0] = headerValues;

    //    auto nCols = componentTableWidget->columnCount();

    //    int i = 0;
    //    for(auto&& rowID : selectedIDs)
    //    {
    //        QStringList rowData;
    //        rowData.reserve(nCols);

    //        for(int j = 0; j<nCols; ++j)
    //        {
    //            auto item = componentTableWidget->item(rowID-1,j).toString();

    //            rowData<<item;
    //        }
    //        selectedData[i+1] = rowData;

    //        ++i;
    //    }

    //    csvTool.saveCSVFile(selectedData,"/Users/steve/Desktop/Selected.csv",err);
    // For testing end

    return true;
}


void NonselectableComponentInputWidget::clear(void)
{
    theComponentDb->clear();
    pathToComponentInputFile.clear();
    componentFileLineEdit->clear();
    componentTableWidget->clear();
    componentTableWidget->hide();
    tableHorizontalHeadings.clear();

    emit headingValuesChanged(QStringList{"N/A"});
}


void NonselectableComponentInputWidget::handleCellChanged(const int row, const int col)
{
    auto ID = componentTableWidget->item(row,0).toInt();

    auto attrib = componentTableWidget->horizontalHeaderItem(col);

    auto attribVal = componentTableWidget->item(row,col);

    theComponentDb->updateComponentAttribute(ID,attrib,attribVal);
}



