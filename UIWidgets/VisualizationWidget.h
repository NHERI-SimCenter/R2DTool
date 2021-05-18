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
#include "GISLegendView.h"

#include "Error.h"

#include <QMap>
#include <QObject>
#include <QUuid>

namespace Esri
{
namespace ArcGISRuntime
{
class Map;
class Error;
class MapGraphicsView;
class Feature;
class FeatureTable;
class FeatureLayer;
class GraphicsOverlay;
class FeatureCollectionLayer;
class FeatureCollectionTable;
class FeatureCollection;
class IdentifyLayerResult;
class FeatureQueryResult;
class ClassBreaksRenderer;
class SimpleRenderer;
class GroupLayer;
class KmlLayer;
class Layer;
class Point;
enum class LoadStatus;
class ArcGISMapImageLayer;
class RasterLayer;
}
}

class ConvexHull;
class PolygonBoundary;
class ComponentInputWidget;
class LayerTreeView;
class LayerTreeItem;
class SimCenterMapGraphicsView;

class TreeModel;
class QGroupBox;
class QComboBox;
class QTreeView;
class QSplitter;
class QVBoxLayout;

class VisualizationWidget : public  SimCenterAppWidget
{
    Q_OBJECT

public:
    explicit VisualizationWidget(QWidget* parent);
    virtual ~VisualizationWidget();

    SimCenterMapGraphicsView* getMapViewWidget() const;

    // Note: the component type must match the "AssetType" value set to the features
    void registerComponentWidget(const QString assetType, ComponentInputWidget* widget);

    ComponentInputWidget* getComponentWidget(const QString type);

    // Add component to 'selected layer'
    LayerTreeItem* addSelectedFeatureLayerToMap(Esri::ArcGISRuntime::Layer* featLayer);

    Esri::ArcGISRuntime::FeatureCollectionLayer* createAndAddJsonLayer(const QString& filePath, const QString& layerName, LayerTreeItem* parentItem, QColor color = QColor(0,0,0,255));

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

    Esri::ArcGISRuntime::Layer* getLayer(const QString& layerID);

    // Get the visualization widget
    QWidget *getVisWidget();

    // Get the latitude or longitude from a point on the screen
    double getLatFromScreenPoint(const QPointF& point);
    double getLongFromScreenPoint(const QPointF& point);

    QPointF getScreenPointFromLatLong(const double& latitude, const double& longitude);

    Esri::ArcGISRuntime::Map *getMapGIS(void) const;

    LayerTreeItem* addLayerToMap(Esri::ArcGISRuntime::Layer* layer, LayerTreeItem* parent = nullptr, Esri::ArcGISRuntime::GroupLayer* groupLayer = nullptr);

    // Removes a given layer from the map
    bool removeLayerFromMap(Esri::ArcGISRuntime::Layer* layer);
    void removeLayerFromMap(const QString layerID);
    bool removeLayerFromMapAndTree(const QString layerID);

    LayerTreeView *getLayersTree() const;

    QString createUniqueID(void);

    void takeScreenShot(void);

    void clear(void);

    // Updates the value of an attribute for a selected component
    void updateSelectedComponent(const QString& assetType, const QString& uid, const QString& attribute, const QVariant& value);

    void setLegendView(GISLegendView* legndView);
    GISLegendView *getLegendView() const;

    // Hides the map legend
    void hideLegend(void);

    // Clear the currently selected features (i.e., features highlighted by clicking on them)
    void clearSelection(void);

    // Get the list of features that are currently selected (i.e., features highlighted by clicking on them)
    QList<Esri::ArcGISRuntime::Feature *> getSelectedFeaturesList() const;

    // Returns a rectangular geometry item of dimensions x and y around a center point
    Esri::ArcGISRuntime::Geometry getRectGeometryFromPoint(const Esri::ArcGISRuntime::Point& pnt, const double sizeX, double sizeY = 0);

    // Returns a geometry from the geojson format
    Esri::ArcGISRuntime::Geometry getPolygonGeometryFromJson(const QString& geoJson);
    Esri::ArcGISRuntime::Geometry getPolygonGeometryFromJson(const QJsonArray& geoJson);

    Esri::ArcGISRuntime::Geometry getMultilineStringGeometryFromJson(const QString& geoJson);
    Esri::ArcGISRuntime::Geometry getMultilineStringGeometryFromJson(const QJsonArray& geoJson);
    Esri::ArcGISRuntime::Geometry getMultiPolygonGeometryFromJson(const QJsonArray& geoJson);

