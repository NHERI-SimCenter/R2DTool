#ifndef RectangleGrid_H
#define RectangleGrid_H

#include <QGraphicsItem>
#include <QObject>

class NodeHandle;
class GridNode;
class SiteConfig;
class VisualizationWidget;
//class RuptureWidget;

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
//    void setRuptureWidget(RuptureWidget *value);

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
//    void handleRuptureLocationChanged(void);

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
//    RuptureWidget* theRuptureWidget;

    QVector<GridNode*> gridNodeVec;

    double latMin;
    double lonMin;
    double latMax;
    double lonMax;

};

#endif // RectangleGrid_H
