#ifndef CustomGraphicsScene_H
#define CustomGraphicsScene_H

#include <QGraphicsScene>
#include <QGraphicsSceneWheelEvent>

class CustomGraphicsView;

class CustomGraphicsScene: public QGraphicsScene
{
  Q_OBJECT

public:
  CustomGraphicsScene(CustomGraphicsView* graphicsView);
  ~CustomGraphicsScene();

  virtual void setSceneRect(qreal x, qreal y, qreal w, qreal h);
  virtual void setSceneRect(const QRectF &rect);

protected:
  virtual void drawBackground(QPainter *painter, const QRectF &rect) Q_DECL_OVERRIDE;
  virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
  virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
  virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
  virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
  virtual void wheelEvent(QGraphicsSceneWheelEvent *event) Q_DECL_OVERRIDE;
  virtual void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
  virtual void keyReleaseEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
  virtual bool event(QEvent *event) Q_DECL_OVERRIDE;

private slots:
  void cleanup();

private:
  CustomGraphicsView* m_graphicsView;
  bool m_initDraw = false;
};

// need to subclass QGraphicsSceneWheelEvent as it doesn't currently provide an angle delta value
class CustomGraphicsSceneWheelEvent : public QGraphicsSceneWheelEvent
{
public:
  explicit CustomGraphicsSceneWheelEvent(QEvent::Type type) : QGraphicsSceneWheelEvent(type) { }

  void setAngleDelta(const QPoint& angleDelta) { m_angleDelta = angleDelta; }
  QPoint angleDelta() const { return m_angleDelta; }

private:
  QPoint m_angleDelta;
};

#endif // CustomGraphicsScene_H
