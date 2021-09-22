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

// Written by: Stevan Gavrilovic

#include "ArcGISBuildingInputWidget.h"
#include "ArcGISVisualizationWidget.h"

#include "ComponentTableView.h"
#include "ComponentDatabaseManager.h"

#include "Field.h"
#include "GroupLayer.h"
#include "FeatureCollection.h"
#include "FeatureCollectionTable.h"
#include "FeatureCollectionLayer.h"
#include "SimpleRenderer.h"
#include "ClassBreaksRenderer.h"
#include "SimpleFillSymbol.h"
#include "SimpleLineSymbol.h"

using namespace Esri::ArcGISRuntime;

ArcGISBuildingInputWidget::ArcGISBuildingInputWidget(QWidget *parent, QString componentType, QString appType) : ComponentInputWidget(parent, componentType, appType)
{
    theComponentDb = ComponentDatabaseManager::getInstance()->getBuildingComponentDb();
}


int ArcGISBuildingInputWidget::loadComponentVisualization()
{
    QList<Field> fields;
    fields.append(Field::createDouble("LossRatio", "0.0"));
    fields.append(Field::createText("ID", "NULL",4));
    fields.append(Field::createText("AssetType", "NULL",4));
    fields.append(Field::createText("TabName", "NULL",4));
    fields.append(Field::createText("UID", "NULL",4));

    // Select a column that will define the building layers
    int columnToMapLayers = 0;

    QString columnFilter = "OccupancyClass";

    // Set the table headers as fields in the table
    for(int i = 1; i<componentTableWidget->columnCount(); ++i)
    {
        auto fieldText = componentTableWidget->horizontalHeaderItem(i);

        if(fieldText.compare(columnFilter) == 0)
            columnToMapLayers = i;

        fields.append(Field::createText(fieldText, fieldText,fieldText.size()));
    }

    // Create the buildings group layer that will hold the sublayers
    auto buildingLayer = new GroupLayer(QList<Layer*>{},this);
    buildingLayer->setName("Buildings");

    auto buildingsItem = theVisualizationWidget->addLayerToMap(buildingLayer);

    if(buildingsItem == nullptr)
    {
        qDebug()<<"Error adding item to the map";
        return -1;
    }

    auto nRows = componentTableWidget->rowCount();

    std::vector<std::string> vecLayerItems;
    for(int i = 0; i<nRows; ++i)
    {
        // Organize the layers according to occupancy type
        auto occupancyType = componentTableWidget->item(i,columnToMapLayers).toString().toStdString();

        vecLayerItems.push_back(occupancyType);
    }

    this->uniqueVec<std::string>(vecLayerItems);

    auto selectedBuildingsFeatureCollection = new FeatureCollection(this);
    selectedBuildingsTable = new FeatureCollectionTable(fields, GeometryType::Polygon, SpatialReference::wgs84(),this);
    selectedBuildingsFeatureCollection->tables()->append(selectedBuildingsTable);
    selectedBuildingsLayer = new FeatureCollectionLayer(selectedBuildingsFeatureCollection,this);
    selectedBuildingsLayer->setName("Selected Buildings");
    selectedBuildingsLayer->setAutoFetchLegendInfos(true);
    selectedBuildingsTable->setRenderer(this->createSelectedBuildingRenderer(1.5));

    // Map to hold the feature tables
    std::map<std::string, FeatureCollectionTable*> tablesMap;
    for(auto&& it : vecLayerItems)
    {
        auto featureCollection = new FeatureCollection(this);

        auto featureCollectionTable = new FeatureCollectionTable(fields, GeometryType::Polygon, SpatialReference::wgs84(),this);

        featureCollection->tables()->append(featureCollectionTable);

        auto newBuildingLayer = new FeatureCollectionLayer(featureCollection,this);

        newBuildingLayer->setName(QString::fromStdString(it));

        newBuildingLayer->setAutoFetchLegendInfos(true);

        featureCollectionTable->setRenderer(this->createBuildingRenderer());

        tablesMap.insert(std::make_pair(it,featureCollectionTable));

        theVisualizationWidget->addLayerToMap(newBuildingLayer,buildingsItem,buildingLayer);
    }

    auto headers = this->getTableHorizontalHeadings();

    // First check if a footprint was provided
    auto indexFootprint = headers.indexOf("Footprint");
    auto indexLatitude = headers.indexOf("Latitude");
    auto indexLongitude = headers.indexOf("Longitude");

    if(indexLongitude == -1 || indexLatitude == -1)
    {
        this->errorMessage("Could not find latitude and longitude in the header columns");
        return -1;
    }

    for(int i = 0; i<nRows; ++i)
    {
        // create the feature attributes
        QMap<QString, QVariant> featureAttributes;

        // Create a new building
        Component building;

        QString buildingIDStr = componentTableWidget->item(i,0).toString();

        int buildingID = buildingIDStr.toInt();

        building.ID = buildingID;

        QMap<QString, QVariant> buildingAttributeMap;

        // The feature attributes are the columns from the table
        for(int j = 1; j<componentTableWidget->columnCount(); ++j)
        {
            auto attrbText = componentTableWidget->horizontalHeaderItem(j);
            auto attrbVal = componentTableWidget->item(i,j).toString();

            buildingAttributeMap.insert(attrbText,attrbVal);

            featureAttributes.insert(attrbText,attrbVal);
        }

        // Create a unique ID for the building
        auto uid = theVisualizationWidget->createUniqueID();

        building.ComponentAttributes = buildingAttributeMap;

        featureAttributes.insert("ID", buildingIDStr);
        featureAttributes.insert("LossRatio", 0.0);
        featureAttributes.insert("AssetType", "BUILDINGS");
        featureAttributes.insert("TabName", buildingIDStr);
        featureAttributes.insert("UID", uid);

        auto latitude = componentTableWidget->item(i,indexLatitude).toDouble();
        auto longitude = componentTableWidget->item(i,indexLongitude).toDouble();

        // Get the feature collection table for this layer
        auto layerTag = componentTableWidget->item(i,columnToMapLayers).toString().toStdString();

        auto featureCollectionTable = tablesMap.at(layerTag);

        Feature* feature = nullptr;

        // If a footprint is given use that
        if(indexFootprint != -1)
        {
            QString footprint = componentTableWidget->item(i,indexFootprint).toString();

            if(footprint.compare("NA") == 0)
            {
                Point point(longitude,latitude);
                auto geom = theVisualizationWidget->getRectGeometryFromPoint(point, 0.0005,0.0005);
                if(geom.isEmpty())
                {
                    qDebug()<<"Error getting the building footprint geometry";
                    return -1;
                }

                feature = featureCollectionTable->createFeature(featureAttributes, geom, this);

                featureCollectionTable->addFeature(feature);
            }
            else
            {
                auto geom = theVisualizationWidget->getPolygonGeometryFromJson(footprint);

                if(geom.isEmpty())
                {
                    qDebug()<<"Error getting the building footprint geometry";
                    return -1;
                }

                feature = featureCollectionTable->createFeature(featureAttributes, geom, this);

                featureCollectionTable->addFeature(feature);
            }

        }
        else
        {
            Point point(longitude,latitude);
            auto geom = theVisualizationWidget->getRectGeometryFromPoint(point, 0.00015,0.00015);
            if(geom.isEmpty())
            {
                qDebug()<<"Error getting the building footprint geometry";
                return -1;
            }

            feature = featureCollectionTable->createFeature(featureAttributes, geom, this);

            featureCollectionTable->addFeature(feature);
        }

        building.UID = uid;
        building.ComponentFeature = feature;

        theComponentDb.addComponent(buildingID, building);
    }

    buildingLayer->load();

    theVisualizationWidget->zoomToLayer(buildingLayer->layerId());

    return 0;
}


