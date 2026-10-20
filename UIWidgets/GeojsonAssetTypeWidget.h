#ifndef GeojsonAssetTypeWidget_H
#define GeojsonAssetTypeWidget_H
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

// Written by: fmk

// Holds the assets of a single asset type read from a GeoJSON file by GeojsonAssetInputWidget.
// The file must give each feature an integer "id"; QGIS uses that id as the feature id (fid),
// so the asset id shown in the table, the QGIS fid, and the id used by the backend are all the same.

#include "AssetInputWidget.h"

#include <QSet>

class QgsCoordinateReferenceSystem;

class GeojsonAssetTypeWidget : public  AssetInputWidget
{
    Q_OBJECT

public:
    explicit GeojsonAssetTypeWidget(QWidget *parent, VisualizationWidget* visWidget, QString assetType);
    virtual ~GeojsonAssetTypeWidget();

    // Set the GeoJSON file containing only the features of this asset type, and the ids of those features
    void setAssetFile(const QString& path, const QSet<qint64>& ids);

    int loadAssetVisualization();

    bool isEmpty();

    void setCRS(const QgsCoordinateReferenceSystem & val);

    // Remove this asset type's layers from the map, which also stops the map selecting into this widget
    // Call before deleting the widget
    void removeLayers(void);

public slots:
    bool loadAssetData(bool message = true);

private:

    QSet<qint64> featureIds;

    // Layers are looked up by id when removed, in case the user already removed them from the map
    QString mainLayerId;
    QString selectedLayerId;
};

#endif // GeojsonAssetTypeWidget_H