    Esri::ArcGISRuntime::FeatureCollectionTable* getMultilineFeatureTable(const QJsonObject& geomObject, const QJsonObject& featObj, const QString& layerName, const QColor color);
    Esri::ArcGISRuntime::FeatureCollectionTable* getMultipolygonFeatureTable(const QJsonObject& geomObject, const QJsonObject& featObj, const QString& layerName, const QColor color);

    // Programatically set the visibility of a layer
    void setLayerVisibility(const QString& layerID, const bool val);

    // Returns a map containing the IDs of all asynchronous tasks
    QMap<QUuid, QString>& getTaskIDMap(void);

    // Returns the tool to select a polygon boundary
    PolygonBoundary* getThePolygonBoundaryTool(void) const;

    // Get the list of features saved from the latest query
    QList<Esri::ArcGISRuntime::FeatureQueryResult *> getFeaturesFromQueryList() const;

    // Sets the view elevation above the ground level
    void setViewElevation(double val);

signals:
    // Emit a screen shot of the current GIS view
    void emitScreenshot(QImage img);
    void taskSelectionComplete();

public slots:    
    void zoomToLayer(const QString layerID);
//    void loadPipelineData(void);
    void changeLayerOrder(const int from, const int to);
    void handleLayerChecked(LayerTreeItem* item);
    void handleOpacityChange(const QString& layerID, const double opacity);

    void exportImageComplete(QUuid id, QImage img);
    void onMouseClicked(QMouseEvent& mouseEvent);
    void onMouseClickedGlobal(QPoint pos);
    void setCurrentlyViewable(bool status);
    void handleLegendChange(const QString layerUID);
    void handleLegendChange(const Esri::ArcGISRuntime::Layer* layer);
    void selectFeaturesForAnalysisQueryCompleted(QUuid taskID, Esri::ArcGISRuntime::FeatureQueryResult* rawResult);

private slots:
    void identifyLayersCompleted(QUuid taskID, const QList<Esri::ArcGISRuntime::IdentifyLayerResult*>& results);
    void fieldQueryCompleted(QUuid taskID, Esri::ArcGISRuntime::FeatureQueryResult* rawResult);

    void handleSelectFeatures(void);
    void handleAsyncLayerLoad(Esri::ArcGISRuntime::Error layerLoadStatus);
    void handleBasemapSelection(const QString selection);
    void handleFieldQuerySelection(void);
    void handleArcGISError(Esri::ArcGISRuntime::Error error);
    void setLegendInfo();

    // Zooms the map to the extents of the data present in the visible map
    void zoomToExtents(void);

    // Asset selection
    void handleSelectAreaMap(void);
    void handleClearSelectAreaMap(void);
    void handleSelectFeaturesForAnalysis(void);

private:

    LayerTreeView* layersTree;

    // Map to hold the component input widgets (key = type of component or asset, e.g., BUILDINGS)
    QMap<QString, ComponentInputWidget*> componentWidgetsMap;

    QComboBox* baseMapCombo;

    // This function runs a query on all features in a table
    // It returns the all of the features in the table where the text in the field "FieldName" matches the search text
    void runFieldQuery(const QString& fieldName, const QString& searchText);

    Esri::ArcGISRuntime::Map* mapGIS = nullptr;
    SimCenterMapGraphicsView *mapViewWidget = nullptr;
    QVBoxLayout *mapViewLayout;

    QList<Esri::ArcGISRuntime::FeatureQueryResult*>  featuresFromQueryList;
    QList<Esri::ArcGISRuntime::FeatureQueryResult*>  fieldQueryFeaturesList;

    QMap<QUuid,QString> taskIDMap;
    QMap<QUuid,QString> layerLoadMap;

    // Map to store the layers
    QMap<QString, Esri::ArcGISRuntime::Layer*> layersMap;

    Esri::ArcGISRuntime::ClassBreaksRenderer* createPointRenderer(void);

    Esri::ArcGISRuntime::GroupLayer* selectedObjectsLayer = nullptr;
    LayerTreeItem* selectedObjectsTreeItem = nullptr;

    // The GIS widget
    QSplitter* visWidget;
    void createVisualizationWidget(void);

    // The legend view
    GISLegendView* legendView;

    // Map to store the legend of a layer according to the layers UID
    QMap<QString, RoleProxyModel*> legendModels;

    Esri::ArcGISRuntime::GraphicsOverlay* selectedFeaturesOverlay = nullptr;
    QList<Esri::ArcGISRuntime::Feature*> selectedFeaturesList;

    std::unique_ptr<ConvexHull> theConvexHullTool;
    std::unique_ptr<PolygonBoundary> thePolygonBoundaryTool;
};

#endif // VISUALIZATIONWIDGET_H
