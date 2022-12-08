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

// Written by: Stevan Gavrilovic, Frank McKenna

#include "CSVReaderWriter.h"
#include "LayerTreeView.h"
#include "UserInputFaultWidget.h"
#include "VisualizationWidget.h"
#include "WorkflowAppR2D.h"
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
#include <QString>

#include "QGISVisualizationWidget.h"

#include <qgsvectorlayer.h>
#include <qgslinesymbol.h>

UserInputFaultWidget::UserInputFaultWidget(VisualizationWidget* visWidget, QWidget *parent) : SimCenterAppWidget(parent)
{
    theVisualizationWidget = static_cast<QGISVisualizationWidget*>(visWidget);

    assert(theVisualizationWidget);

    eventFile = "";

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(this->getUserInputFaultWidget());
    layout->addStretch();
    this->setLayout(layout);


    // Test to remove
//    eventFile = "/Users/steve/Downloads/New\ folder/Task4C_run_Shaking_m5/Input/virtual_fault.csv";
//    this->loadUserGMData();
}


UserInputFaultWidget::~UserInputFaultWidget()
{

}


bool UserInputFaultWidget::outputAppDataToJSON(QJsonObject &jsonObject) {


    jsonObject["Application"] = "UserInputFault";

    QJsonObject appData;

    jsonObject["ApplicationData"]=appData;

    return true;
}


bool UserInputFaultWidget::outputToJSON(QJsonObject &jsonObj)
{

    QFileInfo theFile(eventFile);
    if (theFile.exists()) {
        //jsonObj["FaultFile"]=theFile.path();
        jsonObj["FaultFile"]=theFile.absoluteFilePath();
    } else {

        this->errorMessage("Error, the fault rupture file provided "+eventFile+" does not exist, please check the path and try again");
        return false;
    }

    return true;
}


bool UserInputFaultWidget::inputAppDataFromJSON(QJsonObject &jsonObj)
{
    Q_UNUSED(jsonObj);
    return true;
}


bool UserInputFaultWidget::inputFromJSON(QJsonObject &jsonObject)
{
    //QString fileName;
    //QString pathToFile;

    auto thisObject = jsonObject.value("UserDefinedRupture").toObject();

    if (thisObject.contains("FaultFile"))
    {
        eventFile = thisObject["FaultFile"].toString();
    }
    else
    {
        this->infoMessage("Error: The 'FaultFile' key is missing from the json object");
        return false;
    }


    //QString fullFilePath= pathToFile + QDir::separator() + fileName;
    //QString fullFilePath = fileName;
    //eventFile = fileName;


    // load the motions
    this->loadUserGMData();



    return true;
}


QStackedWidget* UserInputFaultWidget::getUserInputFaultWidget(void)
{
    if (theStackedWidget)
        return theStackedWidget;

    theStackedWidget = new QStackedWidget();

    //
    // file and dir input
    //

    fileInputWidget = new QWidget();
    QGridLayout *fileLayout = new QGridLayout(fileInputWidget);
    fileInputWidget->setLayout(fileLayout);


    QLabel* selectComponentsText = new QLabel("File Listing Fault Ruptures");
    eventFileLineEdit = new QLineEdit();
    QPushButton *browseFileButton = new QPushButton("Browse");

    connect(browseFileButton,SIGNAL(clicked()),this,SLOT(chooseEventFileDialog()));

    fileLayout->addWidget(selectComponentsText, 0,0);
    fileLayout->addWidget(eventFileLineEdit,    0,1);
    fileLayout->addWidget(browseFileButton,     0,2);


    fileLayout->setRowStretch(3,1);

    //
    // progress bar
    //

    progressBarWidget = new QWidget();
    auto progressBarLayout = new QVBoxLayout(progressBarWidget);
    progressBarWidget->setLayout(progressBarLayout);

    auto progressText = new QLabel("Loading user-provdided fault rupture data. This may take a while.");
    progressLabel =  new QLabel(" ");
    progressBar = new QProgressBar();

    auto vspacer = new QSpacerItem(0,0,QSizePolicy::Minimum, QSizePolicy::Expanding);
    auto vspacer2 = new QSpacerItem(0,0,QSizePolicy::Minimum, QSizePolicy::Expanding);
    progressBarLayout->addItem(vspacer);
    progressBarLayout->addWidget(progressText,1, Qt::AlignCenter);
    progressBarLayout->addWidget(progressLabel,1, Qt::AlignCenter);
    progressBarLayout->addWidget(progressBar);
    progressBarLayout->addItem(vspacer2);
    progressBarLayout->addStretch(1);

    //
    // add file and progress widgets to stacked widgets, then set defaults
    //

    theStackedWidget->addWidget(fileInputWidget);
    theStackedWidget->addWidget(progressBarWidget);

    theStackedWidget->setCurrentWidget(fileInputWidget);

    theStackedWidget->setWindowTitle("Select folder containing earthquake fault ruptures");

    return theStackedWidget;
}


void UserInputFaultWidget::showUserGMSelectDialog(void)
{

    if (!theStackedWidget)
    {
        this->getUserInputFaultWidget();
    }

    theStackedWidget->show();
    theStackedWidget->raise();
    theStackedWidget->activateWindow();
}


