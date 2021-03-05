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
#include "VisualizationWidget.h"
#include "XMLAdaptor.h"

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
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
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

    selectingConvexHull = false;

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

    buildingWidget = nullptr;
    pipelineWidget = nullptr;

    // Create the Widget view
    //mapViewWidget = new MapGraphicsView(this);
    mapViewLayout = new QVBoxLayout();

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
    setupConvexHullObjects();

    // Handle the async. queries
    connect(this, &VisualizationWidget::taskSelectionComplete, this, &VisualizationWidget::handleAsyncSelectionTask);

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
    visWidget = new QWidget(this);
    visWidget->setContentsMargins(0,0,0,0);

    QGridLayout* layout = new QGridLayout(visWidget);
    visWidget->setLayout(layout);

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

    connect(selectPointsButton,SIGNAL(clicked()),this,SLOT(getConvexHullInputs()));
    connect(clearButton,SIGNAL(clicked()),this,SLOT(resetConvexHull()));

    QLabel* bottomText = new QLabel(visWidget);
    bottomText->setText("Click the 'Apply' button to\nselect the subset of assets");
    bottomText->setStyleSheet("font-weight: bold; color: black; text-align: center");

    QPushButton *applyButton = new QPushButton(visWidget);
    applyButton->setText(tr("Apply"));
    applyButton->setMaximumWidth(150);

    connect(applyButton,SIGNAL(clicked()),this,SLOT(getItemsInConvexHull()));

    // Add a vertical spacer at the bottom to push everything up
    auto vspacer = new QSpacerItem(0,0,QSizePolicy::Minimum, QSizePolicy::Expanding);

    layout->addItem(smallVSpacer,0,0,1,2);
    layout->addWidget(basemapText,1,0);
    layout->addWidget(baseMapCombo,2,0);
    layout->addItem(smallVSpacer,3,0,1,2);
    layout->addWidget(layersTree,4,0);
    layout->addWidget(topText,5,0);
    layout->addWidget(selectPointsButton,6,0);
    layout->addWidget(clearButton,7,0);
    layout->addWidget(bottomText,8,0);
    layout->addWidget(applyButton,9,0);
    layout->addItem(vspacer,10,0,1,1);


    //layout->addWidget(mapViewWidget,0,1,12,2);
    layout->addLayout(mapViewLayout,0,1,12,2);
}


// Convex hull stuff
void VisualizationWidget::plotConvexHull()
{
    if (m_inputsGraphic->geometry().isEmpty())
        return;

    // normalizing the geometry before performing geometric operations
    const Geometry normalizedPoints = GeometryEngine::normalizeCentralMeridian(m_inputsGraphic->geometry());
    const Geometry convexHull = GeometryEngine::convexHull(normalizedPoints);

    // change the symbol based on the returned geometry type
    if (convexHull.geometryType() == GeometryType::Point)
    {
        m_convexHullGraphic->setSymbol(m_markerSymbol);
    }
    else if (convexHull.geometryType() == GeometryType::Polyline)
    {
        m_convexHullGraphic->setSymbol(m_lineSymbol);
    }
    else if (convexHull.geometryType() == GeometryType::Polygon)
    {
        m_convexHullGraphic->setSymbol(m_fillSymbol);
    }
    else
    {
        qWarning("Not a valid geometry.");
    }

    m_convexHullGraphic->setGeometry(convexHull);
}


void VisualizationWidget::resetConvexHull()
{
    selectingConvexHull = false;

    if (m_multipointBuilder)
        m_multipointBuilder->points()->removeAll();
    if (m_inputsGraphic)
        m_inputsGraphic->setGeometry(Geometry());
    if (m_convexHullGraphic)
        m_convexHullGraphic->setGeometry(Geometry());

    // Turn off point selection for the convex hull
    disconnect(mapViewWidget, &MapGraphicsView::mouseClicked, this, &VisualizationWidget::convexHullPointSelector);
}


