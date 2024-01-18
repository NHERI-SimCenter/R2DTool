#ifndef RasterHazardInputWidget_H
#define RasterHazardInputWidget_H
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

#include <qgscoordinatereferencesystem.h>

#include <memory>

#include <QMap>

class QGISVisualizationWidget;
class QgsRasterDataProvider;
class QgsProjectionSelectionWidget;
class QgsRasterLayer;
class SimCenterIMWidget;
class CRSSelectionWidget;

class QLineEdit;
class QProgressBar;
class QLabel;
class QComboBox;
class QGridLayout;

class RasterHazardInputWidget : public SimCenterAppWidget
{
    Q_OBJECT

public:
    RasterHazardInputWidget(QGISVisualizationWidget* visWidget, QWidget *parent = nullptr);
    ~RasterHazardInputWidget();

    QWidget* getRasterHazardInputWidget(void);

    bool inputFromJSON(QJsonObject &jsonObject);
    bool outputToJSON(QJsonObject &jsonObj);
    bool inputAppDataFromJSON(QJsonObject &jsonObj);
    bool outputAppDataToJSON(QJsonObject &jsonObj);
    bool copyFiles(QString &destDir);
    void clear(void);

    // Returns the value of the raster layer in the given band
    // Note that band numbers start from 1 and not 0!
    double sampleRaster(const double& x, const double& y, const int& bandNumber);

private slots:
    void chooseEventFileDialog(void);
    void handleLayerCrsChanged(const QgsCoordinateReferenceSystem & val);

signals:
    void outputDirectoryPathChanged(QString motionDir, QString eventFile);
    void eventTypeChangedSignal(QString eventType);
    void loadingComplete(const bool value);

private:

    int loadRaster(void);

    QGISVisualizationWidget* theVisualizationWidget = nullptr;

    QString eventFile;
    QString rasterFilePath;
    QLineEdit* rasterPathLineEdit = nullptr;

    QString pathToEventFile;

    QWidget* fileInputWidget = nullptr;

    QgsRasterDataProvider* dataProvider = nullptr;
    QgsRasterLayer* rasterlayer = nullptr;

    SimCenterIMWidget* theIMs = nullptr;  
    CRSSelectionWidget* crsSelectorWidget = nullptr;
    QComboBox* eventTypeCombo = nullptr;

    bool asHdf5 = false;
};

#endif // RasterHazardInputWidget_H
