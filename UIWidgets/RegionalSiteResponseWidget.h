#ifndef RegionalSiteResponseWidget_H
#define RegionalSiteResponseWidget_H
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

// Written by: Stevan Gavrilovic, Frank McKenna

#include "GroundMotionStation.h"
#include "SimCenterAppWidget.h"
#include "SiteConfig.h"

#include <memory>

#include <QMap>
#include <QProcess>

class VisualizationWidget;
class SimCenterUnitsWidget;

class ComponentInputWidget;
class QStackedWidget;
class QLineEdit;
class QProgressBar;
class QLabel;
class QProcess;
class QPushButton;

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

class SiteConfig;
class SiteConfigWidget;
class MapViewWindow;
class RectangleGrid;
class Vs30; // vs30 info
class Vs30Widget; // vs30 setup widget
class BedrockDepth;
class BedrockDepthWidget;
class SoilModel;
class SoilModelWidget;


class RegionalSiteResponseWidget : public SimCenterAppWidget
{
    Q_OBJECT

public:
    RegionalSiteResponseWidget(VisualizationWidget* visWidget, QWidget *parent = nullptr);
    ~RegionalSiteResponseWidget();

    void showUserGMLayers(bool state);

    QStackedWidget* getRegionalSiteResponseWidget(void);
    // get sites and fetch needed data
    QStackedWidget* getSiteWidget(VisualizationWidget* visWidget);

    bool inputFromJSON(QJsonObject &jsonObj);  
    bool outputToJSON(QJsonObject &jsonObj);
    bool inputAppDataFromJSON(QJsonObject &jsonObj);
    bool outputAppDataToJSON(QJsonObject &jsonObj);
  
    bool copyFiles(QString &destDir);

    void clear(void);

#ifdef ARC_GIS
    void setCurrentlyViewable(bool status);
#endif

public slots:

    void showUserGMSelectDialog(void);
    void showGISWindow(void);

    void handleProcessStarted(void);
    void handleProcessTextOutput(void);
    void handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

    void setSiteDataFile(bool flag);
    void setSoilModelWidget(SiteConfig::SiteType siteType);
    void activateSoilModelWidget(bool flag);

private slots:

    void loadUserGMData(void);
    void chooseEventFileDialog(void);
    void chooseMotionDirDialog(void);
    void soilParamaterFileDialog(void);
    void soilScriptFileDialog(void);

signals:
    void eventTypeChangedSignal(QString eventType);
    void outputDirectoryPathChanged(QString motionDir, QString eventFile);
    void loadingComplete(const bool value);
    void writeSiteDataCsv(bool);
    void siteFilterSignal(QString filter);

private:

  void showProgressBar(void);
  void hideProgressBar(void);
  void setFilterString(const QString& filter);
  void getSiteData(void); // invoke regionalGroundMotion tool to fetch Vs30 and DepthToRock
  void setDir(void); // set directories up
  QString getFilterString(void);
  
  QStackedWidget* theStackedWidget;
  QStackedWidget* theSiteStackedWidget;

    VisualizationWidget* theVisualizationWidget;

    QString eventFile;
    QString motionDir;

    QString inputSiteDataDir; // the input site data dir (from user-specified or grid generator)
    QString outputSiteDataDir; // the output site data dir with fetched vs30, depth to rock, and modeling parameters

    QLineEdit *eventFileLineEdit;
    QLineEdit *motionDirLineEdit;
    QLineEdit *soilFileLineEdit;
    QLineEdit *siteResponseScriptLineEdit;
    QLineEdit *filterLineEdit;

    QLabel* progressLabel;
    QWidget* progressBarWidget;
    QWidget* inputWidget;
    QProgressBar* progressBar;

    QVector<GroundMotionStation> stationList;

    SimCenterUnitsWidget* unitsWidget;
    SimCenterUnitsWidget* unitsSiteWidget;
  
    ComponentInputWidget *theInputMotions;

    QWidget* inputSiteWidget;

    SiteConfig* m_siteConfig;
    SiteConfigWidget* m_siteConfigWidget;

    Vs30* m_vs30;
    Vs30Widget* m_vs30Widget;

    BedrockDepth* m_bedrockDepth;
    BedrockDepthWidget* m_bedrockDepthWidget;

    SoilModel* m_soilModel;
    SoilModelWidget* m_soilModelWidget;

    QPushButton* m_runButton;
    QProcess* processSiteData;

    bool siteDataFlag;
    bool soilModelFlag;

#ifdef ARC_GIS
    std::unique_ptr<MapViewSubWidget> mapViewSubWidget;
#endif

#ifdef Q_GIS
    MapViewWindow* mapViewSubWidget;
    RectangleGrid* userGrid;
#endif
};

#endif // RegionalSiteResponseWidget_H
