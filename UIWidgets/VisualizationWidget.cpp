#include "VisualizationWidget.h"

#include "TreeView.h"
#include "TreeItem.h"
#include "PopUpWidget.h"
#include "ComponentInputWidget.h"

//Test
#include "XMLAdaptor.h"

// GIS headers
#include "Basemap.h"
#include "Map.h"
#include "MapGraphicsView.h"
#include "SimpleMarkerSymbol.h"
#include "PictureMarkerSymbol.h"
#include "PictureMarkerSymbolLayer.h"
#include "FeatureCollectionLayer.h"
#include "FeatureLayer.h"
#include "FeatureCollection.h"
#include "FeatureCollectionTable.h"
#include "SimpleRenderer.h"
#include "ClassBreaksRenderer.h"
#include "LayerContent.h"
#include "IdentifyLayerResult.h"
#include "PopupManager.h"
#include "GroupLayer.h"
#include "RasterLayer.h"
#include "ShapefileFeatureTable.h"
#include "KmlDataset.h"
#include "KmlLayer.h"
#include "sectiontitle.h"
#include "PolylineBuilder.h"
#include "LineSegment.h"
#include "CoordinateFormatter.h"
#include "Geodatabase.h"
#include "GeodatabaseFeatureTable.h"
#include "GeographicTransformationStep.h"
#include "GeographicTransformation.h"
#include "TransformationCatalog.h"
#include "ArcGISMapImageLayer.h"

// Convex Hull
#include "SimpleFillSymbol.h"
#include "SimpleLineSymbol.h"
#include "GeometryEngine.h"
#include "MultipointBuilder.h"

#include <QGroupBox>
#include <QComboBox>
#include <QString>
#include <QGridLayout>
#include <QTreeView>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QFileInfo>
#include <QCoreApplication>

using namespace Esri::ArcGISRuntime;

VisualizationWidget::VisualizationWidget(QWidget* parent) : SimCenterAppWidget(parent)
{    

    visWidget = nullptr;
    this->setContentsMargins(0,0,0,0);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setMargin(0);

    selectingConvexHull = false;

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
    mapViewWidget = new MapGraphicsView(this);
    mapViewWidget->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    // Create a map using the topographic Basemap
    mapGIS = new Map(Basemap::topographic(this), this);

    mapGIS->setObjectName("MainMap");

    mapViewWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    // Set map to map view
    mapViewWidget->setMap(mapGIS);

    // Set the initial viewport to UCB
    Viewpoint UCB(37.8717450069, -122.2609607382, 100000.0);
    mapGIS->setInitialViewpoint(UCB);

    // Setup the various convex hull objects
    setupConvexHullObjects();

    // Handle the async. queries
    connect(this, &VisualizationWidget::taskSelectionChanged, this, &VisualizationWidget::handleAsynchronousSelectionTask);

    // Create the visualization widget and set it to the main layout
    this->createVisualizationWidget();
    mainLayout->addWidget(visWidget);

    this->setLayout(mainLayout);
    this->setMinimumWidth(640);

    // Popup stuff
    // Once map is set, connect to MapQuickView mouse clicked signal
    connect(mapViewWidget, &MapGraphicsView::mouseClicked, this, &VisualizationWidget::onMouseClicked);

    // Connect to MapQuickView::identifyLayerCompleted signal
    connect(mapViewWidget, &MapGraphicsView::identifyLayersCompleted, this, &VisualizationWidget::identifyLayersCompleted);

    // Connect to the exportImageCompleted signal
    connect(mapViewWidget, &MapGraphicsView::exportImageCompleted, this, &VisualizationWidget::exportImageComplete);

    // Test
    //    QString filePath = "/Users/steve/Desktop/SimCenter/Examples/SFTallBuildings/TallBuildingInventory.kmz";
    //    QString layerName = "Buildings Foot Print";
    //    QString layerID = this->createUniqueID();
    //    TreeItem* buildingsItem = layersTree->addItemToTree(layerName,layerID);
    //    auto buildingsLayer = this->createAndAddKMLLayer(filePath, layerName, buildingsItem);
    //    buildingsLayer->setLayerId(layerID);
    //    buildingsLayer->setName("SF");
    //    this->addLayerToMap(buildingsLayer,buildingsItem);


    //   QString layerName = "Bathymetry";
    //   QString layerID = this->createUniqueID();
    //   TreeItem* treeItem = layersTree->addItemToTree(layerName,layerID);

    //   QString filePath = "/Users/steve/Downloads/GEBCO_2020_18_Nov_2020_f103650dc2c4/gebco_2020_n30.0_s15.0_w-179.0_e-152.0.tif";
    //   auto rastLayer = this->createAndAddRasterLayer(filePath, layerName, treeItem) ;
    //   rastLayer->setLayerId(layerID);
    //   rastLayer->setName(layerName);
    //   this->addLayerToMap(rastLayer,treeItem);

}


VisualizationWidget::~VisualizationWidget()
{

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
    layersTree = new TreeView(visWidget, this);

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
    bottomText->setText("Click the “Apply” button to\nselect the subset of assets");
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


    layout->addWidget(mapViewWidget,0,1,12,2);
}

