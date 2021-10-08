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

#include "GridNode.h"
#include "NodeHandle.h"
#include "RectangleGrid.h"
#include "SiteConfig.h"
#include "VisualizationWidget.h"

#include <qgsmapcanvas.h>
#include <qgsmapmouseevent.h>

#include <QApplication>
#include <QBitmap>
#include <QCursor>
#include <QDebug>
#include <QDrag>
#include <QGraphicsSceneMouseEvent>
#include <QMimeData>
#include <QObject>
#include <QPainter>
#include <QPixmap>
#include <QRandomGenerator>
#include <QWidget>

RectangleGrid::RectangleGrid(QgsMapCanvas* parent) : QgsMapTool(parent), mapCanvas(parent)
{
    gridSiteConfig = nullptr;

    setCacheMode(DeviceCoordinateCache);
    setZValue(-1);

    changingDimensions = false;
    updateConnectedWidgets = true;

    latMin = 0.0;
    lonMin = 0.0;
    latMax = 0.0;
    lonMax = 0.0;

    numDivisionsHoriz = 5;
    numDivisionsVertical = 5;

    color.setRgb(0,0,255,30);

    auto width = 150;
    auto height = 150;

    rectangleGeometry.setX(0);
    rectangleGeometry.setY(0);
    rectangleGeometry.setWidth(width);
    rectangleGeometry.setHeight(height);

    bottomLeftNode = new NodeHandle(this, mapCanvas);
    bottomRightNode = new NodeHandle(this, mapCanvas);
    topRightNode = new NodeHandle(this, mapCanvas);
    topLeftNode = new NodeHandle(this, mapCanvas);
    centerNode = new NodeHandle(this, mapCanvas);

    centerNode->setColor(QColor(255,0,0,100));
    centerNode->setToolTip("Rupture Location");
    centerNode->setDiameter(20.0);

    connect(bottomLeftNode,&NodeHandle::positionChanged,this,&RectangleGrid::handleBottomLeftCornerChanged);
    connect(bottomRightNode,&NodeHandle::positionChanged,this,&RectangleGrid::handleBottomRightCornerChanged);
    connect(topRightNode,&NodeHandle::positionChanged,this,&RectangleGrid::handleTopRightCornerChanged);
    connect(topLeftNode,&NodeHandle::positionChanged,this,&RectangleGrid::handleTopLeftCornerChanged);
    connect(centerNode,&NodeHandle::positionChanged,this,&RectangleGrid::handleCenterNodeChanged);

    this->updateGeometry();

    // Important! Otherwise events will not get passed down to scene
    mapCanvas->setEnabled(true);

    QgsMapTool::setCursor(Qt::CrossCursor);
}


RectangleGrid::~RectangleGrid()
{

}


void RectangleGrid::show()
{
    QGraphicsScene* mapCanvasScene = mapCanvas->scene();

    auto sceneRect = mapCanvasScene->sceneRect();

    auto centerScene = sceneRect.center();

    auto sceneWidth = sceneRect.width();
    auto sceneHeight = sceneRect.height();

    // Set the initial grid size if it has not already been set
    if(this->getBottomLeftNode()->pos().isNull() || this->getTopRightNode()->pos().isNull() || this->getTopLeftNode()->pos().isNull() || this->getBottomRightNode()->pos().isNull() )
    {
        this->setWidth(0.5*sceneWidth);
        this->setHeight(0.5*sceneHeight);
        this->setPos(centerScene.toPoint());

        mapCanvasScene->addItem(this);
    }

    this->setVisible(true);
}


void RectangleGrid::removeGridFromScene(void)
{
    QGraphicsScene* mapCanvasScene = mapCanvas->scene();

    mapCanvasScene->removeItem(this);
}