void UserInputFaultWidget::chooseEventFileDialog(void)
{

    QFileDialog dialog(this);
    QString newEventFile = QFileDialog::getOpenFileName(this,tr("Fault Rupture File"));
    dialog.close();

    // Return if the user cancels or enters same file
    if(newEventFile.isEmpty() || newEventFile == eventFile)
    {
        return;
    }

    // Set file name & entry in qLine edit
    eventFile = newEventFile;
    eventFileLineEdit->setText(eventFile);


    QFileInfo eventFileInfo(eventFile);

    if(eventFileInfo.exists())
        this->loadUserGMData();
    else
        this->infoMessage("Warning, the file "+eventFile+" does not exist");

    return;
}




void UserInputFaultWidget::clear(void)
{
    eventFile.clear();
    eventFileLineEdit->clear();

}

void UserInputFaultWidget::loadUserGMData(void)
{

    CSVReaderWriter csvTool;

    QString err;
    QVector<QStringList> data = csvTool.parseCSVFile(eventFile, err);

    if(!err.isEmpty())
    {
        this->errorMessage(err);
        return;
    }

    if(data.empty())
        return;

    this->showProgressBar();

    QApplication::processEvents();

    //progressBar->setRange(0,inputFiles.size());
    progressBar->setRange(0, data.count());
    progressBar->setValue(0);

    // Get the headers
    auto stationDataHeadings = data.front();


    // Create the fields
    QList<QgsField> attribFields;
    attribFields.push_back(QgsField("AssetType", QVariant::String));
    attribFields.push_back(QgsField("TabName", QVariant::String));

    for(auto&& it : stationDataHeadings)
        attribFields.push_back(QgsField(it, QVariant::String));

    // Pop off the row that contains the header information
    data.pop_front();

    auto numRows = data.size();

    int count = 0;

    int faultIdx = theVisualizationWidget->getIndexOfVal(stationDataHeadings, "FaultTrace");

    if(faultIdx == -1)
    {
        this->infoMessage("Warning, could not find the index for fault trace geometry in "+eventFile+ ". The fault trace heading should be 'FaultTrace' ");
    }

    // The number of headings in the file
    auto numParams = stationDataHeadings.size();


    QgsFeatureList featureList;
    // Get the data
    for(int i = 0; i<numRows; ++i)
    {
        auto rowStr = data.at(i);

        // create the feature attributes
        QgsAttributes featAttributes(attribFields.size());

        auto faultTrace = rowStr[faultIdx];

        featAttributes[0] = "FaultRuptureTrace";     // "AssetType"
        featAttributes[1] = "Fault Rupture Trace";  // "TabName"

        for(int j = 0; j<numParams; ++j)
        {
            auto str = rowStr[j] ;

            featAttributes[2+j] = str;
        }

        // Create the feature
        QgsFeature feature;

        auto geom = theVisualizationWidget->getMultilineStringGeometryFromJson(faultTrace);
        if(geom.isEmpty())
        {
            this->errorMessage("Error getting the fault trace geometry");
            return;
        }

        feature.setGeometry(geom);


        feature.setAttributes(featAttributes);
        featureList.append(feature);

        ++count;
        progressLabel->clear();
        progressBar->setValue(count);

        QApplication::processEvents();
    }


    auto vectorLayer = theVisualizationWidget->addVectorLayer("linestring", "Fault Ruptures");

    if(vectorLayer == nullptr)
    {
        this->errorMessage("Error creating a layer");
        this->hideProgressBar();
        return;
    }

    auto dProvider = vectorLayer->dataProvider();
    auto res = dProvider->addAttributes(attribFields);

    if(!res)
    {
        this->errorMessage("Error adding attribute fields to layer");
        theVisualizationWidget->removeLayer(vectorLayer);
        this->hideProgressBar();
        return;
    }

    vectorLayer->updateFields(); // tell the vector layer to fetch changes from the provider

    dProvider->addFeatures(featureList);
    vectorLayer->updateExtents();

    QgsLineSymbol* selectedLayerMarkerSymbol = new QgsLineSymbol();

    selectedLayerMarkerSymbol->setWidth(2.0);
    selectedLayerMarkerSymbol->setColor(Qt::darkRed);
    theVisualizationWidget->createSimpleRenderer(selectedLayerMarkerSymbol,vectorLayer);

    progressLabel->setVisible(false);

    // Reset the widget back to the input pane and close
    this->hideProgressBar();

    if(theStackedWidget->isModal())
        theStackedWidget->close();

    emit loadingComplete(true);

    return;
}


void UserInputFaultWidget::showProgressBar(void)
{
    theStackedWidget->setCurrentWidget(progressBarWidget);
    fileInputWidget->setVisible(false);
    progressBarWidget->setVisible(true);
}

bool UserInputFaultWidget::copyFiles(QString &destDir)
{
    // create dir and copy motion files
    QDir destDIR(destDir);
    if (!destDIR.exists()) {
        this->errorMessage("UserInputFaultWidget::copyFiles dest dir does not exist: " + destDir);
        return false;
    }

    QFileInfo eventFileInfo(eventFile);
    if (eventFileInfo.exists()) {
        this->copyFile(eventFile, destDir);
    } else {
        this->errorMessage("UserInputFaultWidget::copyFiles eventFile does not exist: " + eventFile);
        return false;
    }


    return true;
}

void UserInputFaultWidget::hideProgressBar(void)
{
    theStackedWidget->setCurrentWidget(fileInputWidget);
    progressBarWidget->setVisible(false);
    fileInputWidget->setVisible(true);
}