BuildingDatabase* VisualizationWidget::getBuildingDatabase()
{
    return &theBuildingDb;
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

    qDebug()<<"Viz Widget Mouse: "<<e.x()<<","<< e.y();

    const Point clickedPoint = mapViewWidget->screenToLocation(e.x(), e.y());

    qDebug()<<"Viz Widget Point: "<<clickedPoint.x()<<","<< clickedPoint.y();

    m_multipointBuilder->points()->addPoint(clickedPoint);
    m_inputsGraphic->setGeometry(m_multipointBuilder->toGeometry());

    this->plotConvexHull();
}


TreeView *VisualizationWidget::getLayersTree() const
{
    return layersTree;
}


void VisualizationWidget::setPipelineWidget(ComponentInputWidget *value)
{
    pipelineWidget = value;

    connect(pipelineWidget,&ComponentInputWidget::componentDataLoaded,this,&VisualizationWidget::loadPipelineData);
}


void VisualizationWidget::setBuildingWidget(ComponentInputWidget *value)
{
    buildingWidget = value;

    connect(buildingWidget,&ComponentInputWidget::componentDataLoaded,this,&VisualizationWidget::loadBuildingData);
}


void VisualizationWidget::loadBuildingData(void)
{

    auto buildingTableWidget = buildingWidget->getTableWidget();

    QList<Field> fields;
    fields.append(Field::createDouble("LossRatio", "0.0"));
    fields.append(Field::createText("AssetType", "NULL",4));
    fields.append(Field::createText("TabName", "NULL",4));


    // Select a column that will define the building layers
    int columnToMapLayers = 0;

    QString columnFilter = "occupancy";

    // Set the table headers as fields in the table
    for(int i =0; i<buildingTableWidget->columnCount(); ++i)
    {
        auto headerItem = buildingTableWidget->horizontalHeaderItem(i);

        auto fieldText = headerItem->text();

        if(fieldText.compare(columnFilter) == 0)
            columnToMapLayers = i;

        fields.append(Field::createText(fieldText, fieldText,fieldText.size()));
    }

    // Create the buildings group layer that will hold the sublayers
    auto buildingLayer = new GroupLayer(QList<Layer*>{});
    buildingLayer->setName("Buildings");

    auto layerID = this->createUniqueID();
    buildingLayer->setLayerId(layerID);

    // Create the root item in the trees
    auto buildingsItem = layersTree->addItemToTree("Buildings", layerID);

    auto nRows = buildingTableWidget->rowCount();

    std::vector<std::string> vecLayerItems;
    for(int i = 0; i<nRows; ++i)
    {
        // Organize the layers according to occupancy type
        auto occupancyType = buildingTableWidget->item(i,columnToMapLayers)->data(0).toString().toStdString();

        vecLayerItems.push_back(occupancyType);
    }

    this->uniqueVec<std::string>(vecLayerItems);

    // Map to hold the feature tables
    std::map<std::string, FeatureCollectionTable*> tablesMap;
    for(auto&& it : vecLayerItems)
    {
        auto featureCollection = new FeatureCollection();

        auto featureCollectionTable = new FeatureCollectionTable(fields, GeometryType::Point, SpatialReference::wgs84());

        featureCollection->tables()->append(featureCollectionTable);

        auto newBuildingLayer = new FeatureCollectionLayer(featureCollection);

        newBuildingLayer->setName(QString::fromStdString(it));

        buildingLayer->layers()->append(newBuildingLayer);

        featureCollectionTable->setRenderer(this->createBuildingRenderer());

        tablesMap.insert(std::make_pair(it,featureCollectionTable));

        auto layerID = this->createUniqueID();

        newBuildingLayer->setLayerId(layerID);

        layersTree->addItemToTree(QString::fromStdString(it), layerID, buildingsItem);
    }


    for(int i = 0; i<nRows; ++i)
    {
        // create the feature attributes
        QMap<QString, QVariant> featureAttributes;

        // Create a new building
        Building building;

        int buildingID = buildingTableWidget->item(i,0)->data(0).toInt();

        building.ID = buildingID;

        QMap<QString, QVariant> buildingAttributeMap;

        // The feature attributes are the columns from the table
        for(int j = 0; j<buildingTableWidget->columnCount(); ++j)
        {
            auto attrbText = buildingTableWidget->horizontalHeaderItem(j)->text();
            auto attrbVal = buildingTableWidget->item(i,j)->data(0);

            buildingAttributeMap.insert(attrbText,attrbVal.toString());

            featureAttributes.insert(attrbText,attrbVal);
        }

        building.buildingAttributes = buildingAttributeMap;

        featureAttributes.insert("LossRatio", 0.0);
        featureAttributes.insert("AssetType", "BUILDING");
        featureAttributes.insert("TabName", buildingTableWidget->item(i,0)->data(0).toString());

        auto latitude = buildingTableWidget->item(i,1)->data(0).toDouble();
        auto longitude = buildingTableWidget->item(i,2)->data(0).toDouble();

        // Get the feature collection table for this layer
        auto layerTag = buildingTableWidget->item(i,columnToMapLayers)->data(0).toString().toStdString();

        auto featureCollectionTable = tablesMap.at(layerTag);

        // Create the point and add it to the feature table
        Point point(longitude,latitude);
        Feature* feature = featureCollectionTable->createFeature(featureAttributes, point, this);

        building.buildingFeature = feature;

        theBuildingDb.addBuilding(buildingID, building);

        featureCollectionTable->addFeature(feature);
    }


    mapGIS->operationalLayers()->append(buildingLayer);

    // When the layer is done loading, zoom to extents of the data
    //    connect(buildingLayer, &GroupLayer::doneLoading, this, [this, buildingLayer](Error loadError)
    //    {
    //        if (!loadError.isEmpty())
    //        {
    //            auto msg = loadError.additionalMessage();
    //            this->userMessageDialog(msg);
    //            return;
    //        }

    //        mapViewWidget->setViewpointCenter(buildingLayer->fullExtent().center(), 80000);
    //    });

    this->zoomToExtents();
}


