
#include "CustomGraphicsView.h"
#include "Scene.h"
#include "CustomGraphicsScene.h"

#include <QResizeEvent>
#include <QSurfaceFormat>
#include <QOpenGLWidget>
#include <QApplication>

using namespace Esri::ArcGISRuntime;

CustomGraphicsView::CustomGraphicsView(QWidget* parent) :
  MapGraphicsView(parent)
{
  initialize();
}


CustomGraphicsView::~CustomGraphicsView()
{
}

void CustomGraphicsView::initialize()
{
  QSurfaceFormat surfaceFormat = QSurfaceFormat::defaultFormat();
  surfaceFormat.setSwapBehavior(QSurfaceFormat::DoubleBuffer);

  QOpenGLWidget* glWidget = new QOpenGLWidget();
  glWidget->setFormat(surfaceFormat);
  glWidget->setAttribute(Qt::WA_AcceptTouchEvents);

  setViewport(glWidget);
  setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

  m_scene = std::unique_ptr<CustomGraphicsScene>(new CustomGraphicsScene(this));
  QGraphicsView::setScene(m_scene.get());
}

void CustomGraphicsView::setSceneRect(qreal x, qreal y, qreal w, qreal h)
{
  return m_scene->setSceneRect(x, y, w, h);
}

void CustomGraphicsView::resizeEvent(QResizeEvent* event)
{
  QGraphicsView::resizeEvent(event);
  m_scene->setSceneRect(QRectF(QPoint(0, 0), event->size()));
}

void CustomGraphicsView::wheelEvent(QWheelEvent* event)
{
  event->ignore();

  CustomGraphicsSceneWheelEvent wheelEvent(QEvent::GraphicsSceneWheel);
  wheelEvent.setWidget(viewport());
  wheelEvent.setScenePos(mapToScene(event->pos()));
  wheelEvent.setScreenPos(event->globalPos());
  wheelEvent.setButtons(event->buttons());
  wheelEvent.setModifiers(event->modifiers());
  wheelEvent.setDelta(event->delta());
  wheelEvent.setAngleDelta(event->angleDelta()); // set the angle delta
  wheelEvent.setOrientation(event->orientation());
  wheelEvent.setAccepted(false);
  QApplication::sendEvent(m_scene.get(), &wheelEvent);
  event->setAccepted(wheelEvent.isAccepted());
  if (!event->isAccepted())
    QAbstractScrollArea::wheelEvent(event);
}

void CustomGraphicsView::drawRequestedEvent()
{
  m_scene->update();
}

QObject* CustomGraphicsView::qObjectPointer() const
{
  return const_cast<CustomGraphicsView*>(this);
}
