#include "SimCenterMapcanvasWidget.h"
#include "QGISVisualizationWidget.h"

#include <qgsmapcanvas.h>
#include <qgsvectorlayer.h>
#include <qgsappmaptools.h>
#include <qgsmaptool.h>

#include <QVBoxLayout>

SimCenterMapcanvasWidget::SimCenterMapcanvasWidget(const QString &name, QGISVisualizationWidget *mainVisWidget) : theVisualizationWidget(mainVisWidget)
{
    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    setWindowTitle(name);
    thisMapCanvas = new QgsMapCanvas();

    mainCanvas = theVisualizationWidget->getMainCanvas();

    thisMapCanvas->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    this->setAcceptDrops(true);
    thisMapCanvas->setAcceptDrops(true);

    auto mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    mainLayout->addWidget(thisMapCanvas);

    mMapTools = std::make_unique<QgsAppMapTools>(thisMapCanvas, nullptr);
}


void SimCenterMapcanvasWidget::setMapTool(QgsMapTool *mapTool)
{
    thisMapCanvas->setMapTool(mapTool);
}


QgsMapCanvas *SimCenterMapcanvasWidget::mapCanvas()
{
    return thisMapCanvas;
}


void SimCenterMapcanvasWidget::showLabels(bool show)
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


void SimCenterMapcanvasWidget::clear(void)
{
    currentLayer = nullptr;
    selectedIds.clear();
    deselectedIds.clear();
}


QgsFeatureIds SimCenterMapcanvasWidget::getSelectedIds() const
{
    return selectedIds;
}


QgsMapCanvas *SimCenterMapcanvasWidget::getMainCanvas() const
{
    return mainCanvas;
}


void SimCenterMapcanvasWidget::enableSelectionTool(void)
{
    auto selectTool = mMapTools->mapTool(QgsAppMapTools::SelectFeatures);
    thisMapCanvas->setMapTool(selectTool);
    thisMapCanvas->unsetCursor();
}


void SimCenterMapcanvasWidget::enablePolygonSelectionTool(void)
{
    auto selectTool = mMapTools->mapTool(QgsAppMapTools::SelectPolygon);
    thisMapCanvas->setMapTool(selectTool);
}


void SimCenterMapcanvasWidget::enablePanTool(void)
{
    thisMapCanvas->setMapTool(mMapTools->mapTool(QgsAppMapTools::Pan));
}


void SimCenterMapcanvasWidget::setCurrentLayer(QgsVectorLayer* layer)
{
    if(currentLayer != nullptr)
        disconnect(currentLayer,&QgsVectorLayer::selectionChanged,this, &SimCenterMapcanvasWidget::selectionChanged);

    currentLayer = layer;
    thisMapCanvas->setCurrentLayer(currentLayer);

    if(layer != nullptr)
        connect(currentLayer,&QgsVectorLayer::selectionChanged,this, &SimCenterMapcanvasWidget::selectionChanged);
}


void SimCenterMapcanvasWidget::selectionChanged(const QgsFeatureIds &selected, const QgsFeatureIds &deselected, bool clearAndSelect)
{
    Q_UNUSED(clearAndSelect);

    if(!selected.isEmpty())
    {
        auto isVectorLayer = dynamic_cast<QgsVectorLayer*>(QObject::sender());

        if(isVectorLayer)
        {
            auto featureIt = isVectorLayer->getFeatures(selected);

            theVisualizationWidget->showFeaturePopUp(featureIt);
        }
    }

    selectedIds = selected;
    deselectedIds = deselected;
}



