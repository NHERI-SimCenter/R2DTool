#include "GasPipelineInputWidget.h"
#include "ComponentTableView.h"

#include "Field.h"
#include "GroupLayer.h"
#include "FeatureCollection.h"
#include "FeatureCollectionTable.h"
#include "FeatureCollectionLayer.h"
#include "SimpleRenderer.h"
#include "ClassBreaksRenderer.h"
#include "SimpleFillSymbol.h"
#include "SimpleLineSymbol.h"
#include "PolylineBuilder.h"

using namespace Esri::ArcGISRuntime;

GasPipelineInputWidget::GasPipelineInputWidget(QWidget *parent, QString componentType, QString appType) : ComponentInputWidget(parent, componentType, appType)
{

}


int GasPipelineInputWidget::loadComponentVisualization()
{
    // Select a column that will define the pipeline layers
    //    int columnToMapLayers = 0;

    //    QString columnFilter = "OccupancyClass";

    QList<Field> fields;
    fields.append(Field::createDouble("RepairRate", "0.0"));
    fields.append(Field::createText("ID", "NULL",4));
    fields.append(Field::createText("AssetType", "NULL",4));
    fields.append(Field::createText("TabName", "NULL",4));
    fields.append(Field::createText("UID", "NULL",4));

    auto nCols = componentTableWidget->columnCount();

    // Set the table headers as fields in the table
    for(int i =0; i<nCols; ++i)
    {
        auto headerItem = componentTableWidget->horizontalHeaderItem(i);
        auto fieldText = headerItem;
        fields.append(Field::createText(fieldText, fieldText,fieldText.size()));
    }

    // Create the pipelines group layer that will hold the sublayers
    auto pipelineLayer = new GroupLayer(QList<Layer*>{},this);
    pipelineLayer->setName("Gas Pipelines");

    // Add the pipeline layer to the map and get the root tree item
    auto pipelinesItem = theVisualizationWidget->addLayerToMap(pipelineLayer);

    if(pipelinesItem == nullptr)
    {
        errorMessage("Error adding item to the map");
        return -1;
    }

    auto nRows = componentTableWidget->rowCount();

    // Select a column that will define the layers
    //    int columnToMapLayers = 0;

    std::vector<std::string> vecLayerItems;
    //    for(int i = 0; i<nRows; ++i)
    //    {
    //        // Organize the layers according to occupancy type
    //        auto layerTag = pipelineTableWidget->item(i,columnToMapLayers)->data(0).toString().toStdString();

    //        vecLayerItems.push_back(layerTag);
    //    }

    vecLayerItems.push_back("Pipeline Network");

    //    this->uniqueVec<std::string>(vecLayerItems);

    auto selectedComponentsFeatureCollection = new FeatureCollection(this);
    selectedFeaturesTable = new FeatureCollectionTable(fields, GeometryType::Polyline, SpatialReference::wgs84(),this);
    selectedComponentsFeatureCollection->tables()->append(selectedFeaturesTable);
    selectedFeaturesLayer = new FeatureCollectionLayer(selectedComponentsFeatureCollection,this);
    selectedFeaturesLayer->setName("Selected Pipelines");
    selectedFeaturesLayer->setAutoFetchLegendInfos(true);
    selectedFeaturesTable->setRenderer(this->createSelectedPipelineRenderer(1.5));

    // Map to hold the feature tables
    std::map<std::string, FeatureCollectionTable*> tablesMap;
    for(auto&& it : vecLayerItems)
    {
        auto featureCollection = new FeatureCollection(this);

        auto featureCollectionTable = new FeatureCollectionTable(fields, GeometryType::Polyline, SpatialReference::wgs84(),this);

        featureCollection->tables()->append(featureCollectionTable);

        auto newpipelineLayer = new FeatureCollectionLayer(featureCollection,this);

        newpipelineLayer->setName(QString::fromStdString(it));

        newpipelineLayer->setAutoFetchLegendInfos(true);

        featureCollectionTable->setRenderer(this->createPipelineRenderer());

        tablesMap.insert(std::make_pair(it,featureCollectionTable));

        theVisualizationWidget->addLayerToMap(newpipelineLayer,pipelinesItem, pipelineLayer);
    }


    auto headers = this->getTableHorizontalHeadings();

    auto indexLatStart = headers.indexOf("LAT_BEGIN");
    auto indexLonStart = headers.indexOf("LONG_BEGIN");
    auto indexLatEnd = headers.indexOf("LAT_END");
    auto indexLonEnd = headers.indexOf("LONG_END");

    if(indexLatStart == -1 || indexLonStart == -1 || indexLatEnd == -1 || indexLonEnd == -1)
    {
        errorMessage("Could not find the required lat./lon. header labels in the input file");
        return -1;
    }

    for(int i = 0; i<nRows; ++i)
    {

        // create the feature attributes
        QMap<QString, QVariant> featureAttributes;

        // Create a new pipeline
        Component pipeline;

        QString pipelineIDStr = componentTableWidget->item(i,0).toString();

        int pipelineID =  pipelineIDStr.toInt();

        pipeline.ID = pipelineID;

        QMap<QString, QVariant> pipelineAttributeMap;

        // The feature attributes are the columns from the table
        for(int j = 0; j<nCols; ++j)
        {
            auto attrbText = componentTableWidget->horizontalHeaderItem(j);
            auto attrbVal = componentTableWidget->item(i,j);

            pipelineAttributeMap.insert(attrbText,attrbVal.toString());

            featureAttributes.insert(attrbText,attrbVal);
        }

        // Create a unique ID for the pipeline
        auto uid = theVisualizationWidget->createUniqueID();

        pipeline.ComponentAttributes = pipelineAttributeMap;

        featureAttributes.insert("ID", pipelineIDStr);
        featureAttributes.insert("RepairRate", 0.0);
        featureAttributes.insert("AssetType", "GASPIPELINES");
        featureAttributes.insert("TabName", componentTableWidget->item(i,0).toString());
        featureAttributes.insert("UID", uid);

        // Get the feature collection table from the map
        //        auto layerTag = pipelineTableWidget->item(i,columnToMapLayers)->data(0).toString().toStdString();
        auto layerTag = "Pipeline Network";

        auto featureCollectionTable = tablesMap.at(layerTag);

        auto latitudeStart = componentTableWidget->item(i,indexLatStart).toDouble();
        auto longitudeStart = componentTableWidget->item(i,indexLonStart).toDouble();

        auto latitudeEnd = componentTableWidget->item(i,indexLatEnd).toDouble();
        auto longitudeEnd = componentTableWidget->item(i,indexLonEnd).toDouble();

        // Create the points and add it to the feature table
        PolylineBuilder polylineBuilder(SpatialReference::wgs84());

        // Get the two start and end points of the pipeline segment

        Point point1(longitudeStart,latitudeStart);

        Point point2(longitudeEnd,latitudeEnd);

        polylineBuilder.addPoint(point1);
        polylineBuilder.addPoint(point2);

        if(!polylineBuilder.isSketchValid())
        {
            this->errorMessage("Error, cannot create a pipeline feature with the latitude and longitude provided");
            return -1;
        }

        // Create the polyline feature
        auto polyline =  polylineBuilder.toPolyline();

        // Add the feature to the table
        Feature* feature = featureCollectionTable->createFeature(featureAttributes, polyline, this);

        pipeline.UID = uid;
        pipeline.ComponentFeature = feature;

        theComponentDb.addComponent(pipelineID, pipeline);

        featureCollectionTable->addFeature(feature);
    }

    pipelineLayer->load();

    theVisualizationWidget->zoomToLayer(pipelineLayer->layerId());

    return 0;
}


