#ifndef ResultsMapViewWidget_H
#define ResultsMapViewWidget_H

#include <QObject>

#include "MapGraphicsView.h"

class ResultsMapViewWidget : public Esri::ArcGISRuntime::MapGraphicsView
{
    Q_OBJECT
public:
    ResultsMapViewWidget(QWidget* parent, MapGraphicsView* mainView);

signals:

    void mouseClick(QPoint pos);

protected:

    // Custom zoom implementation to get around a bug in the wheel event causing zoom to occur only in one direction
    void wheelEvent(QWheelEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;

    // Override widget events
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private:

    Esri::ArcGISRuntime::MapGraphicsView* mainViewWidget;

    double zoomFactor;
    bool m_initDraw = false;
};

#endif // ResultsMapViewWidget_H
