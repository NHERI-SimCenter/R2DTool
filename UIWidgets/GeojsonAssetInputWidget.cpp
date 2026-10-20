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

#include "GeojsonAssetInputWidget.h"
#include "QGISVisualizationWidget.h"
#include "GeojsonAssetTypeWidget.h"
#include "MultiComponentR2D.h"
#include "SimCenterPreferences.h"

#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QJsonArray>
#include <QSet>

GeojsonAssetInputWidget::GeojsonAssetInputWidget(QWidget *parent, VisualizationWidget* visWidget, QString componentType, QString appType)
    : SimCenterAppWidget(parent), componentType(componentType), appType(appType)
{
    theVisualizationWidget = static_cast<QGISVisualizationWidget*>(visWidget);
    assert(theVisualizationWidget);

    mainLayout = new QVBoxLayout(this);

    auto pathTextLabel = new QLabel("Path to file:");

    componentFileLineEdit = new QLineEdit();
    //    componentFileLineEdit->setMaximumWidth(750);
    componentFileLineEdit->setMinimumWidth(400);
    componentFileLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);


    auto browseFileButton = new QPushButton();
    browseFileButton->setText(tr("Browse"));
    browseFileButton->setMaximumWidth(150);
    connect(browseFileButton,SIGNAL(clicked()),this,SLOT(chooseAssetFileDialog()));

    auto pathLayout = new QHBoxLayout();
    pathLayout->addWidget(pathTextLabel);
    pathLayout->addWidget(componentFileLineEdit);
    pathLayout->addWidget(browseFileButton);

    mainLayout->addLayout(pathLayout);

    // The CRS selection
    crsSelectorWidget = new CRSSelectionWidget();
    connect(crsSelectorWidget,&CRSSelectionWidget::crsChanged,this,&GeojsonAssetInputWidget::handleLayerCrsChanged);
    mainLayout->addWidget(crsSelectorWidget);

    mainAssetWidget = new MultiComponentR2D("Assets", nullptr);

    mainLayout->addWidget(mainAssetWidget);


    mainLayout->insertStretch(-1);

//    // Testing to remove start
//    componentFileLineEdit->setText("/Users/steve/Desktop/sinna.geojson");
//    this->loadAssetData();
//    // Testing to remove end
}


GeojsonAssetInputWidget::~GeojsonAssetInputWidget()
{

}

/**
 * Insert a widget containing line edit to the main layout.
 *
 * See adding road length option for highway roadway as an example.
 *
 * @param index The location to insert the widget.
 * @param *widget A widge that contains a label and a linewideget and has been assigned with a layout
 * @param *LineEdit A pointer to the QLineEdit widget in the widget
 * @return void.
 */
void GeojsonAssetInputWidget::insertLineEditToMainLayout(int index, QWidget* widget, QLineEdit* LineEdit, QString componentType, QString FieldName){
    mainLayout -> insertWidget(index, widget);
    widget -> hide();
    LineEditList.append(LineEdit);
    FieldNameToLineEdit[FieldName] = LineEdit;
    if (!ComponentTypeToAdditionalWidget.contains(componentType)){
        ComponentTypeToAdditionalWidget[componentType] = QList<QWidget*>({widget});
    } else {
        QList<QWidget*>& widgetList = ComponentTypeToAdditionalWidget[componentType];
        widgetList.append(widget);
    }
    if (!ComponentTypeToFieldNames.contains(componentType)){
        ComponentTypeToFieldNames[componentType] = QList<QString>({FieldName});
    } else {
        QList<QString>& FieldList = ComponentTypeToFieldNames[componentType];
        FieldList.append(FieldName);
    }


}

void GeojsonAssetInputWidget::handleLayerCrsChanged(const QgsCoordinateReferenceSystem & val)
{
    QList<QString> activeComponents = mainAssetWidget->getActiveComponents();
    if (!activeComponents.isEmpty()){
        for(QString it : activeComponents)
        {
            auto activeComp = dynamic_cast<GeojsonAssetTypeWidget*>(mainAssetWidget->getComponent(it));

            if(activeComp == nullptr)
                continue;

            activeComp->setCRS(val);
        }
    }
}


