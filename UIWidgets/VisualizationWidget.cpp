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

#include "ComponentInputWidget.h"
#include "PopUpWidget.h"
#include "SimCenterMapGraphicsView.h"
#include "LayerTreeItem.h"
#include "LayerTreeView.h"
#include "LayerTreeModel.h"
#include "VisualizationWidget.h"
#include "XMLAdaptor.h"
#include "ConvexHull.h"
#include "PolygonBoundary.h"

// GIS headers
#include "ArcGISMapImageLayer.h"
#include "ArcGISTiledLayer.h"
#include "Basemap.h"
#include "ClassBreaksRenderer.h"
#include "CoordinateFormatter.h"
#include "FeatureCollection.h"
#include "EnvelopeBuilder.h"
#include "FeatureCollectionLayer.h"
#include "FeatureCollectionTable.h"
#include "FeatureLayer.h"
#include "Geodatabase.h"
#include "GeodatabaseFeatureTable.h"
#include "GeoElement.h"
#include "GeographicTransformation.h"
#include "GeographicTransformationStep.h"
#include "GroupLayer.h"
#include "IdentifyLayerResult.h"
#include "KmlDataset.h"
#include "KmlLayer.h"
#include "LayerContent.h"
#include "LineSegment.h"
#include "Map.h"
#include "MapGraphicsView.h"
#include "PictureMarkerSymbol.h"
#include "PictureMarkerSymbolLayer.h"
#include "PolylineBuilder.h"
#include "PopupManager.h"
#include "RasterLayer.h"
#include "PolygonBuilder.h"
#include "ShapefileFeatureTable.h"
#include "SimpleMarkerSymbol.h"
#include "SimpleRenderer.h"
#include "TransformationCatalog.h"
#include "sectiontitle.h"
// Convex Hull
#include "GeometryEngine.h"
#include "MultipointBuilder.h"
#include "SimpleFillSymbol.h"
#include "SimpleLineSymbol.h"

#include <QComboBox>
#include <QCoreApplication>
#include <QFileInfo>
#include <QJsonArray>
#include <QGridLayout>
#include <QGroupBox>
#include <QToolButton>
#include <QHeaderView>
#include <QSplitter>
#include <QLabel>
#include <QPushButton>
#include <QString>
#include <QTableWidget>
#include <QThread>
#include <QTreeView>
#include <QListView>
#include <QUrl>

#include <utility>

using namespace Esri::ArcGISRuntime;


VisualizationWidget::VisualizationWidget(QWidget* parent) : SimCenterAppWidget(parent)
{    
    legendView = nullptr;
    visWidget = nullptr;
    this->setContentsMargins(0,0,0,0);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setMargin(0);

    setAcceptDrops(true);
    // Create the header layout and add it to the main layout
    //    QHBoxLayout *theHeaderLayout = new QHBoxLayout();
    //    SectionTitle *label = new SectionTitle();
    //    label->setText(QString("Visualization"));
    //    label->setMinimumWidth(150);

    //    theHeaderLayout->addWidget(label);
    //    QSpacerItem *spacer = new QSpacerItem(50,10);
    //    theHeaderLayout->addItem(spacer);
    //    theHeaderLayout->addStretch(1);
    //    mainLayout->addLayout(theHeaderLayout);

    pipelineWidget = nullptr;

    // Create the Widget view
    //mapViewWidget = new MapGraphicsView(this);
    mapViewLayout = new QVBoxLayout();
    mapViewLayout->setMargin(0);
    mapViewLayout->setSpacing(0);

    mapViewWidget = SimCenterMapGraphicsView::getInstance();

    mapViewWidget->setCurrentLayout(mapViewLayout);

    mapViewWidget->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    // Create a map using the topographic Basemap
    mapGIS = new Map(Basemap::topographic(this), this);

    mapGIS->setAutoFetchLegendInfos(false);

    mapGIS->setObjectName("MainMap");

    mapViewWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    // Set map to map view
    mapViewWidget->setMap(mapGIS);

    // Set the initial viewport to UCB
    Viewpoint UCB(37.8717450069, -122.2609607382, 100000.0);
    //    Viewpoint UCB(61.216663, -149.907122, 100000.0);

    mapGIS->setInitialViewpoint(UCB);


    // Setup the various convex hull objects
    theConvexHullTool = std::make_unique<ConvexHull>(this);
    thePolygonBoundaryTool  = std::make_unique<PolygonBoundary>(this);

    // Create the visualization widget and set it to the main layout
    this->createVisualizationWidget();

    mainLayout->addWidget(visWidget);

    this->setLayout(mainLayout);
    //this->setMinimumWidth(640);

    // Popup stuff

    // Once map is set, connect to MapQuickView mouse clicked signal
    connect(mapViewWidget, &MapGraphicsView::mouseClicked, this, &VisualizationWidget::onMouseClicked);
    // Connect to MapQuickView::identifyLayerCompleted signal
    connect(mapViewWidget, &MapGraphicsView::identifyLayersCompleted, this, &VisualizationWidget::identifyLayersCompleted);
    // Connect to the exportImageCompleted signal
    connect(mapViewWidget, &MapGraphicsView::exportImageCompleted, this, &VisualizationWidget::exportImageComplete);

    this->setLegendView(mapViewWidget->getLegendView());

    // Create a new graphics overlay for the features selected by clicking
    selectedFeaturesOverlay = new GraphicsOverlay(this);

    // Add the overlay to the mapview
    mapViewWidget->graphicsOverlays()->append(selectedFeaturesOverlay);


    //        ArcGISTiledLayer* tiledLayer = new ArcGISTiledLayer(QUrl("https://services.arcgisonline.com/ArcGIS/rest/services/Specialty/Soil_Survey_Map/MapServer"), this);
    //        mapGIS->operationalLayers()->append(tiledLayer);

    // Test
    //    QString filePath = "/Users/steve/Desktop/SimCenter/Examples/SFTallBuildings/TallBuildingInventory.kmz";
    //    QString layerName = "Buildings Foot Print";
    //    QString layerID = this->createUniqueID();
    //    LayerTreeItem* buildingsItem = layersTree->addItemToTree(layerName,layerID);
    //    auto buildingsLayer = this->createAndAddKMLLayer(filePath, layerName, buildingsItem);
    //    buildingsLayer->setLayerId(layerID);
    //    buildingsLayer->setName("SF");
    //    this->addLayerToMap(buildingsLayer,buildingsItem);


    //   QString layerName = "Bathymetry";
    //   QString layerID = this->createUniqueID();
    //   LayerTreeItem* LayerTreeItem = layersTree->addItemToTree(layerName,layerID);

    //   QString filePath = "/Users/steve/Downloads/GEBCO_2020_18_Nov_2020_f103650dc2c4/gebco_2020_n30.0_s15.0_w-179.0_e-152.0.tif";
    //   auto rastLayer = this->createAndAddRasterLayer(filePath, layerName, LayerTreeItem) ;
    //   rastLayer->setLayerId(layerID);
    //   rastLayer->setName(layerName);
    //   this->addLayerToMap(rastLayer,LayerTreeItem);

}


VisualizationWidget::~VisualizationWidget()
{
    mapViewWidget->setCurrentLayout(nullptr);
}


void VisualizationWidget::setCurrentlyViewable(bool status)
{
    if (status == true) {
        // emit sendErrorMessage("SWAPPING Visaulization Widget");
        QWidget *tmp = new QWidget();
        mapViewLayout->addWidget(tmp);
        mapViewWidget->setCurrentLayout(mapViewLayout);
        mapViewLayout->removeWidget(tmp);
        delete tmp;
        // emit sendErrorMessage("");
    }

    /*
        connect(mapViewWidget, &MapGraphicsView::mouseClicked, this, &VisualizationWidget::onMouseClicked);
        connect(mapViewWidget, &MapGraphicsView::identifyLayersCompleted, this, &VisualizationWidget::identifyLayersCompleted);
        connect(mapViewWidget, &MapGraphicsView::exportImageCompleted, this, &VisualizationWidget::exportImageComplete);
    } else {
        disconnect(mapViewWidget, &MapGraphicsView::mouseClicked, this, &VisualizationWidget::onMouseClicked);
        disconnect(mapViewWidget, &MapGraphicsView::identifyLayersCompleted, this, &VisualizationWidget::identifyLayersCompleted);
        disconnect(mapViewWidget, &MapGraphicsView::exportImageCompleted, this, &VisualizationWidget::exportImageComplete);
    }
    */
}


