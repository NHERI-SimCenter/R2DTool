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

#include "GeojsonAssetTypeWidget.h"

#include "AssetFilterDelegate.h"
#include "ComponentTableView.h"
#include "ComponentTableModel.h"

#include "QGISVisualizationWidget.h"

#include <QApplication>
#include <QFileInfo>
#include <QHeaderView>
#include <QtGlobal>

#include <qgsfillsymbol.h>
#include <qgsmarkersymbol.h>
#include <qgslinesymbol.h>
#include <qgsproject.h>
#include <qgsvectorlayer.h>


GeojsonAssetTypeWidget::GeojsonAssetTypeWidget(QWidget *parent, VisualizationWidget* visWidget, QString assetType) : AssetInputWidget(parent, visWidget, assetType, "GEOJSON_TO_ASSET")
{
    this->setContentsMargins(0,0,0,0);

    QString txt2 = "Enter the IDs of one or more "+assetType.toLower()+" to analyze."
                   "Define a range of "+assetType.toLower()+" with a dash and separate multiple "+assetType.toLower()+" with a comma.";
    this->setLabel2(txt2);

    QString txt3 = assetType + " Information";
    this->setLabel3(txt3);

    // The file is written by GeojsonAssetInputWidget, user does not select it
    this->hideAssetFilePath();
}


GeojsonAssetTypeWidget::~GeojsonAssetTypeWidget()
{

}


void GeojsonAssetTypeWidget::setAssetFile(const QString& path, const QSet<qint64>& ids)
{
    this->setPathToComponentInputFile(path);
    featureIds = ids;
}


// QVariant::toString() returns "" for lists, e.g. "NumberOfStories": [2,1] in the GeoJSON
// If any feature has a list, QGIS makes the field a list for all features, so a single value 2 comes back as [2]
static QString attributeToString(const QVariant& value)
{
    if (value.userType() == QMetaType::QVariantList || value.userType() == QMetaType::QStringList)
    {
        auto list = value.toList();

        if (list.size() == 1)
            return attributeToString(list.first());

        QStringList items;
        for (const QVariant& it : list)
            items.push_back(attributeToString(it));

        return "[" + items.join(",") + "]";
    }

    return value.toString();
}


bool GeojsonAssetTypeWidget::loadAssetData(bool message)
{
    QFileInfo file(pathToComponentInputFile);

    if (!file.exists())
    {
        this->errorMessage("Cannot find the file: "+ pathToComponentInputFile);
        return false;
    }

    // Clear the old layers if any
    this->removeLayers();

    mainLayer = theVisualizationWidget->addVectorLayer(pathToComponentInputFile, file.fileName(), "ogr");

    if(mainLayer == nullptr)
    {
        this->errorMessage("Error, failed to add GIS layer for "+assetType);
        return false;
    }

    mainLayerId = mainLayer->id();

    auto numFeat = mainLayer->featureCount();

    if(numFeat == 0)
    {
        this->errorMessage("There are no "+assetType+" features in the GeoJSON file");
        return false;
    }

    if(message)
    {
        this->statusMessage("Loading information for " + QString::number(numFeat)+ " assets");
        QApplication::processEvents();
    }

    theVisualizationWidget->registerLayerForSelection(mainLayer->id(),this);

    // The asset id is the feature id, it is not one of the layer fields, so put it in the first column
    QStringList fieldsStrList = {"id"};

    auto fields = mainLayer->fields();
    for(int i = 0; i<fields.size(); ++i)
        fieldsStrList.push_back(fields[i].name());

    tableHorizontalHeadings = fieldsStrList;

    auto numFields = fieldsStrList.size();

    QVector<QStringList> data;
    data.reserve(numFeat);

    auto features = mainLayer->getFeatures();

    QgsFeature feat;
    while (features.nextFeature(feat))
    {
        // QGIS only uses the GeoJSON id as the fid if the ids are unique integers, otherwise it numbers the features itself
        if(!featureIds.contains(feat.id()))
        {
            this->errorMessage("Error, QGIS feature id "+QString::number(feat.id())+" for "+assetType+" does not match an 'id' in the GeoJSON file, check the ids are unique integers");
            return false;
        }

        QStringList attributeStrList = {QString::number(feat.id())};

        auto attributes = feat.attributes();
        for(int i = 0; i<attributes.size(); ++i)
            attributeStrList.push_back(attributeToString(attributes[i]));

        if(attributeStrList.size() != numFields)
        {
            this->errorMessage("Error, the number of attributes: "+QString::number(attributeStrList.size())+" for feature " +QString::number(feat.id())+" does not equal the number of fields "+QString::number(numFields));
            return false;
        }

        data.push_back(attributeStrList);
    }

    if(data.size() != featureIds.size())
    {
        this->errorMessage("Error, loaded "+QString::number(data.size())+" "+assetType+" features but the GeoJSON file has "+QString::number(featureIds.size()));
        return false;
    }

    componentTableWidget->clear();
    componentTableWidget->getTableModel()->populateData(data, tableHorizontalHeadings);
    componentTableWidget->show();
    componentTableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);

    if(this->loadAssetVisualization() != 0)
        return false;

    // Asset id and QGIS fid are the same
    offset = 0;
    theComponentDb->setOffset(offset);

    this->statusMessage("Done loading assets");
    QApplication::processEvents();

    emit doneLoadingComponents();

    return true;
}