bool GeojsonAssetInputWidget::copyFiles(QString &destDir)
{
    // create dir and copy motion files
    QDir destDIR(destDir);
    if (!destDIR.exists()) {
        qDebug() << "userInputGMWidget::copyFiles dest dir does not exist: " << destDir;
        return false;
    }

    auto compLineEditText = componentFileLineEdit->text();

    QFileInfo componentFile(compLineEditText);

    if (!componentFile.exists())
    {
        this->errorMessage("The asset file path does not exist. Did you load any assets?");
        return false;
    }

    QList<QString> activeComponents = mainAssetWidget->getActiveComponents();
    if (!activeComponents.isEmpty()){
        for(QString it : activeComponents)
        {
            auto activeComp = dynamic_cast<AssetInputWidget*>(mainAssetWidget->getComponent(it));

            if(activeComp == nullptr)
                continue;
// Try to write a geojson file containing only the selected features
//            QgsVectorLayer* mainLayer = activeComp->getMainLayer();
//            int numerFeature = mainLayer->featureCount();
//            QgsFeature feat = mainLayer->getFeature(0);
//            QgsJsonExporter exportor = QgsJsonExporter(mainLayer);
//            QString featString = exportor.exportFeature(feat);
//            QgsFeature feat2 = mainLayer->getFeature(1);
//            QString featString2 = exportor.exportFeature(feat2);
//            this->errorMessage(featString2);

        }
    }


//    auto pathToSaveFile = destName + QDir::separator() + componentFile.fileName();
    return this->copyFile(compLineEditText, destDir);

}


bool GeojsonAssetInputWidget::outputAppDataToJSON(QJsonObject &jsonObject)
{

    jsonObject["Application"]="GEOJSON_TO_ASSET";

    QJsonObject data;

    QFileInfo componentFile(componentFileLineEdit->text());
    if (componentFile.exists()){
        data.insert("assetSourceFile", componentFile.fileName());
        data["pathToSource"]=componentFile.absoluteDir().path();
    } else {
        this->errorMessage("Cannot find GeoJSON file " + componentFileLineEdit->text());
        return false;
    }

//    data.insert("assetType",componentType);

    QList<QString> activeComponents = mainAssetWidget->getActiveComponents();

    if (!activeComponents.isEmpty()){
        for(QString it : activeComponents)
        {
            auto activeComp = dynamic_cast<AssetInputWidget*>(mainAssetWidget->getComponent(it));

            if(activeComp == nullptr)
                return false;

            QJsonObject subTypeAppData;
            subTypeAppData.insert("filter", activeComp->getFilterString());

            if(ComponentTypeToFieldNames.contains(it)){
                QList<QString> FieldList = ComponentTypeToFieldNames[it];
                for (QString field : FieldList){
                    QLineEdit* lineEdit = FieldNameToLineEdit[field];
                    QString value = lineEdit->text();
                    subTypeAppData.insert(field, value);
                }
            }

            data.insert(it, subTypeAppData);
        }
    }

    jsonObject["ApplicationData"] = data;

    return true;
}


