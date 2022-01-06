#ifndef SimCenterMapcanvasWidget_H
#define SimCenterMapcanvasWidget_H

#include <QWidget>
#include <qgsappmaptools.h>
#include <qgsfeatureid.h>

class QgsMapCanvas;
class QgsVectorLayer;
class QgsLayerTreeNode;
class QgsLayerTree;
class QgsMapLayer;
class QgsLayerTreeView;

class QGISVisualizationWidget;

class SimCenterMapcanvasWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SimCenterMapcanvasWidget(const QString &name, QGISVisualizationWidget *mainVisWidget);

    QgsMapCanvas *mapCanvas();

    QgsMapCanvas *getMainCanvas() const;

    void enablePanTool(void);

    void enableIdentifyTool(void);

    void enableSelectionTool(void);

    void enablePolygonSelectionTool(void);

    void enableFreehandSelectionTool(void);

    void enableRadiusSelectionTool(void);

    void setCurrentLayer(QgsVectorLayer* layer);

    void setMapTool(QgsMapTool *mapTool);

    QgsFeatureIds getSelectedIds() const;

    void clear(void);

    void setShowPopUpOnSelection(bool value);

    void deselectAllTreeItems(void);

protected:

signals:

private slots:

    void handleSelectionFinished(Qt::KeyboardModifiers modifiers = Qt::NoModifier);

    void selectionChanged(const QgsFeatureIds &selected, const QgsFeatureIds &deselected, bool clearAndSelect);

    void showLabels(bool show);

    void showAnnotations(bool show);

    void layerTreeViewClicked(const QModelIndex &index);

private:

    QgsLayerTreeView* legendTreeView = nullptr;
    QgsMapCanvas *thisMapCanvas = nullptr;
    QGISVisualizationWidget* theVisualizationWidget;
    QgsMapCanvas *mainCanvas = nullptr;

    QgsVectorLayer* currentLayer = nullptr;

    std::unique_ptr<QgsAppMapTools> mMapTools;

    QgsFeatureIds selectedIds;
    QgsFeatureIds deselectedIds;

    bool showPopUpOnSelection = true;

};

#endif // SimCenterMapcanvasWidget_H