void RectangleGrid::canvasPressEvent( QgsMapMouseEvent *event )
{
    // Store some of the event's button-down data.
    auto mousePressViewPoint = event->pos();
    auto mousePressScenePoint = mapCanvas->mapToScene(mousePressViewPoint);
    auto mousePressScreenPoint = event->globalPos();
    auto lastMouseMoveScenePoint = mousePressScenePoint;
    auto lastMouseMoveScreenPoint = mousePressScreenPoint;
    auto mousePressButton = event->button();

    // Convert and deliver the mouse event to the scene.
    QGraphicsSceneMouseEvent mouseEvent(QEvent::GraphicsSceneMousePress);
    mouseEvent.setWidget(mapCanvas->viewport());
    mouseEvent.setButtonDownScenePos(mousePressButton, mousePressScenePoint);
    mouseEvent.setButtonDownScreenPos(mousePressButton, mousePressScreenPoint);
    mouseEvent.setScenePos(mousePressScenePoint);
    mouseEvent.setScreenPos(mousePressScreenPoint);
    mouseEvent.setLastScenePos(lastMouseMoveScenePoint);
    mouseEvent.setLastScreenPos(lastMouseMoveScreenPoint);
    mouseEvent.setButtons(event->buttons());
    mouseEvent.setButton(event->button());
    mouseEvent.setModifiers(event->modifiers());
    mouseEvent.setAccepted(false);

    QApplication::sendEvent(mapCanvas->scene(), &mouseEvent);

    // Update the original mouse event accepted state.
    bool isAccepted = mouseEvent.isAccepted();
    event->setAccepted(isAccepted);
}


void RectangleGrid::canvasMoveEvent( QgsMapMouseEvent *event )
{
    // Store some of the event's button-down data.
    auto mousePressViewPoint = event->pos();
    auto mousePressScenePoint = mapCanvas->mapToScene(mousePressViewPoint);
    auto mousePressScreenPoint = event->globalPos();
    auto lastMouseMoveScenePoint = mousePressScenePoint;
    auto lastMouseMoveScreenPoint = mousePressScreenPoint;
    auto mousePressButton = event->button();

    // Convert and deliver the mouse event to the scene.
    QGraphicsSceneMouseEvent mouseEvent(QEvent::GraphicsSceneMouseMove);
    mouseEvent.setWidget(mapCanvas->viewport());
    mouseEvent.setButtonDownScenePos(mousePressButton, mousePressScenePoint);
    mouseEvent.setButtonDownScreenPos(mousePressButton, mousePressScreenPoint);
    mouseEvent.setScenePos(mousePressScenePoint);
    mouseEvent.setScreenPos(mousePressScreenPoint);
    mouseEvent.setLastScenePos(lastMouseMoveScenePoint);
    mouseEvent.setLastScreenPos(lastMouseMoveScreenPoint);
    mouseEvent.setButtons(event->buttons());
    mouseEvent.setButton(event->button());
    mouseEvent.setModifiers(event->modifiers());
    mouseEvent.setAccepted(false);

    QApplication::sendEvent(mapCanvas->scene(), &mouseEvent);
}

void RectangleGrid::canvasReleaseEvent( QgsMapMouseEvent *event )
{
    // Store some of the event's button-down data.
    auto mousePressViewPoint = event->pos();
    auto mousePressScenePoint = mapCanvas->mapToScene(mousePressViewPoint);
    auto mousePressScreenPoint = event->globalPos();
    auto lastMouseMoveScenePoint = mousePressScenePoint;
    auto lastMouseMoveScreenPoint = mousePressScreenPoint;
    auto mousePressButton = event->button();

    // Convert and deliver the mouse event to the scene.
    QGraphicsSceneMouseEvent mouseEvent(QEvent::GraphicsSceneMouseRelease);
    mouseEvent.setWidget(mapCanvas->viewport());
    mouseEvent.setButtonDownScenePos(mousePressButton, mousePressScenePoint);
    mouseEvent.setButtonDownScreenPos(mousePressButton, mousePressScreenPoint);
    mouseEvent.setScenePos(mousePressScenePoint);
    mouseEvent.setScreenPos(mousePressScreenPoint);
    mouseEvent.setLastScenePos(lastMouseMoveScenePoint);
    mouseEvent.setLastScreenPos(lastMouseMoveScreenPoint);
    mouseEvent.setButtons(event->buttons());
    mouseEvent.setButton(event->button());
    mouseEvent.setModifiers(event->modifiers());
    mouseEvent.setAccepted(false);

    QApplication::sendEvent(mapCanvas->scene(), &mouseEvent);
}