Feature* ArcGISBuildingInputWidget::addFeatureToSelectedLayer(QMap<QString, QVariant>& featureAttributes, Geometry& geom)
{
    Feature* feat = selectedBuildingsTable->createFeature(featureAttributes,geom,this);
    selectedBuildingsTable->addFeature(feat);

    return feat;
}


int ArcGISBuildingInputWidget::removeFeatureFromSelectedLayer(Esri::ArcGISRuntime::Feature* feat)
{
    selectedBuildingsTable->deleteFeature(feat);

    return 0;
}


SimpleRenderer* ArcGISBuildingInputWidget::createBuildingRenderer(void)
{
    SimpleFillSymbol* fillSymbol = new SimpleFillSymbol(SimpleFillSymbolStyle::Solid, QColor(0, 0, 255, 125), this);

    SimpleRenderer* lineRenderer = new SimpleRenderer(fillSymbol, this);

    lineRenderer->setLabel("Building footprint");

    return lineRenderer;
}


Esri::ArcGISRuntime::FeatureCollectionLayer* ArcGISBuildingInputWidget::getSelectedFeatureLayer(void)
{
    return selectedBuildingsLayer;
}


void ArcGISBuildingInputWidget::clear()
{
    delete selectedBuildingsLayer;
    delete selectedBuildingsTable;

    selectedBuildingsLayer = nullptr;
    selectedBuildingsTable = nullptr;

    ComponentInputWidget::clear();
}


