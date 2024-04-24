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

#include "GISGasNetworkInputWidget.h"
#include "QGISVisualizationWidget.h"
#include "GISAssetInputWidget.h"

#include <qgslinesymbol.h>
#include <qgsmarkersymbol.h>
#include "Utils/FileOperations.h"

#include <QFileDialog>
#include <QDir>
#include <QSplitter>
#include <QGroupBox>
#include <QVBoxLayout>

#ifdef OpenSRA
#include "WorkflowAppOpenSRA.h"
#include "WidgetFactory.h"
#include "JsonGroupBoxWidget.h"
#endif


GISGasNetworkInputWidget::GISGasNetworkInputWidget(QWidget *parent, VisualizationWidget* visWidget) : SimCenterAppWidget(parent)
{
    theVisualizationWidget = static_cast<QGISVisualizationWidget*>(visWidget);
    assert(theVisualizationWidget);

    thePipelinesWidget = new GISAssetInputWidget(this, theVisualizationWidget, "Pipeline Network");

    thePipelinesWidget->setLabel1("Load pipeline network information from a GIS file (.shp, .gpkg, .gdb)");

    connect(thePipelinesWidget,&GISAssetInputWidget::doneLoadingComponents,this,&GISGasNetworkInputWidget::handleAssetsLoaded);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(0);

    mainLayout->addWidget(thePipelinesWidget);

    // Testing to remove start
    // theNodesWidget->setPathToComponentFile("/Users/steve/Desktop/SimCenter/Examples/SeasideGas/Nodes/Seaside_wter_nodes.shp");
    // theNodesWidget->loadAssetData();
    // theNodesWidget->setCRS(QgsCoordinateReferenceSystem("EPSG:4326"));


    // thePipelinesWidget->setPathToComponentFile("/Users/steve/Desktop/SimCenter/Examples/SeasideGas/Pipelines/Seaside_Gas_pipelines_wgs84.shp");
    // thePipelinesWidget->loadAssetData();
    // thePipelinesWidget->setCRS(QgsCoordinateReferenceSystem("EPSG:4326"));

    // inpFileLineEdit->setText("/Users/steve/Desktop/SogaExample/central.inp");
    // Testing to remove end
}


GISGasNetworkInputWidget::~GISGasNetworkInputWidget()
{

}


bool GISGasNetworkInputWidget::copyFiles(QString &destName)
{

    // The file containing the network pipelines
    if (!thePipelinesWidget->copyFiles(destName))
        return false;

    if (pathToGISfiles != "")
    {

        // Clean the path and remove the trailing slash
        QString cleanPath = QDir::cleanPath(pathToGISfiles);
        QDir cleanDir(cleanPath);

        // Get the last folder name by extracting the last segment of the clean path
        QString folderName = cleanDir.dirName();

        auto currShakeMapInputPath = pathToGISfiles;
        auto currShakeMapDestPath = destName + QDir::separator() + folderName;
        if(!SCUtils::recursiveCopy(currShakeMapInputPath, currShakeMapDestPath))
        {
            QString msg = "Error copying GIS files over to the directory "+folderName;
            errorMessage(msg);

            return false;
        }
    }


    return true;
}

#ifdef OpenSRA

bool GISGasNetworkInputWidget::outputToJSON(QJsonObject &rvObject)
{
    return thePipelinesWidget->outputToJSON(rvObject);
}


bool GISGasNetworkInputWidget::inputFromJSON(QJsonObject &rvObject)
{

    return thePipelinesWidget->inputFromJSON(rvObject);
}

#endif


bool GISGasNetworkInputWidget::outputAppDataToJSON(QJsonObject &jsonObject)
{

    // The file containing the network pipelines
    thePipelinesWidget->outputAppDataToJSON(jsonObject);

    // Get the json object
    auto appData = jsonObject["ApplicationData"].toObject();

    if (!siteLocationParams.empty())
        appData["siteLocationParams"] = siteLocationParams;

    if ("pathToGISFiles" != "")
        appData["pathToGISFiles"] = pathToGISfiles;


    jsonObject["ApplicationData"] = appData;

    return true;
}


bool GISGasNetworkInputWidget::inputAppDataFromJSON(QJsonObject &jsonObject)
{

    // Check the app type
    if (jsonObject.contains("Application")) {
        if ("GIS_to_PIPELINE" != jsonObject["Application"].toString()) {
            this->errorMessage("GISGasNetworkInputWidget::inputFRommJSON app name conflict");
            return false;
        }
    }


    if (!jsonObject.contains("ApplicationData"))
    {
        this->errorMessage("GISGasNetworkInputWidget::inputFRommJSON app name conflict");
        return false;
    }

    jsonObject["Application"] = "GIS_to_AIM";

    // Input the pipes
    auto res = thePipelinesWidget->inputAppDataFromJSON(jsonObject);

    if(!res)
        return res;

    auto appData=jsonObject["ApplicationData"].toObject();

    if (appData.contains("siteLocationParams"))
        siteLocationParams = appData["siteLocationParams"].toObject();

    if (appData.contains("pathToGISFiles"))
        pathToGISfiles = appData["pathToGISFiles"].toString();


    return true;
}


int GISGasNetworkInputWidget::loadPipelinesVisualization()
{
    pipelinesMainLayer = thePipelinesWidget->getMainLayer();

    if(pipelinesMainLayer==nullptr)
        return -1;


    QgsLineSymbol* markerSymbol = new QgsLineSymbol();

    markerSymbol->setWidth(0.8);
    markerSymbol->setColor(Qt::darkBlue);
    theVisualizationWidget->createSimpleRenderer(markerSymbol,pipelinesMainLayer);

    //    auto numFeat = mainLayer->featureCount();

    theVisualizationWidget->zoomToLayer(pipelinesMainLayer);

    auto tableHeadings = pipelinesMainLayer->fields().names();

    emit headingValuesChanged(tableHeadings);

    return 0;
}


void GISGasNetworkInputWidget::clear()
{
    thePipelinesWidget->clear();

    emit headingValuesChanged(QStringList{"N/A"});

    pathToGISfiles.clear();
    siteLocationParams = QJsonObject();
}


void GISGasNetworkInputWidget::handleAssetsLoaded()
{
    if(thePipelinesWidget->isEmpty())
        return;

    auto res = this->loadPipelinesVisualization();

    if(res != 0)
    {
        this->errorMessage("Error, failed to load the Gas network pipelines visualization");
        return;
    }

}