Feature* GasPipelineInputWidget::addFeatureToSelectedLayer(QMap<QString, QVariant>& featureAttributes, Geometry& geom)
{
    Feature* feat = selectedFeaturesTable->createFeature(featureAttributes,geom,this);
    selectedFeaturesTable->addFeature(feat);

    return feat;
}


int GasPipelineInputWidget::removeFeatureFromSelectedLayer(Esri::ArcGISRuntime::Feature* feat)
{
    selectedFeaturesTable->deleteFeature(feat);

    return 0;
}


Esri::ArcGISRuntime::FeatureCollectionLayer* GasPipelineInputWidget::getSelectedFeatureLayer(void)
{
    return selectedFeaturesLayer;
}


void GasPipelineInputWidget::clear()
{
    delete selectedFeaturesLayer;
    delete selectedFeaturesTable;

    selectedFeaturesLayer = nullptr;
    selectedFeaturesTable = nullptr;

    ComponentInputWidget::clear();
}


Renderer* GasPipelineInputWidget::createSelectedPipelineRenderer(double outlineWidth)
{

    SimpleLineSymbol* lineSymbol1 = new SimpleLineSymbol(SimpleLineSymbolStyle::Solid, QColor(0, 0, 0), 5.0f /*width*/, this);
    SimpleLineSymbol* lineSymbol2 = new SimpleLineSymbol(SimpleLineSymbolStyle::Solid, QColor(255,255,178), 5.0f /*width*/, this);
    SimpleLineSymbol* lineSymbol3 = new SimpleLineSymbol(SimpleLineSymbolStyle::Solid, QColor(253,204,92), 5.0f /*width*/, this);
    SimpleLineSymbol* lineSymbol4 = new SimpleLineSymbol(SimpleLineSymbolStyle::Solid, QColor(253,141,60),  5.0f /*width*/, this);
    SimpleLineSymbol* lineSymbol5 = new SimpleLineSymbol(SimpleLineSymbolStyle::Solid, QColor(240,59,32),  5.0f /*width*/, this);
    SimpleLineSymbol* lineSymbol6 = new SimpleLineSymbol(SimpleLineSymbolStyle::Solid, QColor(189,0,38),  5.0f /*width*/, this);

    QList<ClassBreak*> classBreaks;

    auto classBreak1 = new ClassBreak("0.0-0.001 number of repairs", "0.0-0.001 number of repairs", -0.00001, 1E-03, lineSymbol1, this);
    classBreaks.append(classBreak1);

    auto classBreak2 = new ClassBreak("0.001-0.01 number of repairs", "0.001-0.01 number of repairs", 1.00E-03, 1.00E-02, lineSymbol2, this);
    classBreaks.append(classBreak2);

    auto classBreak3 = new ClassBreak("0.01-0.1 number of repairs", "0.01-0.1 number of repairs", 1.00E-02, 1.00E-01, lineSymbol3, this);
    classBreaks.append(classBreak3);

    auto classBreak4 = new ClassBreak("0.1-1.0 number of repairs", "0.1-1.0 number of repairs", 1.00E-01, 1.00E+00, lineSymbol4, this);
    classBreaks.append(classBreak4);

    auto classBreak5 = new ClassBreak("1.0-10.0 number of repairs", "1.0-10.0 number of repairs", 1.00E+00, 1.00E+01, lineSymbol5, this);
    classBreaks.append(classBreak5);

    auto classBreak6 = new ClassBreak("10.0-100.0 number of repairs", "Loss Ratio Between 75% and 90%", 1.00E+01, 1.00E+10, lineSymbol6, this);
    classBreaks.append(classBreak6);

    return new ClassBreaksRenderer("RepairRate", classBreaks, this);
}

