#ifndef BUILDINGINPUTWIDGET_H
#define BUILDINGINPUTWIDGET_H

#include "ComponentInputWidget.h"

namespace Esri
{
namespace ArcGISRuntime
{
class ClassBreaksRenderer;
class SimpleRenderer;
class Feature;
class Geometry;
}
}

class BuildingInputWidget : public ComponentInputWidget
{
public:
    BuildingInputWidget(QWidget *parent, QString componentType, QString appType = QString());

    int loadComponentVisualization();

    Esri::ArcGISRuntime::Feature* addFeatureToSelectedLayer(QMap<QString, QVariant>& featureAttributes, Esri::ArcGISRuntime::Geometry& geom);
    int removeFeatureFromSelectedLayer(Esri::ArcGISRuntime::Feature* feat);
    Esri::ArcGISRuntime::FeatureCollectionLayer* getSelectedFeatureLayer(void);

private:

    Esri::ArcGISRuntime::SimpleRenderer* createBuildingRenderer(void);
    Esri::ArcGISRuntime::ClassBreaksRenderer* createSelectedBuildingRenderer(double outlineWidth = 0.0);

    Esri::ArcGISRuntime::FeatureCollectionLayer* selectedBuildingsLayer = nullptr;
    Esri::ArcGISRuntime::FeatureCollectionTable* selectedBuildingsTable = nullptr;
};

#endif // BUILDINGINPUTWIDGET_H
