#ifndef VISUALIZATIONWIDGET_H
#define VISUALIZATIONWIDGET_H
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

#include "SimCenterAppWidget.h"
#include "BuildingDatabase.h"

#include <QMap>
#include <QObject>
#include <QUuid>

namespace Esri
{
namespace ArcGISRuntime
{
class Map;
class MapGraphicsView;
class Feature;
class FeatureTable;
class FeatureLayer;
class FeatureCollectionLayer;
class FeatureCollectionTable;
class IdentifyLayerResult;
class FeatureQueryResult;
class ClassBreaksRenderer;
class GroupLayer;
class KmlLayer;
class Layer;
class ArcGISMapImageLayer;
class RasterLayer;

//Convex hull stuff
class GraphicsOverlay;
class SimpleMarkerSymbol;
class Graphic;
class SimpleLineSymbol;
class SimpleFillSymbol;
class MultipointBuilder;
class Geometry;
}
}

class ComponentInputWidget;
class LayerTreeView;
class LayerTreeItem;
class TreeModel;
class QGroupBox;
class QComboBox;
class QTreeView;

class SimCenterMapGraphicsView;
class QVBoxLayout;

class VisualizationWidget : public  SimCenterAppWidget
{
    Q_OBJECT

public:
    explicit VisualizationWidget(QWidget* parent);
    virtual ~VisualizationWidget();

    // Convex hull functionality
    void setupConvexHullObjects();

    Esri::ArcGISRuntime::MapGraphicsView* getMapViewWidget() const;

    // Set the building widget to the visualization engine
    void setBuildingWidget(ComponentInputWidget *value);

    // Set the pipeline widget to the visualization engine
    void setPipelineWidget(ComponentInputWidget *value);

    // Zooms the map to the extents of the data present in the visible map
    void zoomToExtents(void);

    // Adds a raster layer to the map
    Esri::ArcGISRuntime::RasterLayer* createAndAddRasterLayer(const QString& filePath, const QString& layerName, LayerTreeItem* parentItem);

    // Adds a shapefile layer to the map
    Esri::ArcGISRuntime::FeatureLayer* createAndAddShapefileLayer(const QString& filePath, const QString& layerName, LayerTreeItem* parentItem = nullptr);

    // Add a KML (google earth)
    Esri::ArcGISRuntime::KmlLayer* createAndAddKMLLayer(const QString& filePath, const QString& layerName, LayerTreeItem* parentItem, double opacity = 1.0);

    // From a geodatabase
    void createAndAddGeoDatabaseLayer(const QString& filePath, const QString& layerName, LayerTreeItem* parentItem = nullptr);

    // Add a shakemap grid given as an XML file
    Esri::ArcGISRuntime::FeatureCollectionLayer* createAndAddXMLShakeMapLayer(const QString& filePath, const QString& layerName, LayerTreeItem* parentItem);

    // Create a layer from a map server URL
    Esri::ArcGISRuntime::ArcGISMapImageLayer* createAndAddMapServerLayer(const QString& url, const QString& layerName, LayerTreeItem* parentItem);

    Esri::ArcGISRuntime::Layer* findLayer(const QString& layerID);

    // Get the visualization widget
    QWidget *getVisWidget();

    // Get the latitude or longitude from a point on the screen
    double getLatFromScreenPoint(const QPointF& point);
    double getLongFromScreenPoint(const QPointF& point);

    QPointF getScreenPointFromLatLong(const double& latitude, const double& longitude);

    Esri::ArcGISRuntime::Map *getMapGIS() const;

    void addLayerToMap(Esri::ArcGISRuntime::Layer* layer, LayerTreeItem* parent = nullptr);
    void removeLayerFromMap(Esri::ArcGISRuntime::Layer* layer);

    LayerTreeView *getLayersTree() const;

    QString createUniqueID(void);

    void takeScreenShot(void);

    BuildingDatabase* getBuildingDatabase();

    void clear(void);

signals:
    // Convex hull
    void taskSelectionChanged();
    void emitScreenshot(QImage img);

public slots:
    // Convex hull
    void plotConvexHull();
    void getConvexHullInputs();
    void resetConvexHull();
    void loadBuildingData(void);
    void loadPipelineData(void);
    void changeLayerOrder(const int from, const int to);
    void handleLayerSelection(LayerTreeItem* item);
    void handleOpacityChange(const QString& layerID, const double opacity);
    void exportImageComplete(QUuid id, QImage img);
    void onMouseClicked(QMouseEvent& mouseEvent);

    void onMouseClickedGlobal(QPoint pos);


private slots:
    void identifyLayersCompleted(QUuid taskID, const QList<Esri::ArcGISRuntime::IdentifyLayerResult*>& results);
    void featureSelectionQueryCompleted(QUuid taskID, Esri::ArcGISRuntime::FeatureQueryResult* rawResult);
    void handleSelectedFeatures(void);
    void handleAsynchronousSelectionTask(void);
    void handleBasemapSelection(const QString selection);

    // Convex hull stuff
    void getItemsInConvexHull();
    void convexHullPointSelector(QMouseEvent& e);

    void setCurrentlyViewable(bool status);

private:

    LayerTreeView* layersTree;
    ComponentInputWidget* buildingWidget;
    ComponentInputWidget* pipelineWidget;

    QComboBox* baseMapCombo;

    // This function runs a query on all features in a table
    // It returns the all of the features in the table where the text in the field "FieldName" matches the search text
    void runFieldQuery(Esri::ArcGISRuntime::FeatureTable* table, const QString& fieldName, const QString& searchText);

    Esri::ArcGISRuntime::Map* mapGIS = nullptr;
    //FMK Esri::ArcGISRuntime::MapGraphicsView* mapViewWidget = nullptr;
    SimCenterMapGraphicsView *mapViewWidget = nullptr;
    QVBoxLayout *mapViewLayout;

    QList<Esri::ArcGISRuntime::FeatureQueryResult*>  selectedFeaturesList;

    QMap<QUuid,QString> taskIDMap;

    Esri::ArcGISRuntime::ClassBreaksRenderer* createBuildingRenderer(void);
    Esri::ArcGISRuntime::ClassBreaksRenderer* createPipelineRenderer(void);

    // Create a graphic to display the convex hull selection
    Esri::ArcGISRuntime::GraphicsOverlay* m_graphicsOverlay = nullptr;
    Esri::ArcGISRuntime::SimpleMarkerSymbol* m_markerSymbol = nullptr;
    Esri::ArcGISRuntime::Graphic* m_inputsGraphic = nullptr;
    Esri::ArcGISRuntime::Graphic* m_convexHullGraphic = nullptr;
    Esri::ArcGISRuntime::SimpleLineSymbol* m_lineSymbol = nullptr;
    Esri::ArcGISRuntime::SimpleFillSymbol* m_fillSymbol = nullptr;
    Esri::ArcGISRuntime::MultipointBuilder* m_multipointBuilder = nullptr;
    bool selectingConvexHull;

    // Returns a vector of sorted items that are unique
    template <typename T>
    void uniqueVec(std::vector<T>& vec);

    // The GIS widget
    QWidget* visWidget;
    void createVisualizationWidget(void);

    BuildingDatabase theBuildingDb;

};

#endif // VISUALIZATIONWIDGET_H
