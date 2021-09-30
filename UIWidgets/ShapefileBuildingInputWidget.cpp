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

#include "ShapefileBuildingInputWidget.h"

#include "AssetInputDelegate.h"
#include "ComponentTableView.h"
#include "ComponentTableModel.h"
#include "ComponentDatabaseManager.h"
#include <QDir>
#include <QApplication>
#include <QLineEdit>
#include <QLabel>
#include <QFileInfo>
#include <QHeaderView>

#include "QGISVisualizationWidget.h"

#include <qgsfillsymbol.h>
#include <qgsmarkersymbol.h>
#include <qgslinesymbol.h>

// Test to remove
#include <chrono>
using namespace std::chrono;


ShapefileBuildingInputWidget::ShapefileBuildingInputWidget(QWidget *parent, VisualizationWidget* visWidget, QString componentType, QString appType) : ComponentInputWidget(parent, visWidget, appType)
{    

    this->setContentsMargins(0,0,0,0);

    label1 = "Load buildings from a Shapefile (.shp)";
    label2 = "Enter the IDs of one or more " + componentType.toLower() + " to analyze."
    "Define a range of " + componentType.toLower() + " with a dash and separate multiple " + componentType.toLower() + " with a comma.";

    label3 = QStringRef(&componentType, 0, componentType.length()-1) + " Information";


    pathToComponentInputFile = "/Users/steve/Desktop/GalvestonTestbed/GalvestonBuildings/galveston-bldg-v7.shp";
    componentFileLineEdit->setText(pathToComponentInputFile);
    this->loadComponentData();
}


ShapefileBuildingInputWidget::~ShapefileBuildingInputWidget()
{

}


int ShapefileBuildingInputWidget::loadComponentVisualization()
{

    //    enum GeometryType
    //    {
    //      PointGeometry,
    //      LineGeometry,
    //      PolygonGeometry,
    //      UnknownGeometry,
    //      NullGeometry
    //    };
    // , "linestring", "polygon","multipoint","multilinestring","multipolygon"
    auto type = shapeFileLayer->geometryType();

    QString typeStr = "Null";
    if(type == QgsWkbTypes::PointGeometry)
        typeStr = "point";
    else if(type == QgsWkbTypes::LineGeometry)
        typeStr = "multilinestring";
    else if(type == QgsWkbTypes::PolygonGeometry)
        typeStr = "polygon";

    // Create the selected building layer
    auto selectedFeaturesLayer = theVisualizationWidget->addVectorLayer(typeStr,"Selected Buildings");

    if(selectedFeaturesLayer == nullptr)
    {
        this->errorMessage("Error adding the selected assets vector layer");
        return -1;
    }

    selectedFeaturesLayer->setCrs(shapeFileLayer->crs());

    QgsSymbol* selectFeatSymbol = nullptr;
    if(type == QgsWkbTypes::PointGeometry)
    {
        selectFeatSymbol = new QgsMarkerSymbol();
    }
    else if(type == QgsWkbTypes::LineGeometry)
    {
        selectFeatSymbol = new QgsLineSymbol();
    }
    else if(type == QgsWkbTypes::PolygonGeometry)
    {
        selectFeatSymbol = new QgsFillSymbol();
    }

    selectFeatSymbol->setColor(Qt::yellow);

    theVisualizationWidget->createSimpleRenderer(selectFeatSymbol,selectedFeaturesLayer);

    auto pr2 = selectedFeaturesLayer->dataProvider();

    auto fields = shapeFileLayer->dataProvider()->fields();

    auto res2 = pr2->addAttributes(fields.toList());

    if(!res2)
        this->errorMessage("Error adding attributes to the layer");

    selectedFeaturesLayer->updateFields(); // tell the vector layer to fetch changes from the provider

    QVector<QgsMapLayer*> mapLayers;
    mapLayers.push_back(selectedFeaturesLayer);
    mapLayers.push_back(shapeFileLayer);

    theVisualizationWidget->createLayerGroup(mapLayers,"Buildings");

    theComponentDb->setMainLayer(shapeFileLayer);
    theComponentDb->setSelectedLayer(selectedFeaturesLayer);

    return 0;
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

    shapeFileLayer = theVisualizationWidget->addVectorLayer(pathToComponentInputFile, fName, "ogr");

    auto numFeat = shapeFileLayer->featureCount();

    if(numFeat == 0)
    {
        this->errorMessage("Input Shapefile does not have any features");
        return;
    }
    else{
        this->statusMessage("Loading information for " + QString::number(numFeat)+ " assets");
        QApplication::processEvents();
    }

    auto layerId = shapeFileLayer->id();
    theVisualizationWidget->registerLayerForSelection(layerId,this);

    auto features = shapeFileLayer->getFeatures();

    auto fields = shapeFileLayer->fields();

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

    // Add the ID column to the headers
    fieldsStrList.push_front("ID");

    tableHorizontalHeadings = fieldsStrList;

    // Data containing the table
    QVector<QStringList> data(numFeat);

    // Test to remove
    auto start = high_resolution_clock::now();

    QgsFeature feat;
    int i = 0;
    while (features.nextFeature(feat))
    {
        QStringList attributeStrList = {QString::number(i+1)};

        auto attributes = feat.attributes();
        for(int i = 0; i<attributes.size(); ++i)
        {
            auto attribute = attributes[i];
            auto attributeStr = attribute.toString();

            if(attributeStr.compare("ID")==0)
                continue;

            attributeStrList.push_back(attributeStr);
        }

        data[i] = attributeStrList;

        ++i;
    }

    // Test to remove
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
    this->statusMessage("Done shapefile results "+QString::number(duration.count()));


    componentTableWidget->clear();
    componentTableWidget->getTableModel()->populateData(data, tableHorizontalHeadings);

    componentInfoText->show();
    componentTableWidget->show();
    componentTableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);

    this->loadComponentVisualization();

    // Required offset by -1
    offset = -1;

    theComponentDb->setOffset(offset);

    this->statusMessage("Done loading assets");
    QApplication::processEvents();

    return;
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


void ShapefileBuildingInputWidget::clear(void)
{
    theVisualizationWidget->removeLayer(shapeFileLayer);
    ComponentInputWidget::clear();
}