void VisualizationWidget::createVisualizationWidget(void)
{
    visWidget = new QSplitter(this);
    visWidget->setContentsMargins(5,0,0,0);

    auto leftHandWidget = new QWidget(this);
    leftHandWidget->setContentsMargins(0,0,0,0);

    QGridLayout* leftHandLayout = new QGridLayout(leftHandWidget);
    leftHandLayout->setMargin(0);

    auto smallVSpacer = new QSpacerItem(0,2);

    QLabel* basemapText = new QLabel(visWidget);
    basemapText->setText("Select basemap:");

    baseMapCombo = new QComboBox();
    baseMapCombo->addItem("Topographic");
    baseMapCombo->addItem("Terrain");
    baseMapCombo->addItem("Imagery");
    baseMapCombo->addItem("Open Street Map");
    baseMapCombo->addItem("Streets");
    baseMapCombo->addItem("Canvas (Light)");
    baseMapCombo->addItem("National Geographic");

    connect(baseMapCombo, SIGNAL(currentIndexChanged(QString)), this, SLOT(handleBasemapSelection(QString)));

    // The tree view class used to visualize the tree data model
    layersTree = new LayerTreeView(visWidget, this);

    QLabel* topText = new QLabel(visWidget);
    topText->setText("Enclose an area with points\nto select a subset of\nassets to analyze");
    topText->setStyleSheet("font-weight: bold; color: black; text-align: center");

    QPushButton *selectPointsButton = new QPushButton();
    selectPointsButton->setText(tr("Select Points"));
    selectPointsButton->setMaximumWidth(150);

    QPushButton *clearButton = new QPushButton();
    clearButton->setText(tr("Clear"));
    clearButton->setMaximumWidth(150);

    QLabel* bottomText = new QLabel(visWidget);
    bottomText->setText("Click the 'Apply' button to\nselect the subset of assets");
    bottomText->setStyleSheet("font-weight: bold; color: black; text-align: center");

    QPushButton *applyButton = new QPushButton(visWidget);
    applyButton->setText(tr("Apply"));
    applyButton->setMaximumWidth(150);

    connect(selectPointsButton,&QPushButton::clicked,this,&VisualizationWidget::handleSelectAreaMap);
    connect(clearButton,&QPushButton::clicked,this,&VisualizationWidget::handleClearSelectAreaMap);
    connect(applyButton,SIGNAL(clicked()),thePolygonBoundaryTool.get(),SLOT(getItemsInPolygonBoundary()));


    // Add a vertical spacer at the bottom to push everything up
    auto vspacer = new QSpacerItem(0,0,QSizePolicy::Maximum, QSizePolicy::Expanding);

    leftHandLayout->addItem(smallVSpacer,0,0);
    leftHandLayout->addWidget(basemapText,1,0);
    leftHandLayout->addWidget(baseMapCombo,2,0);
    leftHandLayout->addItem(smallVSpacer,3,0);
    leftHandLayout->addWidget(layersTree,4,0);
    leftHandLayout->addWidget(topText,5,0);
    leftHandLayout->addWidget(selectPointsButton,6,0);
    leftHandLayout->addWidget(clearButton,7,0);
    leftHandLayout->addWidget(bottomText,8,0);
    leftHandLayout->addWidget(applyButton,9,0);
    leftHandLayout->addItem(vspacer,10,0);

    QWidget* subWidget = new QWidget(this);
    subWidget->setContentsMargins(0,0,0,0);
    subWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    subWidget->setLayout(mapViewLayout);


    //    QFrame *line = new QFrame(handle);
    //    line->setFrameShape(QFrame::HLine);
    //    line->setFrameShadow(QFrame::Sunken);
    //    layout->addWidget(line);


    //    connect(buttonHandle,&QToolButton::clicked,handle,&QSplitterHandle::hand)
    visWidget->addWidget(leftHandWidget);
    visWidget->addWidget(subWidget);

    visWidget->setStretchFactor(1,1);

    // Now add the line to the splitter handle
    // Note: index 0 handle is always hidden, index 1 is between the two widgets
    QSplitterHandle *handle = visWidget->handle(1);

    if(handle == nullptr)
    {
        qDebug()<<"Error getting the handle";
        return;
    }

    auto buttonHandle = new QToolButton(handle);
    QVBoxLayout *layout = new QVBoxLayout(handle);
    layout->setSpacing(0);
    layout->setMargin(0);

    visWidget->setHandleWidth(15);

    buttonHandle->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    buttonHandle->setDown(false);
    buttonHandle->setAutoRaise(false);
    buttonHandle->setCheckable(false);
    buttonHandle->setArrowType(Qt::RightArrow);
    buttonHandle->setStyleSheet("QToolButton{border:0px solid}; QToolButton:pressed {border:0px solid}");
    buttonHandle->setIconSize(buttonHandle->size());
    layout->addWidget(buttonHandle);

}


PolygonBoundary* VisualizationWidget::getThePolygonBoundaryTool(void) const
{
    return thePolygonBoundaryTool.get();
}


ComponentInputWidget *VisualizationWidget::getPipelineWidget() const
{
    return pipelineWidget;
}


LayerTreeView *VisualizationWidget::getLayersTree() const
{
    return layersTree;
}


void VisualizationWidget::setPipelineWidget(ComponentInputWidget *value)
{
    pipelineWidget = value;
    pipelineWidget->setTheVisualizationWidget(this);

    connect(pipelineWidget,&ComponentInputWidget::componentDataLoaded,this,&VisualizationWidget::loadPipelineData);
}


void VisualizationWidget::loadBuildingData(void)
{



}


void VisualizationWidget::changeLayerOrder(const int from, const int to)
{
    mapGIS->operationalLayers()->move(from, to);
}