Esri::ArcGISRuntime::FeatureCollectionLayer *GasPipelineInputWidget::getSelectedFeaturesLayer() const
{
    return selectedFeaturesLayer;
}


Renderer* GasPipelineInputWidget::createPipelineRenderer(void)
{
    SimpleLineSymbol* lineSymbol1 = new SimpleLineSymbol(SimpleLineSymbolStyle::Solid, QColor(125, 125, 125), 4.0f /*width*/, this);

    SimpleRenderer* lineRenderer = new SimpleRenderer(lineSymbol1, this);

    lineRenderer->setLabel("Gas Pipeline");

    return lineRenderer;
}

//ClassBreaksRenderer* VisualizationWidget::createPipelineRenderer(void)
//{
//    SimpleLineSymbol* lineSymbol1 = new SimpleLineSymbol(SimpleLineSymbolStyle::Solid, QColor(0, 0, 0), 6.0f /*width*/, this);
//    SimpleLineSymbol* lineSymbol2 = new SimpleLineSymbol(SimpleLineSymbolStyle::Solid, QColor(255,255,178), 6.0f /*width*/, this);
//    SimpleLineSymbol* lineSymbol3 = new SimpleLineSymbol(SimpleLineSymbolStyle::Solid, QColor(253,204,92), 6.0f /*width*/, this);
//    SimpleLineSymbol* lineSymbol4 = new SimpleLineSymbol(SimpleLineSymbolStyle::Solid, QColor(253,141,60),  6.0f /*width*/, this);
//    SimpleLineSymbol* lineSymbol5 = new SimpleLineSymbol(SimpleLineSymbolStyle::Solid, QColor(240,59,32),  6.0f /*width*/, this);
//    SimpleLineSymbol* lineSymbol6 = new SimpleLineSymbol(SimpleLineSymbolStyle::Solid, QColor(189,0,38),  6.0f /*width*/, this);

//    QList<ClassBreak*> classBreaks;

//    auto classBreak1 = new ClassBreak("0.0-0.001 number of repairs", "Loss Ratio less than 10%", -0.00001, 1E-03, lineSymbol1, this);
//    classBreaks.append(classBreak1);

//    auto classBreak2 = new ClassBreak("0.001-0.01 number of repairs", "Loss Ratio Between 10% and 25%", 1.00E-03, 1.00E-02, lineSymbol2, this);
//    classBreaks.append(classBreak2);

//    auto classBreak3 = new ClassBreak("0.01-0.1 number of repairs", "Loss Ratio Between 25% and 50%", 1.00E-02, 1.00E-01, lineSymbol3, this);
//    classBreaks.append(classBreak3);

//    auto classBreak4 = new ClassBreak("0.1-1.0 number of repairs", "Loss Ratio Between 50% and 75%", 1.00E-01, 1.00E+00, lineSymbol4, this);
//    classBreaks.append(classBreak4);

//    auto classBreak5 = new ClassBreak("1.0-10.0 number of repairs", "Loss Ratio Between 75% and 90%", 1.00E+00, 1.00E+01, lineSymbol5, this);
//    classBreaks.append(classBreak5);

//    auto classBreak6 = new ClassBreak("10.0-100.0 number of repairs", "Loss Ratio Between 75% and 90%", 1.00E+01, 1.00E+10, lineSymbol6, this);
//    classBreaks.append(classBreak6);

//    return new ClassBreaksRenderer("RepairRate", classBreaks, this);
//}



