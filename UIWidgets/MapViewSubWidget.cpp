#include "MapViewSubWidget.h"
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

MapViewSubWidget::MapViewSubWidget(QWidget* parent, MapGraphicsView* mainView) : MapGraphicsView(parent), mainViewWidget(mainView)
{
    displayText = nullptr;
    zoomFactor = 1.005;

    this->setAcceptDrops(true);
    this->setObjectName("MapSubwindow");

    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    setRenderHint(QPainter::Antialiasing);

    this->setScene(mainViewWidget->scene());

    grid = std::make_unique<RectangleGrid>(this);

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

    connect(mainViewWidget,&MapGraphicsView::rectChanged,this,&MapViewSubWidget::resizeParent);

}


void MapViewSubWidget::addGridToScene(void)
{
    auto scene = mainViewWidget->scene();

    auto sceneRect = scene->sceneRect();

    auto centerScene = sceneRect.center();

    auto sceneWidth = sceneRect.width();
    auto sceneHeight = sceneRect.height();

    // Set the initial grid size if it has not already been set
    if(grid->getBottomLeftNode()->pos().isNull() || grid->getTopRightNode()->pos().isNull() || grid->getTopLeftNode()->pos().isNull() || grid->getBottomRightNode()->pos().isNull() )
    {
        grid->setWidth(0.5*sceneWidth);
        grid->setHeight(0.5*sceneHeight);
        grid->setPos(centerScene.toPoint());

        scene->addItem(grid.get());
    }

    grid->show();
}


RectangleGrid* MapViewSubWidget::getGrid(void)
{
    return grid.get();
}


void MapViewSubWidget::removeGridFromScene(void)
{
    grid->hide();
}


void MapViewSubWidget::dragEnterEvent(QDragEnterEvent *event)
{
    event->accept();
}


void MapViewSubWidget::dragMoveEvent(QDragMoveEvent* event)
{
    event->accept();

}


void MapViewSubWidget::dragLeaveEvent(QDragLeaveEvent *event)
{
    event->accept();
}


void MapViewSubWidget::dropEvent(QDropEvent *event)
{

    //    qDebug()<<"yes";

    //    auto dropPos = event->pos();
    //    qDebug()<<dropPos.x();
    //    qDebug()<<dropPos.y();

    //    test->setPos(dropPos);

    //    event->accept();
}


void MapViewSubWidget::wheelEvent(QWheelEvent* wheelEvent)
{

    auto mousePos = wheelEvent->position();

    // Get the point of the mouse
    Esri::ArcGISRuntime::Point mapPoint = mainViewWidget->screenToLocation(mousePos.x(), mousePos.y());

    auto angle = -1*wheelEvent->angleDelta().ry();

    double scaleFactor = pow(zoomFactor, angle);

    auto currentScale = mainViewWidget->mapScale();

    mainViewWidget->setViewpoint(Esri::ArcGISRuntime::Viewpoint(mapPoint, currentScale*scaleFactor));

}


void MapViewSubWidget::resizeEvent(QResizeEvent *event)
{
    auto mapWidth = mainViewWidget->mapWidth();
    auto mapHeight = mainViewWidget->mapHeight();

    this->setMaximumWidth(mapWidth);
    this->setMaximumHeight(mapHeight);

    QAbstractScrollArea::resizeEvent(event);
}


void MapViewSubWidget::showEvent(QShowEvent *event)
{
    auto width = mainViewWidget->width();
    auto height = mainViewWidget->height();

    this->setMaximumWidth(width);
    this->setMaximumHeight(height);

    QAbstractScrollArea::showEvent(event);

    //    auto displayText = this->scene()->addSimpleText("Test");
    //    QFont sansFont("Helvetica [Cronyx]", 24);
    //    displayText->setFont(sansFont);

    this->addGridToScene();
}

void MapViewSubWidget::closeEvent(QCloseEvent *event)
{
    this->removeGridFromScene();

    QAbstractScrollArea::closeEvent(event);
}


void MapViewSubWidget::resizeParent(QRectF rect)
{
    auto width = rect.width();
    auto height = rect.height();

    this->setMaximumWidth(width);
    this->setMaximumHeight(height);

    this->resize(width,height);

    if(displayText != nullptr)
    {
        displayText->setX(width*0.05);
        displayText->setY(height*0.80);
    }
}


