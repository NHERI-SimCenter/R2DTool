#ifndef CustomGraphicsView_H
#define CustomGraphicsView_H

#include "MapGraphicsView.h"

class CustomGraphicsScene;

class CustomGraphicsView : public Esri::ArcGISRuntime::MapGraphicsView
{
  Q_OBJECT

public:
  explicit CustomGraphicsView(QWidget* parent = nullptr);
  ~CustomGraphicsView();

protected:
  virtual void drawRequestedEvent() Q_DECL_OVERRIDE;

  QObject* qObjectPointer() const Q_DECL_OVERRIDE;

private:
  Q_DISABLE_COPY(CustomGraphicsView)

  friend class CustomGraphicsScene;
  void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;
  void wheelEvent(QWheelEvent* event) Q_DECL_OVERRIDE;
  void setSceneRect(qreal x, qreal y, qreal w, qreal h);
  void makeContextCurrent();
  void initialize();
  std::unique_ptr<CustomGraphicsScene> m_scene;
};

#endif // CustomGraphicsView_H