void VisualizationWidget::changeLayerOrder(const int from, const int to)
{
    mapGIS->operationalLayers()->move(from, to);
}


void VisualizationWidget::loadPipelineData(void)
{
    auto pipelineTableWidget = pipelineWidget->getTableWidget();

    QList<Field> fields;
    fields.append(Field::createDouble("LossRatio", "0.0"));
    fields.append(Field::createText("AssetType", "NULL",4));
    fields.append(Field::createText("TabName", "NULL",4));

    // Set the table headers as fields in the table
    for(int i =0; i<pipelineTableWidget->columnCount(); ++i)
    {
        auto headerItem = pipelineTableWidget->horizontalHeaderItem(i);
        auto fieldText = headerItem->text();
        fields.append(Field::createText(fieldText, fieldText,fieldText.size()));
    }

    // Create the pipelines group layer that will hold the sublayers
    auto pipelineLayer = new GroupLayer(QList<Layer*>{});
    pipelineLayer->setName("Pipelines");

    auto layerID = this->createUniqueID();
    pipelineLayer->setLayerId(layerID);

    // Create the root item in the trees
    auto pipelinesItem = layersTree->addItemToTree("Pipelines",layerID);

    auto nRows = pipelineTableWidget->rowCount();

    // Select a column that will define the layers
    int columnToMapLayers = 5;

    std::vector<std::string> vecLayerItems;
    for(int i = 0; i<nRows; ++i)
    {
        // Organize the layers according to occupancy type
        auto layerTag = pipelineTableWidget->item(i,columnToMapLayers)->data(0).toString().toStdString();

        vecLayerItems.push_back(layerTag);
    }

    this->uniqueVec<std::string>(vecLayerItems);

    // Map to hold the feature tables
    std::map<std::string, FeatureCollectionTable*> tablesMap;

    for(auto&& it : vecLayerItems)
    {
        auto featureCollection = new FeatureCollection();

        auto featureCollectionTable = new FeatureCollectionTable(fields, GeometryType::Polyline, SpatialReference::wgs84());

        featureCollection->tables()->append(featureCollectionTable);

        auto newpipelineLayer = new FeatureCollectionLayer(featureCollection);

        newpipelineLayer->setName(QString::fromStdString(it));

        pipelineLayer->layers()->append(newpipelineLayer);

        featureCollectionTable->setRenderer(this->createPipelineRenderer());

        tablesMap.insert(std::make_pair(it,featureCollectionTable));

        auto layerID = this->createUniqueID();

        newpipelineLayer->setLayerId(layerID);

        layersTree->addItemToTree(QString::fromStdString(it), layerID ,pipelinesItem);
    }

    for(int i = 0; i<nRows; ++i)
    {

        // create the feature attributes
        QMap<QString, QVariant> featureAttributes;

        // The feature attributes are the columns from the table
        for(int j = 0; j<pipelineTableWidget->columnCount(); ++j)
        {
            auto attrbText = pipelineTableWidget->horizontalHeaderItem(j)->text();
            auto attrbVal = pipelineTableWidget->item(i,j)->data(0);

            featureAttributes.insert(attrbText,attrbVal);
        }

        double r = pipelineTableWidget->item(i,7)->data(0).toDouble();

        featureAttributes.insert("LossRatio", r);
        featureAttributes.insert("AssetType", "PIPELINE");
        featureAttributes.insert("TabName", pipelineTableWidget->item(i,0)->data(0).toString());

        // Get the feature collection table from the map
        auto layerTag = pipelineTableWidget->item(i,columnToMapLayers)->data(0).toString().toStdString();

        auto featureCollectionTable = tablesMap.at(layerTag);

        auto latitudeStart = pipelineTableWidget->item(i,1)->data(0).toDouble();
        auto longitudeStart = pipelineTableWidget->item(i,2)->data(0).toDouble();

        auto latitudeEnd = pipelineTableWidget->item(i,3)->data(0).toDouble();
        auto longitudeEnd = pipelineTableWidget->item(i,4)->data(0).toDouble();

        // Create the points and add it to the feature table
        PolylineBuilder polylineBuilder(SpatialReference::wgs84());

        // Get the two start and end points of the pipeline segment

        Point point1(longitudeStart,latitudeStart);

        Point point2(longitudeEnd,latitudeEnd);

        polylineBuilder.addPoint(point1);
        polylineBuilder.addPoint(point2);

        // Create the polyline feature
        auto polyline =  polylineBuilder.toPolyline();

        // Add the feature to the table
        Feature* feature = featureCollectionTable->createFeature(featureAttributes, polyline, this);
        featureCollectionTable->addFeature(feature);
    }

    mapGIS->operationalLayers()->append(pipelineLayer);

    // When the layer is done loading, zoom to extents of the data
    //    connect(pipelineLayer, &GroupLayer::doneLoading, this, [this, pipelineLayer](Error loadError)
    //    {
    //        if (!loadError.isEmpty())
    //        {
    //            auto msg = loadError.additionalMessage();
    //            this->userMessageDialog(msg);
    //            return;
    //        }

    //        auto env = pipelineLayer->fullExtent();

    //        auto depth = env.depth();

    //        auto width = env.width();

    //        mapViewWidget->setViewpointCenter(pipelineLayer->fullExtent().center(), 80000);
    //    });

    this->zoomToExtents();

}


