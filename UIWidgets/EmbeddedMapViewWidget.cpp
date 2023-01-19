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
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
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

#include "EmbeddedMapViewWidget.h"

#ifdef ARC_GIS
#include "SimCenterMapGraphicsView.h"
#endif

#include <QGraphicsSimpleTextItem>
#include <QGraphicsView>
#include <QVBoxLayout>
#include <QDragEnterEvent>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QApplication>
#include <QGraphicsItemGroup>

#ifdef Q_GIS
#include "SimCenterMapcanvasWidget.h"
#include <qgsmapcanvas.h>
#include <qgsmapcanvasdockwidget.h>
#endif

#ifdef ARC_GIS
EmbeddedMapViewWidget::EmbeddedMapViewWidget(QGraphicsView* parent)
#endif

#ifdef Q_GIS
EmbeddedMapViewWidget::EmbeddedMapViewWidget(SimCenterMapcanvasWidget* mapCanvasWidget)
#endif
{
    mapCanvas = mapCanvasWidget->mapCanvas();

    theViewLayout = new QVBoxLayout(this);
    theViewLayout->setSpacing(0);
    theViewLayout->setMargin(0);

    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

#ifdef ARC_GIS
    theNewView = SimCenterMapGraphicsView::getInstance();
    theNewView->setAcceptDrops(true);
    //theNewView->setObjectName("MapSubwindow");

    theNewView->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    theNewView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    theNewView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    theNewView->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    theNewView->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    grid = std::make_unique<RectangleGrid>(theNewView);
#endif

#ifdef Q_GIS
    theViewLayout->addWidget(mapCanvasWidget);

    grid = std::make_unique<RectangleGrid>(mapCanvas);
#endif

    point = std::make_unique<NodeHandle>();
}


#ifdef ARC_GIS
void EmbeddedMapViewWidget::setCurrentlyViewable(bool status)
{
    if (status == true)
        theNewView->setCurrentLayout(theViewLayout);
    else {
        this->hide();
    }
}
#endif


void EmbeddedMapViewWidget::addGridToScene(void)
{

#ifdef ARC_GIS
    auto scene = theNewView->scene();
#endif

#ifdef Q_GIS
    QGraphicsScene* mapCanvasScene = mapCanvas->scene();
#endif

    auto sceneRect = mapCanvasScene->sceneRect();

    auto centerScene = sceneRect.center();

    auto sceneWidth = sceneRect.width();
    auto sceneHeight = sceneRect.height();

    // Set the initial grid size if it has not already been set
    if(grid->getBottomLeftNode()->pos().isNull() || grid->getTopRightNode()->pos().isNull() || grid->getTopLeftNode()->pos().isNull() || grid->getBottomRightNode()->pos().isNull() )
    {
        grid->setWidth(0.5*sceneWidth);
        grid->setHeight(0.5*sceneHeight);
        grid->setPos(centerScene.toPoint());

        mapCanvasScene->addItem(grid.get());
    }

    grid->show();
}


RectangleGrid* EmbeddedMapViewWidget::getGrid(void)
{
    return grid.get();
}




NodeHandle* EmbeddedMapViewWidget::getPoint(void)
{
    return point.get();
}


void EmbeddedMapViewWidget::removeGridFromScene(void)
{
    grid->hide();
}



void EmbeddedMapViewWidget::showEvent(QShowEvent *event)
{
    mapCanvas->zoomToFullExtent();
    this->QWidget::showEvent(event);
}



void EmbeddedMapViewWidget::closeEvent(QCloseEvent *event)
{
    this->removeGridFromScene();

    this->QWidget::closeEvent(event);
}


void EmbeddedMapViewWidget::addPointToScene(void)
{
    return;
#ifdef ARC_GIS
    auto scene = theNewView->scene();
#endif

//#ifdef Q_GIS
//    QGraphicsScene* scene = mapView->scene();
//#endif

//    auto sceneRect = scene->sceneRect();

//    auto centerScene = sceneRect.center();

//    // Set the initial grid size if it has not already been set
//    if(point->pos().isNull() )
//    {
//        point->setPos(centerScene.toPoint());

//        scene->addItem(point.get());
//    }

//    point->show();
}


void EmbeddedMapViewWidget::removePointFromScene(void)
{
    point->hide();
}



