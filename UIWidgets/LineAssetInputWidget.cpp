#include "LineAssetInputWidget.h"
#include "QGISVisualizationWidget.h"
#include "ComponentDatabaseManager.h"
#include "ComponentTableView.h"
#include "ComponentTableModel.h"
#include "AssetFilterDelegate.h"
#include "AssetInputDelegate.h"
#include "PointAssetInputWidget.h"
#include "qjsonarray.h"

#ifdef OpenSRA
#include "WorkflowAppOpenSRA.h"
#include "WidgetFactory.h"
#include "JsonGroupBoxWidget.h"
#endif

#include <qgsfeature.h>
#include <qgslinesymbol.h>

#include <QFileInfo>


LineAssetInputWidget::LineAssetInputWidget(QWidget *parent, VisualizationWidget* visWidget, QString assetType, QString appType) : AssetInputWidget(parent, visWidget, assetType, appType)
{

#ifdef OpenSRA
    LineAssetInputWidget::createComponentsBox();
#endif

}


int LineAssetInputWidget::loadAssetVisualization(void)
{

    auto headers = this->getTableHorizontalHeadings();

#ifdef OpenSRA
    auto indexLatStart = headers.indexOf("LAT_BEGIN");
    auto indexLonStart = headers.indexOf("LON_BEGIN");
    auto indexLatEnd = headers.indexOf("LAT_END");
    auto indexLonEnd = headers.indexOf("LON_END");
#else
    auto indexLatStart = headers.indexOf("LAT_BEGIN");
    auto indexLonStart = headers.indexOf("LONG_BEGIN");
    auto indexLatEnd = headers.indexOf("LAT_END");
    auto indexLonEnd = headers.indexOf("LONG_END");
#endif

    if(indexLatStart == -1 || indexLonStart == -1 || indexLatEnd == -1 || indexLonEnd == -1)
    {
        //Check if nodes are provided in lieu of lat and lon coordinates for the start and end
        // If they are, the pipelines visualization should be handled separately
        if(headers.indexOf("node1") != -1 && headers.indexOf("node2") != -1)
        {
            if(theNodesWidget != nullptr)
            {
                auto numAssets = theNodesWidget->getNumberOfAseets();

                if(numAssets == 0)
                {
                    this->errorMessage("The header columns 'node1' and 'node2' were found in the pipeline input file, but no nodes are found. Please input the nodes before the pipelines.");
                    this->clear();
                    return -1;
                }

                return 0;
            }

            return -1;
        }

        this->errorMessage("Could not find the required lat./lon. header labels in the input file. Alternatively, provide start and end nodes. In the pipeline input file, supply the node IDs under the headers 'node1' and 'node2'");
        return -1;
    }

    QgsFields featFields;
    featFields.append(QgsField("ID", QVariant::Int));
    featFields.append(QgsField("AssetType", QVariant::String));
    featFields.append(QgsField("TabName", QVariant::String));

    // Set the table headers as fields in the table
    for(int i = 1; i<componentTableWidget->columnCount(); ++i)
    {
        auto fieldText = componentTableWidget->horizontalHeaderItemVariant(i);
        featFields.append(QgsField(fieldText.toString(),fieldText.type()));
    }

    auto attribFields = featFields.toList();

    // Create the pipelines layer
    mainLayer = theVisualizationWidget->addVectorLayer("linestring","All Pipelines");

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

    filterDelegateWidget  = new AssetFilterDelegate(mainLayer);

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
//        auto uid = theVisualizationWidget->createUniqueID();

        // "ID"
        // "AssetType"
        // "Tabname"

        featureAttributes[0] = QVariant(pipelineID);
        featureAttributes[1] = QVariant(QString(assetType).remove(" "));
        featureAttributes[2] = QVariant("ID: "+QString::number(pipelineID));

        // The feature attributes are the columns from the table
        for(int j = 1; j<componentTableWidget->columnCount(); ++j)
        {
            auto attrbVal = componentTableWidget->item(i,j);
            featureAttributes[2+j] = attrbVal;
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
    selectedFeaturesLayer = theVisualizationWidget->addVectorLayer("linestring","Selected "+assetType);

    if(selectedFeaturesLayer == nullptr)
    {
        this->errorMessage("Error adding the selected assets vector layer");
        return -1;
    }

    QgsLineSymbol* selectedLayerMarkerSymbol = new QgsLineSymbol();

    selectedLayerMarkerSymbol->setWidth(2.0);
    selectedLayerMarkerSymbol->setColor(Qt::darkBlue);
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

    theVisualizationWidget->createLayerGroup(mapLayers, assetType);

    return 0;
}


#ifdef OpenSRA
bool LineAssetInputWidget::outputToJSON(QJsonObject &rvObject)
{

    locationWidget->outputToJSON(rvObject);

    return true;
}


bool LineAssetInputWidget::inputFromJSON(QJsonObject &rvObject)
{
    return locationWidget->inputFromJSON(rvObject);
}


void LineAssetInputWidget::createComponentsBox(void)
{
    auto methodsAndParams = WorkflowAppOpenSRA::getInstance()->getMethodsAndParamsObj();

    QJsonObject thisObj = methodsAndParams["Infrastructure"].toObject()["LineAsset"].toObject()["SiteLocationParams"].toObject();

    if(thisObj.isEmpty())
    {
        this->errorMessage("Json object is empty in " + QString(__FUNCTION__));
        return;
    }

    auto theWidgetFactory = std::make_unique<WidgetFactory>(this);

    QJsonObject paramsObj = thisObj["Params"].toObject();

    // The string given in the Methods and params json file
    QString nameStr = "SiteLocationParams";

    auto widgetLabelText = thisObj["NameToDisplay"].toString();

    if(widgetLabelText.isEmpty())
    {
        this->errorMessage("Could not find the *NameToDisplay* key in object json for " + nameStr);
        return;
    }

    locationWidget = new JsonGroupBoxWidget(this);
    locationWidget->setObjectName(nameStr);

    locationWidget->setTitle(widgetLabelText);

#ifdef OpenSRA
    QJsonObject paramsLocs;
    paramsLocs["LatBegin"] = paramsObj.value("LatBegin");
    paramsLocs["LonBegin"] = paramsObj.value("LonBegin");
    paramsLocs["LatEnd"] = paramsObj.value("LatEnd");
    paramsLocs["LonEnd"] = paramsObj.value("LonEnd");

    auto displayOrderArray = thisObj["DisplayOrder"].toArray();
    auto displayOrderVarList = displayOrderArray.toVariantList();

    QStringList displayOrder;
    for(auto&& varnt : displayOrderVarList)
        displayOrder.append(varnt.toString());

    auto locsLayout = theWidgetFactory->getLayoutFromParams(paramsLocs,nameStr,locationWidget, Qt::Horizontal, displayOrder);

//    QVBoxLayout* latLonLayout = new QVBoxLayout();
//    latLonLayout->addLayout(latLayout);
//    latLonLayout->addLayout(lonLayout);

    locationWidget->setLayout(locsLayout);
#else
    QJsonObject paramsLat;
    paramsLat["LatBegin"] = paramsObj.value("LatBegin");
    paramsLat["LatMid"] = paramsObj.value("LatMid");
    paramsLat["LatEnd"] = paramsObj.value("LatEnd");

    QJsonObject paramsLon;
    paramsLon["LonBegin"] = paramsObj.value("LonBegin");
    paramsLon["LonMid"] = paramsObj.value("LonMid");
    paramsLon["LonEnd"] = paramsObj.value("LonEnd");

    auto latLayout = theWidgetFactory->getLayoutFromParams(paramsLat,nameStr,locationWidget, Qt::Horizontal);
    auto lonLayout = theWidgetFactory->getLayoutFromParams(paramsLon,nameStr,locationWidget, Qt::Horizontal);

    QVBoxLayout* latLonLayout = new QVBoxLayout();
    latLonLayout->addLayout(latLayout);
    latLonLayout->addLayout(lonLayout);

    locationWidget->setLayout(latLonLayout);
#endif

    // Insert the widget three rows from the bottom
    insertWidgetIntoLayout(locationWidget,3);

}

#endif


void LineAssetInputWidget::clear()
{
    AssetInputWidget::clear();
}


void LineAssetInputWidget::setTheNodesWidget(PointAssetInputWidget *newTheNodesWidget)
{
    theNodesWidget = newTheNodesWidget;
}