MapGraphicsView* VisualizationWidget::getMapViewWidget() const
{
    return mapViewWidget;
}


void VisualizationWidget::getItemsInConvexHull()
{

    // For debug purposes; creates  a shape
    // QString inputGeomStr = "{\"points\":[[-16687850.289930943,8675247.6610443853],[-16687885.484665291,8675189.0031538066],[-16687874.819594277,8675053.5567519218],[-16687798.031082973,8675037.5591454003],[-16687641.254539061,8675065.2883300371],[-16687665.784202393,8675228.4639165588]],\"spatialReference\":{\"wkid\":102100,\"latestWkid\":3857}}";
    // auto impGeom = Geometry::fromJson(inputGeomStr);
    // m_inputsGraphic->setGeometry(impGeom);
    // this->plotConvexHull();
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
    auto envelope = convexHull.extent();
    queryParams.setGeometry(envelope);
    queryParams.setSpatialRelationship(SpatialRelationship::Intersects);

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
                    connect(table, &FeatureTable::queryFeaturesCompleted, this, &VisualizationWidget::featureSelectionQueryCompleted, Qt::UniqueConnection);

                    // Query the table for features - note that this is done asynchronously
                    auto taskWatcher = table->queryFeatures(queryParams);

                    if (!taskWatcher.isValid())
                        qDebug() <<"Error, task not valid in "<<__PRETTY_FUNCTION__;
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


void VisualizationWidget::featureSelectionQueryCompleted(QUuid taskID, Esri::ArcGISRuntime::FeatureQueryResult* rawResult)
{
    if(rawResult == nullptr)
        return;

    // Append the raw result to the list - memory management to be handled later
    selectedFeaturesList.append(rawResult);

    taskIDMap.remove(taskID);
    emit taskSelectionChanged();
}


void VisualizationWidget::handleLayerSelection(TreeItem* item)
{
    auto itemID = item->getItemID();

    auto isChecked = item->getState() == 1 || item->getState() == 2 ? true : false;

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
            //            auto subLayersCont = layer->subLayerContents();
            //            for(auto&& it : subLayersCont)
            //            {
            //                if(it->name() == itemID)
            //                {
            //                    it->setVisible(isChecked);
            //                    return true;
            //                }
            //            }

            if(auto isGroupLayer = dynamic_cast<GroupLayer*>(layer))
            {
                auto subLayers = isGroupLayer->layers();
                auto found = layerIterator(subLayers);

                if(found)
                {
                    //                    isGroupLayer->setVisible(true);
                    return true;
                }
            }
        }

        return false;
    };

    auto res = layerIterator(layersList);

    if(res == false)
        qDebug()<<"Warning, layer "<<item->getName()<<" not found in map layers in "<<__PRETTY_FUNCTION__;

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


void VisualizationWidget::handleSelectedFeatures(void)
{
    auto buildingSelected = false;
    auto pipelineSelected = false;

    for(auto&& it : selectedFeaturesList)
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
        buildingWidget->handleComponentSelection();

    if(pipelineSelected)
        pipelineWidget->handleComponentSelection();

    // Delete the raw results and clear the selection list
    qDeleteAll(selectedFeaturesList);

    selectedFeaturesList.clear();
}


