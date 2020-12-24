#ifndef MAPVIEWSUBWIDGET_H
#define MAPVIEWSUBWIDGET_H

#include <QObject>

#include "RectangleGrid.h"
#include "MapGraphicsView.h"
#include <QDialog>

class RectangleGrid;
class SimCenterMapGraphicsView;
class QGraphicsSimpleTextItem;
class QVBoxLayout;

//class MapViewSubWidget : public Esri::ArcGISRuntime::MapGraphicsView
class MapViewSubWidget: public QDialog
{
    Q_OBJECT
public:
    MapViewSubWidget(QWidget* parent);

    RectangleGrid* getGrid(void);

    void setCurrentlyViewable(bool status);

public slots:

    void resizeParent(QRectF rect);

    void addGridToScene(void);
    void removeGridFromScene(void);

protected:

    // Custom zoom implementation to get around a bug in the wheel event causing zoom to occur only in one direction
    void wheelEvent(QWheelEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

    // Override widget events
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private:

    QGraphicsSimpleTextItem* displayText;

    std::unique_ptr<RectangleGrid> grid;

    Esri::ArcGISRuntime::MapGraphicsView* mainViewWidget;

    double zoomFactor;
    bool m_initDraw = false;

    SimCenterMapGraphicsView *theNewView;
    QVBoxLayout *theViewLayout;
};

#endif // MAPVIEWSUBWIDGET_H