int GeojsonAssetTypeWidget::loadAssetVisualization()
{
    auto type = mainLayer->geometryType();

    QString typeStr = "Null";

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)

    if (type == Qgis::GeometryType::Point)
        typeStr = "point";
    else if (type == Qgis::GeometryType::Line)
        typeStr = "multilinestring";
    else if (type == Qgis::GeometryType::Polygon)
        typeStr = "polygon";

#else

    if (type == QgsWkbTypes::PointGeometry)
        typeStr = "point";
    else if (type == QgsWkbTypes::LineGeometry)
        typeStr = "multilinestring";
    else if (type == QgsWkbTypes::PolygonGeometry)
        typeStr = "polygon";

#endif

    // Create the selected assets layer
    selectedFeaturesLayer = theVisualizationWidget->addVectorLayer(typeStr,"Selected "+assetType);

    if(selectedFeaturesLayer == nullptr)
    {
        this->errorMessage("Error adding the selected assets vector layer");
        return -1;
    }

    selectedLayerId = selectedFeaturesLayer->id();

    selectedFeaturesLayer->setCrs(mainLayer->crs());

    QgsSymbol* selectFeatSymbol = nullptr;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    if (type == Qgis::GeometryType::Point) {
        selectFeatSymbol = new QgsMarkerSymbol();
    } else if (type == Qgis::GeometryType::Line) {
        selectFeatSymbol = new QgsLineSymbol();
    } else if (type == Qgis::GeometryType::Polygon) {
        selectFeatSymbol = new QgsFillSymbol();
    }
#else
    if (type == QgsWkbTypes::PointGeometry) {
        selectFeatSymbol = new QgsMarkerSymbol();
    } else if (type == QgsWkbTypes::LineGeometry) {
        selectFeatSymbol = new QgsLineSymbol();
    } else if (type == QgsWkbTypes::PolygonGeometry) {
        selectFeatSymbol = new QgsFillSymbol();
    }
#endif

    else
    {
        this->errorMessage("Type of geometry is not supported");
        return -1;
    }

    selectFeatSymbol->setColor(Qt::yellow);

    theVisualizationWidget->createSimpleRenderer(selectFeatSymbol,selectedFeaturesLayer);

    auto pr2 = selectedFeaturesLayer->dataProvider();

    auto fields = mainLayer->dataProvider()->fields();

    auto res2 = pr2->addAttributes(fields.toList());

    if(!res2)
        this->errorMessage("Error adding attributes to the layer");

    selectedFeaturesLayer->updateFields(); // tell the vector layer to fetch changes from the provider

    QVector<QgsMapLayer*> mapLayers;
    mapLayers.push_back(selectedFeaturesLayer);
    mapLayers.push_back(mainLayer);

    theVisualizationWidget->createLayerGroup(mapLayers,assetType);

    theComponentDb->setMainLayer(mainLayer);

    filterDelegateWidget  = new AssetFilterDelegate(mainLayer);

    theComponentDb->setSelectedLayer(selectedFeaturesLayer);

    return 0;
}


bool GeojsonAssetTypeWidget::isEmpty()
{
    if(mainLayer == nullptr)
        return true;

    if(mainLayer->featureCount() == 0)
        return true;

    return false;
}


void GeojsonAssetTypeWidget::setCRS(const QgsCoordinateReferenceSystem & val)
{
    if(mainLayer)
        mainLayer->setCrs(val);

    if(selectedFeaturesLayer)
        selectedFeaturesLayer->setCrs(val);
}


void GeojsonAssetTypeWidget::removeLayers(void)
{
    // The asset database is shared by asset type, only clear it if it still points at these layers
    if(mainLayer != nullptr && theComponentDb->getMainLayer() == mainLayer)
        theComponentDb->clear();

    for(auto&& layerId : {selectedLayerId, mainLayerId})
    {
        if(layerId.isEmpty())
            continue;

        auto layer = QgsProject::instance()->mapLayer(layerId);
        if(layer != nullptr)
            theVisualizationWidget->removeLayer(layer);
        else
            theVisualizationWidget->deregisterLayerForSelection(layerId);
    }

    mainLayerId.clear();
    selectedLayerId.clear();

    mainLayer = nullptr;
    selectedFeaturesLayer = nullptr;
}
