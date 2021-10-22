#ifndef ArcGISHurricaneSelectionWidget_H
#define ArcGISHurricaneSelectionWidget_H
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
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
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

#include "HurricaneSelectionWidget.h"
#include "EmbeddedMapViewWidget.h"
#include "ArcGISHurricanePreprocessor.h"
#include "WindFieldStation.h"

#include <memory>

#include <QProcess>
#include <QMap>

class VisualizationWidget;

namespace Esri
{
namespace ArcGISRuntime
{
class Feature;
class FeatureCollectionLayer;
class FeatureCollectionTable;
class SimpleRenderer;
}
}


class ArcGISHurricaneSelectionWidget : public HurricaneSelectionWidget
{
    Q_OBJECT

public:
    ArcGISHurricaneSelectionWidget(VisualizationWidget* visWidget, QWidget *parent = nullptr);
    ~ArcGISHurricaneSelectionWidget();

    QStackedWidget* getArcGISHurricaneSelectionWidget(void);


    void clear(void);

    void createHurricaneVisuals(HurricaneObject* hurricane);

    int loadResults(const QString& outputDir);

public slots:
    // Handle the area selection for track truncation
    void handleSelectAreaMap(void);
    void handleClearSelectAreaMap(void);
    void handleAreaSelected(void);

    int importHurricaneTrackData(const QString &eventFile, QString &err);

private slots:

    void handleHurricaneSelect(void);
    void handleGridSelected(void);
    void handleLandfallPointSelected(void);
    void clearGridFromMap(void);
    void clearLandfallFromMap(void);

    void handleTerrainImport(void);

signals:
    void loadingComplete(const bool value);
    void outputDirectoryPathChanged(QString motionDir, QString eventFile);

private:

    std::unique_ptr<ArcGISHurricanePreprocessor> hurricaneImportTool;
    ArcGISVisualizationWidget* theVisualizationWidget;

    Esri::ArcGISRuntime::FeatureCollectionLayer* gridLayer;

    QMap<QString,WindFieldStation> stationMap;

    Esri::ArcGISRuntime::Feature* selectedHurricaneFeature;
    Esri::ArcGISRuntime::GroupLayer* selectedHurricaneLayer;
    LayerTreeItem* selectedHurricaneItem;
    LayerTreeItem* landfallItem;
    LayerTreeItem* hurricaneTrackItem;
    LayerTreeItem* hurricaneTrackPointsItem;

};

#endif // ArcGISHurricaneSelectionWidget_H
