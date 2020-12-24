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
#include <SimCenterMapGraphicsView.h>


MapViewSubWidget::MapViewSubWidget(QWidget* parent)
    :QDialog(parent)
{
    displayText = nullptr;
    zoomFactor = 1.005;

    theViewLayout = new QVBoxLayout();

    this->setLayout(theViewLayout);

    theNewView = SimCenterMapGraphicsView::getInstance();
    theNewView->setAcceptDrops(true);
    //theNewView->setObjectName("MapSubwindow");

    theNewView->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    theNewView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    theNewView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    theNewView->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    theNewView->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    //theNewView->setScene(theNewView->scene());

    grid = std::make_unique<RectangleGrid>(theNewView);

    connect(theNewView,SIGNAL(wheelEvent()), this, SLOT(wheelEvent()));

    /*
    connect(theNewView, &Esri::ArcGISRuntime::MapGraphicsView::viewpointChanged, theNewView, [theNewView]
    {
        if (theNewView->isNavigating())
            theNewView->setViewpoint(theNewView->currentViewpoint(Esri::ArcGISRuntime::ViewpointType::CenterAndScale), 0);

    }, Qt::UniqueConnection);

    connect(theNewView, &Esri::ArcGISRuntime::MapGraphicsView::viewpointChanged, theNewView, [theNewView]
    {
        if (theNewView->isNavigating())
            theNewView->setViewpoint(theNewView->currentViewpoint(Esri::ArcGISRuntime::ViewpointType::CenterAndScale), 0);

    }, Qt::UniqueConnection);

    connect(theNewView,&MapGraphicsView::rectChanged,theNewView,&MapViewSubWidget::resizeParent);
    */

}

/*
void MapViewSubWidget::MapViewSubWQidget::viewPortChanged(void) {

    if (theNewView->isNavigating())
        theNewView->setViewpoint(theNewView->currentViewpoint(Esri::ArcGISRuntime::ViewpointType::CenterAndScale), 0);
}
*/

void MapViewSubWidget::setCurrentlyViewable(bool status)
{
    if (status == true)
        theNewView->setCurrentLayout(theViewLayout);
    else {
        this->hide();
    }
}


void MapViewSubWidget::addGridToScene(void)
{
    auto scene = theNewView->scene();

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
    Esri::ArcGISRuntime::Point mapPoint = theNewView->screenToLocation(mousePos.x(), mousePos.y());

    auto angle = -1*wheelEvent->angleDelta().ry();

    double scaleFactor = pow(zoomFactor, angle);

    auto currentScale = theNewView->mapScale();

    theNewView->setViewpoint(Esri::ArcGISRuntime::Viewpoint(mapPoint, currentScale*scaleFactor));

}


void MapViewSubWidget::resizeEvent(QResizeEvent *event)
{
    /*
    auto mapWidth = theNewView->mapWidth();
    auto mapHeight = theNewView->mapHeight();

    theNewView->setMaximumWidth(mapWidth);
    theNewView->setMaximumHeight(mapHeight);
    */

   // this->QAbstractScrollArea::resizeEvent(event);
}


void MapViewSubWidget::showEvent(QShowEvent *event)
{
    /*
    auto width = theNewView->width();
    auto height = theNewView->height();

    this->setMaximumWidth(width);
    this->setMaximumHeight(height);
    */

    this->QAbstractScrollArea::showEvent(event);

    //    auto displayText = theNewView->scene()->addSimpleText("Test");
    //    QFont sansFont("Helvetica [Cronyx]", 24);
    //    displayText->setFont(sansFont);
}


void MapViewSubWidget::closeEvent(QCloseEvent *event)
{
    this->removeGridFromScene();

    this->QAbstractScrollArea::closeEvent(event);
}


void MapViewSubWidget::resizeParent(QRectF rect)
{
    auto width = rect.width();
    auto height = rect.height();

    theNewView->setMaximumWidth(width);
    theNewView->setMaximumHeight(height);

    theNewView->resize(width,height);

//    if(displayText != nullptr)
//    {
//        displayText->setX(width*0.05);
//        displayText->setY(height*0.80);
//    }
}


