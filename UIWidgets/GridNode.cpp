
#include "GridNode.h"

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include <QDebug>

GridNode::GridNode(QGraphicsItem *parent) : QGraphicsItem(parent)
{
    setCacheMode(DeviceCoordinateCache);
    setZValue(-1);

    auto diameter = 5;

    nodeGeometry.setX(-0.5*diameter);
    nodeGeometry.setY(-0.5*diameter);
    nodeGeometry.setWidth(diameter);
    nodeGeometry.setHeight(diameter);

    nodeColor.setRgb(0,0,255,100);

    setToolTip("Grid Node");

    xPos = [](){return 0.0;};
    yPos = [](){return 0.0;};
}



QRectF GridNode::boundingRect() const
{
    qreal adjust = 2;
    return QRectF( nodeGeometry.x() - adjust, nodeGeometry.y() - adjust, nodeGeometry.width() + 3 + adjust, nodeGeometry.height() + 3 + adjust);
}


QPainterPath GridNode::shape() const
{
    QPainterPath path;
    path.addEllipse(nodeGeometry);
    return path;
}


void GridNode::paint(QPainter *painter, const QStyleOptionGraphicsItem */*option*/, QWidget *)
{
    QBrush brush(nodeColor);

    painter->setBrush(brush);
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(nodeGeometry);
}


void GridNode::setXPos(const std::function<double(void)> &value)
{
    xPos = value;
}


void GridNode::setYPos(const std::function<double(void)> &value)
{
    yPos = value;
}


double GridNode::getXPos(void)
{
    auto x = xPos();

    return x;
}


double GridNode::getYPos(void)
{
    auto y = yPos();

    return y;
}


QPointF GridNode::getPoint(void)
{
    return this->pos();
}


void GridNode::updateGeometry(void)
{
    auto x = xPos();
    auto y = yPos();

    this->setPos(QPointF(x,y));
}






