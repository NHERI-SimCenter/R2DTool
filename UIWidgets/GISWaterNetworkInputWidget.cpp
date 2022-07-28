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

#include "GISWaterNetworkInputWidget.h"
#include "QGISVisualizationWidget.h"
#include "GISAssetInputWidget.h"

#include <qgslinesymbol.h>
#include <qgsmarkersymbol.h>

#include <QFileDialog>
#include <QSplitter>
#include <QGroupBox>
#include <QVBoxLayout>

GISWaterNetworkInputWidget::GISWaterNetworkInputWidget(QWidget *parent, VisualizationWidget* visWidget) : SimCenterAppWidget(parent)
{
    theVisualizationWidget = static_cast<QGISVisualizationWidget*>(visWidget);
    assert(theVisualizationWidget);

    theNodesWidget = new GISAssetInputWidget(this, theVisualizationWidget, "Water Network Nodes");

    theNodesWidget->setLabel1("Load water network node information from a GIS file");

    thePipelinesWidget = new GISAssetInputWidget(this, theVisualizationWidget, "Water Network Pipelines");

    thePipelinesWidget->setLabel1("Load water network pipeline information from a GIS file");

    connect(theNodesWidget,&GISAssetInputWidget::doneLoadingComponents,this,&GISWaterNetworkInputWidget::handleAssetsLoaded);
    connect(thePipelinesWidget,&GISAssetInputWidget::doneLoadingComponents,this,&GISWaterNetworkInputWidget::handleAssetsLoaded);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QSplitter* verticalSplitter = new QSplitter(Qt::Vertical);

    QGroupBox* nodesGB = new QGroupBox("Water Network Nodes");
    nodesGB->setFlat(true);
    QVBoxLayout* nodesGBlayout = new QVBoxLayout(nodesGB);
    nodesGBlayout->addWidget(theNodesWidget);

    QGroupBox* pipelinesGB = new QGroupBox("Water Network Pipelines");
    pipelinesGB->setFlat(true);
    QVBoxLayout* pipesGBlayout = new QVBoxLayout(pipelinesGB);
    pipesGBlayout->addWidget(thePipelinesWidget);

    verticalSplitter->addWidget(nodesGB);
    verticalSplitter->addWidget(pipelinesGB);

    mainLayout->addWidget(verticalSplitter);

    // Testing to remove start
    // theNodesWidget->setPathToComponentFile("/Users/steve/Desktop/SimCenter/Examples/SeasideWater/Nodes/Seaside_wter_nodes.shp");
    // theNodesWidget->loadAssetData();
    // theNodesWidget->setCRS(QgsCoordinateReferenceSystem("EPSG:4326"));


    // thePipelinesWidget->setPathToComponentFile("/Users/steve/Desktop/SimCenter/Examples/SeasideWater/Pipelines/Seaside_water_pipelines_wgs84.shp");
    // thePipelinesWidget->loadAssetData();
    // thePipelinesWidget->setCRS(QgsCoordinateReferenceSystem("EPSG:4326"));

    // inpFileLineEdit->setText("/Users/steve/Desktop/SogaExample/central.inp");
    // Testing to remove end
}


GISWaterNetworkInputWidget::~GISWaterNetworkInputWidget()
{

}


bool GISWaterNetworkInputWidget::copyFiles(QString &destName)
{

    auto res = theNodesWidget->copyFiles(destName);

    if(!res)
        return res;

    // The file containing the network pipelines
    res = thePipelinesWidget->copyFiles(destName);

    return res;
}


bool GISWaterNetworkInputWidget::outputAppDataToJSON(QJsonObject &jsonObject)
{
    jsonObject["Application"]="GIS_to_WATERNETWORK";

    QJsonObject data;

    QJsonObject nodeData;
    // The file containing the network nodes
    auto res = theNodesWidget->outputAppDataToJSON(nodeData);

    if(!res)
    {
        this->errorMessage("Error, could not get the .json output from the nodes widget in GIS_to_WATERNETWORK");
        return false;
    }

    data["WaterNetworkNodes"] = nodeData;

    QJsonObject pipelineData;
    // The file containing the network pipelines
    res = thePipelinesWidget->outputAppDataToJSON(pipelineData);

    if(!res)
    {
        this->errorMessage("Error, could not get the .json output from the pipelines widget in GIS_to_WATERNETWORK");
        return false;
    }

    data["WaterNetworkPipelines"] = pipelineData;

    jsonObject["ApplicationData"] = data;

    return true;
}


bool GISWaterNetworkInputWidget::inputAppDataFromJSON(QJsonObject &jsonObject)
{

    // Check the app type
    if (jsonObject.contains("Application")) {
        if ("GIS_to_WATERNETWORK" != jsonObject["Application"].toString()) {
            this->errorMessage("GISWaterNetworkInputWidget::inputFRommJSON app name conflict");
            return false;
        }
    }


    if (!jsonObject.contains("ApplicationData"))
    {
        this->errorMessage("GISWaterNetworkInputWidget::inputFRommJSON app name conflict");
        return false;
    }

    QJsonObject appData = jsonObject["ApplicationData"].toObject();


    if (!appData.contains("WaterNetworkNodes") && !appData.contains("WaterNetworkPipelines"))
    {
        this->errorMessage("GISWaterNetworkInputWidget needs WaterNetworkNodes and WaterNetworkPipelines");
        return false;
    }

    QJsonObject nodesData = appData["WaterNetworkNodes"].toObject();

    // Input the nodes
    auto res = theNodesWidget->inputAppDataFromJSON(nodesData);

    if(!res)
        return res;


    QJsonObject pipelinesData = appData["WaterNetworkPipelines"].toObject();


    // Input the pipes
    res = thePipelinesWidget->inputAppDataFromJSON(pipelinesData);

    if(!res)
        return res;


    return true;
}


int GISWaterNetworkInputWidget::loadPipelinesVisualization()
{
    pipelinesMainLayer = thePipelinesWidget->getAssetLayer();

    if(pipelinesMainLayer==nullptr)
        return -1;


    QgsLineSymbol* markerSymbol = new QgsLineSymbol();

    markerSymbol->setWidth(0.8);
    markerSymbol->setColor(Qt::darkBlue);
    theVisualizationWidget->createSimpleRenderer(markerSymbol,pipelinesMainLayer);

    //    auto numFeat = mainLayer->featureCount();

    theVisualizationWidget->zoomToLayer(pipelinesMainLayer);

    return 0;
}


int GISWaterNetworkInputWidget::loadNodesVisualization()
{
    nodesMainLayer = theNodesWidget->getAssetLayer();

    if(nodesMainLayer==nullptr)
        return -1;

    QgsSymbol* markerSymbol = new QgsMarkerSymbol();

    markerSymbol->setColor(Qt::blue);
    theVisualizationWidget->createSimpleRenderer(markerSymbol,nodesMainLayer);

    //    auto numFeat = mainLayer->featureCount();

    theVisualizationWidget->zoomToLayer(nodesMainLayer);

    return 0;
}


void GISWaterNetworkInputWidget::clear()
{
    theNodesWidget->clear();
    thePipelinesWidget->clear();
}


void GISWaterNetworkInputWidget::handleAssetsLoaded()
{
    if(theNodesWidget->isEmpty() || thePipelinesWidget->isEmpty())
        return;

    auto res = this->loadNodesVisualization();

    if(res != 0)
    {
        this->errorMessage("Error, failed to load the water network nodes visualization");
        return;
    }

    res = this->loadPipelinesVisualization();

    if(res != 0)
    {
        this->errorMessage("Error, failed to load the water network pipelines visualization");
        return;
    }

}