void VisualizationWidget::setupConvexHullObjects()
{
    // graphics overlay to show clicked points and convex hull
    m_graphicsOverlay = new GraphicsOverlay(this);

    // create a graphic to show clicked points
    m_markerSymbol = new SimpleMarkerSymbol(SimpleMarkerSymbolStyle::Circle, Qt::red, 10, this);
    m_inputsGraphic = new Graphic(this);
    m_inputsGraphic->setSymbol(m_markerSymbol);
    m_graphicsOverlay->graphics()->append(m_inputsGraphic);

    // create a graphic to display the convex hull
    m_convexHullGraphic = new Graphic(this);
    m_graphicsOverlay->graphics()->append(m_convexHullGraphic);

    // create a graphic to show the convex hull
    m_lineSymbol = new SimpleLineSymbol(SimpleLineSymbolStyle::Solid, Qt::blue, 3, this);
    m_fillSymbol = new SimpleFillSymbol(SimpleFillSymbolStyle::Solid, QColor(0,0,255,25), m_lineSymbol, this);

    // wait for map to load before creating multipoint builder
    connect(mapGIS, &Map::doneLoading, this, [this](Error e){
        if (!e.isEmpty())
        {
            qDebug() << e.message() << e.additionalMessage();
            return;
        }

        if (mapGIS->loadStatus() == LoadStatus::FailedToLoad)
        {
            qWarning( "Failed to load map.");
            return;
        }

        m_multipointBuilder = new MultipointBuilder(mapGIS->spatialReference(), this);
    });

    mapViewWidget->graphicsOverlays()->append(m_graphicsOverlay);
}


void VisualizationWidget::getConvexHullInputs()
{
    selectingConvexHull = true;

    // show clicked points on MapView
    connect(mapViewWidget, &MapGraphicsView::mouseClicked, this, &VisualizationWidget::convexHullPointSelector, Qt::UniqueConnection);
}


void VisualizationWidget::convexHullPointSelector(QMouseEvent& e)
{
    e.accept();

    const Point clickedPoint = mapViewWidget->screenToLocation(e.x(), e.y());

    m_multipointBuilder->points()->addPoint(clickedPoint);
    m_inputsGraphic->setGeometry(m_multipointBuilder->toGeometry());

    this->plotConvexHull();
}


ComponentInputWidget *VisualizationWidget::getPipelineWidget() const
{
    return pipelineWidget;
}