QRectF RectangleGrid::boundingRect() const
{
    qreal adjust = 5;
    return QRectF(rectangleGeometry.x() - adjust, rectangleGeometry.y() - adjust, rectangleGeometry.width() + 5 + adjust, rectangleGeometry.height() + 5 + adjust);
}


QPainterPath RectangleGrid::shape() const
{
    QPainterPath path;
    path.addRect(rectangleGeometry);
    return path;
}


void RectangleGrid::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setPen(Qt::NoPen);
    painter->setBrush(QBrush(color));
    painter->drawRect(rectangleGeometry);
}


void RectangleGrid::updateGeometry(void)
{
    auto bottomLeftPnt = rectangleGeometry.bottomLeft();
    auto topRightPnt = rectangleGeometry.topRight();
    auto centerPnt = rectangleGeometry.center();

    bottomLeftNode->setPos(bottomLeftPnt);
    bottomRightNode->setPos(rectangleGeometry.bottomRight());
    topRightNode->setPos(topRightPnt);
    topLeftNode->setPos(rectangleGeometry.topLeft());
    centerNode->setPos(centerPnt);

    if(gridSiteConfig && updateConnectedWidgets)
    {
        latMin = theVisWidget->getLatFromScreenPoint(bottomLeftPnt);
        latMax = theVisWidget->getLatFromScreenPoint(topRightPnt);

        lonMin = theVisWidget->getLongFromScreenPoint(bottomLeftPnt);
        lonMax = theVisWidget->getLongFromScreenPoint(topRightPnt);

        gridSiteConfig->siteGrid().latitude().set(latMin, latMax, numDivisionsHoriz);
        gridSiteConfig->siteGrid().longitude().set(lonMin, lonMax, numDivisionsVertical);

        //        auto centerPointLat = theVisWidget->getLatFromScreenPoint(centerPnt);
        //        auto centerPointLong = theVisWidget->getLongFromScreenPoint(centerPnt);

        //        theRuptureWidget->setLocation(centerPointLat,centerPointLong);
    }

    emit geometryChanged();
}


QVector<GridNode *> RectangleGrid::getGridNodeVec() const
{
    return gridNodeVec;
}


size_t RectangleGrid::getNumDivisionsVertical() const
{
    return numDivisionsVertical;
}


void RectangleGrid::setNumDivisionsVertical(const size_t &value)
{
    numDivisionsVertical = value;
}


size_t RectangleGrid::getNumDivisionsHoriz() const
{
    return numDivisionsHoriz;
}


void RectangleGrid::setNumDivisionsHoriz(const size_t &value)
{
    numDivisionsHoriz = value;
}


void RectangleGrid::setVisualizationWidget(VisualizationWidget *value)
{
    theVisWidget = value;
}


void RectangleGrid::setSiteGridConfig(SiteConfig *value)
{
    gridSiteConfig = value;

    // Connect grid latitude
    connect(&gridSiteConfig->siteGrid().latitude(), &GridDivision::minChanged, this, &RectangleGrid::handleLatLonChanged);
    connect(&gridSiteConfig->siteGrid().latitude(), &GridDivision::maxChanged, this, &RectangleGrid::handleLatLonChanged);

    // Connect grid longitude
    connect(&gridSiteConfig->siteGrid().longitude(), &GridDivision::minChanged, this, &RectangleGrid::handleLatLonChanged);
    connect(&gridSiteConfig->siteGrid().longitude(), &GridDivision::maxChanged, this, &RectangleGrid::handleLatLonChanged);

    // Connect the grid discretization
    connect(&gridSiteConfig->siteGrid().latitude(), &GridDivision::divisionsChanged, this, &RectangleGrid::handleGridDivisionsChanged);
    connect(&gridSiteConfig->siteGrid().longitude(), &GridDivision::divisionsChanged, this, &RectangleGrid::handleGridDivisionsChanged);
}


