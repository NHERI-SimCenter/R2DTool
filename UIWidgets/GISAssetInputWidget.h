#ifndef GISAssetInputWidget_H
#define GISAssetInputWidget_H
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

#include "AssetInputWidget.h"

class QgsVectorLayer;
class CRSSelectionWidget;

class GISAssetInputWidget : public  AssetInputWidget
{
    Q_OBJECT

public:
    explicit GISAssetInputWidget(QWidget *parent, VisualizationWidget* visWidget, QString componentType, QString appType = "GIS_to_AIM");
    virtual ~GISAssetInputWidget();

    int loadAssetVisualization() override;

    bool outputAppDataToJSON(QJsonObject &jsonObject) override;
    bool inputAppDataFromJSON(QJsonObject &jsonObject) override;

    bool copyFiles(QString &destName) override;

    void clear(void) override;

#ifdef OpenSRA
    bool loadFileFromPath(const QString& filePath);
#endif

    bool isEmpty();

    // Set the coordinate reference system for the layer
    void setCRS(const QgsCoordinateReferenceSystem & val);

    // Get the asset layer
    QgsVectorLayer *getAssetLayer() const;

    // All features in QGIS set/get by their feature id (id set internally by QGIS). e.g., layer->getFeature(feature_id)
    // Problem is that the feature id may be different than the id given by the user. Hence, need to calcualte the offset to reconcile the difference
    int getOffset(void);

public slots:
    bool loadAssetData(void) override;

private slots:
    void handleLayerCrsChanged(const QgsCoordinateReferenceSystem & val);

private:

    QgsVectorLayer* vectorLayer = nullptr;
    CRSSelectionWidget* crsSelectorWidget = nullptr;

};

#endif // GISAssetInputWidget_H
