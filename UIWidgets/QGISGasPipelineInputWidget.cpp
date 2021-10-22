#include "QGISGasPipelineInputWidget.h"
#include "QGISVisualizationWidget.h"
#include "ComponentDatabaseManager.h"
#include "ComponentTableView.h"

#include <qgsfeature.h>
#include <qgslinesymbol.h>


QGISGasPipelineInputWidget::QGISGasPipelineInputWidget(QWidget *parent, VisualizationWidget* visWidget, QString componentType, QString appType) : ComponentInputWidget(parent, visWidget, componentType, appType)
{
    theComponentDb = ComponentDatabaseManager::getInstance()->getPipelineComponentDb();
}


int QGISGasPipelineInputWidget::loadComponentVisualization()
{

    QgsFields featFields;
    featFields.append(QgsField("RepairRate", QVariant::Double));
    featFields.append(QgsField("ID", QVariant::Int));
    featFields.append(QgsField("AssetType", QVariant::String));
    featFields.append(QgsField("TabName", QVariant::String));

    // Set the table headers as fields in the table
    for(int i = 1; i<componentTableWidget->columnCount(); ++i)
    {
        auto fieldText = componentTableWidget->horizontalHeaderItemVariant(i);
        featFields.append(QgsField(fieldText.toString(),fieldText.type()));
    }

    QList<QgsField> attribFields;
    for(int i = 0; i<featFields.size(); ++i)
        attribFields.push_back(featFields[i]);

    // Create the buildings layer
    auto mainLayer = theVisualizationWidget->addVectorLayer("linestring","All Pipelines");

    if(mainLayer == nullptr)
    {
        this->errorMessage("Error adding a vector layer");
        return -1;
    }

    auto pr = mainLayer->dataProvider();

    mainLayer->startEditing();

    auto res = pr->addAttributes(attribFields);

    if(!res)
        this->errorMessage("Error adding attributes to the layer" + mainLayer->name());

    mainLayer->updateFields(); // tell the vector layer to fetch changes from the provider

    theComponentDb->setMainLayer(mainLayer);

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


    // Get the number of rows
    auto nRows = componentTableWidget->rowCount();

    auto numAtrb = attribFields.size();

    for(int i = 0; i<nRows; ++i)
    {

        // create the feature attributes
        QgsAttributes featureAttributes(numAtrb);

        // Create a new pipeline
        QString pipelineIDStr = componentTableWidget->item(i,0).toString();

        int pipelineID =  pipelineIDStr.toInt();

        // Create a unique ID for the building
        auto uid = theVisualizationWidget->createUniqueID();

        // "RepairRate"
        // "ID"
        // "AssetType"
        // "UID"

        featureAttributes[0] = QVariant(0.0);
        featureAttributes[1] = QVariant(pipelineID);
        featureAttributes[2] = QVariant("GASPIPELINES");
        featureAttributes[3] = QVariant(uid);

        // The feature attributes are the columns from the table
        for(int j = 1; j<componentTableWidget->columnCount(); ++j)
        {
            auto attrbVal = componentTableWidget->item(i,j);
            featureAttributes[3+j] = attrbVal;
        }

        QgsFeature feature;
        feature.setFields(featFields);

        auto latitudeStart = componentTableWidget->item(i,indexLatStart).toDouble();
        auto longitudeStart = componentTableWidget->item(i,indexLonStart).toDouble();

        auto latitudeEnd = componentTableWidget->item(i,indexLatEnd).toDouble();
        auto longitudeEnd = componentTableWidget->item(i,indexLonEnd).toDouble();

        // Start and end point of the pipe
        QgsPointXY point1(longitudeStart,latitudeStart);
        QgsPointXY point2(longitudeEnd,latitudeEnd);

        QgsPolylineXY pipeSegment(2);
        pipeSegment[0]=point1;
        pipeSegment[1]=point2;

        feature.setGeometry(QgsGeometry::fromPolylineXY(pipeSegment));

        feature.setAttributes(featureAttributes);

        if(!feature.isValid())
            return -1;

        auto res = pr->addFeature(feature, QgsFeatureSink::FastInsert);
        if(!res)
        {
            this->errorMessage("Error adding the feature to the layer");
            return -1;
        }
    }

    mainLayer->commitChanges(true);
    mainLayer->updateExtents();

    QgsLineSymbol* markerSymbol = new QgsLineSymbol();

    markerSymbol->setWidth(0.8);
    markerSymbol->setColor(Qt::darkGray);
    theVisualizationWidget->createSimpleRenderer(markerSymbol,mainLayer);

//    auto numFeat = mainLayer->featureCount();

    theVisualizationWidget->zoomToLayer(mainLayer);

    auto layerId = mainLayer->id();

    theVisualizationWidget->registerLayerForSelection(layerId,this);

    // Create the selected building layer
    auto selectedFeaturesLayer = theVisualizationWidget->addVectorLayer("linestring","Selected Pipelines");

    if(selectedFeaturesLayer == nullptr)
    {
        this->errorMessage("Error adding the selected assets vector layer");
        return -1;
    }

    QgsLineSymbol* selectedLayerMarkerSymbol = new QgsLineSymbol();

    selectedLayerMarkerSymbol->setWidth(0.8);
    selectedLayerMarkerSymbol->setColor(Qt::yellow);
    theVisualizationWidget->createSimpleRenderer(selectedLayerMarkerSymbol,selectedFeaturesLayer);

    auto pr2 = selectedFeaturesLayer->dataProvider();

    auto res2 = pr2->addAttributes(attribFields);

    if(!res2)
        this->errorMessage("Error adding attributes to the layer");

    selectedFeaturesLayer->updateFields(); // tell the vector layer to fetch changes from the provider

    theComponentDb->setSelectedLayer(selectedFeaturesLayer);

    QVector<QgsMapLayer*> mapLayers;
    mapLayers.push_back(selectedFeaturesLayer);
    mapLayers.push_back(mainLayer);

    theVisualizationWidget->createLayerGroup(mapLayers,"Pipelines");

    return 0;
}


