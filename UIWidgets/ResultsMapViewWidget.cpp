#include "ResultsMapViewWidget.h"
#include "SimCenterMapGraphicsView.h"

#include <QGraphicsSimpleTextItem>
#include <QDebug>

ResultsMapViewWidget::ResultsMapViewWidget(QWidget* parent) : QWidget(parent)
{
    theViewLayout = new QVBoxLayout();

    this->setAcceptDrops(true);
    this->setObjectName("MapSubwindow");

    this->setLayout(theViewLayout);

    theNewView = SimCenterMapGraphicsView::getInstance();
    theNewView->setAcceptDrops(true);
    //theNewView->setObjectName("MapSubwindow");

    theNewView->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    theNewView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    theNewView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    theNewView->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    theNewView->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
}


void ResultsMapViewWidget::setCurrentlyViewable(bool status)
{
    if (status == true)
        theNewView->setCurrentLayout(theViewLayout);
    else {
        this->hide();
    }
}


void ResultsMapViewWidget::resizeParent(QRectF rect)
{
    auto width = rect.width();
    auto height = rect.height();

    theNewView->setMaximumWidth(width);
    theNewView->setMaximumHeight(height);

    theNewView->resize(width,height);
}