ComponentInputWidget *VisualizationWidget::getBuildingWidget() const
{
    return buildingWidget;
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


void VisualizationWidget::setBuildingWidget(ComponentInputWidget *value)
{
    buildingWidget = value;

    buildingWidget->setTheVisualizationWidget(this);

    connect(buildingWidget,&ComponentInputWidget::componentDataLoaded, this, &VisualizationWidget::loadBuildingData);
}


void VisualizationWidget::loadBuildingData(void)
{

    auto buildingTableWidget = buildingWidget->getTableWidget();
    ComponentDatabase* theBuildingDb = buildingWidget->getComponentDatabase();

    QList<Field> fields;
    fields.append(Field::createDouble("LossRatio", "0.0"));
    fields.append(Field::createText("ID", "NULL",4));
    fields.append(Field::createText("AssetType", "NULL",4));
    fields.append(Field::createText("TabName", "NULL",4));
    fields.append(Field::createText("UID", "NULL",4));

    // Select a column that will define the building layers
    int columnToMapLayers = 0;

    QString columnFilter = "OccupancyClass";

    // Set the table headers as fields in the table
    for(int i = 1; i<buildingTableWidget->columnCount(); ++i)
    {
        auto headerItem = buildingTableWidget->horizontalHeaderItem(i);

        auto fieldText = headerItem->text();

        if(fieldText.compare(columnFilter) == 0)
            columnToMapLayers = i;

        fields.append(Field::createText(fieldText, fieldText,fieldText.size()));
    }

    // Create the buildings group layer that will hold the sublayers
    auto buildingLayer = new GroupLayer(QList<Layer*>{},this);
    buildingLayer->setName("Buildings");

    auto buildingsItem = this->addLayerToMap(buildingLayer);

    if(buildingsItem == nullptr)
    {
        qDebug()<<"Error adding item to the map";
        return;
    }

    auto nRows = buildingTableWidget->rowCount();

    std::vector<std::string> vecLayerItems;
    for(int i = 0; i<nRows; ++i)
    {
        // Organize the layers according to occupancy type
        auto occupancyType = buildingTableWidget->item(i,columnToMapLayers)->data(0).toString().toStdString();

        vecLayerItems.push_back(occupancyType);
    }

    this->uniqueVec<std::string>(vecLayerItems);

    auto selectedBuildingsFeatureCollection = new FeatureCollection(this);
    selectedBuildingsTable = new FeatureCollectionTable(fields, GeometryType::Polygon, SpatialReference::wgs84(),this);
    selectedBuildingsFeatureCollection->tables()->append(selectedBuildingsTable);
    selectedBuildingsLayer = new FeatureCollectionLayer(selectedBuildingsFeatureCollection,this);
    selectedBuildingsLayer->setName("Selected Buildings");
    selectedBuildingsLayer->setAutoFetchLegendInfos(true);
    selectedBuildingsTable->setRenderer(this->createBuildingRenderer(2.5));

    // Map to hold the feature tables
    std::map<std::string, FeatureCollectionTable*> tablesMap;
    for(auto&& it : vecLayerItems)
    {
        auto featureCollection = new FeatureCollection(this);

        auto featureCollectionTable = new FeatureCollectionTable(fields, GeometryType::Polygon, SpatialReference::wgs84(),this);

        featureCollection->tables()->append(featureCollectionTable);

        auto newBuildingLayer = new FeatureCollectionLayer(featureCollection,this);

        newBuildingLayer->setName(QString::fromStdString(it));

        newBuildingLayer->setAutoFetchLegendInfos(true);

        featureCollectionTable->setRenderer(this->createBuildingRenderer());

        tablesMap.insert(std::make_pair(it,featureCollectionTable));

        this->addLayerToMap(newBuildingLayer,buildingsItem,buildingLayer);
    }

    // First check if a footprint was provided
    auto indexFootprint = -1;
    for(int i = 0; i<buildingTableWidget->columnCount(); ++i)
    {
        auto headerText = buildingTableWidget->horizontalHeaderItem(i)->text();

        if(headerText.contains("Footprint"))
        {
            indexFootprint = i;
            break;
        }
    }

    for(int i = 0; i<nRows; ++i)
    {
        // create the feature attributes
        QMap<QString, QVariant> featureAttributes;

        // Create a new building
        Component building;

        QString buildingIDStr = buildingTableWidget->item(i,0)->data(0).toString();

        int buildingID = buildingIDStr.toInt();

        building.ID = buildingID;

        QMap<QString, QVariant> buildingAttributeMap;

        // The feature attributes are the columns from the table
        for(int j = 1; j<buildingTableWidget->columnCount(); ++j)
        {
            auto attrbText = buildingTableWidget->horizontalHeaderItem(j)->text();
            auto attrbVal = buildingTableWidget->item(i,j)->data(0);

            buildingAttributeMap.insert(attrbText,attrbVal);

            featureAttributes.insert(attrbText,attrbVal);
        }

        // Create a unique ID for the building
        auto uid = this->createUniqueID();

        building.ComponentAttributes = buildingAttributeMap;

        featureAttributes.insert("ID", buildingIDStr);
        featureAttributes.insert("LossRatio", 0.0);
        featureAttributes.insert("AssetType", "BUILDING");
        featureAttributes.insert("TabName", buildingIDStr);
        featureAttributes.insert("UID", uid);

        auto latitude = buildingTableWidget->item(i,1)->data(0).toDouble();
        auto longitude = buildingTableWidget->item(i,2)->data(0).toDouble();

        // Get the feature collection table for this layer
        auto layerTag = buildingTableWidget->item(i,columnToMapLayers)->data(0).toString().toStdString();

        auto featureCollectionTable = tablesMap.at(layerTag);

        Feature* feature = nullptr;

        // If a footprint is given use that
        if(indexFootprint != -1)
        {
            QString footprint = buildingTableWidget->item(i,indexFootprint)->data(0).toString();

            if(footprint.compare("NA") == 0)
            {
                Point point(longitude,latitude);
                auto geom = getRectGeometryFromPoint(point, 0.0005,0.0005);
                if(geom.isEmpty())
                {
                    qDebug()<<"Error getting the building footprint geometry";
                    return;
                }

                feature = featureCollectionTable->createFeature(featureAttributes, geom, this);

                featureCollectionTable->addFeature(feature);
            }
            else
            {
                auto geom = getGeometryFromJson(footprint);

                if(geom.isEmpty())
                {
                    qDebug()<<"Error getting the building footprint geometry";
                    return;
                }

                feature = featureCollectionTable->createFeature(featureAttributes, geom, this);

                featureCollectionTable->addFeature(feature);
            }

        }
        else
        {
            Point point(longitude,latitude);
            auto geom = getRectGeometryFromPoint(point, 0.0005,0.0005);
            if(geom.isEmpty())
            {
                qDebug()<<"Error getting the building footprint geometry";
                return;
            }

            feature = featureCollectionTable->createFeature(featureAttributes, geom, this);

            featureCollectionTable->addFeature(feature);
        }

        building.UID = uid;
        building.ComponentFeature = feature;

        theBuildingDb->addComponent(buildingID, building);

    }

    buildingLayer->load();

    zoomToLayer(buildingLayer->layerId());

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

    this->uniqueVec<std::string>(vecLayerItems);

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
            this->userMessageDialog("Error, cannot create a pipeline feature with the latitude and longitude provided");
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


MapGraphicsView* VisualizationWidget::getMapViewWidget() const
{
    return mapViewWidget;
}


void VisualizationWidget::getItemsInConvexHull()
{

    // For debug purposes; creates  a shape
    //    QString inputGeomStr = "{\"points\":[[-16687850.289930943,8675247.6610443853],[-16687885.484665291,8675189.0031538066],[-16687874.819594277,8675053.5567519218],[-16687798.031082973,8675037.5591454003],[-16687641.254539061,8675065.2883300371],[-16687665.784202393,8675228.4639165588]],\"spatialReference\":{\"wkid\":102100,\"latestWkid\":3857}}";
    //    auto impGeom = Geometry::fromJson(inputGeomStr);
    //    m_inputsGraphic->setGeometry(impGeom);
    //    this->plotConvexHull();
    //    return;
    // End debug

    // Check that the input geometry is not empty
    if(m_inputsGraphic->geometry().isEmpty())
    {
        // Clear the graphics
        resetConvexHull();

        return;
    }

    // Get the layers from the map, and ensure that they are not empty
    auto layersList = mapGIS->operationalLayers();

    if(layersList->isEmpty())
    {
        // Clear the graphics
        resetConvexHull();

        return;
    }

    // Normalizing the geometry before performing convex hull operation
    const Geometry normalizedPoints = GeometryEngine::normalizeCentralMeridian(m_inputsGraphic->geometry());
    const Geometry convexHull = GeometryEngine::convexHull(normalizedPoints);

    // Set the envelope of the convex hull as the search parameter
    QueryParameters queryParams;
    auto envelope = Polygon(convexHull);
    queryParams.setGeometry(envelope);
    queryParams.setSpatialRelationship(SpatialRelationship::Contains);

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
                    connect(table, &FeatureTable::queryFeaturesCompleted, this, &VisualizationWidget::selectFeaturesForAnalysisQueryCompleted, Qt::UniqueConnection);

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

    layerIterator(layersList);

    // Clear the graphics
    resetConvexHull();

    return;
}


void VisualizationWidget::selectFeaturesForAnalysisQueryCompleted(QUuid taskID, Esri::ArcGISRuntime::FeatureQueryResult* rawResult)
{
    if(rawResult == nullptr)
        return;

    // Append the raw result to the list - memory management to be handled later
    featuresSelectedForAnalysisList.append(rawResult);

    taskIDMap.remove(taskID);
    emit taskSelectionComplete();
}


void VisualizationWidget::handleLayerSelection(LayerTreeItem* item)
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

    auto layer = this->findLayer(layerID);

    if(layer)
        layer->setOpacity(opacity);
    //    else
    //        qDebug()<<"Warning, could not find the layer "<<layerName;

}


Esri::ArcGISRuntime::Layer* VisualizationWidget::findLayer(const QString& layerID)
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

    this->userMessageDialog(error.message());
}