ClassBreaksRenderer* VisualizationWidget::createBuildingRenderer(void)
{
    // Images stored in base64 format
    QByteArray buildingImg1 = "iVBORw0KGgoAAAANSUhEUgAAABwAAAAcCAYAAAByDd+UAAAAAXNSR0IB2cksfwAAAAlwSFlzAAAOxAAADsQBlSsOGwAAAWhJREFUSInt1jFrwkAYxvF/SDpJBaHFtlN0dlP0O3TWSWobOnZvidA2QycHZ9cGhNKlX0FwcPcLCC4OrWKGDMKZdKmg1WpOo1LwgYMb3nt/HHdwp7HjaHsDTdM8GgwGFnADXITU/xN4j0QiD9Vq1Z0Bh8PhE1AOCZrkFLhzXfcYuJ4BPc+7ChmbzuVkMn2GZ1sETxaBUkkkEnQ6Hel1a4GGYZBOp6nX6ySTSVKpFACWZTEajcIFDcMgl8sBUCwW6ff7xGIxAFRVXbleCpzGADRNIx6Py7QIDv7G1k0gMCiWzWZpNBqbgTI7KxQKjMdjms3memCpVCKTyeD7PoqirAQVRSGfzyOEoNVqyYO2bWPbNpVKhWg0uhJst9vUarWlNft7LQ7gAfzfYLkc7Ofh+74U2AP0RUVCiEDgkvQWga/A86ad/8jHHKjr+ku321U9z7sFzkOCvoA3x3Hu50DTNAXw+DO2lp3f0m97bGdscCiEZAAAAABJRU5ErkJggg==";
    QByteArray buildingImg2 = "iVBORw0KGgoAAAANSUhEUgAAABwAAAAcCAYAAAByDd+UAAAAAXNSR0IB2cksfwAAAAlwSFlzAAAOxAAADsQBlSsOGwAAAYNJREFUSInt1b9LAmEcx/H3nVfSYEiU9MNBaHPJwKaKoKGh6YbopqioqYaGoLjAashJ8C9oCZeQg/bAXXBoaGoPhH7YEA1n1z0NYVha3umlBH6mZ3i+39fzCx6FNkfpGKjr9FiW/1gIcw0Y9aj/PZC1bfbTaV6+gK+v/kMwDzyCKhkCtn0+AsDqFxDMFY+xzwjBYmVcfYfDfwUCg/VAVwmHN7m9PXVd1xSoaTlisVmy2XEikQmi0SkAUqkZyuUbb0FNyxGPzwOwvLxLqXRHMPhxYrLc17DeFViNAShKD6HQmJsWzsHvWLNxBDrFYrF18vmd1kA3O1PVLd7eLAqF3ebApaVLJifnEEIgSVJDUJZlVHULyypzdaW7Bw1jAcOAROKJ/v5gQ/D6Ok8mM/3rnM79Fl2wC/5vMJmMIsu9DefZ9rMrsAhE6jcqYttOlvZjijWgJPnPhDCPWmr7Q2ybixpwYMA8eXz0+8DcAEY8sh6A80CAvRpQ17HATAAJj7C6afsrfQdYrmo3mMtmpgAAAABJRU5ErkJggg==";
    QByteArray buildingImg3 = "iVBORw0KGgoAAAANSUhEUgAAABwAAAAcCAYAAAByDd+UAAAAAXNSR0IB2cksfwAAAAlwSFlzAAAOxAAADsQBlSsOGwAAAWJJREFUSInt1s8rw3Ecx/Hn2zbfJpo0xTj4dXKh1ZSQUhzkX9CI0xwcVkQNB3+Gi9tycVPUzuS0qyPKCpFQvtp8HGaz2bd9v9/ta0t5nT59en/ej++PT30+bmocd91AFcejRNsR9AUg4FD/OwUH8sa6hHktBkXbEvRNh6Bc2gVWlJcWIFwECvq8w1g+opjNjQv/YcdvgYDfCLQX3zI87dleVhkYTEDvBJz2g38IukPZ+eNxyFw4DAYTMDCVHY9G4eUWmr++mHhNl9sDCzEAlwd8XbZaWAd/YhXGGmgVCyzC1WqVoJ03G4mASsN1tEJw+AT6JkEpEDEHpQFCEci8w81GBWByBpLA3CM0tZqDl2dwPla2pH6nxT/4D/5x8GgQaDSvU8+2wBTQY1j1kbLyWOWSb/B9iULbF/TtajsbRcFhCSht+q560FyCvgR0OmTdK4iLxlopOE0a9BgQcwgzTM136SeMBkz2tFUt2gAAAABJRU5ErkJggg==";
    QByteArray buildingImg4 = "iVBORw0KGgoAAAANSUhEUgAAABwAAAAcCAYAAAByDd+UAAAAAXNSR0IB2cksfwAAAAlwSFlzAAAOxAAADsQBlSsOGwAAAYxJREFUSInt1csrRGEYx/Hve2bGidwml1wil2xsMDUpl5RioWzYKAmxGgsLRaNcs7DyFyjZnWxsLKRmO0kWsrMTZQqRJvJq5hyLSQ0zY+bMHCPlt3oXz/t83lu9drIc+6+BmhcHL+oqipxAp8qi/nco7EnBwvgWz59A41VdFshFdIuoSMrQmcmBAmD8Eyh0OWYpFRUBAx/j6Dus+CkQKI0HmkpR7TRPV9um56UFukZ9NLR149caKa1voabZDcDhZhdheWEt6Br10eTuBaBjZI7gwy35zsiJCVtu0vmmwGgMwGZ3UFxebaZF6uBXLN2kBKaKVbVOcuWfzQw0s7P2YQ+GHuL6eC49sHXkiEZXD4ZhIIRICgpFwT3kIRx64+bUax480/o502Bw/ZG8wuKk4OX5MSc7nd/W/N5v8Q/+g38bPFhrBiUneaEeNAEaBBDUxSsywgEIp7K0hAnEgkLdBbmSUdsEEbAfA5ZUyI37W9UmdDkFVFpk3WOgqU7mY8A+LyGQS8CSRVjcZP2VvgN6imQ8SFFgygAAAABJRU5ErkJggg==";
    QByteArray buildingImg5 = "iVBORw0KGgoAAAANSUhEUgAAABwAAAAcCAYAAAByDd+UAAAAAXNSR0IB2cksfwAAAAlwSFlzAAAOxAAADsQBlSsOGwAAAXpJREFUSInt1b1LQlEYx/HvMfVQFAkZVBJE0uJiBBKkEQg1BP0JYZLTbXC4UBj0MvRntLRJS0tDCK4hTa1tUZBggoQ0nFBuQ2gvXvRevRVBv+kMz3k+5w2Omx+O+9fALHiQ8hClNoAJh/o/AqcKdhLw/Ak0pNwXSu06BDUyCmx5YQhIfAKFUusOY80IWG2MP97h2HeBgN8MtJXhVIqn42Pb87oC5/J5phcXuQwG8YfDTEYiAFzEYtRvbpwF5/J5ZuJxABZ0nWqpxKD/7cREf3/H+bbAjxhAn8eDLxCw08I6+BXrNpZAq9hEMsldOt0baGdn85qGUatxr+vdgbO5HMGlJQzDQAjRERQuFxFNo/7ywkMmYx+8XlnhGlirVBjw+TqCt4UCV9Fo25rf+y3+wX/wb4PnoRB4vZ0Lq1VbYBGYMisyikUr62qXZoN3UMoTlDrotbNZBJy1gCNKHZWl7BNKbQLjDlllICthuwVchhpK7QF7DmGm+fFX+gonY17k9eIf3wAAAABJRU5ErkJggg==";

    buildingImg1 = QByteArray::fromBase64(buildingImg1);
    buildingImg2 = QByteArray::fromBase64(buildingImg2);
    buildingImg3 = QByteArray::fromBase64(buildingImg3);
    buildingImg4 = QByteArray::fromBase64(buildingImg4);
    buildingImg5 = QByteArray::fromBase64(buildingImg5);

    QImage img1 = QImage::fromData(buildingImg1);
    QImage img2 = QImage::fromData(buildingImg2);
    QImage img3 = QImage::fromData(buildingImg3);
    QImage img4 = QImage::fromData(buildingImg4);
    QImage img5 = QImage::fromData(buildingImg5);

    PictureMarkerSymbol* symbol1 = new PictureMarkerSymbol(img1, this);
    PictureMarkerSymbol* symbol2 = new PictureMarkerSymbol(img2, this);
    PictureMarkerSymbol* symbol3 = new PictureMarkerSymbol(img3, this);
    PictureMarkerSymbol* symbol4 = new PictureMarkerSymbol(img4, this);
    PictureMarkerSymbol* symbol5 = new PictureMarkerSymbol(img5, this);

    QList<ClassBreak*> classBreaks;

    auto classBreak1 = new ClassBreak("Very Low Loss Ratio", "Loss Ratio less than 10%", -0.00001, 0.05, symbol1);
    classBreaks.append(classBreak1);

    auto classBreak2 = new ClassBreak("Low Loss Ratio", "Loss Ratio Between 10% and 25%", 0.05, 0.25, symbol2);
    classBreaks.append(classBreak2);

    auto classBreak3 = new ClassBreak("Medium Loss Ratio", "Loss Ratio Between 25% and 50%", 0.25, 0.5,symbol3);
    classBreaks.append(classBreak3);

    auto classBreak4 = new ClassBreak("High Loss Ratio", "Loss Ratio Between 50% and 75%", 0.50, 0.75,symbol4);
    classBreaks.append(classBreak4);

    auto classBreak5 = new ClassBreak("Very Loss Ratio", "Loss Ratio Between 75% and 90%", 0.75, 1.0,symbol5);
    classBreaks.append(classBreak5);

    return new ClassBreaksRenderer("LossRatio", classBreaks);
}


