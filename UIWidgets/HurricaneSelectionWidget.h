#ifndef HurricaneSelectionWidget_H
#define HurricaneSelectionWidget_H
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

#include "SimCenterAppWidget.h"
#include "EmbeddedMapViewWidget.h"
#include "WindFieldStation.h"
#include "HurricaneObject.h"

#include <memory>

#include <QProcess>
#include <QMap>

class VisualizationWidget;
class HurricaneParameterWidget;
class SiteGrid;

class QStackedWidget;
class QLineEdit;
class QProgressBar;
class QPushButton;
class QLabel;
class QProcess;
class QSpinBox;

class HurricaneSelectionWidget : public SimCenterAppWidget
{
    Q_OBJECT

public:
    HurricaneSelectionWidget(VisualizationWidget* visWidget, QWidget *parent = nullptr);
    ~HurricaneSelectionWidget();

    QStackedWidget* getHurricaneSelectionWidget(void);

    bool outputToJSON(QJsonObject &jsonObj);
    bool inputAppDataFromJSON(QJsonObject &jsonObj);
    bool outputAppDataToJSON(QJsonObject &jsonObj);

    void clear(void);

    void setCurrentlyViewable(bool status);

    int loadResults(const QString& outputDir);

    virtual void createHurricaneVisuals(HurricaneObject* hurricane) = 0;

    virtual int importHurricaneTrackData(const QString &eventFile, QString &err) = 0;

public slots:

    void showHurricaneSelectDialog(void);

    // Handles the results when the user is finished
    void handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

    // Brings up the dialog and tells the user that the process has started
    void handleProcessStarted(void);

    // Displays the text output of the process in the dialog
    void handleProcessTextOutput(void);

private slots:

    void runHazardSimulation(void);
    void handleHurricaneTrackImport(void);
    void loadHurricaneTrackData(void);
    void loadHurricaneButtonClicked(void);
    void showGridOnMap(void);
    void showPointOnMap(void);
    virtual void handleHurricaneSelect(void) = 0;
    virtual void clearGridFromMap(void) = 0;
    virtual void handleGridSelected(void) = 0;
    virtual void handleLandfallPointSelected(void) = 0;
    virtual void clearLandfallFromMap(void) = 0;

signals:
    void loadingComplete(const bool value);
    void outputDirectoryPathChanged(QString motionDir, QString eventFile);

protected:

    QJsonArray getTerrainData(void);

    QPushButton* truncTrackSelectButton = nullptr;
    QPushButton* truncTrackApplyButton = nullptr;
    QPushButton* truncTrackClearButton = nullptr;
    QPushButton* browseTerrainButton = nullptr;

    QLabel* selectedHurricaneName;
    QLabel* selectedHurricaneSID;
    QLabel* selectedHurricaneSeason;

    QVector<QStringList> gridData;

    HurricaneObject selectedHurricaneObj;
    std::unique_ptr<EmbeddedMapViewWidget> mapViewSubWidget;
    HurricaneParameterWidget* hurricaneParamsWidget;

    QProgressBar* progressBar;

private:

    std::unique_ptr<QStackedWidget> theStackedWidget;

    QString eventDatabaseFile;

    QLabel* progressLabel;
    QWidget* progressBarWidget;
    QWidget* fileInputWidget;
    SiteConfig* siteConfig;
    SiteGrid* siteGrid;
    QLineEdit* numIMsLineEdit;
    QLineEdit* trackLineEdit;
    QSpinBox* divLatSpinBox;
    QSpinBox* divLonSpinBox;

    QStackedWidget* typeOfScenarioWidget;
    QWidget* selectHurricaneWidget;
    QWidget* specifyHurricaneWidget;
    QPushButton* loadDbButton;
    QLineEdit* terrainLineEdit;

    QMap<QString,WindFieldStation> stationMap;

    QProcess* process;
    QPushButton* runButton;
};

#endif // HurricaneSelectionWidget_H
