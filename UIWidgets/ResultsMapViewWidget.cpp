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

    connect(this, &Esri::ArcGISRuntime::MapGraphicsView::viewpointChanged, this, [this]
    {
        if (this->isNavigating())
            mainViewWidget->setViewpoint(this->currentViewpoint(Esri::ArcGISRuntime::ViewpointType::CenterAndScale), 0);

    }, Qt::UniqueConnection);

    connect(mainViewWidget, &Esri::ArcGISRuntime::MapGraphicsView::viewpointChanged, this, [this]
    {
        if (mainViewWidget->isNavigating())
            this->setViewpoint(mainViewWidget->currentViewpoint(Esri::ArcGISRuntime::ViewpointType::CenterAndScale), 0);

    }, Qt::UniqueConnection);


}


void ResultsMapViewWidget::wheelEvent(QWheelEvent* wheelEvent)
{

    //    QRect widgetRect = mainViewWidget->geometry();
    //    widgetRect.moveTopLeft(mainViewWidget->parentWidget()->mapToGlobal(widgetRect.topLeft()));

    auto mousePos = wheelEvent->position().toPoint();
    auto mouseAD = wheelEvent->angleDelta();

    // Position

    QRect widgetRect = this->geometry();
    QPoint thisTLGlobal = this->mapToGlobal(widgetRect.center());


    QRect widgetRect2 = mainViewWidget->geometry();
    QPoint thatTLGlobal = mainViewWidget->mapToGlobal(widgetRect2.center());

    auto diff = thatTLGlobal - thisTLGlobal;


    auto thisGlobalMousePos = this->mapToGlobal(mousePos);

    auto thatMousePos = mainViewWidget->mapFromGlobal(thisGlobalMousePos + diff);

    // Angle delta
    const QPoint globalAD = this->mapToGlobal(mouseAD);
    QPoint localAD = mainViewWidget->mapFromGlobal(globalAD);


    // Get the point of the mouse
    Esri::ArcGISRuntime::Point mapPoint = mainViewWidget->screenToLocation(thatMousePos.x(), thatMousePos.y());


    auto ry = localAD.ry();
    auto angle = 0.0;
    if(ry<0)
        angle = 86;
    else
        angle = -86;

    qDebug()<<"rx"<<localAD.rx();
    qDebug()<<"ry"<<localAD.ry();



//    auto angle = -1*localAD.ry();

    double scaleFactor = pow(zoomFactor, angle);

    auto currentScale = mainViewWidget->mapScale();

    mainViewWidget->setViewpoint(Esri::ArcGISRuntime::Viewpoint(mapPoint, currentScale*scaleFactor));

}


void ResultsMapViewWidget::resizeEvent(QResizeEvent *event)
{
    //    auto mapWidth = mainViewWidget->mapWidth();
    //    auto mapHeight = mainViewWidget->mapHeight();

    //    this->setMaximumWidth(mapWidth);
    //    this->setMaximumHeight(mapHeight);

    QAbstractScrollArea::resizeEvent(event);
}


void ResultsMapViewWidget::showEvent(QShowEvent *event)
{
    //    auto width = mainViewWidget->width();
    //    auto height = mainViewWidget->height();

    //    this->setMaximumWidth(width);
    //    this->setMaximumHeight(height);

    QAbstractScrollArea::showEvent(event);
}


void ResultsMapViewWidget::closeEvent(QCloseEvent *event)
{
    QAbstractScrollArea::closeEvent(event);
}



