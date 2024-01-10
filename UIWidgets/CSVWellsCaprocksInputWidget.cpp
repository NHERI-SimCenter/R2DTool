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
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
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

#include "CSVWellsCaprocksInputWidget.h"
#include "QGISVisualizationWidget.h"
#include "ComponentTableView.h"
#include "ComponentDatabaseManager.h"
#include "AssetFilterDelegate.h"
#include "LineAssetInputWidget.h"

#include <QFileInfo>
#include <QFileDialog>
#include <QPushButton>

#include <qgsfield.h>
#include <qgsfillsymbol.h>
#include <qgsvectorlayer.h>
#include <qgsmarkersymbol.h>

//#ifdef OpenSRA
//#include "WorkflowAppOpenSRA.h"
//#include "WidgetFactory.h"
//#include "JsonGroupBoxWidget.h"
//#endif


CSVWellsCaprocksInputWidget::CSVWellsCaprocksInputWidget(QWidget *parent, VisualizationWidget* visWidget, QString assetType, QString appType) : PointAssetInputWidget(parent, visWidget, assetType, appType)
{
    CSVWellsCaprocksInputWidget::createComponentsBox();
}


void CSVWellsCaprocksInputWidget::createComponentsBox(void)
{

    auto inputWidget = new QWidget();
    QVBoxLayout* inputLayout = new QVBoxLayout(inputWidget);

    // top of well lat lon headers
//    auto methodsAndParams = WorkflowAppOpenSRA::getInstance()->getMethodsAndParamsObj();

//    QJsonObject thisObj = methodsAndParams["Infrastructure"].toObject()["PointAsset"].toObject()["SiteLocationParams"].toObject();

//    if(thisObj.isEmpty())
//    {
//        this->errorMessage("Json object is empty in " + QString(__FUNCTION__));
//        return;
//    }

//    auto theWidgetFactory = std::make_unique<WidgetFactory>(this);

//    QJsonObject paramsObj = thisObj["Params"].toObject();

//    // The string given in the Methods and params json file
//    QString nameStr = "SiteLocationParams";

//    auto widgetLabelText = thisObj["NameToDisplay"].toString();

//    if(widgetLabelText.isEmpty())
//    {
//        this->errorMessage("Could not find the *NameToDisplay* key in object json for " + nameStr);
//        return;
//    }

//    locationWidget = new JsonGroupBoxWidget(this);
//    locationWidget->setObjectName(nameStr);

//    locationWidget->setTitle(widgetLabelText);

//    QJsonObject paramsLat;
//    paramsLat["Lat"] = paramsObj.value("Lat");

//    QJsonObject paramsLon;
//    paramsLon["Lon"] = paramsObj.value("Lon");

//    auto latLayout = theWidgetFactory->getLayoutFromParams(paramsLat,nameStr,locationWidget, Qt::Horizontal);
//    auto lonLayout = theWidgetFactory->getLayoutFromParams(paramsLon,nameStr,locationWidget, Qt::Horizontal);

//    QHBoxLayout* latLonLayout = new QHBoxLayout();
//    latLonLayout->addLayout(latLayout);
//    latLonLayout->addLayout(lonLayout);

//    locationWidget->setLayout(latLonLayout);

//    inputLayout->addWidget(locationWidget);

    // box for LON and LAT headers in file
//    QGroupBox* locationWidget = new QGroupBox("Note: Headers to use for 'Latitude' and 'Longitude' in CSV files");
//    QGridLayout* vboxLayout = new QGridLayout(locationWidget);
//    QLabel* lonLabel = new QLabel("1. Header to use for longitude: LON");
//    QLabel* latLabel = new QLabel("2. Header to use for latitude: LAT");
//    vboxLayout->addWidget(lonLabel,0,0,Qt::AlignLeft);
//    vboxLayout->addWidget(latLabel,1,0,Qt::AlignLeft);
//    inputLayout->addWidget(locationWidget);


    // well traces
    QHBoxLayout* welltraceLayout = new QHBoxLayout();

    QLabel* pathWellTraceLabel = new QLabel("Directory containing well traces:");

    QPushButton* pathWellTraceButton = new QPushButton();
    pathWellTraceButton->setText(tr("Browse"));
    pathWellTraceButton->setMaximumWidth(150);
    connect(pathWellTraceButton,&QPushButton::clicked,this,&CSVWellsCaprocksInputWidget::handleWellTracesDirDialog);

    pathWellTraceLE = new QLineEdit();

    welltraceLayout->addWidget(pathWellTraceLabel);
    welltraceLayout->addWidget(pathWellTraceLE);
    welltraceLayout->addWidget(pathWellTraceButton);

    inputLayout->addLayout(welltraceLayout);


    // caprock input
    QHBoxLayout* caprockLayout = new QHBoxLayout();

    QLabel* pathCaprockShp = new QLabel("Path to caprock shapefile or folder with shapefile:");

    QPushButton* pathCaprockShpButton = new QPushButton();
    pathCaprockShpButton->setText(tr("Browse"));
    pathCaprockShpButton->setMaximumWidth(150);
    connect(pathCaprockShpButton,&QPushButton::clicked,this,&CSVWellsCaprocksInputWidget::handleCaprockDialog);

    pathCaprockShpLE = new QLineEdit();

    caprockLayout->addWidget(pathCaprockShp);
    caprockLayout->addWidget(pathCaprockShpLE);
    caprockLayout->addWidget(pathCaprockShpButton);

    inputLayout->addLayout(caprockLayout);


    // notes
    QLabel* notesLabel = new QLabel(
        "Instructions:\n"
        "\t1. In the CSV file:\n"
        "\t\t- Required header to use for longitude: LON\n"
        "\t\t- Required header to use for latitude: LAT\n"
        "\t\t- File must contain a column called \"FILE_NAME\" that links to the well trace files\n"
        "\t2. For the directory with well traces:\n"
        "\t\t- The folder must contain a list of \".txt\" files with file names consistent with the \"FILE_NAME\" column in the CSV file.\n"
        "\t\t- Each text file should contain 3 columns with the following header names delimited by tabs (not commas): lon lat depth_m\n"
        "\t\t- Each trace file should contain at the minimum two points (top and bottom). If the actual trace is not known, then assign the lat/lon at the top of the well to the bottom node and assume a vertical well.\n"
        "\t3. For the caprock shapefile:\n"
        "\t\t- The shapefile must contain the attribute \"depth_m\" (average depth to caprock)"
    );
    inputLayout->addWidget(notesLabel);


    // Insert the widget three rows from the bottom
    insertWidgetIntoLayout(inputWidget,3);

}



