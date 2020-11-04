#ifndef VISUALIZATIONWIDGET_H
#define VISUALIZATIONWIDGET_H

#include "SimCenterAppWidget.h"

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
class TreeView;
class TreeItem;
class TreeModel;
class QGroupBox;
class QComboBox;
class QTreeView;

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
    Esri::ArcGISRuntime::RasterLayer* createAndAddRasterLayer(const QString& filePath, const QString& layerName, TreeItem* parentItem);

    // Adds a shapefile layer to the map
    Esri::ArcGISRuntime::FeatureLayer* createAndAddShapefileLayer(const QString& filePath, const QString& layerName, TreeItem* parentItem = nullptr);

    // Add a KML (google earth)
    Esri::ArcGISRuntime::KmlLayer* createAndAddKMLLayer(const QString& filePath, const QString& layerName, TreeItem* parentItem, double opacity = 1.0);

    // From a geodatabase
    void createAndAddGeoDatabaseLayer(const QString& filePath, const QString& layerName, TreeItem* parentItem = nullptr);

    // Add a shakemap grid given as an XML file
    Esri::ArcGISRuntime::FeatureCollectionLayer* createAndAddXMLShakeMapLayer(const QString& filePath, const QString& layerName, TreeItem* parentItem);

    // Create a layer from a map server URL
    Esri::ArcGISRuntime::ArcGISMapImageLayer* createAndAddMapServerLayer(const QString& url, const QString& layerName, TreeItem* parentItem);

    Esri::ArcGISRuntime::Layer* findLayer(const QString& name);

    // Get the visualization widget
    QWidget *getVisWidget();

    // Get the latitude or longitude from a point on the screen
    double getLatFromScreenPoint(const QPointF& point);
    double getLongFromScreenPoint(const QPointF& point);

    QPointF getScreenPointFromLatLong(const double& latitude, const double& longitude);

    Esri::ArcGISRuntime::Map *getMapGIS() const;

    void addLayerToMap(Esri::ArcGISRuntime::Layer* layer, TreeItem* parent = nullptr);
    void removeLayerFromMap(Esri::ArcGISRuntime::Layer* layer);

    TreeView *getLayersTree() const;

signals:
    // Convex hull
    void taskSelectionChanged();

public slots:
    // Convex hull
    void plotConvexHull();
    void getConvexHullInputs();
    void resetConvexHull();
    void loadBuildingData(void);
    void loadPipelineData(void);
    void changeLayerOrder(const int from, const int to);
    void handleLayerSelection(TreeItem* item);
    void handleOpacityChange(const QString& layerName, const double opacity);

private slots:
    void identifyLayersCompleted(QUuid taskID, const QList<Esri::ArcGISRuntime::IdentifyLayerResult*>& results);
    void featureSelectionQueryCompleted(QUuid taskID, Esri::ArcGISRuntime::FeatureQueryResult* rawResult);
    void onMouseClicked(QMouseEvent& mouseEvent);
    void handleSelectedFeatures(void);
    void handleAsynchronousSelectionTask(void);
    void handleBasemapSelection(const QString selection);

    // Convex hull stuff
    void getItemsInConvexHull();
    void convexHullPointSelector(QMouseEvent& e);

private:

    TreeView* layersTree;
    ComponentInputWidget* buildingWidget;
    ComponentInputWidget* pipelineWidget;

    QComboBox* baseMapCombo;

    // This function runs a query on all features in a table
    // It returns the all of the features in the table where the text in the field "FieldName" matches the search text
    void runFieldQuery(Esri::ArcGISRuntime::FeatureTable* table, const QString& fieldName, const QString& searchText);

    Esri::ArcGISRuntime::Map*             mapGIS = nullptr;
    Esri::ArcGISRuntime::MapGraphicsView* mapViewWidget = nullptr;
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

};

#endif // VISUALIZATIONWIDGET_H
