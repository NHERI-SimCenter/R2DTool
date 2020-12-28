#ifndef SHAKEMAPWIDGET_H
#define SHAKEMAPWIDGET_H
/* *****************************************************************************
Copyright (c) 2016-2021, The Regents of the University of California (Regents).
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the FreeBSD Project.

REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS
PROVIDED "AS IS". REGENTS HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT,
UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

*************************************************************************** */

// Written by: Stevan Gavrilovic

#include "SimCenterAppWidget.h"

#include <QMap>

#include <memory>

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
    ShakeMapWidget(VisualizationWidget* visWidget, QWidget *parent = nullptr);
    ~ShakeMapWidget();

    void showShakeMapLayers(bool state);

    QStackedWidget* getShakeMapWidget(void);

public slots:

    void showLoadShakeMapDialog(void);

private slots:

    void loadShakeMapData(void);
    void chooseShakeMapDirectoryDialog(void);

signals:

    void loadingComplete(const bool value);

private:

    std::unique_ptr<QStackedWidget> shakeMapStackedWidget;

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
