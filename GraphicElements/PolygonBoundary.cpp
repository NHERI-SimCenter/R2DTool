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

#include "PolygonBoundary.h"
#include "SimCenterMapGraphicsView.h"
#include "ArcGISVisualizationWidget.h"

// GIS headers
#include "GeometryEngine.h"
#include "MultipointBuilder.h"
#include "MapGraphicsView.h"
#include "Geometry.h"
#include "GroupLayer.h"
#include "PolygonBuilder.h"
#include "Graphic.h"
#include "Map.h"
#include "Point.h"
#include "QueryParameters.h"
#include "FeatureCollectionLayer.h"
#include "SimpleMarkerSymbol.h"
#include "SimpleFillSymbol.h"

using namespace Esri::ArcGISRuntime;

PolygonBoundary::PolygonBoundary(ArcGISVisualizationWidget* visualizationWidget) : QObject(visualizationWidget), theVisualizationWidget(visualizationWidget)
{
    selectingPolygonBoundary = false;

    mapGIS = theVisualizationWidget->getMapGIS();
    mapViewWidget = theVisualizationWidget->getMapViewWidget();

    setupPolygonBoundaryObjects();
}


void PolygonBoundary::getItemsInPolygonBoundary()
{

    // For debug purposes; creates  a shape
    //    QString inputGeomStr = "{\"points\":[[-16687850.289930943,8675247.6610443853],[-16687885.484665291,8675189.0031538066],[-16687874.819594277,8675053.5567519218],[-16687798.031082973,8675037.5591454003],[-16687641.254539061,8675065.2883300371],[-16687665.784202393,8675228.4639165588]],\"spatialReference\":{\"wkid\":102100,\"latestWkid\":3857}}";
    //    auto impGeom = Geometry::fromJson(inputGeomStr);
    //    m_inputsGraphic->setGeometry(impGeom);
    //    this->plotPolygonBoundary();
    //    return;
    // End debug

    // Check that the input geometry is not empty
    if(m_inputsGraphic->geometry().isEmpty())
    {
        // Clear the graphics
        resetPolygonBoundary();

        return;
    }

    // Get the layers from the map, and ensure that they are not empty
    auto layersList = mapGIS->operationalLayers();

    if(layersList->isEmpty())
    {
        // Clear the graphics
        resetPolygonBoundary();

        return;
    }

    // Get the points from the point collection
    PointCollection*  normalizedPoints =  m_multipointBuilder->points();

    PolygonBuilder polygonBuilder(mapGIS->spatialReference());

    for(auto&& it : *normalizedPoints)
        polygonBuilder.addPoint(it);

    const Geometry polygonGeom = polygonBuilder.toGeometry();

    if(polygonGeom.isEmpty())
        return;

    // Set the envelope of the convex hull as the search parameter
    QueryParameters queryParams;
    auto envelope = Polygon(polygonGeom);
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
                    connect(table, &FeatureTable::queryFeaturesCompleted, theVisualizationWidget, &ArcGISVisualizationWidget::selectFeaturesForAnalysisQueryCompleted, Qt::UniqueConnection);

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
    resetPolygonBoundary();

    return;
}


void PolygonBoundary::PolygonBoundaryPointSelector(QMouseEvent& e)
{
    e.accept();

    auto mapViewWidget = theVisualizationWidget->getMapViewWidget();

    const Point clickedPoint = mapViewWidget->screenToLocation(e.x(), e.y());

    m_multipointBuilder->points()->addPoint(clickedPoint);
    m_inputsGraphic->setGeometry(m_multipointBuilder->toGeometry());

    this->plotPolygonBoundary();
}


bool PolygonBoundary::getSelectingPoints() const
{
    return selectingPolygonBoundary;
}

void PolygonBoundary::setSelectingPoints(bool value)
{
    selectingPolygonBoundary = value;
}


// Convex hull stuff
void PolygonBoundary::plotPolygonBoundary()
{
    if (m_inputsGraphic->geometry().isEmpty())
        return;

    // Get the points from the point collection
    PointCollection*  normalizedPoints =  m_multipointBuilder->points();

    PolygonBuilder polygonBuilder(mapGIS->spatialReference());

    for(auto&& it : *normalizedPoints)
        polygonBuilder.addPoint(it);

    const Geometry polygonGeom = polygonBuilder.toGeometry();

    if(polygonGeom.isEmpty())
        return;

    // change the symbol based on the returned geometry type
    if (normalizedPoints->size() < 3)
    {
        m_PolygonBoundaryGraphic->setSymbol(m_lineSymbol);
    }
    else
    {
        m_PolygonBoundaryGraphic->setSymbol(m_fillSymbol);
    }

    m_PolygonBoundaryGraphic->setGeometry(polygonGeom);
}


void PolygonBoundary::resetPolygonBoundary()
{
    selectingPolygonBoundary = false;

    if (m_multipointBuilder)
        m_multipointBuilder->points()->removeAll();
    if (m_inputsGraphic)
        m_inputsGraphic->setGeometry(Geometry());
    if (m_PolygonBoundaryGraphic)
        m_PolygonBoundaryGraphic->setGeometry(Geometry());

    auto mapViewWidget = theVisualizationWidget->getMapViewWidget();

    // Turn off point selection for the convex hull
    disconnect(mapViewWidget, &MapGraphicsView::mouseClicked, this, &PolygonBoundary::PolygonBoundaryPointSelector);    

    emit selectionEnd();
}


void PolygonBoundary::setupPolygonBoundaryObjects()
{
    // graphics overlay to show clicked points and convex hull
    m_graphicsOverlay = new GraphicsOverlay(this);

    // create a graphic to show clicked points
    m_markerSymbol = new SimpleMarkerSymbol(SimpleMarkerSymbolStyle::Circle, Qt::red, 10, this);
    m_inputsGraphic = new Graphic(this);
    m_inputsGraphic->setSymbol(m_markerSymbol);
    m_graphicsOverlay->graphics()->append(m_inputsGraphic);

    // create a graphic to display the convex hull
    m_PolygonBoundaryGraphic = new Graphic(this);
    m_graphicsOverlay->graphics()->append(m_PolygonBoundaryGraphic);

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


void PolygonBoundary::getPolygonBoundaryInputs()
{
    emit selectionStart();

    selectingPolygonBoundary = true;

    auto mapViewWidget = theVisualizationWidget->getMapViewWidget();

    // show clicked points on MapView
    connect(mapViewWidget, &MapGraphicsView::mouseClicked, this, &PolygonBoundary::PolygonBoundaryPointSelector, Qt::UniqueConnection);
}