void VisualizationWidget::loadPipelineData(void)
{
    auto pipelineTableWidget = pipelineWidget->getTableWidget();
    auto thePipelineDb = pipelineWidget->getComponentDatabase();

    QList<Field> fields;
    fields.append(Field::createDouble("RepairRate", "0.0"));
    fields.append(Field::createText("ID", "NULL",4));
    fields.append(Field::createText("AssetType", "NULL",4));
    fields.append(Field::createText("TabName", "NULL",4));
    fields.append(Field::createText("UID", "NULL",4));

    // Set the table headers as fields in the table
    for(int i =0; i<pipelineTableWidget->columnCount(); ++i)
    {
        auto headerItem = pipelineTableWidget->horizontalHeaderItem(i);
        auto fieldText = headerItem->text();
        fields.append(Field::createText(fieldText, fieldText,fieldText.size()));
    }

    // Create the pipelines group layer that will hold the sublayers
    auto pipelineLayer = new GroupLayer(QList<Layer*>{},this);
    pipelineLayer->setName("Components");

    // Add the pipeline layer to the map and get the root tree item
    auto pipelinesItem = this->addLayerToMap(pipelineLayer);

    if(pipelinesItem == nullptr)
    {
        qDebug()<<"Error adding item to the map";
        return;
    }

    auto nRows = pipelineTableWidget->rowCount();

    // Select a column that will define the layers
    //    int columnToMapLayers = 0;

    std::vector<std::string> vecLayerItems;
    //    for(int i = 0; i<nRows; ++i)
    //    {
    //        // Organize the layers according to occupancy type
    //        auto layerTag = pipelineTableWidget->item(i,columnToMapLayers)->data(0).toString().toStdString();

    //        vecLayerItems.push_back(layerTag);
    //    }

    vecLayerItems.push_back("Pipeline Network");

    //    this->uniqueVec<std::string>(vecLayerItems);

    // Map to hold the feature tables
    std::map<std::string, FeatureCollectionTable*> tablesMap;


    for(auto&& it : vecLayerItems)
    {
        auto featureCollection = new FeatureCollection(this);

        auto featureCollectionTable = new FeatureCollectionTable(fields, GeometryType::Polyline, SpatialReference::wgs84(),this);

        featureCollection->tables()->append(featureCollectionTable);

        auto newpipelineLayer = new FeatureCollectionLayer(featureCollection,this);

        newpipelineLayer->setName(QString::fromStdString(it));

        newpipelineLayer->setAutoFetchLegendInfos(true);

        featureCollectionTable->setRenderer(this->createPipelineRenderer());

        tablesMap.insert(std::make_pair(it,featureCollectionTable));

        this->addLayerToMap(newpipelineLayer,pipelinesItem, pipelineLayer);
    }

    for(int i = 0; i<nRows; ++i)
    {

        // create the feature attributes
        QMap<QString, QVariant> featureAttributes;

        // Create a new pipeline
        Component pipeline;

        QString pipelineIDStr = pipelineTableWidget->item(i,0)->data(0).toString();

        int pipelineID =  pipelineIDStr.toInt();

        pipeline.ID = pipelineID;

        QMap<QString, QVariant> pipelineAttributeMap;

        // The feature attributes are the columns from the table
        for(int j = 0; j<pipelineTableWidget->columnCount(); ++j)
        {
            auto attrbText = pipelineTableWidget->horizontalHeaderItem(j)->text();
            auto attrbVal = pipelineTableWidget->item(i,j)->data(0);

            pipelineAttributeMap.insert(attrbText,attrbVal.toString());

            featureAttributes.insert(attrbText,attrbVal);
        }

        // Create a unique ID for the pipeline
        auto uid = this->createUniqueID();

        pipeline.ComponentAttributes = pipelineAttributeMap;

        featureAttributes.insert("ID", pipelineIDStr);
        featureAttributes.insert("RepairRate", 0.0);
        featureAttributes.insert("AssetType", "PIPELINE");
        featureAttributes.insert("TabName", pipelineTableWidget->item(i,0)->data(0).toString());
        featureAttributes.insert("UID", uid);

        // Get the feature collection table from the map
        //        auto layerTag = pipelineTableWidget->item(i,columnToMapLayers)->data(0).toString().toStdString();
        auto layerTag = "Pipeline Network";

        auto featureCollectionTable = tablesMap.at(layerTag);

        auto latitudeStart = pipelineTableWidget->item(i,3)->data(0).toDouble();
        auto longitudeStart = pipelineTableWidget->item(i,4)->data(0).toDouble();

        auto latitudeEnd = pipelineTableWidget->item(i,5)->data(0).toDouble();
        auto longitudeEnd = pipelineTableWidget->item(i,6)->data(0).toDouble();

        // Create the points and add it to the feature table
        PolylineBuilder polylineBuilder(SpatialReference::wgs84());

        // Get the two start and end points of the pipeline segment

        Point point1(longitudeStart,latitudeStart);

        Point point2(longitudeEnd,latitudeEnd);

        polylineBuilder.addPoint(point1);
        polylineBuilder.addPoint(point2);

        if(!polylineBuilder.isSketchValid())
        {
            this->errorMessage("Error, cannot create a pipeline feature with the latitude and longitude provided");
            return;
        }

        // Create the polyline feature
        auto polyline =  polylineBuilder.toPolyline();

        // Add the feature to the table
        Feature* feature = featureCollectionTable->createFeature(featureAttributes, polyline, this);

        pipeline.UID = uid;
        pipeline.ComponentFeature = feature;

        thePipelineDb->addComponent(pipelineID, pipeline);

        featureCollectionTable->addFeature(feature);
    }

    layersTree->selectRow(1);
}


SimCenterMapGraphicsView* VisualizationWidget::getMapViewWidget() const
{
    return mapViewWidget;
}



void VisualizationWidget::selectFeaturesForAnalysisQueryCompleted(QUuid taskID, Esri::ArcGISRuntime::FeatureQueryResult* rawResult)
{
    if(rawResult == nullptr)
        return;

    // Append the raw result to the list - memory management to be handled later
    featuresFromQueryList.append(rawResult);

    taskIDMap.remove(taskID);

    if(taskIDMap.empty())
        emit taskSelectionComplete();
}


void VisualizationWidget::setLayerVisibility(const QString& layerID, const bool val)
{
    auto layerItem = layersTree->getTreeItem(layerID);

    if(layerItem == nullptr)
    {
        QString err = "Could not find layer item with ID " + layerID;
        qDebug()<<err;

        return;
    }

    auto isUnchecked = layerItem->getState() == 0 ? false : true;

    if(isUnchecked != val)
    {
        auto layerModel = layersTree->getLayersModel();
        auto itemIndex = layerModel->index(layerItem->row());
        auto boolVariant = QVariant(val);
        layerModel->setData(itemIndex,boolVariant,Qt::CheckStateRole);
    }
}


void VisualizationWidget::handleLayerChecked(LayerTreeItem* item)
{
    auto itemID = item->getItemID();

    auto isChecked = item->getState() == 2 ? true : false;

    // Get the list of layers
    auto layersList = mapGIS->operationalLayers();

    // Function to do a nested search through the layers to find the correct layer to turn on/off
    std::function<bool(LayerListModel*)> layerIterator = [&](LayerListModel* layers){

        for(int i = 0; i<layers->size(); ++i)
        {
            auto layer = layers->at(i);
            auto layerID = layer->layerId();

            if(itemID.compare(layerID) == 0)
            {
                layer->setVisible(isChecked);
                return true;
            }

            // Check the sublayers - to do get layer pointer from sublayer contents
            //  auto subLayersCont = layer->subLayerContents();
            //  for(auto&& it : subLayersCont)
            //  {
            //      if(it->name() == itemID)
            //      {
            //          it->setVisible(isChecked);
            //          return true;
            //      }
            //  }

            if(auto isGroupLayer = dynamic_cast<GroupLayer*>(layer))
            {
                auto subLayers = isGroupLayer->layers();
                auto found = layerIterator(subLayers);

                if(found)
                {
                    if(!layer->isVisible())
                        layer->setVisible(true);

                    return true;
                }
            }
        }

        return false;
    };

    auto res = layerIterator(layersList);

    if(res == false)
        qDebug()<<"Warning, layer "<<item->getName()<<" not found in map layers in "<<__FUNCTION__;

}


void VisualizationWidget::handleOpacityChange(const QString& layerID, const double opacity)
{
    if(layerID.isEmpty())
        return;

    auto layer = this->getLayer(layerID);

    if(layer)
        layer->setOpacity(opacity);
    //    else
    //        qDebug()<<"Warning, could not find the layer "<<layerName;

}


Esri::ArcGISRuntime::Layer* VisualizationWidget::getLayer(const QString& layerID)
{
    auto layers = mapGIS->operationalLayers();

    std::function<Layer*(LayerListModel* layers, const QString& name)> layerIterator = [&](LayerListModel* layers, const QString& name) ->Layer*
    {
        for(int i = 0; i<layers->size(); ++i)
        {
            auto layer = layers->at(i);

            if(name.compare(layer->layerId()) == 0)
                return layer;

            // Check for sublayers
            if(auto isGroupLayer = dynamic_cast<GroupLayer*>(layer))
            {
                auto subLayers = isGroupLayer->layers();

                if(auto foundLayer = layerIterator(subLayers, name))
                    return foundLayer;
            }

        }

        return nullptr;
    };


    return layerIterator(layers,layerID);
}


void VisualizationWidget::fieldQueryCompleted(QUuid taskID, Esri::ArcGISRuntime::FeatureQueryResult* rawResult)
{
    if(rawResult == nullptr)
        return;

    // Append the raw result to the list - memory management to be handled later
    fieldQueryFeaturesList.append(rawResult);

    taskIDMap.remove(taskID);

    //    emit taskFeatureQueryComplete();
}


