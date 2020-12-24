#ifndef ResultsMapViewWidget_H
#define ResultsMapViewWidget_H

#include <QObject>
#include <QWidget>

class SimCenterMapGraphicsView;
class QGraphicsSimpleTextItem;
class QVBoxLayout;

class ResultsMapViewWidget : public QWidget
{
    Q_OBJECT
public:
    ResultsMapViewWidget(QWidget* parent);

    void setCurrentlyViewable(bool status);

public slots:

    void resizeParent(QRectF rect);

private:

    QGraphicsSimpleTextItem* displayText;

    SimCenterMapGraphicsView *theNewView;
    QVBoxLayout *theViewLayout;
};

#endif // ResultsMapViewWidget_H
