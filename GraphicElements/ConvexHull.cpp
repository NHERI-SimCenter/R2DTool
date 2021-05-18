#include "ConvexHull.h"
#include "SimCenterMapGraphicsView.h"

#include "GeometryEngine.h"
#include "MultipointBuilder.h"
#include "MapGraphicsView.h"
#include "Geometry.h"
#include "GroupLayer.h"
#include "Graphic.h"
#include "Map.h"
#include "QueryParameters.h"
#include "FeatureCollectionLayer.h"
#include "VisualizationWidget.h"
#include "SimpleMarkerSymbol.h"
#include "SimpleFillSymbol.h"

using namespace Esri::ArcGISRuntime;

ConvexHull::ConvexHull(VisualizationWidget* visualizationWidget) : QObject(visualizationWidget), theVisualizationWidget(visualizationWidget)
{
    selectingConvexHull = false;

    mapGIS = theVisualizationWidget->getMapGIS();
    mapViewWidget = theVisualizationWidget->getMapViewWidget();

    setupConvexHullObjects();
}


void ConvexHull::getItemsInConvexHull()
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

    QMap<QUuid, QString>& taskIDMap = theVisualizationWidget->getTaskIDMap();

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
                    connect(table, &FeatureTable::queryFeaturesCompleted, theVisualizationWidget, &VisualizationWidget::selectFeaturesForAnalysisQueryCompleted, Qt::UniqueConnection);

                    // Query the table for features - note that this is done asynchronously
                    auto taskWatcher = table->queryFeatures(queryParams);

                    if (!taskWatcher.isValid())
                        qDebug() <<"Error, task not valid in "<<__FUNCTION__;
                    else
                        taskIDMap[taskWatcher.taskId()] = taskWatcher.description();
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


void ConvexHull::convexHullPointSelector(QMouseEvent& e)
{
    e.accept();

    auto mapViewWidget = theVisualizationWidget->getMapViewWidget();

    const Point clickedPoint = mapViewWidget->screenToLocation(e.x(), e.y());

    m_multipointBuilder->points()->addPoint(clickedPoint);
    m_inputsGraphic->setGeometry(m_multipointBuilder->toGeometry());

    this->plotConvexHull();
}


bool ConvexHull::getSelectingPoints() const
{
    return selectingConvexHull;
}


void ConvexHull::setSelectingPoints(bool value)
{
    selectingConvexHull = value;
}


// Convex hull stuff
void ConvexHull::plotConvexHull()
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


void ConvexHull::resetConvexHull()
{
    selectingConvexHull = false;

    if (m_multipointBuilder)
        m_multipointBuilder->points()->removeAll();
    if (m_inputsGraphic)
        m_inputsGraphic->setGeometry(Geometry());
    if (m_convexHullGraphic)
        m_convexHullGraphic->setGeometry(Geometry());

    auto mapViewWidget = theVisualizationWidget->getMapViewWidget();

    // Turn off point selection for the convex hull
    disconnect(mapViewWidget, &MapGraphicsView::mouseClicked, this, &ConvexHull::convexHullPointSelector);
}


void ConvexHull::setupConvexHullObjects()
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


void ConvexHull::getConvexHullInputs()
{
    selectingConvexHull = true;

    auto mapViewWidget = theVisualizationWidget->getMapViewWidget();

    // show clicked points on MapView
    connect(mapViewWidget, &MapGraphicsView::mouseClicked, this, &ConvexHull::convexHullPointSelector, Qt::UniqueConnection);
}


