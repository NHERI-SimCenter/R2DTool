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

#include "LayerTreeView.h"
#include "RasterHazardInputWidget.h"
#include "VisualizationWidget.h"
#include "WorkflowAppR2D.h"

#include "SimCenterIMWidget.h"
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


RasterHazardInputWidget::RasterHazardInputWidget(QGISVisualizationWidget* visWidget, QWidget *parent) : SimCenterAppWidget(parent), theVisualizationWidget(visWidget)
{

    rasterFilePath = "";

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(this->getRasterHazardInputWidget());
    layout->setSpacing(0);
    layout->addStretch();
    this->setLayout(layout);

}


RasterHazardInputWidget::~RasterHazardInputWidget()
{

}


bool RasterHazardInputWidget::outputAppDataToJSON(QJsonObject &jsonObject) {

    emit eventTypeChangedSignal(eventTypeCombo->currentData().toString());

    jsonObject["Application"] = "UserInputRasterHazard";

    QJsonObject appData;

    QFileInfo rasterFile (rasterPathLineEdit->text());

    appData["eventClassification"] = eventTypeCombo->currentText();
    appData["rasterFile"] = rasterFile.fileName();
    appData["pathToSource"]=rasterFile.path();
    crsSelectorWidget->outputAppDataToJSON(appData);

    jsonObject["ApplicationData"]=appData;

    return true;
}


bool RasterHazardInputWidget::outputToJSON(QJsonObject &jsonObj)
{

    QFileInfo theFile(rasterFilePath);
    if (theFile.exists()) {
        jsonObj["eventFile"]= theFile.fileName();
        jsonObj["eventFilePath"]=theFile.path();
    } else {
        return false;
    }

    bool res = theIMs->outputToJSON(jsonObj);

    return res;
}


bool RasterHazardInputWidget::inputFromJSON(QJsonObject &jsonObject)
{
    // Set the units

    bool res = theIMs->inputFromJSON(jsonObject);
    if (res == false) 
      errorMessage("RasterHazard::input of intensity measures failed" );
    
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
        if (appData.contains("pathToSource"))
            pathToFile = appData["pathToSource"].toString();
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

        auto res = this->loadRaster();
	
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

        theIMs->handleHazardChange(eventType);

        if(res !=0)
        {
            this->errorMessage("Failed to load the raster");
            return false;
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

    theIMs = new SimCenterIMWidget("Intensity Measures of Raster");

    connect(eventTypeCombo,&QComboBox::currentTextChanged,theIMs,&SimCenterIMWidget::handleHazardChange);
    
    QLabel* crsTypeLabel = new QLabel("Set the coordinate reference system (CRS):",this);

    fileLayout->addWidget(eventTypeLabel, 1,0);
    fileLayout->addWidget(eventTypeCombo, 1,1,1,2);

    fileLayout->addWidget(crsTypeLabel,2,0);
    fileLayout->addWidget(crsSelectorWidget,2,1,1,2);

    fileLayout->addWidget(theIMs, 3,0,1,3);

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

    this->clear();
    
    rasterFilePath = newEventFile;
    rasterPathLineEdit->setText(rasterFilePath);

    // eventFile.clear();

    auto res = this->loadRaster();

    if(res !=0)
    {
        this->errorMessage("Failed to load the raster");
        return;
    }

    auto numBands = rasterlayer->bandCount();

    //unitsWidget->clear();
    for(int i = 0; i<numBands; ++i)
    {
        // Note that band numbers start from 1 and not 0!
        auto labelName = "Band No. " + QString::number(i+1);
        theIMs->addNewIMItem(labelName,"drivel");  // drivel as this Event does not utilize the name
    }

    return;
}


void RasterHazardInputWidget::clear(void)
{
    rasterFilePath.clear();
    rasterPathLineEdit->clear();
    crsSelectorWidget->clear();

    //unitsWidget->clear();
    theIMs->clear();    
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

    QFileInfo rasterFileNameInfo(rasterFilePath);

    auto rasterFileName = rasterFileNameInfo.fileName();

    if (!QFile::copy(rasterFilePath, destDir + QDir::separator() + rasterFileName))
        return false;

    emit outputDirectoryPathChanged(destDir, rasterFilePath);


    return true;
}