ClassBreaksRenderer* VisualizationWidget::createPipelineRenderer(void)
{
    SimpleLineSymbol* lineSymbol1 = new SimpleLineSymbol(SimpleLineSymbolStyle::Solid, QColor(254, 229, 217), 5.0f /*width*/, this);
    SimpleLineSymbol* lineSymbol2 = new SimpleLineSymbol(SimpleLineSymbolStyle::Solid, QColor(252, 187, 161), 5.0f /*width*/, this);
    SimpleLineSymbol* lineSymbol3 = new SimpleLineSymbol(SimpleLineSymbolStyle::Solid, QColor(252, 146, 114), 5.0f /*width*/, this);
    SimpleLineSymbol* lineSymbol4 = new SimpleLineSymbol(SimpleLineSymbolStyle::Solid, QColor(251, 106, 74),  5.0f /*width*/, this);
    SimpleLineSymbol* lineSymbol5 = new SimpleLineSymbol(SimpleLineSymbolStyle::Solid, QColor(222, 45 , 38),  5.0f /*width*/, this);
    SimpleLineSymbol* lineSymbol6 = new SimpleLineSymbol(SimpleLineSymbolStyle::Solid, QColor(165, 15 , 21),  5.0f /*width*/, this);

    QList<ClassBreak*> classBreaks;

    auto classBreak1 = new ClassBreak("Very Low Loss Ratio", "Loss Ratio less than 10%", 0.0, 1E-03, lineSymbol1);
    classBreaks.append(classBreak1);

    auto classBreak2 = new ClassBreak("Low Loss Ratio", "Loss Ratio Between 10% and 25%", 1.00E-03, 1.00E-02, lineSymbol2);
    classBreaks.append(classBreak2);

    auto classBreak3 = new ClassBreak("Medium Loss Ratio", "Loss Ratio Between 25% and 50%", 1.00E-02, 1.00E-01, lineSymbol3);
    classBreaks.append(classBreak3);

    auto classBreak4 = new ClassBreak("High Loss Ratio", "Loss Ratio Between 50% and 75%", 1.00E-01, 1.00E+00, lineSymbol4);
    classBreaks.append(classBreak4);

    auto classBreak5 = new ClassBreak("Very High Loss Ratio", "Loss Ratio Between 75% and 90%", 1.00E+00, 1.00E+01, lineSymbol5);
    classBreaks.append(classBreak5);

    auto classBreak6 = new ClassBreak("Total Loss Ratio", "Loss Ratio Between 75% and 90%", 1.00E+01, 1.00E+10, lineSymbol6);
    classBreaks.append(classBreak6);

    return new ClassBreaksRenderer("LossRatio", classBreaks);
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

    QList<std::pair<GeoElement*,QString>> elemList;

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
        qDebug() <<"Error, task not valid in "<<__PRETTY_FUNCTION__;
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
        qDebug() <<"Error, task not valid in "<<__PRETTY_FUNCTION__;
    else
        taskIDMap[taskWatcher.taskId()] = taskWatcher.description();
}


