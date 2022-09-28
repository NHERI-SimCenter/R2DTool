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

#include "GISAssetInputWidget.h"

#include "AssetInputDelegate.h"
#include "AssetFilterDelegate.h"
#include "ComponentTableView.h"
#include "ComponentTableModel.h"
#include "ComponentDatabaseManager.h"
#include "CRSSelectionWidget.h"
#include "CSVReaderWriter.h"

#include "QGISVisualizationWidget.h"

#include <QDir>
#include <QApplication>
#include <QLineEdit>
#include <QLabel>
#include <QFileInfo>
#include <QHeaderView>
#include <QMessageBox>

#include <qgsfillsymbol.h>
#include <qgsmarkersymbol.h>
#include <qgslinesymbol.h>


#ifdef OpenSRA
#include "WorkflowAppOpenSRA.h"
#include "WidgetFactory.h"
#include "JsonGroupBoxWidget.h"
#endif


// Test to remove
#include <chrono>
using namespace std::chrono;


GISAssetInputWidget::GISAssetInputWidget(QWidget *parent, VisualizationWidget* visWidget, QString componentType, QString appType) : AssetInputWidget(parent, visWidget, componentType, appType)
{
    this->setContentsMargins(0,0,0,0);


    QString txt1 = "Load "+componentType.toLower()+" from a GIS file (.shp, .gdb., etc.)";
    this->setLabel1(txt1);

    QString txt2 = "Enter the IDs of one or more "+componentType.toLower()+" to analyze."
                   "Define a range of "+componentType.toLower()+" with a dash and separate multiple "+componentType.toLower()+" with a comma.";
    this->setLabel2(txt2);

    QString txt3 = componentType + " Information";
    this->setLabel3(txt3);

    // The CRS selection
    crsSelectorWidget = new CRSSelectionWidget();

    connect(crsSelectorWidget,&CRSSelectionWidget::crsChanged,this,&GISAssetInputWidget::handleLayerCrsChanged);

    auto insPoint = mainWidgetLayout->count();
    mainWidgetLayout->insertWidget(insPoint-3,crsSelectorWidget);

    //    pathToComponentInputFile = "/Users/steve/Desktop/GalvestonTestbed/GalvestonGIS/GalvestonBuildings/galveston-bldg-v7.shp";
    //    componentFileLineEdit->setText(pathToComponentInputFile);
    //    this->loadComponentData();


}


GISAssetInputWidget::~GISAssetInputWidget()
{

}


#ifdef OpenSRA
bool GISAssetInputWidget::loadFileFromPath(const QString& filePath)
{
    QFileInfo fileInfo;
    if (!fileInfo.exists(filePath))
        return false;

    pathToComponentInputFile = filePath;
    componentFileLineEdit->setText(filePath);

    this->loadAssetData();

    return true;
}


bool GISAssetInputWidget::outputToJSON(QJsonObject &rvObject)
{

    return true;
}


bool GISAssetInputWidget::inputFromJSON(QJsonObject &rvObject)
{
    QJsonObject data;

    QJsonObject appData;

    appData["assetGISFile"] = rvObject["SiteDataFile"];

    appData["CRS"] = rvObject["CRS"];

    data["ApplicationData"] = appData;

    return inputAppDataFromJSON(data);
}

#endif


int GISAssetInputWidget::loadAssetVisualization()
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
    auto type = mainLayer->geometryType();

    QString typeStr = "Null";
    if(type == QgsWkbTypes::PointGeometry)
        typeStr = "point";
    else if(type == QgsWkbTypes::LineGeometry)
        typeStr = "multilinestring";
    else if(type == QgsWkbTypes::PolygonGeometry)
        typeStr = "polygon";

    // Create the selected building layer
    selectedFeaturesLayer = theVisualizationWidget->addVectorLayer(typeStr,"Selected "+assetType);

    if(selectedFeaturesLayer == nullptr)
    {
        this->errorMessage("Error adding the selected assets vector layer");
        return -1;
    }

    selectedFeaturesLayer->setCrs(mainLayer->crs());

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
    else
    {
        this->errorMessage("Type of geometry is not supported");
        return -1;
    }

    selectFeatSymbol->setColor(Qt::yellow);

    theVisualizationWidget->createSimpleRenderer(selectFeatSymbol,selectedFeaturesLayer);

    auto pr2 = selectedFeaturesLayer->dataProvider();

    auto fields = mainLayer->dataProvider()->fields();

    auto res2 = pr2->addAttributes(fields.toList());

    if(!res2)
        this->errorMessage("Error adding attributes to the layer");

    selectedFeaturesLayer->updateFields(); // tell the vector layer to fetch changes from the provider

    QVector<QgsMapLayer*> mapLayers;
    mapLayers.push_back(selectedFeaturesLayer);
    mapLayers.push_back(mainLayer);

    theVisualizationWidget->createLayerGroup(mapLayers,assetType);

    theComponentDb->setMainLayer(mainLayer);

    filterDelegateWidget  = new AssetFilterDelegate(mainLayer);

    theComponentDb->setSelectedLayer(selectedFeaturesLayer);

    return 0;
}


