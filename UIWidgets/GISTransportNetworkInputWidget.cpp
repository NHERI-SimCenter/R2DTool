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

    theBridgesWidget = new GISAssetInputWidget(this, theVisualizationWidget, "Bridges");

    theBridgesWidget->setLabel1("Load Bridge Data from a GIS file");

    theRoadwaysWidget = new GISAssetInputWidget(this, theVisualizationWidget, "Roads");

    theRoadwaysWidget->setLabel1("Load Roadway Data from a GIS file");

    theTunnelsWidget = new GISAssetInputWidget(this, theVisualizationWidget, "Tunnels");

    theTunnelsWidget->setLabel1("Load Tunnel Data from a GIS file");


    connect(theBridgesWidget,&GISAssetInputWidget::doneLoadingComponents,this,&GISTransportNetworkInputWidget::handleAssetsLoaded);
    connect(theRoadwaysWidget,&GISAssetInputWidget::doneLoadingComponents,this,&GISTransportNetworkInputWidget::handleAssetsLoaded);
    connect(theTunnelsWidget,&GISAssetInputWidget::doneLoadingComponents,this,&GISTransportNetworkInputWidget::handleAssetsLoaded);


    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QSplitter* verticalSplitter = new QSplitter(Qt::Vertical);

    QGroupBox* roadwaysGB = new QGroupBox("Roadways");
    roadwaysGB->setFlat(true);
    QVBoxLayout* roadwaysGBlayout = new QVBoxLayout(roadwaysGB);
    roadwaysGBlayout->addWidget(theRoadwaysWidget);

//    roadLengthWidget = new QWidget();
//    QHBoxLayout* roadLengthLayout = new QHBoxLayout(roadLengthWidget);
//    QLabel* roadLengthLabel = new QLabel("Maximum roadway length (m) per AIM",this);
//    roadLengthLineEdit = new QLineEdit(this);
//    roadLengthLineEdit->setText("100.0");
//    QDoubleValidator *validator = new QDoubleValidator(this);
//    validator->setBottom(0.0);
//    roadLengthLayout->addWidget(roadLengthLabel);
//    roadLengthLayout->addWidget(roadLengthLineEdit);
//    roadwaysGBlayout->addWidget(roadLengthWidget);
//    connect(roadLengthLineEdit, &QLineEdit::editingFinished, this, &GISTransportNetworkInputWidget::printRoadLengthInput);
    
    QGroupBox* bridgesGB = new QGroupBox("Bridges");
    bridgesGB->setFlat(true);
    QVBoxLayout* bridgesGBlayout = new QVBoxLayout(bridgesGB);
    bridgesGBlayout->addWidget(theBridgesWidget);

    QGroupBox* tunnelsGB = new QGroupBox("Tunnels");
    tunnelsGB->setFlat(true);
    QVBoxLayout* tunnelsGBlayout = new QVBoxLayout(tunnelsGB);
    tunnelsGBlayout->addWidget(theTunnelsWidget);

    verticalSplitter->addWidget(roadwaysGB);    
    verticalSplitter->addWidget(bridgesGB);
    verticalSplitter->addWidget(tunnelsGB);

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
    if(theTunnelsWidget->isEmpty() && theBridgesWidget->isEmpty() && theRoadwaysWidget->isEmpty()){
        QString msg = "No asset selected for Transportation Network.\n Please load or unselect Transportation Network in General Information";
        this->errorMessage(msg);
        return false;
    }
    destFolder = destName;
    QJsonArray featuresArray;
    QJsonObject combinedGeoJSON;
    combinedGeoJSON["type"]="FeatureCollection";

    QString crsString = R"(
        {
            "type": "name",
            "properties": {
                "name": "urn:ogc:def:crs:OGC:1.3:CRS84"
            }
        }
    )";
    QJsonDocument crsDoc = QJsonDocument::fromJson(crsString.toUtf8());
    QJsonObject crsJson = crsDoc.object();
    combinedGeoJSON.insert("crs", crsJson);
    if(theBridgesWidget->getSelectedLayer() != nullptr){
        QgsVectorLayer* layer = theBridgesWidget->getSelectedLayer();
        QString type = "Bridge";
        exportLayerToGeoJSON(layer, featuresArray, type);
    }
    if(theRoadwaysWidget->getSelectedLayer() != nullptr){
        QgsVectorLayer* layer = theRoadwaysWidget->getSelectedLayer();
        QString type = "Roadway";
        exportLayerToGeoJSON(layer, featuresArray, type);
    }
    if(theTunnelsWidget->getSelectedLayer() != nullptr){
        QgsVectorLayer* layer = theTunnelsWidget->getSelectedLayer();
        QString type = "Tunnel";
        exportLayerToGeoJSON(layer, featuresArray, type);
    }
    combinedGeoJSON.insert("features", featuresArray);
    QString destFile = destFolder + QDir::separator() + tr("simcenter_trnsp_inventory.geojson");
    QFile file(destFile);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        return false;
    }
    QJsonDocument doc(combinedGeoJSON);
    file.write(doc.toJson());
    file.close();

    return true;


}

