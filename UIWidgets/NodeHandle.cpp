
#include "NodeHandle.h"

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include <QDebug>

NodeHandle::NodeHandle(QGraphicsItem *parent) : QGraphicsItem(parent)
{
    setFlag(ItemIsMovable);
    setFlag(ItemSendsGeometryChanges);
    setCacheMode(DeviceCoordinateCache);
    setZValue(-1);

    setAcceptedMouseButtons(Qt::LeftButton);
    setCursor(Qt::OpenHandCursor);

    diameter = 15;
    this->setDiameter(diameter);

    nodeColor.setRgb(0,0,255,150);

    setToolTip("Click and drag this object to define a grid area");
}



QRectF NodeHandle::boundingRect() const
{
    qreal adjust = 2;
    return QRectF( nodeGeometry.x() - adjust, nodeGeometry.y() - adjust, nodeGeometry.width() + 3 + adjust, nodeGeometry.height() + 3 + adjust);
}


QPainterPath NodeHandle::shape() const
{
    QPainterPath path;
    path.addEllipse(nodeGeometry);
    return path;
}


void NodeHandle::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    QBrush brush(nodeColor);

    if (option->state & QStyle::State_Sunken)
        brush.setColor(nodeColor.lighter());


    painter->setBrush(brush);
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(nodeGeometry);
}



QVariant NodeHandle::itemChange(GraphicsItemChange change, const QVariant &value)
{
    switch (change) {
    case ItemPositionHasChanged:
        emit  positionChanged(this->pos());
        break;
    default:
        break;
    };

    return QGraphicsItem::itemChange(change, value);
}


void NodeHandle::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    update();
    setCursor(Qt::ClosedHandCursor);
    QGraphicsItem::mousePressEvent(event);
}


void NodeHandle::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    update();
    setCursor(Qt::OpenHandCursor);
    QGraphicsItem::mouseReleaseEvent(event);
}

void NodeHandle::setColor(const QColor &value)
{
    nodeColor = value;
}

void NodeHandle::setDiameter(const double diameter)
{
    nodeGeometry.setX(-0.5*diameter);
    nodeGeometry.setY(-0.5*diameter);
    nodeGeometry.setWidth(diameter);
    nodeGeometry.setHeight(diameter);
}