bool GISAssetInputWidget::loadAssetData(void)
{
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

    // Clear the old layers if any
    if(mainLayer != nullptr)
        theVisualizationWidget->removeLayer(mainLayer);

    if(selectedFeaturesLayer != nullptr)
        theVisualizationWidget->removeLayer(selectedFeaturesLayer);

    // Name the layer according to the filename
    auto fName = file.fileName();

    mainLayer = theVisualizationWidget->addVectorLayer(pathToComponentInputFile, fName, "ogr");

    if(mainLayer == nullptr)
    {
        this->errorMessage("Error, failed to add GIS layer");
        return false;
    }

    auto numFeat = mainLayer->featureCount();

    if(numFeat == 0)
    {
        this->errorMessage("Input Shapefile does not have any features");
        return false;
    }
    else{
        this->statusMessage("Loading information for " + QString::number(numFeat)+ " assets");
        QApplication::processEvents();
    }

    auto layerId = mainLayer->id();
    theVisualizationWidget->registerLayerForSelection(layerId,this);

    auto features = mainLayer->getFeatures();

    auto fields = mainLayer->fields();

    if(fields.size() == 0)
    {
        this->errorMessage("Input Shapefile layer does not have any fields. Will not show table");
        return false;
    }

    QStringList fieldsStrList;
    for(int i = 0; i<fields.size(); ++i)
    {
        auto field = fields[i];
        auto fieldName = field.name();

        fieldsStrList.push_back(fieldName);
    }

    // Add the ID column to the headers
    //fieldsStrList.push_front("ID");

    tableHorizontalHeadings = fieldsStrList;

    auto numFields = fieldsStrList.size();

    // Data containing the table
    QVector<QStringList> data(numFeat);

    // Test to remove
    // auto start = high_resolution_clock::now();

    QgsFeature feat;
    int i = 0;
    while (features.nextFeature(feat))
    {
        //QStringList attributeStrList = {QString::number(i+1)};
        QStringList attributeStrList;

        auto attributes = feat.attributes();
        for(int i = 0; i<attributes.size(); ++i)
        {
            auto attribute = attributes[i];
            auto attributeStr = attribute.toString();

            attributeStrList.push_back(attributeStr);
        }

        if(attributeStrList.size() != numFields)
        {
            this->errorMessage("Error, the number of attributes: "+QString::number(attributeStrList.size())+" for feature " +QString::number(feat.id())+" does not equal the number of fields "+QString::number(numFields));
            return false;
        }

        data[i] = attributeStrList;

        ++i;
    }

    // Test to remove
    //    auto stop = high_resolution_clock::now();
    //    auto duration = duration_cast<milliseconds>(stop - start);
    //    this->statusMessage("Done shapefile results "+QString::number(duration.count()));

    componentTableWidget->clear();
    componentTableWidget->getTableModel()->populateData(data, tableHorizontalHeadings);

    label3->show();
    componentTableWidget->show();
    componentTableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);

    this->loadAssetVisualization();

    offset = this->getOffset();
    theComponentDb->setOffset(offset);

    this->statusMessage("Done loading assets");
    QApplication::processEvents();

    emit doneLoadingComponents();

    return true;
}


bool GISAssetInputWidget::outputAppDataToJSON(QJsonObject &jsonObject)
{

    // First get the default information and then modify
    AssetInputWidget::outputAppDataToJSON(jsonObject);

    // Here we will export everything to a .csv
    jsonObject["Application"] = appType;

    // It assumes the file name stays the same, but we need to modify the extension so that it is in csv and not .gdb, or .shp, or whatever
    QFileInfo componentFile(componentFileLineEdit->text());

    QString baseNameCSV = componentFile.baseName() + ".csv";

    auto appData = jsonObject.value("ApplicationData").toObject();

    if(appData.isEmpty())
        return false;

    crsSelectorWidget->outputAppDataToJSON(appData);

    appData["assetSourceFile"] = baseNameCSV;
    appData["assetGISFile"] = componentFile.fileName();
    jsonObject["ApplicationData"] = appData;

    return true;
}


