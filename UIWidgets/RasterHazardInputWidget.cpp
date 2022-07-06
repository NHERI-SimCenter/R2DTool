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

#include "CSVReaderWriter.h"
#include "LayerTreeView.h"
#include "RasterHazardInputWidget.h"
#include "VisualizationWidget.h"
#include "WorkflowAppR2D.h"
#include "SimCenterUnitsCombo.h"
#include "SimCenterUnitsWidget.h"
#include "ComponentDatabaseManager.h"
#include "ComponentDatabase.h"
#include "CRSSelectionWidget.h"

#include <cstdlib>

#include <QApplication>
#include <QDialog>
#include <QFile>
#include <QFileDialog>
#include <QJsonObject>
#include <QFileInfo>
#include <QGridLayout>
#include <QJsonArray>
#include <QLabel>
#include <QLineEdit>
#include <QProgressBar>
#include <QComboBox>
#include <QPushButton>
#include <QSpacerItem>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QDir>

#include "QGISVisualizationWidget.h"
#include <qgsrasterlayer.h>
#include <qgshuesaturationfilter.h>
#include <qgsrasterdataprovider.h>
#include <qgscollapsiblegroupbox.h>
#include <qgsproject.h>

// Test to remove start
#include <chrono>
using namespace std::chrono;
// Test to remove end


RasterHazardInputWidget::RasterHazardInputWidget(VisualizationWidget* visWidget, QWidget *parent) : SimCenterAppWidget(parent)
{
    theVisualizationWidget = dynamic_cast<QGISVisualizationWidget*>(visWidget);
    assert(theVisualizationWidget != nullptr);

    unitsWidget = nullptr;
    dataProvider = nullptr;
    rasterlayer = nullptr;
    eventTypeCombo = nullptr;

    fileInputWidget = nullptr;
    rasterFilePath = "";

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(this->getRasterHazardInputWidget());
    layout->setSpacing(0);
    layout->addStretch();
    this->setLayout(layout);

    // Test to remove start
    //     eventFile = "/Users/steve/Desktop/GalvestonTestbed/Surge_Raster.tif";
    //     eventFileLineEdit->setText(eventFile);
    //     this->loadRaster();
    // Test to remove end
}


RasterHazardInputWidget::~RasterHazardInputWidget()
{

}


bool RasterHazardInputWidget::outputAppDataToJSON(QJsonObject &jsonObject) {

    emit eventTypeChangedSignal(eventTypeCombo->currentData().toString());

    jsonObject["Application"] = "UserInputRasterHazard";

    QJsonObject appData;

    QFileInfo rasterFile (rasterPathLineEdit->text());

    appData["rasterFile"] = rasterFile.fileName();

    crsSelectorWidget->outputAppDataToJSON(appData);

    appData["eventClassification"] = eventTypeCombo->currentText();

    QJsonArray bandArray;

    for(auto&& it : bandNames)
        bandArray.append(it);

    appData["bands"] = bandArray;

    jsonObject["ApplicationData"]=appData;


    return true;
}


bool RasterHazardInputWidget::outputToJSON(QJsonObject &jsonObj)
{

    QFileInfo theFile(pathToEventFile);

    if (theFile.exists()) {
        jsonObj["eventFile"]= theFile.fileName();
        jsonObj["eventFilePath"]=theFile.path();
    } else {
        jsonObj["eventFile"]=pathToEventFile; // may be valid on others computer
        jsonObj["eventFilePath"]=QString("");
    }

    auto res = unitsWidget->outputToJSON(jsonObj);

    if(!res)
        this->errorMessage("Could not get the units in 'Raster Defined Hazard'");

    return res;
}


bool RasterHazardInputWidget::inputFromJSON(QJsonObject &jsonObject)
{
    // Set the units
    auto res = unitsWidget->inputFromJSON(jsonObject);

    //    auto list = unitsWidget->getParameterNames();

    // If setting of units failed, provide default units and issue a warning
    if(!res)
    {
        auto paramNames = unitsWidget->getParameterNames();

        this->infoMessage("Warning \\!/: Failed to find/import the units in 'Raster Defined Hazard' widget. Please set the units for the following parameters:");

        for(auto&& it : paramNames)
            this->infoMessage("For parameter: "+it);

    }

    if (jsonObject.contains("eventFile"))
        eventFile = jsonObject["eventFile"].toString();

    if(eventFile.isEmpty())
    {
        this->errorMessage("Raster hazard input widget -Error could not find the eventFile");
        return false;
    }

    return res;
}