ClassBreaksRenderer* ArcGISBuildingInputWidget::createSelectedBuildingRenderer(double outlineWidth)
{
    // Images stored in base64 format
    //    QByteArray buildingImg1 = "iVBORw0KGgoAAAANSUhEUgAAABwAAAAcCAYAAAByDd+UAAAAAXNSR0IB2cksfwAAAAlwSFlzAAAOxAAADsQBlSsOGwAAAWhJREFUSInt1jFrwkAYxvF/SDpJBaHFtlN0dlP0O3TWSWobOnZvidA2QycHZ9cGhNKlX0FwcPcLCC4OrWKGDMKZdKmg1WpOo1LwgYMb3nt/HHdwp7HjaHsDTdM8GgwGFnADXITU/xN4j0QiD9Vq1Z0Bh8PhE1AOCZrkFLhzXfcYuJ4BPc+7ChmbzuVkMn2GZ1sETxaBUkkkEnQ6Hel1a4GGYZBOp6nX6ySTSVKpFACWZTEajcIFDcMgl8sBUCwW6ff7xGIxAFRVXbleCpzGADRNIx6Py7QIDv7G1k0gMCiWzWZpNBqbgTI7KxQKjMdjms3memCpVCKTyeD7PoqirAQVRSGfzyOEoNVqyYO2bWPbNpVKhWg0uhJst9vUarWlNft7LQ7gAfzfYLkc7Ofh+74U2AP0RUVCiEDgkvQWga/A86ad/8jHHKjr+ku321U9z7sFzkOCvoA3x3Hu50DTNAXw+DO2lp3f0m97bGdscCiEZAAAAABJRU5ErkJggg==";
    //    QByteArray buildingImg2 = "iVBORw0KGgoAAAANSUhEUgAAABwAAAAcCAYAAAByDd+UAAAAAXNSR0IB2cksfwAAAAlwSFlzAAAOxAAADsQBlSsOGwAAAYNJREFUSInt1b9LAmEcx/H3nVfSYEiU9MNBaHPJwKaKoKGh6YbopqioqYaGoLjAashJ8C9oCZeQg/bAXXBoaGoPhH7YEA1n1z0NYVha3umlBH6mZ3i+39fzCx6FNkfpGKjr9FiW/1gIcw0Y9aj/PZC1bfbTaV6+gK+v/kMwDzyCKhkCtn0+AsDqFxDMFY+xzwjBYmVcfYfDfwUCg/VAVwmHN7m9PXVd1xSoaTlisVmy2XEikQmi0SkAUqkZyuUbb0FNyxGPzwOwvLxLqXRHMPhxYrLc17DeFViNAShKD6HQmJsWzsHvWLNxBDrFYrF18vmd1kA3O1PVLd7eLAqF3ebApaVLJifnEEIgSVJDUJZlVHULyypzdaW7Bw1jAcOAROKJ/v5gQ/D6Ok8mM/3rnM79Fl2wC/5vMJmMIsu9DefZ9rMrsAhE6jcqYttOlvZjijWgJPnPhDCPWmr7Q2ybixpwYMA8eXz0+8DcAEY8sh6A80CAvRpQ17HATAAJj7C6afsrfQdYrmo3mMtmpgAAAABJRU5ErkJggg==";
    //    QByteArray buildingImg3 = "iVBORw0KGgoAAAANSUhEUgAAABwAAAAcCAYAAAByDd+UAAAAAXNSR0IB2cksfwAAAAlwSFlzAAAOxAAADsQBlSsOGwAAAWJJREFUSInt1s8rw3Ecx/Hn2zbfJpo0xTj4dXKh1ZSQUhzkX9CI0xwcVkQNB3+Gi9tycVPUzuS0qyPKCpFQvtp8HGaz2bd9v9/ta0t5nT59en/ej++PT30+bmocd91AFcejRNsR9AUg4FD/OwUH8sa6hHktBkXbEvRNh6Bc2gVWlJcWIFwECvq8w1g+opjNjQv/YcdvgYDfCLQX3zI87dleVhkYTEDvBJz2g38IukPZ+eNxyFw4DAYTMDCVHY9G4eUWmr++mHhNl9sDCzEAlwd8XbZaWAd/YhXGGmgVCyzC1WqVoJ03G4mASsN1tEJw+AT6JkEpEDEHpQFCEci8w81GBWByBpLA3CM0tZqDl2dwPla2pH6nxT/4D/5x8GgQaDSvU8+2wBTQY1j1kbLyWOWSb/B9iULbF/TtajsbRcFhCSht+q560FyCvgR0OmTdK4iLxlopOE0a9BgQcwgzTM136SeMBkz2tFUt2gAAAABJRU5ErkJggg==";
    //    QByteArray buildingImg4 = "iVBORw0KGgoAAAANSUhEUgAAABwAAAAcCAYAAAByDd+UAAAAAXNSR0IB2cksfwAAAAlwSFlzAAAOxAAADsQBlSsOGwAAAYxJREFUSInt1csrRGEYx/Hve2bGidwml1wil2xsMDUpl5RioWzYKAmxGgsLRaNcs7DyFyjZnWxsLKRmO0kWsrMTZQqRJvJq5hyLSQ0zY+bMHCPlt3oXz/t83lu9drIc+6+BmhcHL+oqipxAp8qi/nco7EnBwvgWz59A41VdFshFdIuoSMrQmcmBAmD8Eyh0OWYpFRUBAx/j6Dus+CkQKI0HmkpR7TRPV9um56UFukZ9NLR149caKa1voabZDcDhZhdheWEt6Br10eTuBaBjZI7gwy35zsiJCVtu0vmmwGgMwGZ3UFxebaZF6uBXLN2kBKaKVbVOcuWfzQw0s7P2YQ+GHuL6eC49sHXkiEZXD4ZhIIRICgpFwT3kIRx64+bUax480/o502Bw/ZG8wuKk4OX5MSc7nd/W/N5v8Q/+g38bPFhrBiUneaEeNAEaBBDUxSsywgEIp7K0hAnEgkLdBbmSUdsEEbAfA5ZUyI37W9UmdDkFVFpk3WOgqU7mY8A+LyGQS8CSRVjcZP2VvgN6imQ8SFFgygAAAABJRU5ErkJggg==";
    //    QByteArray buildingImg5 = "iVBORw0KGgoAAAANSUhEUgAAABwAAAAcCAYAAAByDd+UAAAAAXNSR0IB2cksfwAAAAlwSFlzAAAOxAAADsQBlSsOGwAAAXpJREFUSInt1b1LQlEYx/HvMfVQFAkZVBJE0uJiBBKkEQg1BP0JYZLTbXC4UBj0MvRntLRJS0tDCK4hTa1tUZBggoQ0nFBuQ2gvXvRevRVBv+kMz3k+5w2Omx+O+9fALHiQ8hClNoAJh/o/AqcKdhLw/Ak0pNwXSu06BDUyCmx5YQhIfAKFUusOY80IWG2MP97h2HeBgN8MtJXhVIqn42Pb87oC5/J5phcXuQwG8YfDTEYiAFzEYtRvbpwF5/J5ZuJxABZ0nWqpxKD/7cREf3/H+bbAjxhAn8eDLxCw08I6+BXrNpZAq9hEMsldOt0baGdn85qGUatxr+vdgbO5HMGlJQzDQAjRERQuFxFNo/7ywkMmYx+8XlnhGlirVBjw+TqCt4UCV9Fo25rf+y3+wX/wb4PnoRB4vZ0Lq1VbYBGYMisyikUr62qXZoN3UMoTlDrotbNZBJy1gCNKHZWl7BNKbQLjDlllICthuwVchhpK7QF7DmGm+fFX+gonY17k9eIf3wAAAABJRU5ErkJggg==";

    //    buildingImg1 = QByteArray::fromBase64(buildingImg1);
    //    buildingImg2 = QByteArray::fromBase64(buildingImg2);
    //    buildingImg3 = QByteArray::fromBase64(buildingImg3);
    //    buildingImg4 = QByteArray::fromBase64(buildingImg4);
    //    buildingImg5 = QByteArray::fromBase64(buildingImg5);

    //    QImage img1 = QImage::fromData(buildingImg1);
    //    QImage img2 = QImage::fromData(buildingImg2);
    //    QImage img3 = QImage::fromData(buildingImg3);
    //    QImage img4 = QImage::fromData(buildingImg4);
    //    QImage img5 = QImage::fromData(buildingImg5);

    //    PictureMarkerSymbol* symbol1 = new PictureMarkerSymbol(img1, this);
    //    PictureMarkerSymbol* symbol2 = new PictureMarkerSymbol(img2, this);
    //    PictureMarkerSymbol* symbol3 = new PictureMarkerSymbol(img3, this);
    //    PictureMarkerSymbol* symbol4 = new PictureMarkerSymbol(img4, this);
    //    PictureMarkerSymbol* symbol5 = new PictureMarkerSymbol(img5, this);

    // SimpleMarkerSymbol* symbol1 = new SimpleMarkerSymbol(SimpleMarkerSymbolStyle::Square, QColor(0, 0, 255,125), 8.0f, this);
    // SimpleMarkerSymbol* symbol2 = new SimpleMarkerSymbol(SimpleMarkerSymbolStyle::Square, QColor(255,255,178), 8.0f, this);
    // SimpleMarkerSymbol* symbol3 = new SimpleMarkerSymbol(SimpleMarkerSymbolStyle::Square, QColor(253,204,92), 8.0f, this);
    // SimpleMarkerSymbol* symbol4 = new SimpleMarkerSymbol(SimpleMarkerSymbolStyle::Square, QColor(253,141,60), 8.0f, this);
    // SimpleMarkerSymbol* symbol5 = new SimpleMarkerSymbol(SimpleMarkerSymbolStyle::Square, QColor(240,59,32), 8.0f, this);

    SimpleFillSymbol* symbol1 = new SimpleFillSymbol(SimpleFillSymbolStyle::Solid, QColor(99, 99, 99, 200), this);
    SimpleFillSymbol* symbol2 = new SimpleFillSymbol(SimpleFillSymbolStyle::Solid, QColor(254,217,142), this);
    SimpleFillSymbol* symbol3 = new SimpleFillSymbol(SimpleFillSymbolStyle::Solid, QColor(254,153,41), this);
    SimpleFillSymbol* symbol4 = new SimpleFillSymbol(SimpleFillSymbolStyle::Solid, QColor(217,95,14), this);
    SimpleFillSymbol* symbol5 = new SimpleFillSymbol(SimpleFillSymbolStyle::Solid, QColor(153,52,4), this);

    if(outlineWidth != 0.0)
    {
        SimpleLineSymbol* outlineSymbol = new SimpleLineSymbol(SimpleLineSymbolStyle::Solid, QColor(255, 255, 0, 200), outlineWidth, this);
        symbol1->setOutline(outlineSymbol);
        symbol2->setOutline(outlineSymbol);
        symbol3->setOutline(outlineSymbol);
        symbol4->setOutline(outlineSymbol);
        symbol5->setOutline(outlineSymbol);
    }

    QList<ClassBreak*> classBreaks;

    auto classBreak1 = new ClassBreak("0.00-0.05 Loss Ratio", "Loss Ratio less than 5%", -0.00001, 0.05, symbol1,this);
    classBreaks.append(classBreak1);

    auto classBreak2 = new ClassBreak("0.05-0.25 Loss Ratio", "Loss Ratio Between 5% and 25%", 0.05, 0.25, symbol2,this);
    classBreaks.append(classBreak2);

    auto classBreak3 = new ClassBreak("0.25-0.50 Loss Ratio", "Loss Ratio Between 25% and 50%", 0.25, 0.5,symbol3,this);
    classBreaks.append(classBreak3);

    auto classBreak4 = new ClassBreak("0.50-0.75 Loss Ratio", "Loss Ratio Between 50% and 75%", 0.50, 0.75,symbol4,this);
    classBreaks.append(classBreak4);

    auto classBreak5 = new ClassBreak("0.75-1.00 Loss Ratio", "Loss Ratio Between 75% and 100%", 0.75, 1.0,symbol5,this);
    classBreaks.append(classBreak5);

    return new ClassBreaksRenderer("LossRatio", classBreaks, this);
}

