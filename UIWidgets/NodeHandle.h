#ifndef NODEHANDLE_H
#define NODEHANDLE_H

#include <QGraphicsItem>
#include <QObject>

class NodeHandle : public QObject, public QGraphicsItem
{
    Q_OBJECT
public:
    NodeHandle(QGraphicsItem *parent);

    enum { Type = UserType + 1 };
    int type() const override { return Type; }

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void setColor(const QColor &value);
    void setDiameter(const double diameter);

signals:
    void positionChanged(const QPointF& pos);

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
//    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

private:
    QRectF nodeGeometry;
    QColor nodeColor;
    double diameter;
};


#endif // NODEHANDLE_H