void RectangleGrid::setPos(const QPoint& pos)
{
    this->rectangleGeometry.moveCenter(pos);
    this->updateGeometry();
}


void RectangleGrid::setWidth(const double& val)
{
    this->rectangleGeometry.setWidth(val);
    this->updateGeometry();
}


void RectangleGrid::setHeight(const double& val)
{
    this->rectangleGeometry.setHeight(val);
    this->updateGeometry();
}


QVariant RectangleGrid::itemChange(GraphicsItemChange change, const QVariant &value)
{
    switch (change) {
    case ItemPositionHasChanged:
        emit geometryChanged();
        break;
    default:
        break;
    };

    return QGraphicsItem::itemChange(change, value);
}


void RectangleGrid::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mousePressEvent(event);
}


void RectangleGrid::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseReleaseEvent(event);
}


NodeHandle *RectangleGrid::getCenterNode() const
{
    return centerNode;
}


NodeHandle *RectangleGrid::getTopLeftNode() const
{
    return topLeftNode;
}


NodeHandle *RectangleGrid::getTopRightNode() const
{
    return topRightNode;
}


NodeHandle *RectangleGrid::getBottomRightNode() const
{
    return bottomRightNode;
}


NodeHandle *RectangleGrid::getBottomLeftNode() const
{
    return bottomLeftNode;
}


void RectangleGrid::setTopLeftNode(NodeHandle *value)
{
    topLeftNode = value;
}


void RectangleGrid::setTopRightNode(NodeHandle *value)
{
    topRightNode = value;
}


void RectangleGrid::setTopRightNode(const double latitude, const double longitude)
{
    auto scrnPnt = theVisWidget->getScreenPointFromLatLong(latitude,longitude);

    this->handleTopRightCornerChanged(scrnPnt);
}


void RectangleGrid::setCenterNode(const double latitude, const double longitude)
{
    auto scrnPnt = theVisWidget->getScreenPointFromLatLong(latitude,longitude);

    this->handleCenterNodeChanged(scrnPnt);
}


void RectangleGrid::setBottomRightNode(NodeHandle *value)
{
    bottomRightNode = value;
}


void RectangleGrid::setBottomLeftNode(NodeHandle *value)
{
    bottomLeftNode = value;
}


void RectangleGrid::setBottomLeftNode(const double latitude, const double longitude)
{
    auto scrnPnt = theVisWidget->getScreenPointFromLatLong(latitude,longitude);

    this->handleBottomLeftCornerChanged(scrnPnt);
}


void RectangleGrid::handleBottomLeftCornerChanged(const QPointF& pos)
{
    if(changingDimensions == true)
        return;

    changingDimensions = true;
    rectangleGeometry.setBottomLeft(pos.toPoint());
    this->updateGeometry();
    changingDimensions = false;
}


void RectangleGrid::handleBottomRightCornerChanged(const QPointF& pos)
{
    if(changingDimensions == true)
        return;

    changingDimensions = true;
    rectangleGeometry.setBottomRight(pos.toPoint());
    this->updateGeometry();
    changingDimensions = false;
}


void RectangleGrid::handleTopLeftCornerChanged(const QPointF& pos)
{
    if(changingDimensions == true)
        return;

    changingDimensions = true;
    rectangleGeometry.setTopLeft(pos.toPoint());
    this->updateGeometry();
    changingDimensions = false;
}


void RectangleGrid::handleTopRightCornerChanged(const QPointF& pos)
{
    if(changingDimensions == true)
        return;

    changingDimensions = true;
    rectangleGeometry.setTopRight(pos.toPoint());
    this->updateGeometry();
    changingDimensions = false;
}