void VisualizationWidget::handleFieldQuerySelection(void)
{
    for(auto&& it : fieldQueryFeaturesList)
    {
        FeatureIterator iter = it->iterator();
        while (iter.hasNext())
        {
            Feature* feature = iter.next();

            auto atrbList = feature->attributes();

            auto artbMap = atrbList->attributesMap();

            auto assetID = artbMap.value("UID").toString();

        }
    }

    // Delete the raw results and clear the selection list
    qDeleteAll(fieldQueryFeaturesList);

    // Clear the field query list
    fieldQueryFeaturesList.clear();
}


void VisualizationWidget::handleArcGISError(Esri::ArcGISRuntime::Error error)
{
    if(error.isEmpty())
        return;

    this->errorMessage(error.message());
}


void VisualizationWidget::handleSelectAreaMap(void)
{
    connect(this, &VisualizationWidget::taskSelectionComplete, this, &VisualizationWidget::handleSelectFeaturesForAnalysis);
    thePolygonBoundaryTool->getPolygonBoundaryInputs();
}


void VisualizationWidget::handleClearSelectAreaMap(void)
{
    thePolygonBoundaryTool->resetPolygonBoundary();
    disconnect(this, &VisualizationWidget::taskSelectionComplete, this, &VisualizationWidget::handleSelectFeaturesForAnalysis);
}


void VisualizationWidget::handleSelectFeaturesForAnalysis(void)
{

    for(auto&& it : featuresFromQueryList)
    {
        FeatureIterator iter = it->iterator();
        while (iter.hasNext())
        {
            Feature* feature = iter.next();

            auto atrbList = feature->attributes();

            auto artbMap = atrbList->attributesMap();

            auto assetID = artbMap.value("ID").toInt();

            auto assetType = artbMap.value("AssetType").toString();

            auto componentWidget = componentWidgetsMap.value(assetType,nullptr);

            if(componentWidget)
                componentWidget->insertSelectedComponent(assetID);
        }
    }

    for(auto&& it :componentWidgetsMap)
        it->handleComponentSelection();

    // Delete the raw results and clear the selection list
    qDeleteAll(featuresFromQueryList);

    featuresFromQueryList.clear();

    disconnect(this, &VisualizationWidget::taskSelectionComplete, this, &VisualizationWidget::handleSelectFeaturesForAnalysis);

}


ClassBreaksRenderer* VisualizationWidget::createPointRenderer(void)
{

    SimpleMarkerSymbol* symbol1 = new SimpleMarkerSymbol(SimpleMarkerSymbolStyle::Square, QColor(0, 0, 255,125), 8.0f, this);
    SimpleMarkerSymbol* symbol2 = new SimpleMarkerSymbol(SimpleMarkerSymbolStyle::Square, QColor(255,255,178), 8.0f, this);
    SimpleMarkerSymbol* symbol3 = new SimpleMarkerSymbol(SimpleMarkerSymbolStyle::Square, QColor(253,204,92), 8.0f, this);
    SimpleMarkerSymbol* symbol4 = new SimpleMarkerSymbol(SimpleMarkerSymbolStyle::Square, QColor(253,141,60), 8.0f, this);
    SimpleMarkerSymbol* symbol5 = new SimpleMarkerSymbol(SimpleMarkerSymbolStyle::Square, QColor(240,59,32), 8.0f, this);

    QList<ClassBreak*> classBreaks;

    auto classBreak1 = new ClassBreak("0.00-0.05 Loss Ratio", "Loss Ratio less than 10%", -0.00001, 0.05, symbol1,this);
    classBreaks.append(classBreak1);

    auto classBreak2 = new ClassBreak("0.50-0.25 Loss Ratio", "Loss Ratio Between 10% and 25%", 0.05, 0.25, symbol2,this);
    classBreaks.append(classBreak2);

    auto classBreak3 = new ClassBreak("0.25-0.50 Loss Ratio", "Loss Ratio Between 25% and 50%", 0.25, 0.5,symbol3,this);
    classBreaks.append(classBreak3);

    auto classBreak4 = new ClassBreak("0.50-0.75 Loss Ratio", "Loss Ratio Between 50% and 75%", 0.50, 0.75,symbol4,this);
    classBreaks.append(classBreak4);

    auto classBreak5 = new ClassBreak("0.75-1.00 Loss Ratio", "Loss Ratio Between 75% and 90%", 0.75, 1.0,symbol5,this);
    classBreaks.append(classBreak5);

    return new ClassBreaksRenderer("LossRatio", classBreaks, this);
}


ClassBreaksRenderer* VisualizationWidget::createPipelineRenderer(void)
{
    SimpleLineSymbol* lineSymbol1 = new SimpleLineSymbol(SimpleLineSymbolStyle::Solid, QColor(0, 0, 0), 6.0f /*width*/, this);
    SimpleLineSymbol* lineSymbol2 = new SimpleLineSymbol(SimpleLineSymbolStyle::Solid, QColor(255,255,178), 6.0f /*width*/, this);
    SimpleLineSymbol* lineSymbol3 = new SimpleLineSymbol(SimpleLineSymbolStyle::Solid, QColor(253,204,92), 6.0f /*width*/, this);
    SimpleLineSymbol* lineSymbol4 = new SimpleLineSymbol(SimpleLineSymbolStyle::Solid, QColor(253,141,60),  6.0f /*width*/, this);
    SimpleLineSymbol* lineSymbol5 = new SimpleLineSymbol(SimpleLineSymbolStyle::Solid, QColor(240,59,32),  6.0f /*width*/, this);
    SimpleLineSymbol* lineSymbol6 = new SimpleLineSymbol(SimpleLineSymbolStyle::Solid, QColor(189,0,38),  6.0f /*width*/, this);

    QList<ClassBreak*> classBreaks;

    auto classBreak1 = new ClassBreak("0.0-0.001 number of repairs", "Loss Ratio less than 10%", -0.00001, 1E-03, lineSymbol1, this);
    classBreaks.append(classBreak1);

    auto classBreak2 = new ClassBreak("0.001-0.01 number of repairs", "Loss Ratio Between 10% and 25%", 1.00E-03, 1.00E-02, lineSymbol2, this);
    classBreaks.append(classBreak2);

    auto classBreak3 = new ClassBreak("0.01-0.1 number of repairs", "Loss Ratio Between 25% and 50%", 1.00E-02, 1.00E-01, lineSymbol3, this);
    classBreaks.append(classBreak3);

    auto classBreak4 = new ClassBreak("0.1-1.0 number of repairs", "Loss Ratio Between 50% and 75%", 1.00E-01, 1.00E+00, lineSymbol4, this);
    classBreaks.append(classBreak4);

    auto classBreak5 = new ClassBreak("1.0-10.0 number of repairs", "Loss Ratio Between 75% and 90%", 1.00E+00, 1.00E+01, lineSymbol5, this);
    classBreaks.append(classBreak5);

    auto classBreak6 = new ClassBreak("10.0-100.0 number of repairs", "Loss Ratio Between 75% and 90%", 1.00E+01, 1.00E+10, lineSymbol6, this);
    classBreaks.append(classBreak6);

    return new ClassBreaksRenderer("RepairRate", classBreaks, this);
}


QWidget *VisualizationWidget::getVisWidget()
{
    if(visWidget == nullptr)
        this->createVisualizationWidget();

    return visWidget;
}