void VisualizationWidget::handleAsynchronousSelectionTask(void)
{
    // Only handle the selected features when all tasks are complete
    if(taskIDMap.empty())
    {
        this->handleSelectedFeatures();
    }

    //    QMap<QUuid, QString>::const_iterator i = m_taskIds.constBegin();
    //    while (i != m_taskIds.constEnd())
    //    {
    //        auto isDone = i.value();
    //        qDebug()<< i.key() << ": "<<isDone << Qt::endl;
    //        ++i;
    //    }

}


void VisualizationWidget::runFieldQuery(FeatureTable* table, const QString& fieldName, const QString& searchText)
{
    // create a query parameter object and set the where clause
    QueryParameters queryParams;

    // Follows the format of an SQL 'where' clause
    const auto whereClause = fieldName + QString(" LIKE '" + searchText + "%'");

    queryParams.setWhereClause(whereClause);
    table->queryFeatures(queryParams);
}


Esri::ArcGISRuntime::Map *VisualizationWidget::getMapGIS() const
{
    return mapGIS;
}


void VisualizationWidget::zoomToExtents(void)
{
    LayerListModel* layers = mapGIS->operationalLayers();

    if(layers->size() == 0)
        return;

    layers->at(0)->load();

    Envelope bbox;

    std::function<void(const LayerListModel*)> getExtentsNestedLayers = [&](const LayerListModel* layers)
    {
        for(int i = 0; i<layers->size(); ++i)
        {
            auto layer = layers->at(i);

            // Continue if the layer is turned off
            if(!layer->isVisible())
                continue;

            if(auto featureCollectLayer = dynamic_cast<FeatureCollectionLayer*>(layer))
            {
                featureCollectLayer->load();

                auto layerExtent = featureCollectLayer->fullExtent();

                if(layerExtent.isValid())
                {
                    //                    auto width = layerExtent.width();
                    //                    auto depth = layerExtent.depth();

                    if(bbox.isValid())
                        bbox = GeometryEngine::unionOf(bbox, layerExtent);
                    else
                        bbox = layerExtent;
                }

            }
            else if(auto isGroupLayer = dynamic_cast<GroupLayer*>(layer))
            {
                auto subLayers = isGroupLayer->layers();
                getExtentsNestedLayers(subLayers);
            }
        }
    };

    getExtentsNestedLayers(layers);

    if(bbox.isValid())
        mapViewWidget->setViewpointGeometry(bbox, 40);  //  mapViewWidget->setViewpointCenter(bbox->fullExtent().center(), 80000);

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


RasterLayer* VisualizationWidget::createAndAddRasterLayer(const QString& filePath, const QString& layerName, TreeItem* parentItem)
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


FeatureLayer* VisualizationWidget::createAndAddShapefileLayer(const QString& filePath, const QString& layerName, TreeItem* parentItem)
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


ArcGISMapImageLayer* VisualizationWidget::createAndAddMapServerLayer(const QString& url, const QString& layerName, TreeItem* parentItem)
{
    ArcGISMapImageLayer* layer  = new ArcGISMapImageLayer(QUrl(url), this);

    // Add the layers to the layer tree
    auto layerID = this->createUniqueID();
    auto layerTreeItem = layersTree->addItemToTree(layerName, layerID, parentItem);

    connect(layer, &ArcGISMapImageLayer::doneLoading, this, [this, layer, layerTreeItem](Error loadError)
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

            layersTree->addItemToTree(subLayerName, QString(), layerTreeItem);
        }


        // If the layer was loaded successfully, set the map extent to the full extent of the layer
        mapViewWidget->setViewpointCenter(layer->fullExtent().center(), 80000);
    });

    layer->setName(layerName);
    layer->setLayerId(layerID);


    return layer;
}