void RectangleGrid::handleCenterNodeChanged(const QPointF& pos)
{
    if(changingDimensions == true)
        return;

    this->rectangleGeometry.moveCenter(pos.toPoint());
    bottomLeftNode->setPos(rectangleGeometry.bottomLeft());
    bottomRightNode->setPos(rectangleGeometry.bottomRight());
    topRightNode->setPos(rectangleGeometry.topRight());
    topLeftNode->setPos(rectangleGeometry.topLeft());
}


void RectangleGrid::clearGrid()
{
    qDeleteAll(gridNodeVec);
    gridNodeVec.clear();
}


void RectangleGrid::createGrid()
{
    auto ni = numDivisionsHoriz;
    auto nj = numDivisionsVertical;

    gridNodeVec.reserve(ni*nj);

    for (size_t i=0; i<=ni; ++i)
    {
        for (size_t j=0;j<=nj; ++j)
        {

            // X COORDINATE
            std::function<double(void)> xCoordinatelambda = [=]
            {
                auto divisor = static_cast<double>(ni*nj);

                auto n1X = bottomLeftNode->pos().x();
                auto n2X = bottomRightNode->pos().x();
                auto n3X = topRightNode->pos().x();
                auto n4X = topLeftNode->pos().x();

                return static_cast<double>(i*j*n1X-i*j*n2X+i*j*n3X-i*j*n4X-i*n1X*nj+i*n2X*nj-j*n1X*ni+j*n4X*ni+n1X*ni*nj)/(divisor);
            };

            // Y COORDINATE
            std::function<double(void)> yCoordinatelambda = [=]
            {
                auto divisor = static_cast<double>(ni*nj);

                auto n1Y = bottomLeftNode->pos().y();
                auto n2Y = bottomRightNode->pos().y();
                auto n3Y = topRightNode->pos().y();
                auto n4Y = topLeftNode->pos().y();

                return static_cast<double>(i*j*n1Y-i*j*n2Y+i*j*n3Y-i*j*n4Y-i*n1Y*nj+i*n2Y*nj-j*n1Y*ni+j*n4Y*ni+n1Y*ni*nj)/(divisor);
            };

            auto x = xCoordinatelambda();
            auto y = yCoordinatelambda();

            // Create the grid node
            GridNode* newGridNode = new GridNode(this);

            newGridNode->setX(x);
            newGridNode->setY(y);

            newGridNode->setXPos(xCoordinatelambda);
            newGridNode->setYPos(yCoordinatelambda);

            gridNodeVec.push_back(newGridNode);

            connect(this,&RectangleGrid::geometryChanged,newGridNode,&GridNode::updateGeometry);
        }
    }
}


void RectangleGrid::handleGridDivisionsChanged(void)
{
    auto numDivH = static_cast<size_t>(gridSiteConfig->siteGrid().longitude().divisions());
    auto numDivV = static_cast<size_t>(gridSiteConfig->siteGrid().latitude().divisions());

    if(numDivV == this->numDivisionsVertical && numDivH == this->numDivisionsHoriz)
        return;

    this->numDivisionsVertical = numDivV;
    this->numDivisionsHoriz = numDivH;

    this->clearGrid();
    this->createGrid();
}


void RectangleGrid::handleLatLonChanged(void)
{
    if(changingDimensions == true)
        return;

    auto lat_Min = gridSiteConfig->siteGrid().latitude().min();
    auto lat_Max = gridSiteConfig->siteGrid().latitude().max();

    auto lon_Min = gridSiteConfig->siteGrid().longitude().min();
    auto lon_Max = gridSiteConfig->siteGrid().longitude().max();

    if(lat_Min != latMin || lon_Min != lonMin)
    {
        updateConnectedWidgets = false;
        latMin = lat_Min;
        lonMin = lon_Min;
        this->setBottomLeftNode(lat_Min,lon_Min);
        updateConnectedWidgets = true;
    }

    if(lat_Max != latMax || lon_Max != lonMax)
    {
        updateConnectedWidgets = false;
        latMax = lat_Max;
        lonMax = lon_Max;
        this->setTopRightNode(lat_Max,lon_Max);
        updateConnectedWidgets = true;
    }
}


