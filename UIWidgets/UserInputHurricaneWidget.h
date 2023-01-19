#ifndef UserInputHurricaneWidget_H
#define UserInputHurricaneWidget_H
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

#include "SimCenterAppWidget.h"

#include <memory>

#include <QMap>

class VisualizationWidget;
class SimCenterUnitsWidget;

class QStackedWidget;
class QLineEdit;
class QProgressBar;
class QLabel;

#ifdef Arc_GIS
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
#endif

class UserInputHurricaneWidget : public SimCenterAppWidget
{
    Q_OBJECT

public:
    UserInputHurricaneWidget(VisualizationWidget* visWidget, QWidget *parent = nullptr);
    ~UserInputHurricaneWidget();

    QStackedWidget* getUserInputHurricaneWidget(void);

    bool inputFromJSON(QJsonObject &jsonObject);
    bool outputToJSON(QJsonObject &jsonObj);
    bool inputAppDataFromJSON(QJsonObject &jsonObj);
    bool outputAppDataToJSON(QJsonObject &jsonObj);
    bool copyFiles(QString &destDir);
    void clear(void);

public slots:

    void showEventSelectDialog(void);

private slots:

    void loadUserWFData(void);
    void loadHurricaneTrackData(void);
    void chooseEventFileDialog(void);
    void chooseEventDirDialog(void);

signals:
    void outputDirectoryPathChanged(QString eventDir, QString eventFile);
    void loadingComplete(const bool value);
    void eventTypeChangedSignal(QString eventType);

private:

    void showProgressBar(void);
    void hideProgressBar(void);

    QStackedWidget* theStackedWidget;

    VisualizationWidget* theVisualizationWidget;

    QString eventFile;
    QString eventDir;

    QLineEdit *eventFileLineEdit;
    QLineEdit *eventDirLineEdit;

    QLabel* progressLabel;
    QWidget* progressBarWidget;
    QWidget* fileInputWidget;
    QProgressBar* progressBar;

    SimCenterUnitsWidget* unitsWidget;

};

#endif // UserInputHurricaneWidget_H