bool RasterHazardInputWidget::inputAppDataFromJSON(QJsonObject &jsonObj)
{
    if (jsonObj.contains("ApplicationData")) {
        QJsonObject appData = jsonObj["ApplicationData"].toObject();

        QString fileName;
        QString pathToFile;

        if (appData.contains("rasterFile"))
            fileName = appData["rasterFile"].toString();
        if (appData.contains("eventFileDir"))
            pathToFile = appData["eventFileDir"].toString();
        else
            pathToFile=QDir::currentPath();

        QString fullFilePath= pathToFile + QDir::separator() + fileName;

        // adam .. adam .. adam
        if (!QFileInfo::exists(fullFilePath)){
            fullFilePath = pathToFile + QDir::separator()
                    + "input_data" + QDir::separator() + fileName;

            if (!QFile::exists(fullFilePath)) {
                this->errorMessage("Raster hazard input widget - could not find the raster file");
                return false;
            }
        }

        rasterPathLineEdit->setText(fullFilePath);
        rasterFilePath = fullFilePath;

        // Get the event type
        auto eventType = appData["eventClassification"].toString();

        if(eventType.isEmpty())
        {
            this->errorMessage("Error, please provide an event classification in the json input file");
            return false;
        }

        auto eventIndex = eventTypeCombo->findText(eventType);

        if(eventIndex == -1)
        {
            this->errorMessage("Error, the event classification "+eventType+" is not recognized");
            return false;
        }
        else
        {
            eventTypeCombo->setCurrentIndex(eventIndex);
        }


        auto res = this->loadRaster();

        if(res !=0)
        {
            this->errorMessage("Failed to load the raster");
            return false;
        }

        auto bandArray = appData["bands"].toArray();

        auto numBands = rasterlayer->bandCount();

        if(bandArray.size() != numBands)
        {
            this->errorMessage("Error in loading rasater. The number of provided bands in the json file should be equal to the number of bands in the raster");
            return false;
        }

        for(int i = 0; i<numBands; ++i)
        {
            // Note that band numbers start from 1 and not 0!
            //auto bandName = rasterlayer->bandName(i+1);

            auto bandName = bandArray.at(i).toString();

            bandNames.append(bandName);

            unitsWidget->addNewUnitItem(bandName);
        }


        // Set the CRS
        QString errMsg;
        if(!crsSelectorWidget->inputAppDataFromJSON(appData,errMsg))
            this->infoMessage(errMsg);

        return true;
    }

    return false;
}


QWidget* RasterHazardInputWidget::getRasterHazardInputWidget(void)
{

    // file  input
    fileInputWidget = new QWidget();
    QGridLayout *fileLayout = new QGridLayout(fileInputWidget);
    fileInputWidget->setLayout(fileLayout);

    crsSelectorWidget = new CRSSelectionWidget();

    connect(crsSelectorWidget,&CRSSelectionWidget::crsChanged,this,&RasterHazardInputWidget::handleLayerCrsChanged);


    QLabel* selectComponentsText = new QLabel("Event Raster File");
    rasterPathLineEdit = new QLineEdit();
    QPushButton *browseFileButton = new QPushButton("Browse");

    connect(browseFileButton,SIGNAL(clicked()),this,SLOT(chooseEventFileDialog()));

    fileLayout->addWidget(selectComponentsText, 0,0);
    fileLayout->addWidget(rasterPathLineEdit,    0,1);
    fileLayout->addWidget(browseFileButton,     0,2);

    QLabel* eventTypeLabel = new QLabel("Event Type:",this);
    eventTypeCombo = new QComboBox(this);
    eventTypeCombo->addItem("Earthquake","Earthquake");
    eventTypeCombo->addItem("Hurricane","Hurricane");
    eventTypeCombo->addItem("Tsunami","Tsunami");
    eventTypeCombo->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);

    unitsWidget = new SimCenterUnitsWidget();

    QLabel* crsTypeLabel = new QLabel("Set the coordinate reference system (CRS):",this);

    fileLayout->addWidget(eventTypeLabel, 1,0);
    fileLayout->addWidget(eventTypeCombo, 1,1,1,2);

    fileLayout->addWidget(crsTypeLabel,2,0);
    fileLayout->addWidget(crsSelectorWidget,2,1,1,2);

    fileLayout->addWidget(unitsWidget, 3,0,1,3);

    fileLayout->setRowStretch(4,1);

    return fileInputWidget;
}


void RasterHazardInputWidget::chooseEventFileDialog(void)
{

    QFileDialog dialog(this);
    QString newEventFile = QFileDialog::getOpenFileName(this,tr("Event Raster File"));
    dialog.close();

    // Return if the user cancels or enters same file
    if(newEventFile.isEmpty() || newEventFile == rasterFilePath)
        return;

    rasterFilePath = newEventFile;
    rasterPathLineEdit->setText(rasterFilePath);

    eventFile.clear();

    auto res = this->loadRaster();

    if(res !=0)
    {
        this->errorMessage("Failed to load the raster");
        return;
    }

    auto numBands = rasterlayer->bandCount();

    unitsWidget->clear();

    for(int i = 0; i<numBands; ++i)
    {
        // Note that band numbers start from 1 and not 0!
        auto bandName = rasterlayer->bandName(i+1);

        bandNames.append(bandName);

        unitsWidget->addNewUnitItem(bandName);
    }

    return;
}


