#ifndef GasPipelineInputWidget_H
#define GasPipelineInputWidget_H

#include "ComponentInputWidget.h"

namespace Esri
{
namespace ArcGISRuntime
{
class ClassBreaksRenderer;
class Renderer;
class SimpleRenderer;
class Feature;
class Geometry;
}
}

class GasPipelineInputWidget : public ComponentInputWidget
{
public:
    GasPipelineInputWidget(QWidget *parent, QString componentType, QString appType = QString());

    int loadComponentVisualization();

    Esri::ArcGISRuntime::Feature* addFeatureToSelectedLayer(QMap<QString, QVariant>& featureAttributes, Esri::ArcGISRuntime::Geometry& geom);
    int removeFeatureFromSelectedLayer(Esri::ArcGISRuntime::Feature* feat);
    Esri::ArcGISRuntime::FeatureCollectionLayer* getSelectedFeatureLayer(void);

    void clear();

private:

    Esri::ArcGISRuntime::Renderer* createPipelineRenderer(void);
    Esri::ArcGISRuntime::Renderer* createSelectedPipelineRenderer(double outlineWidth = 0.0);

    Esri::ArcGISRuntime::FeatureCollectionLayer* selectedFeaturesLayer = nullptr;
    Esri::ArcGISRuntime::FeatureCollectionTable* selectedFeaturesTable = nullptr;
};

#endif // GasPipelineInputWidget_H
