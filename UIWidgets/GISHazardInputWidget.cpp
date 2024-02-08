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
#include "GISHazardInputWidget.h"
#include "VisualizationWidget.h"
#include "WorkflowAppR2D.h"
//#include "SimCenterUnitsCombo.h"
//#include "SimCenterUnitsWidget.h"
#include "SimCenterIMWidget.h"
#include "ComponentDatabaseManager.h"
#include "ComponentDatabase.h"
#include "CRSSelectionWidget.h"
#include "QGISVisualizationWidget.h"

#include "Utils/FileOperations.h"

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

#include <qgsvectorlayer.h>
#include <qgshuesaturationfilter.h>
#include <qgsrasterdataprovider.h>
#include <qgscollapsiblegroupbox.h>
#include <qgsproject.h>

// Test to remove start
#include <chrono>
using namespace std::chrono;
// Test to remove end



GISHazardInputWidget::GISHazardInputWidget(QGISVisualizationWidget* visWidget, QWidget *parent) : SimCenterAppWidget(parent), theVisualizationWidget(visWidget)
{
    GISFilePath = "";

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(this->getGISHazardInputWidget());
    layout->setSpacing(0);
    layout->addStretch();
    this->setLayout(layout);

}


GISHazardInputWidget::~GISHazardInputWidget()
{

}


bool GISHazardInputWidget::outputAppDataToJSON(QJsonObject &jsonObject) {

    emit eventTypeChangedSignal(eventTypeCombo->currentData().toString());

    jsonObject["Application"] = "UserInputGISHazard";

    QJsonObject appData;

    QFileInfo GISFile (GISPathLineEdit->text());

    appData["GISFile"] = GISFile.fileName();
    appData["pathToSource"]=GISFile.path();
    crsSelectorWidget->outputAppDataToJSON(appData);

    jsonObject["ApplicationData"]=appData;

    return true;
}


bool GISHazardInputWidget::outputToJSON(QJsonObject &jsonObj)
{
  /*
    if (theFile.exists()) {
        jsonObj["eventFile"]= theFile.fileName();
        jsonObj["eventFilePath"]=theFile.path();
    } else {
        jsonObj["eventFile"]=eventFile;
        jsonObj["eventFilePath"]=QString("");
    }
  */
  
  return theIMs->outputToJSON(jsonObj);

}


bool GISHazardInputWidget::inputFromJSON(QJsonObject &jsonObject)
{
    // Set the units
    bool res = theIMs->inputFromJSON(jsonObject);

    // If setting of units failed, provide default units and issue a warning
    if(!res) {
      errorMessage("GIS Hazard INput - failed to load intensity measures");
    }

    return res;
}


bool GISHazardInputWidget::inputAppDataFromJSON(QJsonObject &jsonObj)
{
    if (jsonObj.contains("ApplicationData")) {
        QJsonObject appData = jsonObj["ApplicationData"].toObject();

        QString fileName;
        QString pathToFile;

        if (appData.contains("GISFile"))
            fileName = appData["GISFile"].toString();
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
                this->errorMessage("GIS hazard input widget - could not find the GIS file");
                return false;
            }
        }

        GISPathLineEdit->setText(fullFilePath);
        GISFilePath = fullFilePath;

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

        auto res = this->loadGISFile();

        if(res !=0)
        {
            this->errorMessage("Failed to load the GIS file");
            return false;
        }

        // Set the hazard type
        theIMs->handleHazardChange(eventType);

        // Set the CRS
        auto layerCrs = vectorLayer->crs();
        crsSelectorWidget->setCRS(layerCrs);

        return true;
    }

    return false;
}


QWidget* GISHazardInputWidget::getGISHazardInputWidget(void)
{
    // file  input
    fileInputWidget = new QWidget();
    QGridLayout *fileLayout = new QGridLayout(fileInputWidget);
    fileInputWidget->setLayout(fileLayout);

    crsSelectorWidget = new CRSSelectionWidget();

    connect(crsSelectorWidget,&CRSSelectionWidget::crsChanged,this,&GISHazardInputWidget::handleLayerCrsChanged);


    QLabel* selectComponentsText = new QLabel("Hazard GIS File");
    GISPathLineEdit = new QLineEdit();
    QPushButton *browseFileButton = new QPushButton("Browse");

    connect(browseFileButton,SIGNAL(clicked()),this,SLOT(chooseEventFileDialog()));

    fileLayout->addWidget(selectComponentsText, 0,0);
    fileLayout->addWidget(GISPathLineEdit,    0,1);
    fileLayout->addWidget(browseFileButton,     0,2);

    QLabel* eventTypeLabel = new QLabel("Hazard Type:",this);
    eventTypeCombo = new QComboBox(this);
    eventTypeCombo->addItem("Earthquake","Earthquake");
    eventTypeCombo->addItem("Hurricane","Hurricane");
    eventTypeCombo->addItem("Tsunami","Tsunami");
    eventTypeCombo->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);

    // unitsWidget = new SimCenterUnitsWidget();

    theIMs = new SimCenterIMWidget();

    connect(eventTypeCombo,&QComboBox::currentTextChanged,theIMs,&SimCenterIMWidget::handleHazardChange);

    fileLayout->addWidget(eventTypeLabel, 1,0);
    fileLayout->addWidget(eventTypeCombo, 1,1,1,2);

    fileLayout->addWidget(crsSelectorWidget,2,0,1,3);

    // fileLayout->addWidget(unitsWidget, 3,0,1,3);
    fileLayout->addWidget(theIMs, 3,0,1,3);

    fileLayout->setRowStretch(4,1);

    return fileInputWidget;
}


