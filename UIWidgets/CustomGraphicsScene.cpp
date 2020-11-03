#include "CustomGraphicsScene.h"
#include "CustomGraphicsView.h"
#include <QOpenGLWidget>
#include <QOpenGLContext>
#include <QPaintEngine>
#include <QOpenGLFunctions>

using namespace Esri::ArcGISRuntime;

CustomGraphicsScene::CustomGraphicsScene(CustomGraphicsView *graphicsView) :
  m_graphicsView(graphicsView)
{
}

CustomGraphicsScene::~CustomGraphicsScene()
{
  cleanup();
}

void CustomGraphicsScene::cleanup()
{
  m_graphicsView->pauseAndRecycleResources();
}

void CustomGraphicsScene::setSceneRect(qreal x, qreal y, qreal w, qreal h)
{
  if (!m_graphicsView)
    return;

  QRectF oldRect = sceneRect();
  QGraphicsScene::setSceneRect(x, y, w, h);
  QRectF newRect = sceneRect();

  m_graphicsView->sendGeoViewRectChangedEvent(QRectF(x, y, w, h));

  if (newRect != oldRect)
  {
    int newWidth = width();
    int newHeight = height();
    if (newWidth > 0 && newHeight > 0)
    {
      m_graphicsView->resizeView(static_cast<int>(newWidth), static_cast<int>(newHeight));
    }
  }
}

void CustomGraphicsScene::drawBackground(QPainter *painter, const QRectF &rect)
{
  Q_UNUSED(rect)

  if (!m_initDraw)
  {
    m_initDraw = true;

    // Connect to the context's aboutToBeDestroyed signal to cleanup the map's resources.
    connect(static_cast<QOpenGLWidget*>(m_graphicsView->viewport())->context(), SIGNAL(aboutToBeDestroyed()), this, SLOT(cleanup()), Qt::DirectConnection);

    m_graphicsView->resume();
  }

  // OpenGL
  QPaintEngine::Type type = painter->paintEngine()->type();
  if (!painter                  ||
      !painter->isActive()      ||
      (type != QPaintEngine::OpenGL2 && type != QPaintEngine::OpenGL))
    return;

  //all OpenGL code should go here...
  painter->beginNativePainting();

  if (m_graphicsView->isReadyToDraw())
  {
    // workaround ArcGIS Runtime Qt bug which requires blending to be enabled before
    // each draw request
    QOpenGLFunctions* functions = QOpenGLContext::currentContext()->functions();
    functions->glEnable(GL_BLEND);

    m_graphicsView->draw();
  }

  painter->endNativePainting();
}

void CustomGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  Q_ASSERT(event);
  QGraphicsScene::mousePressEvent(event);

  QMouseEvent pressEvent(QEvent::MouseButtonPress,
                         event->scenePos().toPoint(),
                         event->button(),
                         event->buttons(),
                         event->modifiers());

  m_graphicsView->sendGeoViewMousePressEvent(&pressEvent, event->isAccepted());
} // end mousePressEvent

void CustomGraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
  Q_ASSERT(event);
  QGraphicsScene::mouseMoveEvent(event);

  QMouseEvent moveEvent(QEvent::MouseMove,
                        event->scenePos().toPoint(),
                        event->button(),
                        event->buttons(),
                        event->modifiers());

  m_graphicsView->sendGeoViewMouseMoveEvent(&moveEvent, event->isAccepted());
} // end mouseMoveEvent

void CustomGraphicsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  Q_ASSERT(event);
  QGraphicsScene::mouseReleaseEvent(event);

  QMouseEvent releaseEvent(QEvent::MouseButtonRelease,
                           event->scenePos().toPoint(),
                           event->button(),
                           event->buttons(),
                           event->modifiers());

  m_graphicsView->sendGeoViewMouseReleaseEvent(&releaseEvent, event->isAccepted());
} // end mouseReleaseEvent

void CustomGraphicsScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
  Q_ASSERT(event);
  QGraphicsScene::mouseDoubleClickEvent(event);

  QMouseEvent doubleClickEvent(QEvent::MouseButtonDblClick,
                               event->scenePos().toPoint(),
                               event->button(),
                               event->buttons(),
                               event->modifiers());


  m_graphicsView->sendGeoViewMouseDoubleClickEvent(&doubleClickEvent, event->isAccepted());
} // end mouseDoubleClickEvent

void CustomGraphicsScene::wheelEvent(QGraphicsSceneWheelEvent *event)
{
  Q_ASSERT(event);
  QGraphicsScene::wheelEvent(event);

  CustomGraphicsSceneWheelEvent* gsWheelEvent = static_cast<CustomGraphicsSceneWheelEvent*>(event);
  if (!gsWheelEvent)
    return;

  QWheelEvent wheelEvent(event->scenePos().toPoint(),
                         QPointF(),
                         QPoint(),
                         gsWheelEvent->angleDelta(),
                         event->delta(),
                         event->orientation(),
                         event->buttons(),
                         event->modifiers());

  m_graphicsView->sendGeoViewMouseWheelEvent(&wheelEvent, event->isAccepted());
} // end wheel_event

void CustomGraphicsScene::keyPressEvent(QKeyEvent *event)
{
  Q_ASSERT(event);
  QGraphicsScene::keyPressEvent(event);

  m_graphicsView->sendGeoViewKeyPressEvent(event, event->isAccepted());
} // end keyPressEvent

void CustomGraphicsScene::keyReleaseEvent(QKeyEvent *event)
{
  Q_ASSERT(event);
  QGraphicsScene::keyReleaseEvent(event);

  m_graphicsView->sendGeoViewKeyReleaseEvent(event, event->isAccepted());

} // end keyReleaseEvent

bool CustomGraphicsScene::event(QEvent *event)
{
  Q_ASSERT(event);
  QGraphicsScene::event(event);

  if (event->type() == QEvent::TouchBegin ||
      event->type() == QEvent::TouchUpdate ||
      event->type() == QEvent::TouchEnd)
  {
    QTouchEvent* touchEvent = static_cast<QTouchEvent*>(event);

    return m_graphicsView->sendGeoViewTouchEvent(touchEvent, event->isAccepted());
  }

  return true;

} // end event

void CustomGraphicsScene::setSceneRect(const QRectF &rect)
{
  this->setSceneRect(rect.x(), rect.y(), rect.width(), rect.height());
} // end setSceneRect
