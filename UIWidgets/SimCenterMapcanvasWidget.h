#ifndef SimCenterMapcanvasWidget_H
#define SimCenterMapcanvasWidget_H

#include <QWidget>
#include <qgsappmaptools.h>
#include <qgsfeatureid.h>

class QgsMapCanvas;
class QgsVectorLayer;
class QGISVisualizationWidget;

class SimCenterMapcanvasWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SimCenterMapcanvasWidget(const QString &name, QGISVisualizationWidget *mainVisWidget);

    QgsMapCanvas *mapCanvas();

    QgsMapCanvas *getMainCanvas() const;

    void enablePanTool(void);

    void enableSelectionTool(void);

    void enablePolygonSelectionTool(void);

    void setCurrentLayer(QgsVectorLayer* layer);

    void setMapTool(QgsMapTool *mapTool);

    QgsFeatureIds getSelectedIds() const;

    void clear(void);

protected:


private slots:

    void selectionChanged(const QgsFeatureIds &selected, const QgsFeatureIds &deselected, bool clearAndSelect);

    void showLabels(bool show);

    void showAnnotations(bool show);

private:

    QgsMapCanvas *thisMapCanvas = nullptr;
    QGISVisualizationWidget* theVisualizationWidget;
    QgsMapCanvas *mainCanvas = nullptr;

    QgsVectorLayer* currentLayer = nullptr;

    std::unique_ptr<QgsAppMapTools> mMapTools;

    QgsFeatureIds selectedIds;
    QgsFeatureIds deselectedIds;

};

#endif // SimCenterMapcanvasWidget_H
