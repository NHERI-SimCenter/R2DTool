#include "SimCenterMapcanvasWidget.h"

#include "qgsmapcanvas.h"

#include <QVBoxLayout>

SimCenterMapcanvasWidget::SimCenterMapcanvasWidget(const QString &name, QgsMapCanvas *mainMapCanvas) : mainCanvas(mainMapCanvas)
{
    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    setWindowTitle(name);
    thisMapCanvas = new QgsMapCanvas(this);

    thisMapCanvas->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    this->setAcceptDrops(true);
    thisMapCanvas->setAcceptDrops(true);

    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    mainLayout->addWidget(thisMapCanvas);
}


QgsMapCanvas *SimCenterMapcanvasWidget::mapCanvas()
{
    return thisMapCanvas;
}


void SimCenterMapcanvasWidget::resizeEvent( QResizeEvent * e)
{
    QWidget::resizeEvent(e);
}


void SimCenterMapcanvasWidget::showEvent(QShowEvent* e)
{
    auto sizeCanvas = thisMapCanvas->size();

    this->resize(sizeCanvas);
}


void SimCenterMapcanvasWidget::showLabels( bool show )
{
    QgsMapSettings::Flags flags = thisMapCanvas->mapSettings().flags();
    if ( show )
        flags = flags | QgsMapSettings::DrawLabeling;
    else
        flags = flags & ~QgsMapSettings::DrawLabeling;
    thisMapCanvas->setMapSettingsFlags( flags );
}


void SimCenterMapcanvasWidget::showAnnotations(bool show)
{
    thisMapCanvas->setAnnotationsVisible(show);
}

QgsMapCanvas *SimCenterMapcanvasWidget::getMainCanvas() const
{
    return mainCanvas;
}