void CSVWellsCaprocksInputWidget::handleWellTracesDirDialog(void)
{
    auto newPathToInputFile = QFileDialog::getExistingDirectory(this,tr("Directory containing well traces"));

    // Return if the user cancels
    if(newPathToInputFile.isEmpty())
        return;

    pathWellTraceLE->setText(newPathToInputFile);

}


bool CSVWellsCaprocksInputWidget::inputFromJSON(QJsonObject &rvObject)
{
    auto pathWellTraceDir = rvObject.value("WellTraceDir").toString();

    // well trace
    if(pathWellTraceDir.isEmpty())
    {
        this->errorMessage("Error, the required input 'WellTraceDir' is missing in "+QString(__FUNCTION__));
        return false;
    }

    QFileInfo fileInfoWT(pathWellTraceDir);

    if(!fileInfoWT.exists())
        pathWellTraceDir = QDir::currentPath() + QDir::separator() + pathWellTraceDir;

    fileInfoWT.setFile(pathWellTraceDir);

    if(!fileInfoWT.exists())
    {
        this->errorMessage("Error, could not find the well trace directory at any one of the following paths: "+rvObject.value("WellTraceDir").toString()+","+ pathWellTraceDir+" in "+QString(__FUNCTION__));
        return false;
    }

    pathWellTraceLE->setText(fileInfoWT.absoluteFilePath());

    // caprock shapefile path
    auto pathCaprockShpFile = rvObject.value("PathToCaprockShapefile").toString();

    if(pathCaprockShpFile.isEmpty())
    {
        this->errorMessage("Error, the required input 'PathToCaprockShapefile' is missing in "+QString(__FUNCTION__));
        return false;
    }

    QFileInfo fileInfo(pathCaprockShpFile);

    if(!fileInfo.exists())
        pathCaprockShpFile = QDir::currentPath() + QDir::separator() + pathCaprockShpFile;

    fileInfo.setFile(pathCaprockShpFile);

    if(!fileInfo.exists())
    {
        this->errorMessage("Error, could not find the caprock shapefile file at any one of the following paths: "+rvObject.value("PathToCaprockShapefile").toString()+","+ pathCaprockShpFile+" in "+QString(__FUNCTION__));
        return false;
    }

    pathCaprockShpLE->setText(fileInfo.absoluteFilePath());

    this->loadCaprocksLayer();

    // locations
//    locationWidget->inputFromJSON(rvObject);

    // rest of input
    return PointAssetInputWidget::inputFromJSON(rvObject);
}


bool CSVWellsCaprocksInputWidget::outputToJSON(QJsonObject &rvObject)
{
   auto res = PointAssetInputWidget::outputToJSON(rvObject);

   if(!res)
   {
       this->errorMessage("Error output to json in "+QString(__FUNCTION__));
       return false;
   }

   // locations
//   locationWidget->outputToJSON(rvObject);

   // well trace
   auto pathWellTraceDir = pathWellTraceLE->text();

   if(pathWellTraceDir.isEmpty())
   {
       this->errorMessage("The path to well trace directory is empty in "+QString(__FUNCTION__));
       return false;
   }

   rvObject.insert("WellTraceDir",pathWellTraceDir);


   // caprock shapefile path
   auto pathCaprockShp = pathCaprockShpLE->text();

   if(pathCaprockShp.isEmpty())
   {
       this->errorMessage("The path to caprock shapefile is empty in "+QString(__FUNCTION__));
       return false;
   }

   rvObject.insert("PathToCaprockShapefile",pathCaprockShp);

   return true;
}


void CSVWellsCaprocksInputWidget::handleCaprockDialog(void)
{
    auto newPathToInputFile = QFileDialog::getOpenFileName(this,tr("Path to caprock shapefile or folder with shapefile"));

    // Return if the user cancels
    if(newPathToInputFile.isEmpty())
        return;

    pathCaprockShpLE->setText(newPathToInputFile);

    this->loadCaprocksLayer();
}


void CSVWellsCaprocksInputWidget::clear()
{    
    caprocksLayer = nullptr;
    pathCaprockShpLE->clear();
    pathWellTraceLE->clear();
    PointAssetInputWidget::clear();
}


void CSVWellsCaprocksInputWidget::loadCaprocksLayer()
{
    auto pathToCaprockFile = pathCaprockShpLE->text();

    if(!QFileInfo::exists(pathToCaprockFile))
    {
        this->errorMessage("Error, the caprock file :"+pathToCaprockFile+ "does not exist");
    }

    if(caprocksLayer)
        theVisualizationWidget->removeLayer(caprocksLayer);

    caprocksLayer = theVisualizationWidget->addVectorLayer(pathToCaprockFile,"Caprocks","ogr");

    if(!caprocksLayer)
    {
        this->errorMessage("Error creating the caprock layer in "+QString(__FUNCTION__));
    }
}