void GISTransportNetworkInputWidget::exportLayerToGeoJSON(QgsVectorLayer* layer, QJsonArray& featArray, QString assetType){
    QgsJsonExporter exporter = QgsJsonExporter(layer);
    QgsCoordinateReferenceSystem source_crs = layer->sourceCrs();
    QgsCoordinateReferenceSystem target_crs = QgsCoordinateReferenceSystem("EPSG:4326");
    bool need_reproject = (source_crs.toWkt() != target_crs.toWkt());
    QgsFeatureIterator featIt = layer->getFeatures();
    QgsFeature feat;
    if (need_reproject){
        while (featIt.nextFeature(feat))
        {
            auto newFeat = QgsFeature(feat); // Use the feature copy constructor
            // Do a deep clone of the geometry
            QgsGeometry newGeom = QgsGeometry(feat.geometry().get()->clone());
            newGeom.transform(QgsCoordinateTransform(source_crs, target_crs, QgsProject::instance()));
            newFeat.setGeometry(newGeom);
            QString featData = exporter.exportFeature(newFeat);
            QJsonDocument doc = QJsonDocument::fromJson(featData.toUtf8());
            QJsonObject featJSON = doc.object();
            QJsonObject prop = featJSON["properties"].toObject();
            prop.insert("type",assetType);
            featJSON["properties"] = prop;
            featArray.append(featJSON);
        }
    } else {
        while (featIt.nextFeature(feat))
        {
            QString featData = exporter.exportFeature(feat);
            QJsonDocument doc = QJsonDocument::fromJson(featData.toUtf8());
            QJsonObject featJSON = doc.object();
            QJsonObject prop = featJSON["properties"].toObject();
            prop.insert("type",assetType);
            featJSON["properties"] = prop;
            featArray.append(featJSON);
        }
    }

}


bool GISTransportNetworkInputWidget::outputAppDataToJSON(QJsonObject &jsonObject)
{
    jsonObject["Application"]="GEOJSON_TO_ASSET";

    QJsonObject data;

    if(theBridgesWidget->isEmpty() && theTunnelsWidget->isEmpty() && theRoadwaysWidget->isEmpty())
    {
        this->errorMessage("Error, could not get the .json output from the transportation widget in GIS_to_TRANSPORTNETWORK");
        return false;
    }

    if (!theBridgesWidget->isEmpty()){
        QJsonObject filter {{"filter",""}};
        data.insert("Bridge", filter);
    }
    if (!theTunnelsWidget->isEmpty()){
        QJsonObject filter {{"filter",""}};
        data.insert("Tunnel", filter);
    }
    if (!theRoadwaysWidget->isEmpty()){
        QJsonObject filter {{"filter",""}};
        data.insert("Roadway", filter);
    }
    if (destFolder.compare("")==0){
        return false;
    }
    QString destFile = destFolder + QDir::separator() + tr("simcenter_trnsp_inventory.geojson");
    data.insert("assetSourceFile", destFile);
    jsonObject["ApplicationData"] = data;

    return true;
}