void RasterHazardInputWidget::clear(void)
{
    rasterFilePath.clear();
    rasterPathLineEdit->clear();
    bandNames.clear();

    eventFile.clear();
    pathToEventFile.clear();

    crsSelectorWidget->clear();

    eventTypeCombo->setCurrentIndex(0);

    unitsWidget->clear();
}


double RasterHazardInputWidget::sampleRaster(const double& x, const double& y, const int& bandNumber)
{
    if(dataProvider == nullptr)
    {
        this->errorMessage("Error, attempting to sample a raster layer that has not been loaded");
        return std::numeric_limits<double>::quiet_NaN();;
    }

    auto numBands = rasterlayer->bandCount();

    if(bandNumber>numBands)
    {
        this->errorMessage("Error, the band number given "+QString::number(bandNumber)+" is greater than the number of bands in the raster: "+QString::number(numBands));
        return std::numeric_limits<double>::quiet_NaN();
    }


    QgsPointXY point(x,y);

    // Use the sample method below as this is considerably more efficient than
    bool OK;
    auto testVal = dataProvider->sample(point,bandNumber,&OK);  // Test val will be NAN at failure

    if(!OK)
    {
        this->infoMessage("Warning, error sampling the raster, asset may be out of bounds. Setting raster value to zero");
        testVal = 0.0;
    }

    //this->statusMessage(QString::number(testVal));

    return testVal;
}


int RasterHazardInputWidget::loadRaster(void)
{
    this->statusMessage("Loading Raster Hazard Layer");

    QApplication::processEvents();

    auto evtType = eventTypeCombo->currentText();

    rasterlayer = theVisualizationWidget->addRasterLayer(rasterFilePath, evtType+" Raster Hazard", "gdal");

    if(rasterlayer == nullptr)
    {
        this->errorMessage("Error adding a raster layer to the map");
        return -1;
    }

    rasterlayer->setOpacity(0.5);

    // Color it differently for the various hazards
    if(evtType.compare("Tsunami") == 0)
    {
        QgsHueSaturationFilter *hueSaturationFilter = rasterlayer->hueSaturationFilter();
        hueSaturationFilter->setSaturation(100);
        hueSaturationFilter->setGrayscaleMode(QgsHueSaturationFilter::GrayscaleMode::GrayscaleOff);
        hueSaturationFilter->setColorizeOn(true);
        QColor col(Qt::blue);
        hueSaturationFilter->setColorizeColor(col);
        hueSaturationFilter->setColorizeStrength(100);
        rasterlayer->setBlendMode( QPainter::CompositionMode_SourceOver);
    }
    else if(evtType.compare("Earthquake") == 0)
    {
        QgsHueSaturationFilter *hueSaturationFilter = rasterlayer->hueSaturationFilter();
        hueSaturationFilter->setSaturation(100);
        hueSaturationFilter->setGrayscaleMode(QgsHueSaturationFilter::GrayscaleMode::GrayscaleOff);
        hueSaturationFilter->setColorizeOn(true);
        QColor col(Qt::darkRed);
        hueSaturationFilter->setColorizeColor(col);
        hueSaturationFilter->setColorizeStrength(100);
        rasterlayer->setBlendMode( QPainter::CompositionMode_SourceOver);
    }
    else if(evtType.compare("Hurricane") == 0)
    {
        QgsHueSaturationFilter *hueSaturationFilter = rasterlayer->hueSaturationFilter();
        hueSaturationFilter->setSaturation(100);
        hueSaturationFilter->setGrayscaleMode(QgsHueSaturationFilter::GrayscaleMode::GrayscaleOff);
        hueSaturationFilter->setColorizeOn(true);
        QColor col(Qt::darkGray);
        hueSaturationFilter->setColorizeColor(col);
        hueSaturationFilter->setColorizeStrength(100);
        rasterlayer->setBlendMode( QPainter::CompositionMode_SourceOver);
    }

    dataProvider = rasterlayer->dataProvider();

    theVisualizationWidget->zoomToLayer(rasterlayer);

    //    // Test to remove start
    //    auto start = high_resolution_clock::now();
    //    // Test to remove end

    //    for(int i = 0; i<1000000; ++i)
    //    {
    //        auto rnd = static_cast<double>(std::rand()/((RAND_MAX + 1u)/0.1));
    //        this->sampleRaster(-94.87183+rnd,29.24216+rnd,1);
    //    }

    //    // Test to remove start
    //    auto stop = high_resolution_clock::now();
    //    auto duration = duration_cast<milliseconds>(stop - start);
    //    this->statusMessage("Duration: "+QString::number(duration.count()));
    //    // Test to remove end


    return 0;
}


