#ifndef RectangleGrid_H
#define RectangleGrid_H
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

#include <QGraphicsItem>
#include <QObject>

class NodeHandle;
class GridNode;
class SiteConfig;
class VisualizationWidget;

class RectangleGrid : public QObject, public QGraphicsItem
{
    Q_OBJECT
public:
    RectangleGrid(QObject* parent);
    ~RectangleGrid();

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void setPos(const QPoint& pos);
    void setWidth(const double& val);
    void setHeight(const double& val);

    void setBottomLeftNode(NodeHandle *value);
    void setBottomLeftNode(const double latitude, const double longitude);
    void setBottomRightNode(NodeHandle *value);
    void setTopRightNode(NodeHandle *value);
    void setTopRightNode(const double latitude, const double longitude);
    void setTopLeftNode(NodeHandle *value);

    void setCenterNode(const double latitude, const double longitude);

    NodeHandle *getBottomLeftNode() const;
    NodeHandle *getBottomRightNode() const;
    NodeHandle *getTopRightNode() const;
    NodeHandle *getTopLeftNode() const;
    NodeHandle *getCenterNode() const;

    void setGMSiteConfig(SiteConfig *value);
    void setVisualizationWidget(VisualizationWidget *value);

    size_t getNumDivisionsHoriz() const;
    void setNumDivisionsHoriz(const size_t &value);

    size_t getNumDivisionsVertical() const;
    void setNumDivisionsVertical(const size_t &value);

    void clearGrid();
    void createGrid();

    QVector<GridNode *> getGridNodeVec() const;

private slots:
    void handleBottomLeftCornerChanged(const QPointF& pos);
    void handleBottomRightCornerChanged(const QPointF& pos);
    void handleTopLeftCornerChanged(const QPointF& pos);
    void handleTopRightCornerChanged(const QPointF& pos);
    void handleCenterNodeChanged(const QPointF& pos);
    void handleLatLonChanged(void);
    void handleGridDivisionsChanged(void);

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

    void updateGeometry(void);

signals:
    void geometryChanged();

private:
    QColor color;
    QRect rectangleGeometry;
    bool changingDimensions;
    bool updateConnectedWidgets;

    size_t numDivisionsHoriz;
    size_t numDivisionsVertical;

    NodeHandle* bottomLeftNode;
    NodeHandle* bottomRightNode;
    NodeHandle* topRightNode;
    NodeHandle* topLeftNode;
    NodeHandle* centerNode;

    SiteConfig* GMSiteConfig;
    VisualizationWidget* theVisWidget;

    QVector<GridNode*> gridNodeVec;

    double latMin;
    double lonMin;
    double latMax;
    double lonMax;

};

#endif // RectangleGrid_H
