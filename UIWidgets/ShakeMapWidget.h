#ifndef SHAKEMAPWIDGET_H
#define SHAKEMAPWIDGET_H

#include <memory>
#include <SimCenterAppWidget.h>

#include <QMap>

class VisualizationWidget;

class QStackedWidget;
class QLineEdit;
class QProgressBar;
class QLabel;

namespace Esri
{
namespace ArcGISRuntime
{
class ArcGISMapImageLayer;
class GroupLayer;
class FeatureCollectionLayer;
class KmlLayer;
class Layer;
}
}

struct ShakeMap{

    QString eventName;

    Esri::ArcGISRuntime::GroupLayer* eventLayer = nullptr;
    Esri::ArcGISRuntime::Layer* gridLayer = nullptr;
    Esri::ArcGISRuntime::Layer* faultLayer = nullptr;
    Esri::ArcGISRuntime::Layer* eventKMZLayer = nullptr;
    Esri::ArcGISRuntime::Layer* pgaPolygonLayer = nullptr;
    Esri::ArcGISRuntime::Layer* pgaOverlayLayer = nullptr;
    Esri::ArcGISRuntime::Layer* pgaContourLayer = nullptr;
    Esri::ArcGISRuntime::Layer* epicenterLayer = nullptr;

    inline std::vector<Esri::ArcGISRuntime::Layer*> getAllActiveSubLayers(void)
    {
        std::vector<Esri::ArcGISRuntime::Layer*> layers;

        if(gridLayer)
            layers.push_back(gridLayer);
        if(faultLayer)
            layers.push_back(faultLayer);
        if(eventKMZLayer)
            layers.push_back(eventKMZLayer);
        if(pgaPolygonLayer)
            layers.push_back(pgaPolygonLayer);
        if(pgaOverlayLayer)
            layers.push_back(pgaOverlayLayer);
        if(pgaContourLayer)
            layers.push_back(pgaContourLayer);
        if(epicenterLayer)
            layers.push_back(epicenterLayer);

        return layers;
    }

};

class ShakeMapWidget : public SimCenterAppWidget
{
    Q_OBJECT

public:
    ShakeMapWidget(VisualizationWidget* visWidget);
    ~ShakeMapWidget();

    void showShakeMapLayers(bool state);

public slots:

    void showLoadShakeMapDialog(void);

private slots:

    void loadShakeMapData(void);
    void chooseShakeMapDirectoryDialog(void);

private:

    std::unique_ptr<QStackedWidget> loadShakeMapStackedWidget;

    VisualizationWidget* theVisualizationWidget;
    QLineEdit *shakeMapDirectoryLineEdit;
    QLabel* progressLabel;
    QWidget* progressBarWidget;
    QWidget* directoryInputWidget;
    QProgressBar* progressBar;
    QString pathToShakeMapDirectory;

    QMap<QString,ShakeMap*> shakeMapContainer;

};

#endif // SHAKEMAPWIDGET_H
