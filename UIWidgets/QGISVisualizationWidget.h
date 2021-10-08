#ifndef QGISVISUALIZATIONWIDGET_H
#define QGISVISUALIZATIONWIDGET_H
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

#include "VisualizationWidget.h"

#include <qgsfeatureid.h>
#include <qgsgeometry.h>
#include <qgsmarkersymbollayer.h>
#include <qgsfeatureiterator.h>

#include <QPointer>

class GISSelectable;
class SimCenterMapcanvasWidget;

class QMainWindow;
class QComboBox;

class QgsRasterLayer;
class QgisApp;
class QgsLayerTreeView;
class QgsMapCanvasItem;
class QgsVectorLayer;
class QgsSymbol;
class QgsMapLayer;
class QgsMapCanvas;

class QGISVisualizationWidget : public VisualizationWidget
{
public:
    QGISVisualizationWidget(QMainWindow *parent = nullptr);
    ~QGISVisualizationWidget();

    QWidget *getVisWidget();
    double getLatFromScreenPoint(const QPointF& point);
    double getLongFromScreenPoint(const QPointF& point);

    QPointF getScreenPointFromLatLong(const double& latitude, const double& longitude);

    void clear(void);
    void clearSelection(void);

    // If you want the QGIS widget to send back selected features, you need to register the layer and corresponding input widget
    void registerLayerForSelection(const QString layerId, GISSelectable* widget);

    // The layerPath is a path to a file if the vector layer is being loaded from a file
    // Otherwise, if the layer is being created programatically within R2D, the layerPath needs to define the type of geometry that will be present in the layer, e.g., "point", "linestring", "polygon","multipoint","multilinestring","multipolygon"
    // The provider key tells QGIS where the layer data is coming from. Examples of provider keys used here in R2D include:
    //     1) Use provider key "ogr" if the layer is a vector layer being loaded from a file on the users computer
    //     2) Use provider key "memory" if layer is created within the R2D program
    //     3) Use provider key "wms" if layer is loaded from an online 'wms' GIS server, e.g., basemaps
    //     4) Use provider key "gdal" if the layer is a raster layer being loaded from a file on the users computer
    QgsVectorLayer* addVectorLayer(const QString &layerPath, const QString &name, const QString &providerKey = "memory");

    QgsRasterLayer* addRasterLayer(const QString &layerPath, const QString &name, const QString &providerKey);

    QgsGeometry getPolygonGeometryFromJson(const QString& geoJson);
    QgsGeometry getPolygonGeometryFromJson(const QJsonArray& geoJson);

    void removeLayer(QgsMapLayer* layer);

    SimCenterMapcanvasWidget* getMapViewWidget(const QString& name);

    void markDirty();

    void createSymbolRenderer(QgsSimpleMarkerSymbolLayerBase::Shape symbolShape, QColor color, double size, QgsVectorLayer * layer);

    void createPrettyGraduatedRenderer(const QString attrName, const QColor color1, const QColor color2, const int nclasses, QgsVectorLayer* vlayer);

    void createCategoryRenderer(const QString attrName, QgsVectorLayer * vlayer, QgsSymbol* symbol, QVector<QColor> colors = QVector<QColor>());

    void createCustomClassBreakRenderer(const QString attrName, const QVector<QPair<double,double>>& classBreaks, const QVector<QColor>& colors, QgsVectorLayer * vlayer);

    // The same symbol will be used to render every feature
    void createSimpleRenderer(QgsSymbol* symbol, QgsVectorLayer * layer);

    void setLayerVisibility(QgsMapLayer* layer, bool value);

    // Create a group of layers
    // Must pass at least one layer
    void createLayerGroup(const QVector<QgsMapLayer*>& layers, const QString groupName);

    void turnOnSelectionTool();

    void setActiveLayer(QgsMapLayer* layer);

    void selectLayerInTree(QgsMapLayer* layer);

    void deselectAllTreeItems(void);

    QgisApp* getQgis(void);

    QgsMapCanvas* getMainCanvas(void);

    void showFeaturePopUp(QgsFeatureIterator& features);

public slots:

    void handleSelectButton(void);
    void handleIdentifyButton(void);

    void handleBasemapSelection(int index);

    void handleSelectAssetsMap(void);
    void handleClearAssetsMap(void);

    void selectionChanged(const QgsFeatureIds &selected, const QgsFeatureIds &deselected, bool clearAndSelect);

    void handleLegendChange(const QString layerUID);
    void zoomToLayer(const QString layerID);
    void zoomToLayer(QgsMapLayer* layer);

private:
    QgsLayerTreeView* layerTreeView = nullptr;

    QPointer<QgisApp> qgis;

    QComboBox* baseMapCombo = nullptr;

    void testVectorLayer();
    SimCenterMapcanvasWidget* testNewMapCanvas();
    void testNewMapCanvas2();

    QModelIndex getLayerIndex(QgsMapLayer* layer);

    // Select layers programmatically
    void selectLayersInTree(const QVector<QgsMapLayer*>& layers);

    QMap<QString,GISSelectable*> mapSelectableAssetWidgets;

};

#endif // QGISVISUALIZATIONWIDGET_H
