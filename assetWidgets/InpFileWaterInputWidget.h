#ifndef InpFileWaterInputWidget_H
#define InpFileWaterInputWidget_H
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

// Written by: Dr. Stevan Gavrilovic

#include "SimCenterAppWidget.h"
#include "CRSSelectionWidget.h"
#include <QList>
#include <QBoxLayout>

class QGISVisualizationWidget;
class VisualizationWidget;
class GISAssetInputWidget;
class MultiComponentR2D;

class QLineEdit;

class QgsMapLayer;
class QgsFeature;
class QgsGeometry;
class CRSSelectionWidget;

class InpFileWaterInputWidget : public SimCenterAppWidget
{
    Q_OBJECT

public:
    InpFileWaterInputWidget(QWidget *parent, VisualizationWidget* visWidget, QString componentType, QString appType);
    virtual ~InpFileWaterInputWidget();

    void insertLineEditToMainLayout(int index, QWidget* widget, QLineEdit* LineEdit, QString componentType, QString FieldName);

    void clear();

    bool outputAppDataToJSON(QJsonObject &jsonObject);
    bool inputAppDataFromJSON(QJsonObject &jsonObject);
    bool copyFiles(QString &destDir);

protected slots:
    bool loadAssetData();
    void chooseAssetFileDialog(void);

private slots:
    void handleLayerCrsChanged(const QgsCoordinateReferenceSystem & val);

protected:

    QLineEdit* inpFileLineEdit = nullptr;
    QString    geoJsonFileName;
    QString    crsAuthID;
  
    QGISVisualizationWidget* theVisualizationWidget = nullptr;
    MultiComponentR2D* mainAssetWidget = nullptr;

    QList<GISAssetInputWidget*> theAssetInputWidgetList;
    QList<QgsMapLayer*> theAssetLayerList;

    QString componentType;
    QString appType;

    CRSSelectionWidget* crsSelectorWidget = nullptr;

    QVBoxLayout* mainLayout = nullptr;

    // Additional fields:
    QList<QLineEdit*> LineEditList;
    QMap<QString, QLineEdit*> FieldNameToLineEdit;
    QMap<QString, QList<QWidget*>> ComponentTypeToAdditionalWidget;
    QMap<QString, QList<QString>> ComponentTypeToFieldNames;

};

#endif // InpFileWaterInputWidget_H