bool GeojsonAssetInputWidget::inputAppDataFromJSON(QJsonObject &jsonObject)
{

    // Check the app type
    if (jsonObject.contains("Application")) {
        if ("GEOJSON_TO_ASSET" != jsonObject["Application"].toString()) {
            this->errorMessage("GeojsonAssetInputWidget::inputFRommJSON app name conflict");
            return false;
        }
    }


    if (!jsonObject.contains("ApplicationData"))
    {
        this->errorMessage("Could not find the 'ApplicationData' key in 'GeojsonAssetInputWidget' input");
        return false;
    }

    QJsonObject appData = jsonObject["ApplicationData"].toObject();

    QString fileName;

    if (appData.contains("assetSourceFile")){
        fileName = appData["assetSourceFile"].toString();
    }

    QFileInfo fileInfo(fileName);

    if (fileInfo.exists()) {

        componentFileLineEdit->setText(fileInfo.absoluteFilePath());

        if(this->loadAssetData() == false)
            return false;

    } else {
        QString pathToSource;
        if (appData.contains("pathToSource")){
            pathToSource = appData["pathToSource"].toString();
        } else{
            pathToSource = QDir::currentPath() + QDir::separator() + "input_data";
        }
        QString pathToComponentInputFile = pathToSource + QDir::separator() + fileName;
        if (fileInfo.exists(pathToComponentInputFile)) {
            componentFileLineEdit->setText(pathToComponentInputFile);

            if(this->loadAssetData() == false)
               return false;
        } else {
            QString errMessage = appType + " - The file " + fileName + " could not be found";
            this->errorMessage(errMessage);
            return false;
        }
    }

    QList<QString> activeComponents = mainAssetWidget->getActiveComponents();

    if (!activeComponents.isEmpty()){
        for(QString it : activeComponents)
        {
            auto activeComp = dynamic_cast<AssetInputWidget*>(mainAssetWidget->getComponent(it));

            if(activeComp == nullptr)
                return false;

            if(appData.contains(it)){
                QJsonObject subTypeAppData = appData[it].toObject();
                if (subTypeAppData.contains("filter")){
                   activeComp->setFilterString(subTypeAppData["filter"].toString());
                }
                if(ComponentTypeToFieldNames.contains(it)){
                   QList<QString> FieldList = ComponentTypeToFieldNames[it];
                   for (QString field : FieldList){
                       if(subTypeAppData.contains(field)){
                           QLineEdit* lineEdit = FieldNameToLineEdit[field];
                           lineEdit->setText(QString::number(subTypeAppData[field].toDouble()));
                       }
                   }
                }

            }
        }
    }


    return true;
}


void GeojsonAssetInputWidget::removeAllAssets(void)
{
    // Remove the layers first, removeAllComponents deletes the widgets the map would otherwise still select into
    for(auto&& it : theAssetWidgetList)
        it->removeLayers();

    theAssetWidgetList.clear();
    theAssetLayerList.clear();
    mainAssetWidget->removeAllComponents();
}


void GeojsonAssetInputWidget::clear()
{
    this->removeAllAssets();
    componentFileLineEdit->clear();
    for(auto field : FieldNameToLineEdit.keys())
    {
        FieldNameToLineEdit.value(field)->clear();
    }
    for(auto componentType:ComponentTypeToAdditionalWidget.keys()){
        QList<QWidget*> widgetList = ComponentTypeToAdditionalWidget.value(componentType);
        for (QWidget* widget : widgetList){
            widget->hide();
        }
    }
//    for(auto activ)
}


void GeojsonAssetInputWidget::chooseAssetFileDialog(void)
{
    auto newPathToComponentInputFile = QFileDialog::getOpenFileName(this,tr("Geojson Asset File"));

    // Return if the user cancels
    if(newPathToComponentInputFile.isEmpty())
        return;

    // Set file name & entry in qLine edit
    componentFileLineEdit->setText(newPathToComponentInputFile);

    this->loadAssetData();

    return;
}



// GeojsonAssetTypeWidget loads each asset type into a single QGIS vector layer, and
// that layer only supports one geometry family (point/line/polygon) at a time -
// see GeojsonAssetTypeWidget::loadAssetVisualization(). If an asset type's features
// are all the same family we can load them as-is; only truly mixed-family asset
// types need to be collapsed down to a common (point) representation.
static QString geometryFamily(const QString& gtype)
{
    if (gtype == "Point" || gtype == "MultiPoint")
        return "point";
    else if (gtype == "LineString" || gtype == "MultiLineString")
        return "line";
    else if (gtype == "Polygon" || gtype == "MultiPolygon")
        return "polygon";

    return "other";
}


static QJsonObject geometryToMultiPoint(const QJsonObject& geom)
{
    QString gtype = geom["type"].toString();
    QJsonArray coords = geom["coordinates"].toArray();
    QJsonArray pts;

    if (gtype == "Point") {
        pts.append(coords);
    } else if (gtype == "MultiPoint") {
        pts = coords;
    } else if (gtype == "LineString") {
        pts = coords;
    } else if (gtype == "MultiLineString") {
        for (const QJsonValue& line : coords)
            for (const QJsonValue& pt : line.toArray())
                pts.append(pt);
    } else if (gtype == "Polygon") {
        for (const QJsonValue& ring : coords)
            for (const QJsonValue& pt : ring.toArray())
                pts.append(pt);
    } else if (gtype == "MultiPolygon") {
        for (const QJsonValue& poly : coords)
            for (const QJsonValue& ring : poly.toArray())
                for (const QJsonValue& pt : ring.toArray())
                    pts.append(pt);
    }

    QJsonObject result;
    result["type"] = "MultiPoint";
    result["coordinates"] = pts;
    return result;
}


