#ifndef ArcGISGasPipelineInputWidget_H
#define ArcGISGasPipelineInputWidget_H

#include "ComponentInputWidget.h"

namespace Esri
{
namespace ArcGISRuntime
{
class ClassBreaksRenderer;
class Renderer;
class FeatureCollectionTable;
class SimpleRenderer;
class Feature;
class Geometry;
}
}

class ArcGISGasPipelineInputWidget : public ComponentInputWidget
{
public:
    ArcGISGasPipelineInputWidget(QWidget *parent, QString componentType, QString appType = QString());

    int loadComponentVisualization();

    Esri::ArcGISRuntime::Feature* addFeatureToSelectedLayer(QMap<QString, QVariant>& featureAttributes, Esri::ArcGISRuntime::Geometry& geom);
    int removeFeatureFromSelectedLayer(Esri::ArcGISRuntime::Feature* feat);
    Esri::ArcGISRuntime::FeatureCollectionLayer* getSelectedFeatureLayer(void);

    void clear();

    Esri::ArcGISRuntime::FeatureCollectionLayer *getSelectedFeaturesLayer() const;

private:

    Esri::ArcGISRuntime::Renderer* createPipelineRenderer(void);
    Esri::ArcGISRuntime::Renderer* createSelectedPipelineRenderer(double outlineWidth = 0.0);

    Esri::ArcGISRuntime::FeatureCollectionLayer* selectedFeaturesLayer = nullptr;
    Esri::ArcGISRuntime::FeatureCollectionTable* selectedFeaturesTable = nullptr;
};

#endif // ArcGISGasPipelineInputWidget_H