bool GISTransportNetworkInputWidget::inputAppDataFromJSON(QJsonObject &jsonObject)
{

    // Check the app type
    if (jsonObject.contains("Application")) {
        if ("GIS_to_TRANSPORTNETWORK" != jsonObject["Application"].toString()) {
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


    if (!appData.contains("TransportNetworkBridges") && !appData.contains("TransportNetworkRoadways") && !appData.contains("TransportNetworkTunnels"))
    {
        this->errorMessage("GISTransportNetworkInputWidget needs TransportNetworkBridges or TransportNetworkRoadways or TransportNetworkTunnels");
        return false;
    }


    if (appData.contains("TransportNetworkBridges")){
        QJsonObject bridgesData = appData["TransportNetworkBridges"].toObject();

        // Input the bridges
        auto res = theBridgesWidget->inputAppDataFromJSON(bridgesData);

        if(!res)
            return res;
    }

    if (appData.contains("TransportNetworkRoadways")){
        QJsonObject roadwaysData = appData["TransportNetworkRoadways"].toObject();

        // Input the roadways
        auto res = theRoadwaysWidget->inputAppDataFromJSON(roadwaysData);

        if(!res){
            return res;
        }
        if (roadwaysData.contains("ApplicationData")){
            QJsonObject roadwaysAppData = roadwaysData["ApplicationData"].toObject();
            if (roadwaysAppData.contains("roadSegLength")) {
//                roadLengthLineEdit->setText(QString::number(roadwaysAppData["roadSegLength"].toDouble()));
            }
        }
    }

    if (appData.contains("TransportNetworkTunnels")){
        QJsonObject tunnelsData = appData["TransportNetworkTunnels"].toObject();

        // Input the roadways
        auto res = theTunnelsWidget->inputAppDataFromJSON(tunnelsData);

        if(!res)
            return res;
    }


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

    markerSymbol->setColor(Qt::red);
    theVisualizationWidget->createSimpleRenderer(markerSymbol,bridgesMainLayer);

    //    auto numFeat = mainLayer->featureCount();

    theVisualizationWidget->zoomToLayer(bridgesMainLayer);

    return 0;
}

int GISTransportNetworkInputWidget::loadTunnelsVisualization()
{
    tunnelsMainLayer = theTunnelsWidget->getMainLayer();

    if(bridgesMainLayer==nullptr)
        return -1;

    QgsSymbol* markerSymbol = new QgsMarkerSymbol();

    markerSymbol->setColor(Qt::red);
    theVisualizationWidget->createSimpleRenderer(markerSymbol,tunnelsMainLayer);

    //    auto numFeat = mainLayer->featureCount();

    theVisualizationWidget->zoomToLayer(tunnelsMainLayer);

    return 0;
}


void GISTransportNetworkInputWidget::clear()
{
    theBridgesWidget->clear();
    theRoadwaysWidget->clear();
    theTunnelsWidget->clear();
//    roadLengthLineEdit->clear();

    bridgesMainLayer = nullptr;
    roadwaysMainLayer = nullptr;
    tunnelsMainLayer = nullptr;
}


void GISTransportNetworkInputWidget::handleAssetsLoaded()
{
    if(theBridgesWidget->isEmpty() ){}
    else{
        auto res = this->loadBridgesVisualization();

        if(res != 0)
        {
            this->errorMessage("Error, failed to load the Bridges visualization");
            return;
        }
    }

    if(theTunnelsWidget->isEmpty()){}
    else{
        auto res = this->loadTunnelsVisualization();

        if(res != 0)
        {
            this->errorMessage("Error, failed to load the Tunnels visualization");
            return;
        }
    }

    if (theRoadwaysWidget->isEmpty()){}
    else{
        auto res = this->loadRoadwaysVisualization();

        if(res != 0)
        {
            this->errorMessage("Error, failed to load the Roadways visualization");
            return;
        }
    }
}


void GISTransportNetworkInputWidget::printRoadLengthInput(void){
//    QString msg = "Roadway length per AIM is set as "+ roadLengthLineEdit->text() + " meters";
//    this->statusMessage(msg);
}