void QGISGasPipelineInputWidget::clear()
{
    ComponentInputWidget::clear();
}



//Renderer* QGISGasPipelineInputWidget::createSelectedPipelineRenderer(double outlineWidth)
//{

//    SimpleLineSymbol* lineSymbol1 = new SimpleLineSymbol(SimpleLineSymbolStyle::Solid, QColor(0, 0, 0), 5.0f /*width*/, this);
//    SimpleLineSymbol* lineSymbol2 = new SimpleLineSymbol(SimpleLineSymbolStyle::Solid, QColor(255,255,178), 5.0f /*width*/, this);
//    SimpleLineSymbol* lineSymbol3 = new SimpleLineSymbol(SimpleLineSymbolStyle::Solid, QColor(253,204,92), 5.0f /*width*/, this);
//    SimpleLineSymbol* lineSymbol4 = new SimpleLineSymbol(SimpleLineSymbolStyle::Solid, QColor(253,141,60),  5.0f /*width*/, this);
//    SimpleLineSymbol* lineSymbol5 = new SimpleLineSymbol(SimpleLineSymbolStyle::Solid, QColor(240,59,32),  5.0f /*width*/, this);
//    SimpleLineSymbol* lineSymbol6 = new SimpleLineSymbol(SimpleLineSymbolStyle::Solid, QColor(189,0,38),  5.0f /*width*/, this);

//    QList<ClassBreak*> classBreaks;

//    auto classBreak1 = new ClassBreak("0.0-0.001 number of repairs", "0.0-0.001 number of repairs", -0.00001, 1E-03, lineSymbol1, this);
//    classBreaks.append(classBreak1);

//    auto classBreak2 = new ClassBreak("0.001-0.01 number of repairs", "0.001-0.01 number of repairs", 1.00E-03, 1.00E-02, lineSymbol2, this);
//    classBreaks.append(classBreak2);

//    auto classBreak3 = new ClassBreak("0.01-0.1 number of repairs", "0.01-0.1 number of repairs", 1.00E-02, 1.00E-01, lineSymbol3, this);
//    classBreaks.append(classBreak3);

//    auto classBreak4 = new ClassBreak("0.1-1.0 number of repairs", "0.1-1.0 number of repairs", 1.00E-01, 1.00E+00, lineSymbol4, this);
//    classBreaks.append(classBreak4);

//    auto classBreak5 = new ClassBreak("1.0-10.0 number of repairs", "1.0-10.0 number of repairs", 1.00E+00, 1.00E+01, lineSymbol5, this);
//    classBreaks.append(classBreak5);

//    auto classBreak6 = new ClassBreak("10.0-100.0 number of repairs", "Loss Ratio Between 75% and 90%", 1.00E+01, 1.00E+10, lineSymbol6, this);
//    classBreaks.append(classBreak6);

//    return new ClassBreaksRenderer("RepairRate", classBreaks, this);
//}




