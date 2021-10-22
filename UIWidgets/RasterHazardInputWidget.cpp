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

#include <QApplication>
#include <QDialog>
#include <QFile>
#include <QFileDialog>
#include <QJsonObject>
#include <QFileInfo>
#include <QGridLayout>
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
#include <qgsrasterdataprovider.h>

RasterHazardInputWidget::RasterHazardInputWidget(VisualizationWidget* visWidget, QWidget *parent) : SimCenterAppWidget(parent)
{
    theVisualizationWidget = dynamic_cast<QGISVisualizationWidget*>(visWidget);
    assert(theVisualizationWidget != nullptr);

    unitsWidget = nullptr;
    dataProvider = nullptr;
    rasterlayer = nullptr;
    eventTypeCombo = nullptr;

    fileInputWidget = nullptr;
    eventFile = "";

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(this->getRasterHazardInputWidget());
    layout->setSpacing(0);
    layout->addStretch();
    this->setLayout(layout);

    // Test to remove start
    // eventFile = "/Users/steve/Desktop/GalvestonTestbed/Surge_Raster.tif";
    // eventFileLineEdit->setText(eventFile);
    // this->loadRaster();
    // Test to remove end
}


RasterHazardInputWidget::~RasterHazardInputWidget()
{

}


bool RasterHazardInputWidget::outputAppDataToJSON(QJsonObject &jsonObject) {

    emit eventTypeChangedSignal(eventTypeCombo->currentData().toString());

    jsonObject["Application"] = "UserInputRasterHazard";

    QJsonObject appData;
    QFileInfo theFile(eventFile);
    if (theFile.exists()) {
        appData["eventFile"]=theFile.fileName();
        appData["eventFileDir"]=theFile.path();
    } else {
        appData["eventFile"]=eventFile; // may be valid on others computer
        appData["eventFileDir"]=QString("");
    }

    unitsWidget->outputToJSON(jsonObject);

    jsonObject["ApplicationData"]=appData;

    return true;
}


bool RasterHazardInputWidget::outputToJSON(QJsonObject &jsonObj)
{

    return true;
}


bool RasterHazardInputWidget::inputAppDataFromJSON(QJsonObject &jsonObj)
{
    if (jsonObj.contains("ApplicationData")) {
        QJsonObject appData = jsonObj["ApplicationData"].toObject();

        QString fileName;
        QString pathToFile;

        if (appData.contains("eventFile"))
            fileName = appData["eventFile"].toString();
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
                this->errorMessage("UserInputGM - could not find event file");
                return false;
            }
        }

        eventFileLineEdit->setText(fullFilePath);
        eventFile = fullFilePath;


        this->loadRaster();

        return true;
    }

    return false;
}


QWidget* RasterHazardInputWidget::getRasterHazardInputWidget(void)
{


    // file  input
    fileInputWidget = new QWidget(this);
    QGridLayout *fileLayout = new QGridLayout(fileInputWidget);
    fileInputWidget->setLayout(fileLayout);


    QLabel* selectComponentsText = new QLabel("Event Raster File");
    eventFileLineEdit = new QLineEdit();
    QPushButton *browseFileButton = new QPushButton("Browse");

    connect(browseFileButton,SIGNAL(clicked()),this,SLOT(chooseEventFileDialog()));

    fileLayout->addWidget(selectComponentsText, 0,0);
    fileLayout->addWidget(eventFileLineEdit,    0,1);
    fileLayout->addWidget(browseFileButton,     0,2);

    QLabel* eventTypeLabel = new QLabel("Event Type:",this);
    eventTypeCombo = new QComboBox(this);
    eventTypeCombo->addItem("Earthquake","Earthquake");
    eventTypeCombo->addItem("Hurricane","Hurricane");
    eventTypeCombo->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Maximum);

    unitsWidget = new SimCenterUnitsWidget();

    fileLayout->addWidget(eventTypeLabel, 1,0);
    fileLayout->addWidget(eventTypeCombo, 1,1);
    fileLayout->addWidget(unitsWidget, 2,0,1,3);

    fileLayout->setRowStretch(3,1);

    return fileInputWidget;
}


void RasterHazardInputWidget::chooseEventFileDialog(void)
{

    QFileDialog dialog(this);
    QString newEventFile = QFileDialog::getOpenFileName(this,tr("Event Raster File"));
    dialog.close();

    // Return if the user cancels or enters same file
    if(newEventFile.isEmpty() || newEventFile == eventFile)
        return;

    eventFile = newEventFile;
    eventFileLineEdit->setText(eventFile);

    this->loadRaster();

    return;
}


void RasterHazardInputWidget::clear(void)
{
    eventFile.clear();
    eventFileLineEdit->clear();

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
    auto testVal = dataProvider->sample(point,bandNumber,&OK);

    if(!OK)
        this->errorMessage("Error, sampling the raster");

    this->statusMessage(QString::number(testVal));

    // Test val will be NAN at failure
    return testVal;
}



int RasterHazardInputWidget::loadRaster(void)
{
    this->statusMessage("Loading Raster Hazard Layer");

    QApplication::processEvents();

    rasterlayer = theVisualizationWidget->addRasterLayer(eventFile, "Raster Hazard", "gdal");

    if(rasterlayer == nullptr)
    {
        this->errorMessage("Error adding a raster layer to the map");
        return -1;
    }

    rasterlayer->setOpacity(0.5);

    dataProvider = rasterlayer->dataProvider();

    auto numBands = rasterlayer->bandCount();

    for(int i = 0; i<numBands; ++i)
    {
        // Note that band numbers start from 1 and not 0!
        auto bandName = rasterlayer->bandName(i+1);

        unitsWidget->addNewUnitItem(bandName);
    }

    this->sampleRaster(-94.87183,29.24216,1);

    return 0;
}