void VisualizationWidget::handleSelectFeaturesForAnalysis(void)
{
    auto buildingSelected = false;
    auto pipelineSelected = false;

    //    QList<Feature*> buildingFeatures;

    for(auto&& it : featuresSelectedForAnalysisList)
    {
        FeatureIterator iter = it->iterator();
        while (iter.hasNext())
        {
            Feature* feature = iter.next();

            auto atrbList = feature->attributes();

            auto artbMap = atrbList->attributesMap();

            auto assetID = artbMap.value("ID").toInt();

            auto assetType = artbMap.value("AssetType").toString();

            if(assetType.compare("BUILDING") == 0)
            {
                buildingSelected = true;
                buildingWidget->insertSelectedComponent(assetID);
                //                buildingFeatures<<feature;
            }
            else if(assetType.compare("PIPELINE") == 0)
            {
                pipelineSelected = true;
                pipelineWidget->insertSelectedComponent(assetID);
            }
            //else if...

        }
    }

    if(buildingSelected)
    {
        //        this->addComponentsToSelectedLayer(buildingFeatures);
        buildingWidget->handleComponentSelection();
    }

    if(pipelineSelected)
        pipelineWidget->handleComponentSelection();

    // Delete the raw results and clear the selection list
    qDeleteAll(featuresSelectedForAnalysisList);

    featuresSelectedForAnalysisList.clear();
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


ClassBreaksRenderer* VisualizationWidget::createBuildingRenderer(double outlineWidth)
{
    // Images stored in base64 format
    //    QByteArray buildingImg1 = "iVBORw0KGgoAAAANSUhEUgAAABwAAAAcCAYAAAByDd+UAAAAAXNSR0IB2cksfwAAAAlwSFlzAAAOxAAADsQBlSsOGwAAAWhJREFUSInt1jFrwkAYxvF/SDpJBaHFtlN0dlP0O3TWSWobOnZvidA2QycHZ9cGhNKlX0FwcPcLCC4OrWKGDMKZdKmg1WpOo1LwgYMb3nt/HHdwp7HjaHsDTdM8GgwGFnADXITU/xN4j0QiD9Vq1Z0Bh8PhE1AOCZrkFLhzXfcYuJ4BPc+7ChmbzuVkMn2GZ1sETxaBUkkkEnQ6Hel1a4GGYZBOp6nX6ySTSVKpFACWZTEajcIFDcMgl8sBUCwW6ff7xGIxAFRVXbleCpzGADRNIx6Py7QIDv7G1k0gMCiWzWZpNBqbgTI7KxQKjMdjms3memCpVCKTyeD7PoqirAQVRSGfzyOEoNVqyYO2bWPbNpVKhWg0uhJst9vUarWlNft7LQ7gAfzfYLkc7Ofh+74U2AP0RUVCiEDgkvQWga/A86ad/8jHHKjr+ku321U9z7sFzkOCvoA3x3Hu50DTNAXw+DO2lp3f0m97bGdscCiEZAAAAABJRU5ErkJggg==";
    //    QByteArray buildingImg2 = "iVBORw0KGgoAAAANSUhEUgAAABwAAAAcCAYAAAByDd+UAAAAAXNSR0IB2cksfwAAAAlwSFlzAAAOxAAADsQBlSsOGwAAAYNJREFUSInt1b9LAmEcx/H3nVfSYEiU9MNBaHPJwKaKoKGh6YbopqioqYaGoLjAashJ8C9oCZeQg/bAXXBoaGoPhH7YEA1n1z0NYVha3umlBH6mZ3i+39fzCx6FNkfpGKjr9FiW/1gIcw0Y9aj/PZC1bfbTaV6+gK+v/kMwDzyCKhkCtn0+AsDqFxDMFY+xzwjBYmVcfYfDfwUCg/VAVwmHN7m9PXVd1xSoaTlisVmy2XEikQmi0SkAUqkZyuUbb0FNyxGPzwOwvLxLqXRHMPhxYrLc17DeFViNAShKD6HQmJsWzsHvWLNxBDrFYrF18vmd1kA3O1PVLd7eLAqF3ebApaVLJifnEEIgSVJDUJZlVHULyypzdaW7Bw1jAcOAROKJ/v5gQ/D6Ok8mM/3rnM79Fl2wC/5vMJmMIsu9DefZ9rMrsAhE6jcqYttOlvZjijWgJPnPhDCPWmr7Q2ybixpwYMA8eXz0+8DcAEY8sh6A80CAvRpQ17HATAAJj7C6afsrfQdYrmo3mMtmpgAAAABJRU5ErkJggg==";
    //    QByteArray buildingImg3 = "iVBORw0KGgoAAAANSUhEUgAAABwAAAAcCAYAAAByDd+UAAAAAXNSR0IB2cksfwAAAAlwSFlzAAAOxAAADsQBlSsOGwAAAWJJREFUSInt1s8rw3Ecx/Hn2zbfJpo0xTj4dXKh1ZSQUhzkX9CI0xwcVkQNB3+Gi9tycVPUzuS0qyPKCpFQvtp8HGaz2bd9v9/ta0t5nT59en/ej++PT30+bmocd91AFcejRNsR9AUg4FD/OwUH8sa6hHktBkXbEvRNh6Bc2gVWlJcWIFwECvq8w1g+opjNjQv/YcdvgYDfCLQX3zI87dleVhkYTEDvBJz2g38IukPZ+eNxyFw4DAYTMDCVHY9G4eUWmr++mHhNl9sDCzEAlwd8XbZaWAd/YhXGGmgVCyzC1WqVoJ03G4mASsN1tEJw+AT6JkEpEDEHpQFCEci8w81GBWByBpLA3CM0tZqDl2dwPla2pH6nxT/4D/5x8GgQaDSvU8+2wBTQY1j1kbLyWOWSb/B9iULbF/TtajsbRcFhCSht+q560FyCvgR0OmTdK4iLxlopOE0a9BgQcwgzTM136SeMBkz2tFUt2gAAAABJRU5ErkJggg==";
    //    QByteArray buildingImg4 = "iVBORw0KGgoAAAANSUhEUgAAABwAAAAcCAYAAAByDd+UAAAAAXNSR0IB2cksfwAAAAlwSFlzAAAOxAAADsQBlSsOGwAAAYxJREFUSInt1csrRGEYx/Hve2bGidwml1wil2xsMDUpl5RioWzYKAmxGgsLRaNcs7DyFyjZnWxsLKRmO0kWsrMTZQqRJvJq5hyLSQ0zY+bMHCPlt3oXz/t83lu9drIc+6+BmhcHL+oqipxAp8qi/nco7EnBwvgWz59A41VdFshFdIuoSMrQmcmBAmD8Eyh0OWYpFRUBAx/j6Dus+CkQKI0HmkpR7TRPV9um56UFukZ9NLR149caKa1voabZDcDhZhdheWEt6Br10eTuBaBjZI7gwy35zsiJCVtu0vmmwGgMwGZ3UFxebaZF6uBXLN2kBKaKVbVOcuWfzQw0s7P2YQ+GHuL6eC49sHXkiEZXD4ZhIIRICgpFwT3kIRx64+bUax480/o502Bw/ZG8wuKk4OX5MSc7nd/W/N5v8Q/+g38bPFhrBiUneaEeNAEaBBDUxSsywgEIp7K0hAnEgkLdBbmSUdsEEbAfA5ZUyI37W9UmdDkFVFpk3WOgqU7mY8A+LyGQS8CSRVjcZP2VvgN6imQ8SFFgygAAAABJRU5ErkJggg==";
    //    QByteArray buildingImg5 = "iVBORw0KGgoAAAANSUhEUgAAABwAAAAcCAYAAAByDd+UAAAAAXNSR0IB2cksfwAAAAlwSFlzAAAOxAAADsQBlSsOGwAAAXpJREFUSInt1b1LQlEYx/HvMfVQFAkZVBJE0uJiBBKkEQg1BP0JYZLTbXC4UBj0MvRntLRJS0tDCK4hTa1tUZBggoQ0nFBuQ2gvXvRevRVBv+kMz3k+5w2Omx+O+9fALHiQ8hClNoAJh/o/AqcKdhLw/Ak0pNwXSu06BDUyCmx5YQhIfAKFUusOY80IWG2MP97h2HeBgN8MtJXhVIqn42Pb87oC5/J5phcXuQwG8YfDTEYiAFzEYtRvbpwF5/J5ZuJxABZ0nWqpxKD/7cREf3/H+bbAjxhAn8eDLxCw08I6+BXrNpZAq9hEMsldOt0baGdn85qGUatxr+vdgbO5HMGlJQzDQAjRERQuFxFNo/7ywkMmYx+8XlnhGlirVBjw+TqCt4UCV9Fo25rf+y3+wX/wb4PnoRB4vZ0Lq1VbYBGYMisyikUr62qXZoN3UMoTlDrotbNZBJy1gCNKHZWl7BNKbQLjDlllICthuwVchhpK7QF7DmGm+fFX+gonY17k9eIf3wAAAABJRU5ErkJggg==";

    //    buildingImg1 = QByteArray::fromBase64(buildingImg1);
    //    buildingImg2 = QByteArray::fromBase64(buildingImg2);
    //    buildingImg3 = QByteArray::fromBase64(buildingImg3);
    //    buildingImg4 = QByteArray::fromBase64(buildingImg4);
    //    buildingImg5 = QByteArray::fromBase64(buildingImg5);

    //    QImage img1 = QImage::fromData(buildingImg1);
    //    QImage img2 = QImage::fromData(buildingImg2);
    //    QImage img3 = QImage::fromData(buildingImg3);
    //    QImage img4 = QImage::fromData(buildingImg4);
    //    QImage img5 = QImage::fromData(buildingImg5);

    //    PictureMarkerSymbol* symbol1 = new PictureMarkerSymbol(img1, this);
    //    PictureMarkerSymbol* symbol2 = new PictureMarkerSymbol(img2, this);
    //    PictureMarkerSymbol* symbol3 = new PictureMarkerSymbol(img3, this);
    //    PictureMarkerSymbol* symbol4 = new PictureMarkerSymbol(img4, this);
    //    PictureMarkerSymbol* symbol5 = new PictureMarkerSymbol(img5, this);

    //        SimpleMarkerSymbol* symbol1 = new SimpleMarkerSymbol(SimpleMarkerSymbolStyle::Square, QColor(0, 0, 255,125), 8.0f, this);
    //        SimpleMarkerSymbol* symbol2 = new SimpleMarkerSymbol(SimpleMarkerSymbolStyle::Square, QColor(255,255,178), 8.0f, this);
    //        SimpleMarkerSymbol* symbol3 = new SimpleMarkerSymbol(SimpleMarkerSymbolStyle::Square, QColor(253,204,92), 8.0f, this);
    //        SimpleMarkerSymbol* symbol4 = new SimpleMarkerSymbol(SimpleMarkerSymbolStyle::Square, QColor(253,141,60), 8.0f, this);
    //        SimpleMarkerSymbol* symbol5 = new SimpleMarkerSymbol(SimpleMarkerSymbolStyle::Square, QColor(240,59,32), 8.0f, this);

    SimpleFillSymbol* symbol1 = new SimpleFillSymbol(SimpleFillSymbolStyle::Solid, QColor(0, 0, 255,125), this);
    SimpleFillSymbol* symbol2 = new SimpleFillSymbol(SimpleFillSymbolStyle::Solid, QColor(255,255,178), this);
    SimpleFillSymbol* symbol3 = new SimpleFillSymbol(SimpleFillSymbolStyle::Solid, QColor(253,204,92), this);
    SimpleFillSymbol* symbol4 = new SimpleFillSymbol(SimpleFillSymbolStyle::Solid, QColor(253,141,60), this);
    SimpleFillSymbol* symbol5 = new SimpleFillSymbol(SimpleFillSymbolStyle::Solid, QColor(240,59,32), this);

    if(outlineWidth != 0.0)
    {
        SimpleLineSymbol* outlineSymbol = new SimpleLineSymbol(SimpleLineSymbolStyle::Solid, QColor(255, 255, 0, 200), outlineWidth, this);
        symbol1->setOutline(outlineSymbol);
        symbol2->setOutline(outlineSymbol);
        symbol3->setOutline(outlineSymbol);
        symbol4->setOutline(outlineSymbol);
        symbol5->setOutline(outlineSymbol);
    }

    QList<ClassBreak*> classBreaks;

    auto classBreak1 = new ClassBreak("0.00-0.05 Loss Ratio", "Loss Ratio less than 10%", -0.00001, 0.05, symbol1,this);
    classBreaks.append(classBreak1);

    auto classBreak2 = new ClassBreak("0.05-0.25 Loss Ratio", "Loss Ratio Between 10% and 25%", 0.05, 0.25, symbol2,this);
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

            attrbKeyList.append(atrb);

            auto atrbVal = elemAttrib->attributeValue(atrb);

            attrbValList.append(atrbVal.toString());
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
    if (mapGIS->loadStatus() != LoadStatus::Loaded || selectingConvexHull == true)
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


void VisualizationWidget::handleAsyncSelectionTask(void)
{
    // Only handle the selected features when all tasks are complete
    if(taskIDMap.empty())
    {
        this->handleSelectFeaturesForAnalysis();
    }

    //    QMap<QUuid, QString>::const_iterator i = m_taskIds.constBegin();
    //    while (i != m_taskIds.constEnd())
    //    {
    //        auto isDone = i.value();
    //        qDebug()<< i.key() << ": "<<isDone << Qt::endl;
    //        ++i;
    //    }

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


Esri::ArcGISRuntime::Map *VisualizationWidget::getMapGIS() const
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


void VisualizationWidget::addComponentsToSelectedLayer(const QList<Feature*>& features)
{

    if(features.empty())
        return;

    // Handle selection of buildings
    if(selectedBuildingsTable)
    {
        auto canAdd = selectedBuildingsTable->canAdd();

        if(canAdd == false)
            return;

        for(auto&& it : features)
        {
            auto atrb = it->attributes()->attributesMap();
            auto id = atrb.value("UID").toString();
            //        auto nid = atrb.value("ID").toString();

            auto atrVals = atrb.values();
            auto atrKeys = atrb.keys();

            if(selectedFeaturesForAnalysis.contains(id))
                continue;

            // qDebug()<<"Num atributes: "<<atrb.size();

            QMap<QString, QVariant> featureAttributes;
            for(int i = 0; i<atrb.size();++i)
            {
                auto key = atrKeys.at(i);
                auto val = atrVals.at(i);

                // Including the ObjectID causes a crash!!! Do not include it when creating an object
                if(key == "ObjectID")
                    continue;

                // qDebug()<< nid<<"-key:"<<key<<"-value:"<<atrVals.at(i).toString();

                featureAttributes[key] = val;
            }

            // featureAttributes.insert("ID", "99");
            // featureAttributes.insert("LossRatio", 0.0);
            // featureAttributes.insert("AssetType", "BUILDING");
            // featureAttributes.insert("TabName", "99");
            // featureAttributes.insert("UID", "99");

            auto geom = it->geometry();
            Feature* feat = selectedBuildingsTable->createFeature(featureAttributes,geom,this);
            selectedBuildingsTable->addFeature(feat);
            selectedFeaturesForAnalysis.insert(id,feat);
        }
    }

    // Create the tree item if
    if(selectedComponentsTreeItem == nullptr && !selectedFeaturesForAnalysis.empty())
    {
        if(selectedComponentsLayer == nullptr)
        {
            // Create the buildings group layer that will hold the sublayers
            selectedComponentsLayer = new GroupLayer(QList<Layer*>{}, this);
            selectedComponentsLayer->setName("Selected Components");

            selectedComponentsTreeItem = this->addLayerToMap(selectedComponentsLayer);

            this->addLayerToMap(selectedBuildingsLayer, selectedComponentsTreeItem, selectedComponentsLayer);
        }
    }
}


void VisualizationWidget::clearLayerSelectedForAnalysis(void)
{

    if(selectedFeaturesForAnalysis.empty())
        return;

    for(auto&& it : selectedFeaturesForAnalysis)
    {
        selectedBuildingsTable->deleteFeature(it);
    }

    // selectedBuildingsTable->deleteFeatures(selectedFeatures);
    selectedFeaturesForAnalysis.clear();
}


void VisualizationWidget::updateSelectedComponent(const QString& uid, const QString& attribute, const QVariant& value)
{
    if(selectedFeaturesForAnalysis.empty())
    {
        qDebug()<<"Selected features map is empty";
        return;
    }

    if(!selectedFeaturesForAnalysis.contains(uid))
    {
        qDebug()<<"Feature not found in selected components map";
        return;
    }

    // Get the feature
    Feature* feat = selectedFeaturesForAnalysis[uid];

    if(feat == nullptr)
    {
        qDebug()<<"Feature is a nullptr";
        return;
    }

    feat->attributes()->replaceAttribute(attribute,value);
    feat->featureTable()->updateFeature(feat);

    if(feat->attributes()->attributeValue(attribute).isNull())
    {
        qDebug()<<"Failed to update feature "<<feat->attributes()->attributeValue("ID").toString();
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
        this->userMessageDialog(msg);
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
            this->userMessageDialog(msg);
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
            this->userMessageDialog(msg);
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
            this->userMessageDialog(msg);

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
        this->userMessageDialog(msg);
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
        this->userMessageDialog(msg);
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
            this->userMessageDialog(msg);
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
        this->userMessageDialog(errMess);
        return nullptr;
    }

    XMLlayer->setName(layerName);

    auto layerID = this->createUniqueID();

    XMLlayer->setLayerId(layerID);

    // Add the layers to the layer tree
    layersTree->addItemToTree(layerName, layerID, parentItem);

    return XMLlayer;
}


template <typename T>
void VisualizationWidget::uniqueVec(std::vector<T>& vec)
{
    std::sort(vec.begin(), vec.end());

    // Using std::unique to get the unique items in the vector
    auto ip = std::unique(vec.begin(), vec.end());

    // Resizing the vector so as to remove the terms that became undefined after the unique operation
    vec.resize(std::distance(vec.begin(), ip));
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


void VisualizationWidget::removeLayerFromMap(Esri::ArcGISRuntime::Layer* layer)
{
    mapGIS->operationalLayers()->removeOne(layer);

    //    delete layer;
}


void VisualizationWidget::removeLayerFromMap(const QString layerID)
{
    auto layer = this->findLayer(layerID);
    this->removeLayerFromMap(layer);
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

    featuresSelectedForAnalysisList.clear();

    mapGIS->operationalLayers()->clear();

    delete selectedComponentsLayer;

    legendView->clear();

    selectedComponentsTreeItem = nullptr;
    selectedComponentsLayer = nullptr;

    this->clearSelection();
}


void VisualizationWidget::setLegendView(GISLegendView* legndView)
{
    if(legndView == nullptr)
        return;

    legendView = legndView;

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