void VisualizationWidget::createAndAddGeoDatabaseLayer(const QString& filePath, const QString& layerName, TreeItem* parentItem)
{
    auto m_geodatabase = new Geodatabase(filePath, this);

    connect(m_geodatabase, &Geodatabase::errorOccurred, this, [this](Error error)
    {
        auto msg = error.message() + error.additionalMessage();
        this->userMessageDialog(msg);
        return;
    });

    auto layerID = this->createUniqueID();

    connect(m_geodatabase, &Geodatabase::doneLoading, this, [=](Error error)
    {
        if (error.isEmpty())
        {
            GeodatabaseFeatureTable* featureTable = m_geodatabase->geodatabaseFeatureTable("layerName");

            // create a feature layer from the feature table
            FeatureLayer* featureLayer = new FeatureLayer(featureTable, this);

            featureLayer->setName(layerName);
            featureLayer->setLayerId(layerID);

            // add the feature layer to the map
            mapGIS->operationalLayers()->append(featureLayer);
        }
    });

    // load the geodatabase
    m_geodatabase->load();


    // Add the layers to the layer tree
    layersTree->addItemToTree(layerName, layerID, parentItem);
}


KmlLayer*  VisualizationWidget::createAndAddKMLLayer(const QString& filePath, const QString& layerName, TreeItem* parentItem, double opacity)
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
FeatureCollectionLayer* VisualizationWidget::createAndAddXMLShakeMapLayer(const QString& filePath, const QString& layerName, TreeItem* parentItem)
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


void VisualizationWidget::addLayerToMap(Esri::ArcGISRuntime::Layer* layer, TreeItem* parent)
{
    mapGIS->operationalLayers()->append(layer);



    //        connect(layer, &Layer::doneLoading, this, [this, layer, layerTreeItem](Error loadError)
    //        {
    //            if (!loadError.isEmpty())
    //            {
    //                auto msg = loadError.message() + loadError.additionalMessage();
    //                this->userMessageDialog(msg);

    //                return;
    //            }

    //            auto subLayers = layer->subLayerContents();

    //            for(auto&& it : subLayers)
    //            {
    //                auto subLayerName = it->name();

    //                layersModel->addItemToTree(subLayerName,layerTreeItem);
    //            }


    //            // If the layer was loaded successfully, set the map extent to the full extent of the layer
    //            mapViewWidget->setViewpointCenter(layer->fullExtent().center(), 80000);
    //        });



}


void VisualizationWidget::removeLayerFromMap(Esri::ArcGISRuntime::Layer* layer)
{
    mapGIS->operationalLayers()->removeOne(layer);
    layersTree->removeItemFromTree(layer->name());
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


//     connect to the mouse clicked signal on the MapQuickView
//     This code snippet adds a point to where the mouse click is
//            connect(mapViewWidget, &MapGraphicsView::mouseClicked, this, [this](QMouseEvent& mouseEvent)
//            {
//              // obtain the map point
//              const double screenX = mouseEvent.x();
//              const double screenY = mouseEvent.y();
//              Point newPoint = mapViewWidget->screenToLocation(screenX, screenY);

//              // create the feature attributes
//              QMap<QString, QVariant> featureAttributes;
//              featureAttributes.insert("typdamage", "Minor");
//              featureAttributes.insert("primcause", "Earthquake");

//              // create a new feature and add it to the feature table
//              Feature* feature = featureCollectionTable->createFeature(featureAttributes, newPoint, this);
//              featureCollectionTable->addFeature(feature);
//            });

// This snippet will do a query on a field and return all features that match the search string
//    QString fieldName = "Occupancy";
//    QString searchString = "Residential Single Family";
//    this->runFieldQuery(featureCollectionTable,fieldName,searchString);
//    return;
