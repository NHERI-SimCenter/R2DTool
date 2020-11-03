#ifndef MAPVIEWSUBWIDGET_H
#define MAPVIEWSUBWIDGET_H

#include <QObject>

#include "RectangleGrid.h"
#include "MapGraphicsView.h"

class RectangleGrid;

class QGraphicsSimpleTextItem;

class MapViewSubWidget : public Esri::ArcGISRuntime::MapGraphicsView
{
    Q_OBJECT
public:
    MapViewSubWidget(QWidget* parent, MapGraphicsView* mainView);

    RectangleGrid* getGrid(void);

public slots:

    void resizeParent(QRectF rect);

protected:

    // Custom zoom implementation to get around a bug in the wheel event causing zoom to occur only in one direction
    void wheelEvent(QWheelEvent *event) override;

    void resizeEvent(QResizeEvent *event) override;

    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

//    void mousePressEvent(QMouseEvent * mouseEvent) override;

    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

    void addGridToScene(void);
    void removeGridFromScene(void);

private:

    QGraphicsSimpleTextItem* displayText;

    std::unique_ptr<RectangleGrid> grid;

    Esri::ArcGISRuntime::MapGraphicsView* mainViewWidget;

    double zoomFactor;
    bool m_initDraw = false;
};

#endif // MAPVIEWSUBWIDGET_H