bool GISAssetInputWidget::inputAppDataFromJSON(QJsonObject &jsonObject)
{

    //jsonObject["Application"]=appType;
    if (jsonObject.contains("Application")) {
        if (appType != jsonObject["Application"].toString()) {
            this->errorMessage("GISAssetInputWidget::inputFomJSON app name conflict");
            return false;
        }
    }


    if (jsonObject.contains("ApplicationData")) {
        QJsonObject appData = jsonObject["ApplicationData"].toObject();

        QString fileName;
        QString pathToFile;
        bool foundFile = false;
        if (appData.contains("assetGISFile"))
            fileName = appData["assetGISFile"].toString();

        if (appData.contains("pathToSource")) {
            QJsonValue theName = appData["pathToSource"];
            pathToFile = theName.toString();
            fileName = pathToFile + QDir::separator() + fileName;
	}
	
        QFileInfo fileInfo(fileName);

        if (fileInfo.exists()) {

            componentFileLineEdit->setText(fileInfo.absoluteFilePath());
            pathToComponentInputFile = fileInfo.absoluteFilePath();

            if(this->loadAssetData() == false)
                return false;

            foundFile = true;

        } else {

            if (!appData.contains("pathToSource"))
                pathToFile=QDir::currentPath();

            pathToComponentInputFile = pathToFile + QDir::separator() + fileName;

            if (fileInfo.exists(pathToComponentInputFile)) {
                componentFileLineEdit->setText(pathToComponentInputFile);
                foundFile = true;

                if(this->loadAssetData() == false)
                    return false;

            } else {
                // adam .. adam .. adam
                pathToComponentInputFile = pathToFile + QDir::separator()
                        + "input_data" + QDir::separator() + fileName;
                if (fileInfo.exists(pathToComponentInputFile)) {
                    componentFileLineEdit->setText(pathToComponentInputFile);
                    foundFile = true;

                    if(this->loadAssetData() == false)
                        return false;                }
                else
                {
                    QString errMessage = appType + " - The file " + fileName + " could not be found";
                    this->errorMessage(errMessage);
                    return false;
                }
            }
        }

        if(foundFile == false)
        {
            QString errMessage = appType + " - The file " + fileName + " could not be found";
            this->errorMessage(errMessage);
            return false;
        }

        if (appData.contains("filter"))
            this->setFilterString(appData["filter"].toString());


        // Set the CRS
        QString errMsg;
        if(!crsSelectorWidget->inputAppDataFromJSON(appData,errMsg))
            this->infoMessage(errMsg);

    }


    return true;
}


bool GISAssetInputWidget::isEmpty()
{
    if(mainLayer == nullptr)
        return true;

    if(mainLayer->featureCount() == 0)
        return true;

    return false;
}


bool GISAssetInputWidget::copyFiles(QString &destName)
{
    // Copy over the gis file
    auto compLineEditText = componentFileLineEdit->text();

    QFileInfo componentFile(compLineEditText);

    if (!componentFile.exists())
        return false;

    if (!QFile::copy(compLineEditText, destName + QDir::separator() + componentFile.fileName()))
        return false;

    // Do not copy the file, output a new csv which will have the changes that the user makes in the table
    //        if (componentFile.exists()) {
    //            return this->copyFile(componentFileLineEdit->text(), destName);
    //        }

    // Then create the csv file
    return AssetInputWidget::copyFiles(destName);
}


void GISAssetInputWidget::setCRS(const QgsCoordinateReferenceSystem & val)
{
    crsSelectorWidget->setCRS(val);
}


void GISAssetInputWidget::handleLayerCrsChanged(const QgsCoordinateReferenceSystem & val)
{
    if(mainLayer)
        mainLayer->setCrs(val);
}


void GISAssetInputWidget::clear(void)
{
    crsSelectorWidget->clear();

    AssetInputWidget::clear();
}


int GISAssetInputWidget::getOffset(void)
{
    auto delta = 0;

    // Get the first id of the fid
    auto firstIdLayer = 0;
    auto features2 = mainLayer->getFeatures();
    QgsFeature feat2;
    if (features2.nextFeature(feat2))
        firstIdLayer = feat2.id();

    // Get the ID of the first asset from the table
    bool OK;
    auto firstIDTable = componentTableWidget->item(0,0).toInt(&OK);

    if(!OK)
    {
        QString msg = "Error in getting the component ID in " + QString(__FUNCTION__);
        this->errorMessage(msg);
        return 0;
    }

    delta = firstIdLayer - firstIDTable;

    return delta;
}
