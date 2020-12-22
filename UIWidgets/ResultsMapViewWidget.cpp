#include "ResultsMapViewWidget.h"
#include "NodeHandle.h"

#include "Map.h"

#include <QScrollBar>
#include <QCoreApplication>
#include <QScreen>
#include <QGraphicsRectItem>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsScene>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDebug>

ResultsMapViewWidget::ResultsMapViewWidget(QWidget* parent, MapGraphicsView* mainView) : MapGraphicsView(parent), mainViewWidget(mainView)
{
    zoomFactor = 1.003;

    this->setAcceptDrops(true);
    this->setObjectName("MapSubwindow");

    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    this->setScene(mainViewWidget->scene());

    this->setGeometry(mainViewWidget->geometry());
}


void ResultsMapViewWidget::wheelEvent(QWheelEvent* wheelEvent)
{ 
    auto hThis = this->geometry().height();
    auto wThis =this->geometry().width();

    auto hThat = mainViewWidget->geometry().height();
    auto wThat = mainViewWidget->geometry().width();

    auto mousePos = wheelEvent->position().toPoint();

    // Position
    auto x = mousePos.x( ) + 0.5*(wThat-wThis);
    auto y =  mousePos.y() + 0.5*(hThat-hThis);

    // Get the point of the mouse
    Esri::ArcGISRuntime::Point mapPoint = mainViewWidget->screenToLocation(x,y);

    auto angle = -1*wheelEvent->angleDelta().ry();

    double scaleFactor = pow(zoomFactor, angle);

    auto currentScale = mainViewWidget->mapScale();

    mainViewWidget->setViewpoint(Esri::ArcGISRuntime::Viewpoint(mapPoint, currentScale*scaleFactor));

}


void ResultsMapViewWidget::mouseReleaseEvent(QMouseEvent *event)
{
    auto globalPoint = event->globalPos();

    emit mouseClick(globalPoint);
}


void ResultsMapViewWidget::resizeEvent(QResizeEvent *event)
{
    auto mapWidth = mainViewWidget->mapWidth();
    auto mapHeight = mainViewWidget->mapHeight();

    this->setMaximumWidth(mapWidth);
    this->setMaximumHeight(mapHeight);

    QAbstractScrollArea::resizeEvent(event);
}


void ResultsMapViewWidget::showEvent(QShowEvent *event)
{
    auto width = mainViewWidget->width();
    auto height = mainViewWidget->height();

    this->setMaximumWidth(width);
    this->setMaximumHeight(height);

    QAbstractScrollArea::showEvent(event);
}


void ResultsMapViewWidget::closeEvent(QCloseEvent *event)
{
    QAbstractScrollArea::closeEvent(event);
}



