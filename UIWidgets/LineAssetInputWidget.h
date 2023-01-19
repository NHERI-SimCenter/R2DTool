#ifndef LineAssetInputWidget_H
#define LineAssetInputWidget_H
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

// Use this class when your asset geometry is represented as a line, i.e., pipelines
// The input file must be in .csv format
// The loadAssetVisualization function is specialized for line geometry rendering

#include "AssetInputWidget.h"

class PointAssetInputWidget;

class QgsVectorLayer;
class QgsFeature;
class QgsGeometry;

#ifdef OpenSRA
class JsonGroupBoxWidget;
#endif

class LineAssetInputWidget : public AssetInputWidget
{
public:
    LineAssetInputWidget(QWidget *parent, VisualizationWidget* visWidget, QString assetType, QString appType = QString());

#ifdef OpenSRA
    bool loadFileFromPath(const QString& filePath) override;

    bool inputFromJSON(QJsonObject &rvObject) override;
    bool outputToJSON(QJsonObject &rvObject) override;

    void createComponentsBox(void) override;
#endif

    int loadAssetVisualization(void) override;

    void clear() override;

    void setTheNodesWidget(PointAssetInputWidget *newTheNodesWidget);

private:

#ifdef OpenSRA
    JsonGroupBoxWidget* locationWidget = nullptr;
#endif

    PointAssetInputWidget* theNodesWidget = nullptr;

};

#endif // LineAssetInputWidget_H