// Pop-up stuff
void VisualizationWidget::identifyLayersCompleted(QUuid taskID, const QList<IdentifyLayerResult*>& results)
{
    int count = 0;

    this->clearSelection();

    QVector<std::pair<GeoElement*,QString>> elemList;

    // lambda for calculating result count
    auto geoElementsCountFromResult = [&] (IdentifyLayerResult* result) -> int
    {
        auto layerName = result->layerContent()->name();

        // create temp list
        QList<IdentifyLayerResult*> tempResults{result};

        // use Depth First Search approach to handle recursion
        int count = 0;
        int index = 0;

        while (index < tempResults.length())
        {
            //get the result object from the array
            IdentifyLayerResult* identifyResult = tempResults[index];

            // update count with geoElements from the result
            auto elems = identifyResult->geoElements();

            for(auto&& it : elems)
                elemList.append(std::make_pair(it,layerName));

            count += elems.length();

            // check if the result has any sublayer results
            // if yes then add those result objects in the tempResults
            // array after the current result
            if (identifyResult->sublayerResults().length() > 0)
            {
                tempResults.append(identifyResult->sublayerResults().at(index));
            }

            // update the count and repeat
            index += 1;
        }
        return count;
    };

    for (IdentifyLayerResult* result : results)
        count += geoElementsCountFromResult(result);

    if(count == 0)
    {
        qDeleteAll(results);
        taskIDMap.remove(taskID);
        return;
    }

    std::unique_ptr<PopUpWidget> popUp = std::make_unique<PopUpWidget>(this);

    for(auto&& it : elemList)
    {
        auto geomElem = it.first;

        // cast the GeoElement to a Feature
        Feature* feature = static_cast<Feature*>(geomElem);

        if(feature)
        {
            auto feature2 = new Feature(feature->getImpl(),this);
            selectedFeaturesList.append(feature2);
        }

        auto elemAttrib = geomElem->attributes();

        auto listOfAttributes = elemAttrib->attributeNames();

        QStringList attrbKeyList;
        QStringList attrbValList;

        for(auto&& atrb : listOfAttributes)
        {
            if(QString::compare(atrb,"ObjectID") == 0 || QString::compare(atrb,"AssetType") == 0 || QString::compare(atrb,"TabName") == 0)
                continue;

            auto atrbVal = elemAttrib->attributeValue(atrb).toString();

            // Do not list empty attributes in the popup
            if(atrbVal.isEmpty())
                continue;

            attrbKeyList.append(atrb);


            attrbValList.append(atrbVal);
        }

        // Create a table to display the attributes of this element
        auto attributeTableWidget = new QTableWidget();
        attributeTableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        attributeTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        attributeTableWidget->horizontalHeader()->hide();
        attributeTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

        auto numRows = attrbValList.size();

        attributeTableWidget->setColumnCount(1);
        attributeTableWidget->setRowCount(numRows);

        attributeTableWidget->setVerticalHeaderLabels(attrbKeyList);

        for(int i = 0; i<numRows; ++i)
        {
            auto item = new QTableWidgetItem(attrbValList[i]);

            attributeTableWidget->setItem(i,0, item);
        }

        QString label = elemAttrib->attributeValue("TabName").toString();

        // If the label is empty, append the layer name
        if(label.isEmpty())
        {
            label = it.second;
        }

        popUp->addTab(attributeTableWidget,label);
    }

    // If got this far then some features were selected
    this->handleSelectFeatures();

    popUp->exec();

    // Delete the results
    qDeleteAll(results);
    taskIDMap.remove(taskID);
}


void VisualizationWidget::onMouseClicked(QMouseEvent& mouseEvent)
{
    // Do not show popups if the map is not loaded, or if selecting objects with an convex hull
    if (mapGIS->loadStatus() != LoadStatus::Loaded || thePolygonBoundaryTool->getSelectingPoints() == true)
        return;

    constexpr double tolerance = 12;
    constexpr bool returnPopups = false;
    const int maxResults = 10;

    auto taskWatcher =  mapViewWidget->identifyLayers(mouseEvent.x(), mouseEvent.y(), tolerance, returnPopups, maxResults);

    if (!taskWatcher.isValid())
        qDebug() <<"Error, task not valid in "<<__FUNCTION__;
    else
        taskIDMap[taskWatcher.taskId()] = taskWatcher.description();
}


void VisualizationWidget::onMouseClickedGlobal(QPoint pos)
{

    auto localPos = this->mapFromGlobal(pos);

    constexpr double tolerance = 12;
    constexpr bool returnPopups = false;
    const int maxResults = 10;

    auto taskWatcher =  mapViewWidget->identifyLayers(localPos.x(), localPos.y(), tolerance, returnPopups, maxResults);

    if (!taskWatcher.isValid())
        qDebug() <<"Error, task not valid in "<<__FUNCTION__;
    else
        taskIDMap[taskWatcher.taskId()] = taskWatcher.description();
}


void VisualizationWidget::handleAsyncLayerLoad(Esri::ArcGISRuntime::Error layerLoadStatus)
{

    if (!layerLoadStatus.isEmpty())
    {
        qDebug() << layerLoadStatus.message() << layerLoadStatus.additionalMessage();
        return;
    }

    QObject* obj = sender();

    Layer* layerSender = qobject_cast<Layer*>(obj);

    if(layerSender == nullptr)
    {
        qDebug()<<"The sender must be a layer";
        return;
    }

    auto layerID = layerSender->layerId();
    auto layerName = layerSender->name();

    // Populate the legend info for that layer
    if(layerSender->isAutoFetchLegendInfos())
    {
        layerSender->legendInfos();

        auto legendInfos = layerSender->legendInfos();
        if(legendInfos == nullptr)
            return;

        legendInfos->setProperty("UID",layerID);

        connect(legendInfos, &LegendInfoListModel::fetchLegendInfosCompleted, this, &VisualizationWidget::setLegendInfo, Qt::UniqueConnection);
    }

    layerLoadMap.remove(layerID);

    // Only zoom to extents when all of the layers are loaded
    if(layerLoadMap.empty())
    {
        //        this->zoomToExtents();
    }
}


void VisualizationWidget::runFieldQuery(const QString& fieldName, const QString& searchText)
{
    // create a query parameter object and set the where clause
    QueryParameters queryParams;

    // Follows the format of an SQL 'where' clause
    const auto whereClause = fieldName + QString(" LIKE '" + searchText + "%'");

    queryParams.setWhereClause(whereClause);

    // Iterate through the layers
    // Function to do a nested search through the layers - this is needed because some layers may have sub-layers
    std::function<void(const LayerListModel*)> layerIterator = [&](const LayerListModel* layers)
    {
        for(int i = 0; i<layers->size(); ++i)
        {
            auto layer = layers->at(i);

            // Continue if the layer is turned off
            if(!layer->isVisible())
                continue;

            if(auto featureCollectLayer = dynamic_cast<FeatureCollectionLayer*>(layer))
            {
                auto tables = featureCollectLayer->featureCollection()->tables();

                for(int j = 0; j<tables->size(); ++j)
                {
                    auto table = tables->at(j);

                    // Make this a unique, i.e., one-off connection so that it does not call the slot multiple times
                    connect(table, &FeatureTable::queryFeaturesCompleted, this, &VisualizationWidget::fieldQueryCompleted, Qt::UniqueConnection);

                    // Query the table for features - note that this is done asynchronously
                    auto taskWatcher = table->queryFeatures(queryParams);

                    if (!taskWatcher.isValid())
                        qDebug() <<"Error, task not valid in "<<__FUNCTION__;
                    else
                        taskIDMap[taskWatcher.taskId()] = taskIDMap[taskWatcher.taskId()] = taskWatcher.description();
                }
            }
            else if(auto isGroupLayer = dynamic_cast<GroupLayer*>(layer))
            {
                auto subLayers = isGroupLayer->layers();
                layerIterator(subLayers);
            }
        }
    };

    // Get the layers from the map, and ensure that they are not empty
    auto layersList = mapGIS->operationalLayers();

    layerIterator(layersList);

}


QList<Esri::ArcGISRuntime::FeatureQueryResult *> VisualizationWidget::getFeaturesFromQueryList() const
{
    return featuresFromQueryList;
}


QMap<QUuid, QString>& VisualizationWidget::getTaskIDMap(void)
{
    return taskIDMap;
}


Esri::ArcGISRuntime::Map *VisualizationWidget::getMapGIS(void) const
{
    return mapGIS;
}


