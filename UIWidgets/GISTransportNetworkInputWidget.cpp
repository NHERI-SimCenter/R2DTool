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

#include "GISTransportNetworkInputWidget.h"
#include "QGISVisualizationWidget.h"
#include "GISAssetInputWidget.h"

#include <qgslinesymbol.h>
#include <qgsmarkersymbol.h>

#include <QFileDialog>
#include <QSplitter>
#include <QGroupBox>
#include <QVBoxLayout>

GISTransportNetworkInputWidget::GISTransportNetworkInputWidget(QWidget *parent, VisualizationWidget* visWidget) : SimCenterAppWidget(parent)
{
    theVisualizationWidget = static_cast<QGISVisualizationWidget*>(visWidget);
    assert(theVisualizationWidget);

    theBridgesWidget = new GISAssetInputWidget(this, theVisualizationWidget, "Bridge Network");

    theBridgesWidget->setLabel1("Load Bridge Data from a GIS file");

    theRoadwaysWidget = new GISAssetInputWidget(this, theVisualizationWidget, "Roadway Network");

    theRoadwaysWidget->setLabel1("Load Roadway Data from a GIS file");

    connect(theBridgesWidget,&GISAssetInputWidget::doneLoadingComponents,this,&GISTransportNetworkInputWidget::handleAssetsLoaded);
    connect(theRoadwaysWidget,&GISAssetInputWidget::doneLoadingComponents,this,&GISTransportNetworkInputWidget::handleAssetsLoaded);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QSplitter* verticalSplitter = new QSplitter(Qt::Vertical);

    QGroupBox* roadwaysGB = new QGroupBox("Roadways");
    roadwaysGB->setFlat(true);
    QVBoxLayout* roadwaysGBlayout = new QVBoxLayout(roadwaysGB);
    roadwaysGBlayout->addWidget(theRoadwaysWidget);
    
    QGroupBox* bridgesGB = new QGroupBox("Bridges");
    bridgesGB->setFlat(true);
    QVBoxLayout* bridgesGBlayout = new QVBoxLayout(bridgesGB);
    bridgesGBlayout->addWidget(theBridgesWidget);

    verticalSplitter->addWidget(roadwaysGB);    
    verticalSplitter->addWidget(bridgesGB);

    mainLayout->addWidget(verticalSplitter);

    // Testing to remove start
    // theBridgesWidget->setPathToComponentFile("/Users/steve/Desktop/SimCenter/Examples/SeasideTransport/Bridges/Seaside_wter_bridges.shp");
    // theBridgesWidget->loadAssetData();
    // theBridgesWidget->setCRS(QgsCoordinateReferenceSystem("EPSG:4326"));


    // theRoadwaysWidget->setPathToComponentFile("/Users/steve/Desktop/SimCenter/Examples/SeasideTransport/Roadways/Seaside_Transport_roadways_wgs84.shp");
    // theRoadwaysWidget->loadAssetData();
    // theRoadwaysWidget->setCRS(QgsCoordinateReferenceSystem("EPSG:4326"));

    // inpFileLineEdit->setText("/Users/steve/Desktop/SogaExample/central.inp");
    // Testing to remove end
}


GISTransportNetworkInputWidget::~GISTransportNetworkInputWidget()
{

}


bool GISTransportNetworkInputWidget::copyFiles(QString &destName)
{

    auto res = theBridgesWidget->copyFiles(destName);

    if(!res)
        return res;

    // The file containing the network roadways
    res = theRoadwaysWidget->copyFiles(destName);

    return res;
}