void RasterHazardInputWidget::handleLayerCrsChanged(const QgsCoordinateReferenceSystem & val)
{
    if(rasterlayer)
        rasterlayer->setCrs(val);
}


bool RasterHazardInputWidget::copyFiles(QString &destDir)
{
    if(eventFile.isEmpty())
    {
        this->errorMessage("In raster hazard input widget, no eventFile given in copy files");
        return false;
    }

    auto numBands = rasterlayer->bandCount();

    if(numBands != bandNames.size())
    {
        this->infoMessage("In raster hazard input widget, the number of bands in the raster is not equal to the number of band names. Using generic names");

        for(int i = 0; i<numBands; ++i)
        {
            auto bName = rasterlayer->bandName(i+1);
            bandNames.append(bName);
        }
    }

    pathToEventFile = destDir + QDir::separator() + eventFile;

    QFileInfo rasterFileNameInfo(rasterFilePath);

    auto rasterFileName = rasterFileNameInfo.fileName();

    if (!QFile::copy(rasterFilePath, destDir + QDir::separator() + rasterFileName))
        return false;

    emit outputDirectoryPathChanged(destDir, pathToEventFile);

    auto theBuildingDB = ComponentDatabaseManager::getInstance()->getBuildingComponentDb();

    auto numPoints = theBuildingDB->getSelectedLayer()->featureCount();

    QVector<QStringList> pointDataVector;
    pointDataVector.reserve(numPoints);

    QgsFeatureIterator fit = theBuildingDB->getSelectedLayer()->getFeatures();

    QgsFeature feature;
    while (fit.nextFeature(feature))
    {

        // Get the latitude and lon of the asset
        auto featAtrb = feature.attributes();
        auto latIndx = feature.fieldNameIndex("Latitude");
        auto lonIndx = feature.fieldNameIndex("Longitude");

        double x = 0.0;
        double y = 0.0;

        // First check if the lat/lon is explicitly provided
        if(latIndx != -1 || lonIndx != -1)
        {

            bool OK = false;
            x = featAtrb.at(lonIndx).toDouble(&OK);
            if(!OK)
            {
                this->errorMessage("Could not get the latitude from the asset");
                return false;
            }
            y = featAtrb.at(latIndx).toDouble(&OK);
            if(!OK)
            {
                this->errorMessage("Could not get the latitude from the asset");
                return false;
            }
        }
        else // Get the centroid of the geometry and use that as asset location
        {
            // Sample the raster at the centroid of the geometry
            auto centroid = feature.geometry().centroid().asPoint();
            x = centroid.x();
            y = centroid.y();
        }

        auto xstr = QString::number(x,'g', 10);
        auto ystr = QString::number(y,'g', 10);

        QStringList pointData;
        pointData.append(xstr);
        pointData.append(ystr);

        for(int i = 0; i< bandNames.size(); ++i)
        {
            auto val = this->sampleRaster(x, y, i+1);
            auto valStr = QString::number(val);

            pointData.append(valStr);
        }

        pointDataVector.push_back(pointData);
    }

    CSVReaderWriter csvTool;

    // First create the event grid file
    QVector<QStringList> gridData;

    QStringList headerRow = {"GP_file", "Latitude", "Longitude"};
    gridData.push_back(headerRow);

    QStringList stationHeader = bandNames;

    QApplication::processEvents();

    for(int i = 0; i<pointDataVector.size(); ++i)
    {
        auto stationFile = "Site_"+QString::number(i)+".csv";

        auto point = pointDataVector.at(i);

        auto lon = point.at(0);
        auto lat = point.at(1);

        // Get the grid row
        QStringList gridRow = {stationFile, lat, lon};
        gridData.push_back(gridRow);

        QStringList stationRow = {point.begin()+2,point.end()};

        // Save the station data
        QVector<QStringList> stationData = {stationHeader};

        stationData.push_back(stationRow);

        QString pathToStationFile = destDir + QDir::separator() + stationFile;

        QString err;
        auto res2 = csvTool.saveCSVFile(stationData, pathToStationFile, err);
        if(res2 != 0)
        {
            this->errorMessage(err);
            return false;
        }

    }

    // Now save the site grid .csv file
    QString err2;
    auto res2 = csvTool.saveCSVFile(gridData, pathToEventFile, err2);
    if(res2 != 0)
    {
        this->errorMessage(err2);
        return false;
    }


    return true;
}


