#ifndef GridNode_H
#define GridNode_H

#include <QGraphicsItem>
#include <QObject>

class GridNode : public QObject, public QGraphicsItem
{
    Q_OBJECT
public:
    GridNode(QGraphicsItem *parent);

    enum { Type = UserType + 1 };
    int type() const override { return Type; }

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void setXPos(const std::function<double(void)> &value);
    void setYPos(const std::function<double(void)> &value);

    double getXPos(void);
    double getYPos(void);

    QPointF getPoint(void);

public slots:
    void updateGeometry(void);

private:
    QRectF nodeGeometry;
    QColor nodeColor;

    std::function<double(void)> xPos;
    std::function<double(void)> yPos;
};


#endif // GridNode_H