bool GISTransportNetworkInputWidget::outputAppDataToJSON(QJsonObject &jsonObject)
{
    jsonObject["Application"]="GIS_to_TransportNETWORK";

    QJsonObject data;

    QJsonObject nodeData;
    // The file containing the network bridges
    auto res = theBridgesWidget->outputAppDataToJSON(nodeData);

    if(!res)
    {
        this->errorMessage("Error, could not get the .json output from the bridges widget in GIS_to_TransportNETWORK");
        return false;
    }

    data["TransportNetworkBridges"] = nodeData;

    QJsonObject pipelineData;
    // The file containing the network roadways
    res = theRoadwaysWidget->outputAppDataToJSON(pipelineData);

    if(!res)
    {
        this->errorMessage("Error, could not get the .json output from the roadways widget in GIS_to_TransportNETWORK");
        return false;
    }

    data["TransportNetworkRoadways"] = pipelineData;

    jsonObject["ApplicationData"] = data;

    return true;
}


bool GISTransportNetworkInputWidget::inputAppDataFromJSON(QJsonObject &jsonObject)
{

    // Check the app type
    if (jsonObject.contains("Application")) {
        if ("GIS_to_TransportNETWORK" != jsonObject["Application"].toString()) {
            this->errorMessage("GISTransportNetworkInputWidget::inputFRommJSON app name conflict");
            return false;
        }
    }


    if (!jsonObject.contains("ApplicationData"))
    {
        this->errorMessage("Could not find the 'ApplicationData' key in 'GISTransportNetworkInputWidget' input");
        return false;
    }

    QJsonObject appData = jsonObject["ApplicationData"].toObject();


    if (!appData.contains("TransportNetworkBridges") && !appData.contains("TransportNetworkRoadways"))
    {
        this->errorMessage("GISTransportNetworkInputWidget needs TransportNetworkBridges and TransportNetworkRoadways");
        return false;
    }

    QJsonObject bridgesData = appData["TransportNetworkBridges"].toObject();

    // Input the bridges
    auto res = theBridgesWidget->inputAppDataFromJSON(bridgesData);

    if(!res)
        return res;


    QJsonObject roadwaysData = appData["TransportNetworkRoadways"].toObject();


    // Input the roadways
    res = theRoadwaysWidget->inputAppDataFromJSON(roadwaysData);

    if(!res)
        return res;


    return true;
}


int GISTransportNetworkInputWidget::loadRoadwaysVisualization()
{
    roadwaysMainLayer = theRoadwaysWidget->getMainLayer();

    if(roadwaysMainLayer==nullptr)
        return -1;


    QgsLineSymbol* markerSymbol = new QgsLineSymbol();

    markerSymbol->setWidth(0.8);
    markerSymbol->setColor(Qt::darkBlue);
    theVisualizationWidget->createSimpleRenderer(markerSymbol,roadwaysMainLayer);

    //    auto numFeat = mainLayer->featureCount();

    theVisualizationWidget->zoomToLayer(roadwaysMainLayer);

    return 0;
}


int GISTransportNetworkInputWidget::loadBridgesVisualization()
{
    bridgesMainLayer = theBridgesWidget->getMainLayer();

    if(bridgesMainLayer==nullptr)
        return -1;

    QgsSymbol* markerSymbol = new QgsMarkerSymbol();

    markerSymbol->setColor(Qt::blue);
    theVisualizationWidget->createSimpleRenderer(markerSymbol,bridgesMainLayer);

    //    auto numFeat = mainLayer->featureCount();

    theVisualizationWidget->zoomToLayer(bridgesMainLayer);

    return 0;
}


void GISTransportNetworkInputWidget::clear()
{
    theBridgesWidget->clear();
    theRoadwaysWidget->clear();

    bridgesMainLayer = nullptr;
    roadwaysMainLayer = nullptr;
}


void GISTransportNetworkInputWidget::handleAssetsLoaded()
{
    if(theBridgesWidget->isEmpty() || theRoadwaysWidget->isEmpty())
        return;

    auto res = this->loadBridgesVisualization();

    if(res != 0)
    {
        this->errorMessage("Error, failed to load the Bridges visualization");
        return;
    }

    res = this->loadRoadwaysVisualization();

    if(res != 0)
    {
        this->errorMessage("Error, failed to load the Roadways visualization");
        return;
    }

}