bool GeojsonAssetInputWidget::loadAssetData(void)
{
    // Clear any assets from a previous load
    this->removeAllAssets();

    QString pathGeojson = componentFileLineEdit->text();
    QFile jsonFile(pathGeojson);

    QMap<QString, QList<QJsonObject>> assetDictionary;
    QMap<QString, QSet<qint64>> assetIds;

    QJsonObject crs;
    if (jsonFile.exists() && jsonFile.open(QFile::ReadOnly)) {

        QJsonParseError parseError;
        QJsonDocument exDoc = QJsonDocument::fromJson(jsonFile.readAll(), &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            this->errorMessage("Error reading " + pathGeojson + ", invalid JSON at character " + QString::number(parseError.offset) + ": " + parseError.errorString());
            return false;
        }
        if (!exDoc.isObject() || !exDoc.object()["features"].isArray()) {
            this->errorMessage("Error reading " + pathGeojson + ", it is not a GeoJSON FeatureCollection with a 'features' array");
            return false;
        }
        QJsonObject jsonObject = exDoc.object();

        // GeoJSON without a crs is WGS84 (RFC 7946)
        QgsCoordinateReferenceSystem qgsCRS("EPSG:4326");
        if(jsonObject.contains("crs")) {
            crs = jsonObject["crs"].toObject();
            QString crsString = crs["properties"].toObject()["name"].toString();
            QgsCoordinateReferenceSystem fileCRS(crsString);
            if (!fileCRS.isValid()){
                fileCRS.createFromOgcWmsCrs(crsString);
            }
            if (fileCRS.isValid()){
                qgsCRS = fileCRS;
            } else {
                QString msg = "The CRS: " + crsString + " defined in " + pathGeojson + " is invalid and ignored, using WGS84 (EPSG:4326)";
                errorMessage(msg);
                // Leave it out of the asset type files too, so QGIS also uses WGS84
                crs = QJsonObject();
            }
        }
        crsSelectorWidget->setCRS(qgsCRS);
        QJsonArray features = jsonObject["features"].toArray();

        if (features.isEmpty()) {
            this->errorMessage("There are no features in " + pathGeojson);
            return false;
        }

        int featureIndex = -1;
        for (const QJsonValue& valueIt : features) {

            ++featureIndex;
            QJsonObject value = valueIt.toObject();

            // Get the type
            QJsonObject assetProperties = value["properties"].toObject();

            QString assetType = assetProperties["type"].toString();

            if (assetType.isEmpty()) {
                this->errorMessage("Feature " + QString::number(featureIndex) + " in " + pathGeojson + " is missing the 'type' property that defines the asset type");
                return false;
            }

            // The asset id is the integer feature "id", QGIS then uses it as the feature id
            QJsonValue idValue = value["id"];
            bool idOK = false;
            qint64 assetId = 0;
            if (idValue.isDouble()) {
                double idDouble = idValue.toDouble();
                assetId = static_cast<qint64>(idDouble);
                idOK = (static_cast<double>(assetId) == idDouble);
            }

            if (!idOK) {
                QString idText = idValue.isUndefined() ? QString("missing") : QString::fromUtf8(QJsonDocument(QJsonArray({idValue})).toJson(QJsonDocument::Compact).mid(1).chopped(1));
                this->errorMessage("A " + assetType + " feature in " + pathGeojson + " has id " + idText + ", each feature must have an integer 'id'");
                return false;
            }

            QSet<qint64>& typeIds = assetIds[assetType];
            if (typeIds.contains(assetId)) {
                this->errorMessage("The id " + QString::number(assetId) + " is used by more than one " + assetType + " feature in " + pathGeojson);
                return false;
            }
            typeIds.insert(assetId);

            // The backend also takes the id from the feature, so an id in the properties is not used
            assetProperties.remove("id");
            value["properties"] = assetProperties;
            value["id"] = assetId;

            if (!assetDictionary.contains(assetType))
            {
                assetDictionary[assetType] = QList<QJsonObject>({value});
            }
            else
            {
                QList<QJsonObject>& featList = assetDictionary[assetType];
                featList.append(value);
            }
        }
    }
    else{
        this->errorMessage("Failed to open file at location: "+pathGeojson);
        return false;
    }

    for (auto it = assetDictionary.begin(); it != assetDictionary.end(); ++it)
    {
        QString assetType = it.key();
        QList<QJsonObject> features = it.value();

        // Only collapse geometries down to points if this asset type genuinely
        // mixes geometry families (e.g. some points and some lines). A single
        // QGIS layer can't render mixed families, but it can render a uniform
        // set of polygons or lines just fine, so leave those alone.
        QSet<QString> families;
        for (const auto& obj : features) {
            QJsonObject geom = obj["geometry"].toObject();
            if (!geom.isEmpty())
                families.insert(geometryFamily(geom["type"].toString()));
        }
        bool mixedGeometry = families.size() > 1;

        QJsonArray featuresArray;
        for (const auto& obj : features) {
            QJsonObject feat = obj;
            QJsonObject geom = feat["geometry"].toObject();
            if (!geom.isEmpty() && mixedGeometry)
                feat["geometry"] = geometryToMultiPoint(geom);
            featuresArray.append(feat);
        }

        QJsonObject assetDictionary;
        assetDictionary["type"]="FeatureCollection";
        assetDictionary["features"]=featuresArray;

        if(!crs.isEmpty())
            assetDictionary["crs"]=crs;

        QString workingDir = SimCenterPreferences::getInstance()->getLocalWorkDir();
        QDir dirWork(workingDir);
        if (!dirWork.exists()){
            if (!dirWork.mkpath(workingDir))
            {
                QString errorMessage = QString("Could not load the Working Directory: ") + workingDir
                                       + QString(". Change the Local Jobs Directory location in preferences.");

                this->errorMessage(errorMessage);

                return false;
            }
        }
        // Store data in a ground motions folder under hazard simulation
        workingDir += QDir::separator() + QString("temp_files");
        QDir tempDir(workingDir);
        if (!tempDir.exists()){
            if (!tempDir.mkpath(workingDir))
            {
                QString errorMessage = QString("Could not make the temp_files directory in ") + workingDir
                                       + QString(". Change the Local Jobs Directory location in preferences.");

                this->errorMessage(errorMessage);

                return false;
            }
        }
        QString outputFile = tempDir.absolutePath() + QDir::separator() + assetType + ".geojson";

        QFile file(outputFile);
        if (!file.open(QFile::WriteOnly | QFile::Text))
        {
            this->errorMessage("Error creating the asset output json file in GeojsonAssetInputWidget");
            return false;
        }

        // Write the file to the folder
        QJsonDocument doc(assetDictionary);
        file.write(doc.toJson());
        file.close();

        this->statusMessage("Loading asset type "+assetType+" with "+ QString::number(features.size())+" features");

        GeojsonAssetTypeWidget *thisAssetWidget = new GeojsonAssetTypeWidget(nullptr, theVisualizationWidget, assetType);
        theAssetWidgetList.append(thisAssetWidget);

        thisAssetWidget->setAssetFile(outputFile, assetIds[assetType]);
        if (!thisAssetWidget->loadAssetData(false)) {
            this->errorMessage("Failed to load asset data for asset type" + assetType);
            // Not yet added to mainAssetWidget, so removeAllAssets would not delete it
            thisAssetWidget->removeLayers();
            theAssetWidgetList.removeLast();
            delete thisAssetWidget;
            return false;
        }

        theAssetLayerList.append(thisAssetWidget->getMainLayer());

        mainAssetWidget->addComponent(assetType, thisAssetWidget);

//        if (!file.remove()) {
//            this->errorMessage("Warning, failed to remove the temporary file "+outputFile);
//        }


        if (ComponentTypeToAdditionalWidget.contains(assetType)){
            for (QWidget* it:ComponentTypeToAdditionalWidget[assetType]){
                it->show();
            }
        }

    }



    return true;


}

