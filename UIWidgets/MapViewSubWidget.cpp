#include "MapViewSubWidget.h"
#include "NodeHandle.h"
#include "SimCenterMapGraphicsView.h"

#include <QScrollBar>
#include <QCoreApplication>
#include <QScreen>
#include <QGraphicsRectItem>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsScene>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDebug>

MapViewSubWidget::MapViewSubWidget(QWidget* parent)
    :QDialog(parent)
{
    displayText = nullptr;

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

}

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


void MapViewSubWidget::dropEvent(QDropEvent */*event*/)
{

}


void MapViewSubWidget::resizeEvent(QResizeEvent *event)
{
    this->QDialog::resizeEvent(event);
}


void MapViewSubWidget::showEvent(QShowEvent *event)
{
    this->QDialog::showEvent(event);
}


void MapViewSubWidget::closeEvent(QCloseEvent *event)
{
    this->removeGridFromScene();

    this->QDialog::closeEvent(event);
}


void MapViewSubWidget::resizeParent(QRectF rect)
{
    auto width = rect.width();
    auto height = rect.height();

    theNewView->setMaximumWidth(width);
    theNewView->setMaximumHeight(height);

    theNewView->resize(width,height);
}


