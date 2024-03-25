#ifndef QGISSITEINPUTWIDGET_H
#define QGISSITEINPUTWIDGET_H
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

// Written by: Kuanshi Zhong

#include "AssetInputWidget.h"

class QgsVectorLayer;
class QgsFeature;
class QgsGeometry;
class QStringList;
class ComponentTableView;

class QGISSiteInputWidget : public AssetInputWidget
{
    Q_OBJECT
public:
    QGISSiteInputWidget(QWidget *parent, VisualizationWidget* visWidget, QString componentType, QString appType = QString());

    int loadAssetVisualization();

    void reloadComponentData(QString newDataFile);

    bool inputAppDataFromJSON(QJsonObject &jsonObject);

    void clear();

signals:
    void soilDataCompleteSignal(bool flag);
    void activateSoilModelWidget(bool flag);

public slots:
    void setSiteFilter(QString filter);
    void showSiteTableWindow();

private:

    void checkSoilPropComplete(void);
    int checkSoilParamComplete(void);
    void checkSoilDataComplete(void);

    // minimum required attributes
    QStringList attrbMinReqSite = {"Latitude", "Longitude"}; // componentType = site
    QStringList attrbMinReqSoil = {"Latitude", "Longitude", "SoilModel"}; // componentType = soil

    // Soil properties attributes completeness
    bool soilPropComplete = false;

    // Soil model full attributes
    QStringList attrbFullSoilEI = {"Den"}; // soil model = EI
    QStringList attrbFullSoilBA = {"Den", "Su_rat", "h/G", "m", "h0", "chi"}; // soil model = BA

    // Soil model attributes completeness
    bool soilParamComplete = false;

    ComponentTableView* componentTableView;
};

#endif // QGISSITEINPUTWIDGET_H
