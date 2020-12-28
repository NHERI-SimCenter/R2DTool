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

// Written by: Stevan Gavrilovic, Frank McKenna

#include "MapViewSubWidget.h"
#include "NodeHandle.h"
#include "SimCenterMapGraphicsView.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDragEnterEvent>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QGraphicsSimpleTextItem>
#include <QMimeData>
#include <QScreen>
#include <QScrollBar>

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