void GISHazardInputWidget::chooseEventFileDialog(void)
{

    QFileDialog dialog(this);
    QString newEventFile = QFileDialog::getOpenFileName(this,tr("Event GIS File"));
    dialog.close();

    // Return if the user cancels or enters same file
    if(newEventFile.isEmpty() || newEventFile == GISFilePath)
        return;

    GISFilePath = newEventFile;
    GISPathLineEdit->setText(GISFilePath);

    //    eventFile.clear();

    auto res = this->loadGISFile();

    if(res !=0)
    {
        this->errorMessage("Failed to load the GIS file");
        return;
    }

    auto attributeList = vectorLayer->fields();

    // unitsWidget->clear();
    theIMs->clear();

    for(int i = 0; i<attributeList.size(); ++i)
    {
        // Note that band numbers start from 1 and not 0!
        auto attributeName = attributeList.at(i).name();

        attributeNames.append(attributeName);

        // auto labelName = "Attribute: " + attributeName;

        // unitsWidget->addNewUnitItem(attributeName,labelName);
        theIMs->addNewIMItem(attributeName, "drivel");
    }

    auto layerCrs = vectorLayer->crs();
    crsSelectorWidget->setCRS(layerCrs);

    auto hazType = eventTypeCombo->currentText();
    theIMs->handleHazardChange(hazType);

    return;
}


void GISHazardInputWidget::clear(void)
{
    GISFilePath.clear();
    GISPathLineEdit->clear();
    attributeNames.clear();

    crsSelectorWidget->clear();

    eventTypeCombo->setCurrentIndex(0);
    theIMs->clear();
}


int GISHazardInputWidget::loadGISFile(void)
{
    this->statusMessage("Loading GIS Hazard Layer");

    QApplication::processEvents();

    auto evtType = eventTypeCombo->currentText();

    vectorLayer = theVisualizationWidget->addVectorLayer(GISFilePath, evtType+" Hazard", "ogr");

    if(vectorLayer == nullptr)
    {
        this->errorMessage("Error adding a GIS (vector) layer to the map");
        return -1;
    }

    // Color it differently for the various hazards
    if(evtType.compare("Tsunami") == 0)
    {
        theVisualizationWidget->createSimpleRenderer(Qt::blue,vectorLayer);
    }
    else if(evtType.compare("Earthquake") == 0)
    {
        theVisualizationWidget->createSimpleRenderer(Qt::darkRed ,vectorLayer);
    }
    else if(evtType.compare("Hurricane") == 0)
    {
        theVisualizationWidget->createSimpleRenderer(Qt::darkGray ,vectorLayer);
    }

    dataProvider = vectorLayer->dataProvider();

    theVisualizationWidget->zoomToLayer(vectorLayer);

    return 0;
}


void GISHazardInputWidget::handleLayerCrsChanged(const QgsCoordinateReferenceSystem & val)
{
    if(vectorLayer)
        vectorLayer->setCrs(val);
}


bool GISHazardInputWidget::copyFiles(QString &destDir)
{
//    if(eventFile.isEmpty())
//    {
//        this->errorMessage("In raster hazard input widget, no eventFile given in copy files");
//        return false;
//    }

//    pathToEventFile = destDir + QDir::separator() + eventFile;

    QFileInfo gisFileNameInfo(GISFilePath);

    if (!gisFileNameInfo.exists())
        return false;

    QDir dirInfo = gisFileNameInfo.dir();

    auto sourceDir = dirInfo.dirName();

    // Recursive copy everything in the folder containing the main file

    auto destPath = destDir + QDir::separator() + sourceDir;

    QDir dirDest(destPath);

    if (!dirDest.exists())
    {
        if (!dirDest.mkpath(destPath))
        {
            QString errMsg = QString("Could not create destination Dir: ") + destPath;
            this->errorMessage(errMsg);

            return false;
        }
    }

    auto res = SCUtils::recursiveCopy(dirInfo.absolutePath(), destPath);

    if(!res)
    {
        QString msg = "Error copying GIS files over to the directory " + destPath;
        errorMessage(msg);

        return res;
    }

    emit outputDirectoryPathChanged(destDir, GISFilePath);

    return true;
}