void VisualizationWidget::zoomToExtents(void)
{

    LayerListModel* layers = mapGIS->operationalLayers();

    Envelope bbox;
    for (int j = 0; j < layers->size(); j++)
    {
        auto layer = layers->at(j);

        if(layer->loadStatus() != Esri::ArcGISRuntime::LoadStatus::Loaded)
            continue;

        if(!bbox.isValid())
        {
            bbox = layer->fullExtent();
            continue;
        }

        auto layerExtent = layer->fullExtent();

        if(layerExtent.isValid())
            bbox = GeometryEngine::unionOf(bbox, layerExtent);
    }

    if(!bbox.isValid())
        return;

    //    auto center = bbox.center();
    //    auto x = center.x();
    //    auto y = center.y();
    //    mapViewWidget->setViewpointCenter(center,10000);
    mapViewWidget->setViewpointGeometry(bbox,50);

}


void VisualizationWidget::zoomToLayer(const QString layerID)
{

    Layer* gisLayer = layersMap.value(layerID,nullptr);
    //    Layer* gisLayer = this->findLayer(layerID);

    // Continue if the layer is turned off
    if(gisLayer == nullptr || gisLayer->loadStatus() != Esri::ArcGISRuntime::LoadStatus::Loaded || !gisLayer->isVisible())
        return;

    auto layerName = gisLayer->name();

    Envelope bbox;

    if(auto featureCollectLayer = dynamic_cast<FeatureCollectionLayer*>(gisLayer))
    {
        auto tables = featureCollectLayer->featureCollection()->tables();

        for(int j = 0; j<tables->size(); ++j)
        {
            auto table = tables->at(j);

            Envelope tableExt = table->extent();

            if(tableExt.isValid())
            {
                if(!bbox.isValid())
                    bbox = tableExt;

                bbox = GeometryEngine::combineExtents(bbox, tableExt);
            }
        }
    }
    else if(auto isGroupLayer = dynamic_cast<GroupLayer*>(gisLayer))
    {
        auto subLayers = isGroupLayer->layers();
        for(int i = 0; i<subLayers->size(); ++i)
        {
            Layer* subLayer = subLayers->at(i);

            auto featureCollectLayer = dynamic_cast<FeatureCollectionLayer*>(subLayer);

            if(featureCollectLayer == nullptr)
                continue;

            auto tables = featureCollectLayer->featureCollection()->tables();

            for(int j = 0; j<tables->size(); ++j)
            {
                auto table = tables->at(j);

                Envelope tableExt = table->extent();

                if(tableExt.isValid())
                {
                    if(!bbox.isValid())
                        bbox = tableExt;

                    bbox = GeometryEngine::combineExtents(bbox, tableExt);
                }
            }
        }
    }


    if(!bbox.isValid())
        return;

    //    auto center = bbox.center();
    //    mapViewWidget->setViewpointCenter(center,10000);
    mapViewWidget->setViewpointGeometry(bbox,20);
}


void VisualizationWidget::handleBasemapSelection(const QString selection)
{
    if(selection.compare("Topographic") == 0)
    {
        mapGIS->setBasemap(Basemap::topographic(this));
    }
    else if(selection.compare("Terrain") == 0)
    {
        mapGIS->setBasemap(Basemap::terrainWithLabelsVector(this));
    }
    else if(selection.compare("Imagery") == 0)
    {
        mapGIS->setBasemap(Basemap::imageryWithLabels(this));
    }
    else if(selection.compare("Open Street Map") == 0)
    {
        mapGIS->setBasemap(Basemap::openStreetMap(this));
    }
    else if(selection.compare("Streets") == 0)
    {
        mapGIS->setBasemap(Basemap::streets(this));
    }
    else if(selection.compare("Canvas (Light)") == 0)
    {
        mapGIS->setBasemap(Basemap::lightGrayCanvasVector(this));
    }
    else if(selection.compare("National Geographic") == 0)
    {
        mapGIS->setBasemap(Basemap::nationalGeographic(this));
    }
}


LayerTreeItem* VisualizationWidget::addSelectedFeatureLayerToMap(Esri::ArcGISRuntime::Layer* featLayer)
{

    selectedObjectsTreeItem = layersTree->getTreeItem("Selected Objects",layersTree->getLayersModel()->getRootItem()->objectName());
    // Create the tree item if it does not exist
    if(selectedObjectsTreeItem == nullptr)
    {
        // Create the buildings group layer that will hold the sublayers
        selectedObjectsLayer = new GroupLayer(QList<Layer*>{}, this);
        selectedObjectsLayer->setName("Selected Objects");

        selectedObjectsTreeItem = this->addLayerToMap(selectedObjectsLayer);
    }

    return this->addLayerToMap(featLayer, selectedObjectsTreeItem, selectedObjectsLayer);
}


void VisualizationWidget::updateSelectedComponent(const QString& assetType, const QString& uid, const QString& attribute, const QVariant& value)
{

    auto inputWidget = componentWidgetsMap.value(assetType,nullptr);

    if(inputWidget)
    {
        inputWidget->updateSelectedComponentAttribute(uid,attribute,value);
    }
    else
    {
        QString err = "Could not find the widget for the corresponding asset type";
        qDebug()<<err;
        return;
    }

}


RasterLayer* VisualizationWidget::createAndAddRasterLayer(const QString& filePath, const QString& layerName, LayerTreeItem* parentItem)
{
    QFileInfo check_file(filePath);

    // Check if something at that path exists and cheack that it really is a file and not a directory
    if (!check_file.exists() && !check_file.isFile())
    {
        QString msg = "Error, the file at location "+filePath+" does not exist ";
        this->errorMessage(msg);
        return nullptr;
    }

    // Create the raster and its layer layer
    Raster* raster = new Raster(filePath, this);

    raster->load();

    RasterLayer* layer = new RasterLayer(raster, this);

    // When the layer is done loading, zoom to extents of the data
    connect(layer, &RasterLayer::doneLoading, this, [this, layer](Error loadError)
    {
        if (!loadError.isEmpty())
        {
            auto msg = loadError.message();
            this->errorMessage(msg);
            return;
        }

        mapViewWidget->setViewpointCenter(layer->fullExtent().center(), 80000);
    });

    layer->setName(layerName);
    auto layerID = this->createUniqueID();
    layer->setLayerId(layerID);

    // Add the layers to the layer tree
    layersTree->addItemToTree(layerName,layerID,parentItem);

    return layer;
}


FeatureLayer* VisualizationWidget::createAndAddShapefileLayer(const QString& filePath, const QString& layerName, LayerTreeItem* parentItem)
{
    // Create the ShapefileFeatureTable
    ShapefileFeatureTable* featureTable = new ShapefileFeatureTable(filePath, this);

    // Create the feature layer from the ShapefileFeatureTable
    FeatureLayer* layer = new FeatureLayer(featureTable, this);

    connect(layer, &FeatureLayer::doneLoading, this, [this, layer](Error loadError)
    {
        if (!loadError.isEmpty())
        {
            auto msg = loadError.message() + loadError.additionalMessage();
            this->errorMessage(msg);
            return;
        }

        // If the layer was loaded successfully, set the map extent to the full extent of the layer
        mapViewWidget->setViewpointCenter(layer->fullExtent().center(), 80000);
    });

    layer->setName(layerName);

    auto layerID = this->createUniqueID();
    layer->setLayerId(layerID);

    // Add the layers to the layer tree
    layersTree->addItemToTree(layerName, layerID, parentItem);

    return layer;
}


