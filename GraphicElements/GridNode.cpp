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






