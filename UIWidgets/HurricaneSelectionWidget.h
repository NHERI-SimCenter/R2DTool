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
#include "HurricanePreprocessor.h"

#include <memory>

#include <QMap>

class VisualizationWidget;
class SiteConfig;
class SiteGrid;
class HurricaneParameterWidget;

class QStackedWidget;
class QLineEdit;
class QProgressBar;
class QPushButton;
class QLabel;


class HurricaneSelectionWidget : public SimCenterAppWidget
{
    Q_OBJECT

public:
    HurricaneSelectionWidget(VisualizationWidget* visWidget, QWidget *parent = nullptr);
    ~HurricaneSelectionWidget();

    void showUserGMLayers(bool state);

    QStackedWidget* getHurricaneSelectionWidget(void);

    bool outputToJSON(QJsonObject &jsonObj);
    bool inputAppDataFromJSON(QJsonObject &jsonObj);
    bool outputAppDataToJSON(QJsonObject &jsonObj);

    void clear(void);

    void setCurrentlyViewable(bool status);

public slots:

    void showHurricaneSelectDialog(void);

private slots:

    void handleHurricaneSelect(void);
    void loadHurricaneTrackData(void);
    void loadHurricaneButtonClicked(void);
    void showGridOnMap(void);
    void handleGridSelected(void);
    void clearGridFromMap(void);

signals:
    void loadingComplete(const bool value);

private:

    std::unique_ptr<QStackedWidget> theStackedWidget;
    std::unique_ptr<EmbeddedMapViewWidget> mapViewSubWidget;
    std::unique_ptr<HurricanePreprocessor> hurricaneImportTool;
    HurricaneParameterWidget* hurricaneParamsWidget;
    VisualizationWidget* theVisualizationWidget;

    QString eventDatabaseFile;

    QLabel* selectedHurricaneName;
    QLabel* selectedHurricaneSID;
    QLabel* selectedHurricaneSeason;

    QLabel* progressLabel;
    QWidget* progressBarWidget;
    QWidget* fileInputWidget;
    QProgressBar* progressBar;
    SiteConfig* siteConfig;
    SiteGrid* siteGrid;

    QStackedWidget* typeOfScenarioWidget;
    QWidget* selectHurricaneWidget;
    QWidget* specifyHurricaneWidget;
    QPushButton* loadDbButton;
};

#endif // HurricaneSelectionWidget_H