ArcGISMapImageLayer* VisualizationWidget::createAndAddMapServerLayer(const QString& url, const QString& layerName, LayerTreeItem* parentItem)
{
    ArcGISMapImageLayer* layer  = new ArcGISMapImageLayer(QUrl(url), this);

    // Add the layers to the layer tree
    auto layerID = this->createUniqueID();
    auto layerLayerTreeItem = layersTree->addItemToTree(layerName, layerID, parentItem);

    connect(layer, &ArcGISMapImageLayer::doneLoading, this, [this, layer, layerLayerTreeItem](Error loadError)
    {
        if (!loadError.isEmpty())
        {
            auto msg = loadError.message() + loadError.additionalMessage();
            this->errorMessage(msg);

            return;
        }

        auto subLayers = layer->subLayerContents();

        for(auto&& it : subLayers)
        {
            auto subLayerName = it->name();

            layersTree->addItemToTree(subLayerName, QString(), layerLayerTreeItem);
        }


        // If the layer was loaded successfully, set the map extent to the full extent of the layer
        mapViewWidget->setViewpointCenter(layer->fullExtent().center(), 80000);
    });

    layer->setName(layerName);
    layer->setLayerId(layerID);


    return layer;
}


void VisualizationWidget::createAndAddGeoDatabaseLayer(const QString& filePath, const QString& layerName, LayerTreeItem* parentItem)
{
    auto m_geodatabase = new Geodatabase(filePath, this);

    connect(m_geodatabase, &Geodatabase::errorOccurred, this, [this](Error error)
    {
        auto msg = error.message() + error.additionalMessage();
        this->errorMessage(msg);
        return;
    });

    connect(m_geodatabase, &Geodatabase::doneLoading, this, [=](Error error)
    {
        if (error.isEmpty())
        {
            GeodatabaseFeatureTable* featureTable = m_geodatabase->geodatabaseFeatureTable("layerName");

            // create a feature layer from the feature table
            FeatureLayer* featureLayer = new FeatureLayer(featureTable, this);
            featureLayer->setName(layerName);

            // add the feature layer to the map
            this->addLayerToMap(featureLayer,parentItem);
        }
    });

    // load the geodatabase
    m_geodatabase->load();
}


KmlLayer*  VisualizationWidget::createAndAddKMLLayer(const QString& filePath, const QString& layerName, LayerTreeItem* parentItem, double opacity)
{
    QFileInfo check_file(filePath);

    // Check if something at that path exists and cheack that it really is a file and not a directory
    if (!check_file.exists() && !check_file.isFile())
    {
        QString msg = "Error, the file at location "+filePath+" does not exist ";
        this->errorMessage(msg);
        return nullptr;
    }

    // Create the Dataset from an Online URL
    auto m_kmlDataset = new KmlDataset(filePath, this);

    // Create the Layer
    auto kmlLayer = new KmlLayer(m_kmlDataset, this);

    kmlLayer->setOpacity(opacity);

    auto layerID = this->createUniqueID();

    kmlLayer->setName(layerName);
    kmlLayer->setLayerId(layerID);

    // When the layer is done loading, zoom to extents of the data
    connect(kmlLayer, &KmlLayer::doneLoading, this, [this, kmlLayer](Error loadError)
    {
        if (!loadError.isEmpty())
        {
            auto msg = loadError.message() + loadError.additionalMessage();
            this->errorMessage(msg);
            return;
        }

        mapViewWidget->setViewpointCenter(kmlLayer->fullExtent().center(), 80000);
    });

    // Add the layers to the layer tree
    layersTree->addItemToTree(layerName, layerID, parentItem);

    return kmlLayer;
}


// Add a shakemap grid given as an XML file
FeatureCollectionLayer* VisualizationWidget::createAndAddXMLShakeMapLayer(const QString& filePath, const QString& layerName, LayerTreeItem* parentItem)
{
    XMLAdaptor XMLImportAdaptor;

    QString errMess;
    auto XMLlayer = XMLImportAdaptor.parseXMLFile(filePath, errMess, this);

    if(XMLlayer == nullptr)
    {
        this->errorMessage(errMess);
        return nullptr;
    }

    XMLlayer->setName(layerName);

    auto layerID = this->createUniqueID();

    XMLlayer->setLayerId(layerID);

    // Add the layers to the layer tree
    layersTree->addItemToTree(layerName, layerID, parentItem);

    return XMLlayer;
}


double VisualizationWidget::getLatFromScreenPoint(const QPointF& point)
{
    auto mapPoint = mapViewWidget->screenToLocation(point.x(),point.y());

    auto latStr = CoordinateFormatter::toLatitudeLongitude(mapPoint, LatitudeLongitudeFormat::DecimalDegrees, 6);

    auto newPoint = CoordinateFormatter::fromLatitudeLongitude(latStr,SpatialReference::wgs84());

    // Lat is in 'y' coordinate of a point
    auto asDouble = newPoint.y();

    return asDouble;
}


double VisualizationWidget::getLongFromScreenPoint(const QPointF& point)
{
    auto mapPoint = mapViewWidget->screenToLocation(point.x(),point.y());

    auto latStr = CoordinateFormatter::toLatitudeLongitude(mapPoint, LatitudeLongitudeFormat::DecimalDegrees, 6);

    auto newPoint = CoordinateFormatter::fromLatitudeLongitude(latStr,SpatialReference::wgs84());

    // Long is in 'y' coordinate of a point
    auto asDouble = newPoint.x();

    return asDouble;
}


QPointF VisualizationWidget::getScreenPointFromLatLong(const double& latitude, const double& longitude)
{
    Point mapPnt(longitude,latitude,SpatialReference::wgs84());

    auto screenPnt = mapViewWidget->locationToScreen(mapPnt);

    return screenPnt;
}


LayerTreeItem* VisualizationWidget::addLayerToMap(Esri::ArcGISRuntime::Layer* layer, LayerTreeItem* parent, Esri::ArcGISRuntime::GroupLayer* groupLayer)
{

    // Create a unique ID for the layer
    auto layerID = this->createUniqueID();
    layer->setLayerId(layerID);

    // Add it to the map or to a group layer if it is part of a group
    if(groupLayer == nullptr)
        mapGIS->operationalLayers()->append(layer);
    else
        groupLayer->layers()->append(layer);

    // Insert the layer into the map
    layersMap.insert(layerID, layer);

    // Add the layer to the layer tree
    auto layerName = layer->name();
    auto treeItem = layersTree->addItemToTree(layerName, layerID, parent);

    // Add layer to the map
    layerLoadMap[layerID] = layerName;

    connect(layer, &Layer::doneLoading, this, &VisualizationWidget::handleAsyncLayerLoad);

    layer->load();

    return treeItem;
}


bool VisualizationWidget::removeLayerFromMap(Esri::ArcGISRuntime::Layer* layer)
{
    if(layer == nullptr)
        return false;

    auto mapLayers = mapGIS->operationalLayers();

    auto hasLayer = mapLayers->contains(layer);

    // First check to see if this layer is directly added to the operational layers and it is not part of a group layer
    if(hasLayer)
    {
        mapLayers->removeOne(layer);

        legendView->hide();

        return true;
    }
    else
    {
        std::function<bool(Layer*)> nestedLayerRemover = [&](Layer* layerToCheck){

            auto grpLayer = dynamic_cast<GroupLayer*>(layerToCheck);

            if(grpLayer)
            {
                auto layersInGroup = grpLayer->layers();
                if(layersInGroup->contains(layer))
                {
                    layersInGroup->removeOne(layer);
                    legendView->hide();
                    return true;
                }
                else
                {
                    for(auto&& subLayer : *layersInGroup)
                    {
                        if(nestedLayerRemover(subLayer))
                            return true;
                    }
                }
            }

            return false;
        };

        // Then check any group layers
        for(auto it : *mapLayers)
        {
            if(nestedLayerRemover(it))
                return true;
        }
    }

    return false;
}


void VisualizationWidget::removeLayerFromMap(const QString layerID)
{
    auto layer = this->getLayer(layerID);
    this->removeLayerFromMap(layer);
}


bool VisualizationWidget::removeLayerFromMapAndTree(const QString layerID)
{
    return layersTree->removeItemFromTree(layerID);
}


QString VisualizationWidget::createUniqueID(void)
{
    auto id = QUuid::createUuid();

    return id.toString();
}


void VisualizationWidget::takeScreenShot(void)
{
    mapViewWidget->exportImage();
}


