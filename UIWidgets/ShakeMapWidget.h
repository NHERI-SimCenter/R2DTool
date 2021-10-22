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
#include "GroundMotionStation.h"

#include <QMap>

#include <memory>

class CustomListWidget;
class VisualizationWidget;

class QStackedWidget;
class QLineEdit;
class QProgressBar;
class QLabel;
class QSplitter;

#ifdef ARC_GIS
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

    QVector<GroundMotionStation> stationList;
};
#endif

#ifdef Q_GIS

class QgsVectorLayer;

struct ShakeMap{

    QString eventName;

    QgsVectorLayer* eventLayer = nullptr;
    QgsVectorLayer* gridLayer = nullptr;
    QgsVectorLayer* faultLayer = nullptr;
    QgsVectorLayer* eventKMZLayer = nullptr;
    QgsVectorLayer* pgaPolygonLayer = nullptr;
    QgsVectorLayer* pgaOverlayLayer = nullptr;
    QgsVectorLayer* pgaContourLayer = nullptr;
    QgsVectorLayer* epicenterLayer = nullptr;

    inline std::vector<QgsVectorLayer*> getAllActiveSubLayers(void)
    {
        std::vector<QgsVectorLayer*> layers;

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

    QVector<GroundMotionStation> stationList;
};
#endif


class ShakeMapWidget : public SimCenterAppWidget
{
    Q_OBJECT

public:
    ShakeMapWidget(VisualizationWidget* visWidget, QWidget *parent = nullptr);
    ~ShakeMapWidget();

    QWidget* getShakeMapWidget(void);

    QStackedWidget* getStackedWidget(void);

    bool outputToJSON(QJsonObject &jsonObject);
    bool inputFromJSON(QJsonObject &jsonObject);
    bool outputAppDataToJSON(QJsonObject &jsonObject);
    bool inputAppDataFromJSON(QJsonObject &jsonObject);
    bool copyFiles(QString &destDir);
  
    void clear();
    int getNumShakeMapsLoaded();

public slots:

    void showLoadShakeMapDialog(void);

private slots:

    int loadShakeMapData(void);
    int loadDataFromDirectory(const QString& dir);
    void chooseShakeMapDirectoryDialog(void);

signals:

    void outputDirectoryPathChanged(QString motionDir, QString eventFile);
    void eventTypeChangedSignal(QString eventType);
    void loadingComplete(const bool value);

private:

    QStackedWidget* shakeMapStackedWidget;

    QStringList shakeMapList;

    CustomListWidget *listWidget;
    VisualizationWidget* theVisualizationWidget;
    QLineEdit *shakeMapDirectoryLineEdit;
    QLabel* progressLabel;
    QWidget* progressBarWidget;
    QWidget* directoryInputWidget;
    QProgressBar* progressBar;
    QString pathToShakeMapDirectory;

    QString motionDir;
    QString pathToEventFile;

    QMap<QString,ShakeMap*> shakeMapContainer;

    QVector<QString> eventsVec;

    bool recursiveCopy(const QString &sourcePath, const QString &destPath);

};

#endif // SHAKEMAPWIDGET_H
