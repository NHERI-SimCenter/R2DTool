#ifndef OpenQuakeSelectionWidget_H
#define OpenQuakeSelectionWidget_H
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

#include <qdom.h>

class SimCenterMapcanvasWidget;
class QGISVisualizationWidget;

class QgsMapLayer;
class QgsVectorLayer;
class QgsLayerTreeGroup;

class QDomNodeList;
class QStackedWidget;
class QProgressBar;
class QLabel;
class QLineEdit;

class OpenQuakeSelectionWidget : public SimCenterAppWidget
{
    Q_OBJECT

public:
    OpenQuakeSelectionWidget(QGISVisualizationWidget* visWidget, QWidget *parent = nullptr);
    ~OpenQuakeSelectionWidget();

    QStackedWidget* getOpenQuakeSelectionWidget(void);


public slots:
    void clear(void);

signals:
    void loadingComplete(const bool value);
    void outputDirectoryPathChanged(QString motionDir, QString eventFile);

protected:

    void showEvent(QShowEvent *e);

private slots:
    void clearSelection(void);
    void chooseImportFileDialog(void);
    void chooseExportFileDialog(void);

    void handleRectangleSelect(void);
    void handlePolygonSelect(void);
    void handleRadiusSelect(void);
    void handleFreehandSelect(void);
    void handleNoneSelect(void);

    void handleSelectionDone(void);

private:
    void loadOpenQuakeXMLData(void);

    QDomDocument xmlGMs;

    QVector<QgsMapLayer*> referenceLayerGroup;
    QVector<QgsMapLayer*> selectedLayerGroup;

    int getPointSources(QDomNodeList* pointSources);

    int getCharacteristicComplexLineSources(QDomNodeList* lineSources, bool isCharacteristic);

    int getSimpleFaultSources(QDomNodeList* lineSources);

    int getComplexFaultSources(QDomNodeList* lineSources);

    int getAreaSources(QDomNodeList* lineSources);

    QWidget* fileInputWidget = nullptr;
    QProgressBar* progressBar = nullptr;
    QLabel* progressLabel = nullptr;
    QWidget* progressBarWidget = nullptr;
    QLineEdit* xmlImportPathLineEdit = nullptr;
    QStackedWidget* theStackedWidget = nullptr;

    QLineEdit* xmlExportPathLineEdit = nullptr;

    std::unique_ptr<SimCenterMapcanvasWidget> mapViewSubWidget;
    QGISVisualizationWidget* theVisualizationWidget = nullptr;

    QgsVectorLayer* pointReferenceLayer = nullptr;
    QgsVectorLayer* lineReferenceLayer = nullptr;
    QgsVectorLayer* areaReferenceLayer = nullptr;

    QgsVectorLayer* pointSelectedLayer = nullptr;
    QgsVectorLayer* lineSelectedLayer = nullptr;
    QgsVectorLayer* areaSelectedLayer = nullptr;
};

#endif // OpenQuakeSelectionWidget_H