void VisualizationWidget::exportImageComplete(QUuid id, QImage img)
{
    emitScreenshot(img);
}


void VisualizationWidget::clear(void)
{
    layersTree->clear();

    baseMapCombo->setCurrentIndex(0);

    taskIDMap.clear();
    layerLoadMap.clear();
    layersMap.clear();
    legendModels.clear();

    featuresFromQueryList.clear();

    mapGIS->operationalLayers()->clear();

    delete selectedObjectsLayer;

    legendView->clear();

    selectedObjectsTreeItem = nullptr;
    selectedObjectsLayer = nullptr;

    this->clearSelection();
}


void VisualizationWidget::setLegendView(GISLegendView* legndView)
{
    if(legndView == nullptr)
        return;

    legendView = legndView;
}


void VisualizationWidget::hideLegend(void)
{
    legendView->clear();
}


void VisualizationWidget::setLegendInfo()
{
    QObject* obj = sender();

    auto legendInfo = qobject_cast<LegendInfoListModel*>(obj);

    if(legendView == nullptr || legendInfo == nullptr)
        return;

    QString layerUID = obj->property("UID").toString();

    // set the legend info list model
    RoleProxyModel* roleModel = new RoleProxyModel(this);

    if(roleModel == nullptr)
        return;

    roleModel->setObjectName(layerUID);
    roleModel->setSourceModel(legendInfo);

    legendModels.insert(layerUID,roleModel);

    this->handleLegendChange(layerUID);
}


void VisualizationWidget::handleLegendChange(const Layer* layer)
{
    if(layer == nullptr)
        return;

    auto UID = layer->layerId();
    this->handleLegendChange(UID);
}


void VisualizationWidget::handleLegendChange(const QString layerUID)
{
    if(legendView  == nullptr)
        return;

    if(layerUID.isEmpty())
    {
        legendView->hide();
        return;
    }


    RoleProxyModel* roleModel = legendModels.value(layerUID,nullptr);

    if(roleModel == nullptr)
    {
        legendView->hide();
        return;
    }

    legendView->setModel(roleModel);

    legendView->show();
}


GISLegendView *VisualizationWidget::getLegendView() const
{
    return legendView;
}


Esri::ArcGISRuntime::Geometry VisualizationWidget::getGeometryFromJson(const QString& geoJson)
{
    QRegularExpression rx("[^\\[\\]]+(?=\\])");

    QRegularExpressionMatchIterator i = rx.globalMatch(geoJson);

    QStringList pointsList;
    while (i.hasNext())
    {
        QRegularExpressionMatch match = i.next();

        if(!match.hasMatch())
            continue;

        QString word = match.captured(0);
        pointsList << word;
    }

    if(pointsList.empty())
        return Geometry();

    PolygonBuilder polygonBuilder(SpatialReference::wgs84());

    for(auto&& it : pointsList)
    {
        auto points = it.split(",");

        if(points.size() != 2)
            return Geometry();

        bool OK = false;
        double lat = points.at(0).toDouble(&OK);

        if(!OK)
            return Geometry();

        double lon = points.at(1).toDouble(&OK);
        if(!OK)
            return Geometry();

        polygonBuilder.addPoint(lat,lon);
    }

    return polygonBuilder.toGeometry();
}


Esri::ArcGISRuntime::Geometry VisualizationWidget::getGeometryFromJson(const QJsonArray& geoJson)
{

    if(geoJson.size() == 0)
        return Geometry();


    PolygonBuilder polygonBuilder(SpatialReference::wgs84());

    for(auto&& it : geoJson)
    {

        auto points = it.toArray();

        if(points.size() != 2)
            return Geometry();

        double lat = points.at(1).toDouble(360.0);
        double lon = points.at(0).toDouble(360.0);

        if(lat == 360.0 || lon == 360.0)
            return Geometry();

        polygonBuilder.addPoint(lat,lon);
    }

    return polygonBuilder.toGeometry();
}


Esri::ArcGISRuntime::Geometry VisualizationWidget::getRectGeometryFromPoint(const Esri::ArcGISRuntime::Point& pnt, const double sizeX, double sizeY)
{

    if(sizeY == 0)
        sizeY=sizeX;

    Envelope envelope(pnt,sizeX,sizeY);

    auto xMin = envelope.xMin();
    auto xMax = envelope.xMax();
    auto yMin = envelope.yMin();
    auto yMax = envelope.yMax();

    PolygonBuilder polygonBuilder(SpatialReference::wgs84());
    polygonBuilder.addPoint(xMin,yMin);
    polygonBuilder.addPoint(xMax,yMin);
    polygonBuilder.addPoint(xMax,yMax);
    polygonBuilder.addPoint(xMin,yMax);

    return polygonBuilder.toGeometry();
}


QList<Esri::ArcGISRuntime::Feature *> VisualizationWidget::getSelectedFeaturesList() const
{
    return selectedFeaturesList;
}


void VisualizationWidget::handleSelectFeatures(void)
{
    auto selectedColor = QColor(255, 170, 29);

    for(auto&& it: selectedFeaturesList)
    {
        auto geom = it->geometry();

        auto geomType = geom.geometryType();

        if(geomType == GeometryType::Polyline)
        {
            SimpleLineSymbol* lineSymbol = new SimpleLineSymbol(SimpleLineSymbolStyle::Solid, selectedColor, 4.0f /*width*/, this);
            Graphic* graphic = new Graphic(geom, this);
            graphic->setSymbol(lineSymbol);
            selectedFeaturesOverlay->graphics()->append(graphic);
        }
        else if(geomType == GeometryType::Polygon)
        {
            SimpleFillSymbol* fillSymbol = new SimpleFillSymbol(SimpleFillSymbolStyle::Solid, selectedColor, this);
            Graphic* graphic = new Graphic(geom, this);
            graphic->setSymbol(fillSymbol);
            selectedFeaturesOverlay->graphics()->append(graphic);
        }
        else if(geomType == GeometryType::Point)
        {
            SimpleMarkerSymbol* pointSymbol = new SimpleMarkerSymbol(SimpleMarkerSymbolStyle::Square, selectedColor, 4.0f, this);
            Graphic* graphic = new Graphic(geom, this);
            graphic->setSymbol(pointSymbol);
            selectedFeaturesOverlay->graphics()->append(graphic);
        }
    }

}


void VisualizationWidget::clearSelection(void)
{
    selectedFeaturesOverlay->graphics()->clear();
    qDeleteAll(selectedFeaturesList);
    selectedFeaturesList.clear();
}


void VisualizationWidget::registerComponentWidget(QString type, ComponentInputWidget* widget)
{
    componentWidgetsMap[type] = widget;

    widget->setTheVisualizationWidget(this);
}


ComponentInputWidget* VisualizationWidget::getComponentWidget(const QString type)
{
    return componentWidgetsMap.value(type, nullptr);
}


//     connect to the mouse clicked signal on the MapQuickView
//     This code snippet adds a point to where the mouse click is
//        connect(mapViewWidget, &MapGraphicsView::mouseClicked, this, [this](QMouseEvent& mouseEvent)
//        {
//            // obtain the map point
//            const double screenX = mouseEvent.x();
//            const double screenY = mouseEvent.y();
//            Point newPoint = mapViewWidget->screenToLocation(screenX, screenY);

//            // create the feature attributes
//            QMap<QString, QVariant> featureAttributes;
//            featureAttributes.insert("ID", "99");
//            featureAttributes.insert("LossRatio", 0.0);
//            featureAttributes.insert("AssetType", "BUILDING");
//            featureAttributes.insert("TabName", "99");
//            featureAttributes.insert("UID", "99");
//            // create a new feature and add it to the feature table
//            Feature* feature = selectedBuildingsTable->createFeature(featureAttributes, newPoint, this);
//            selectedBuildingsTable->addFeature(feature);

//            auto numFeat = selectedBuildingsTable->numberOfFeatures();
//            qDebug()<<numFeat;
//        });
