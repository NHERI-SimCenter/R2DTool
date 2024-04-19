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

#include "InpFileWaterInputWidget.h"
#include "QGISVisualizationWidget.h"
#include "GISAssetInputWidget.h"
#include "MultiComponentR2D.h"

#include <qgslinesymbol.h>
#include <qgsmarkersymbol.h>
#include <qgsjsonutils.h>

#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QSplitter>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QJsonArray>
#include <QApplication>
#include <QStandardPaths>
#include <QJsonObject>
#include <QTemporaryFile>
#include <QJsonDocument>

InpFileWaterInputWidget::InpFileWaterInputWidget(QWidget *parent, VisualizationWidget* visWidget, QString componentType, QString appType) : SimCenterAppWidget(parent), componentType(componentType), appType(appType)
{
    theVisualizationWidget = static_cast<QGISVisualizationWidget*>(visWidget);
    assert(theVisualizationWidget);

    mainLayout = new QVBoxLayout(this);

    auto pathTextLabel = new QLabel("Path to file:");

    inpFileLineEdit = new QLineEdit();
    //    inpFileLineEdit->setMaximumWidth(750);
    inpFileLineEdit->setMinimumWidth(400);
    inpFileLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);


    auto browseFileButton = new QPushButton();
    browseFileButton->setText(tr("Browse"));
    browseFileButton->setMaximumWidth(150);
    connect(browseFileButton,SIGNAL(clicked()),this,SLOT(chooseAssetFileDialog()));

    auto pathLayout = new QHBoxLayout();
    pathLayout->addWidget(pathTextLabel);
    pathLayout->addWidget(inpFileLineEdit);
    pathLayout->addWidget(browseFileButton);

    mainLayout->addLayout(pathLayout);

        // The CRS selection
    crsSelectorWidget = new CRSSelectionWidget();
    connect(crsSelectorWidget,&CRSSelectionWidget::crsChanged,this,&InpFileWaterInputWidget::handleLayerCrsChanged);
    mainLayout->addWidget(crsSelectorWidget);

    mainAssetWidget = new MultiComponentR2D("Assets", nullptr);

    mainLayout->addWidget(mainAssetWidget);


    mainLayout->insertStretch(-1);

//    // Testing to remove start
//    inpFileLineEdit->setText("/Users/steve/Desktop/sinna.geojson");
//    this->loadAssetData();
//    // Testing to remove end
}


InpFileWaterInputWidget::~InpFileWaterInputWidget()
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
void InpFileWaterInputWidget::insertLineEditToMainLayout(int index, QWidget* widget, QLineEdit* LineEdit, QString componentType, QString FieldName){
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

void InpFileWaterInputWidget::handleLayerCrsChanged(const QgsCoordinateReferenceSystem & val)
{
    crsAuthID = val.authid();
    
    QList<QString> activeComponents = mainAssetWidget->getActiveComponents();
    if (!activeComponents.isEmpty()){
        for(QString it : activeComponents)
        {
            auto activeComp = dynamic_cast<GISAssetInputWidget*>(mainAssetWidget->getComponent(it));

            if(activeComp == nullptr)
                continue;

            activeComp->setCRS(val);
        }
    }
}


bool InpFileWaterInputWidget::copyFiles(QString &destDir)
{
    qDebug()<<"--------------\n"<<"Sina in copyFiles\n";
    // create dir and copy motion files
    QDir destDIR(destDir);
    if (!destDIR.exists()) {
        qDebug() << "userInputGMWidget::copyFiles dest dir does not exist: " << destDir;
        return false;
    }

    auto compLineEditText = inpFileLineEdit->text();

    QFileInfo componentFile(compLineEditText);
    QFileInfo geoJSONFile(geoJsonFileName);    

    if (!componentFile.exists())
    {
        this->errorMessage("The asset file path does not exist. Did you load any assets?");
        return false;
    }

    if (!geoJSONFile.exists())
    {
        this->errorMessage("The geoJSON file does not exist! Please report the bug to SimCenter.");
        return false;
    }   else {
      
      // add crs to geoJSON file
      
      QFile fileJSON(geoJsonFileName);          
      if(fileJSON.open( QIODevice::ReadOnly)) {

        QByteArray bytes = fileJSON.readAll();
        fileJSON.close();

        QJsonParseError jsonError;
        QJsonDocument document = QJsonDocument::fromJson(bytes, &jsonError);
        if(jsonError.error != QJsonParseError::NoError) {
	  qDebug() << "InpFileWater::copyFiles failed to open file to read: " << jsonError.errorString();
	  return false;
        }
        if(document.isObject()) {
	  QJsonObject jsonObj = document.object();
	  jsonObj.remove(QString("crs"));
	  jsonObj.remove(QString("CRS"));
	  QJsonObject crsObject;
	  QJsonObject propertiesObject;
	  propertiesObject.insert(QString("name"), crsAuthID);
	  crsObject.insert(QString("type"), QString("name"));
	  crsObject.insert(QString("properties"), propertiesObject);
	  jsonObj.insert("crs", crsObject);
	  crsSelectorWidget->outputAppDataToJSON(crsObject);
	  document.setObject(jsonObj);
	  if(!fileJSON.open( QIODevice::WriteOnly)) {
	    qDebug() << "InpFileWaater::copyFILES failed to open file to write:  " << jsonError.errorString();
	    return false;	    
	  }
	  fileJSON.write(document.toJson());
	  fileJSON.close();
	}
      }
    }
    
    QList<QString> activeComponents = mainAssetWidget->getActiveComponents();
    if (!activeComponents.isEmpty()){
      for(QString it : activeComponents)
        {
	  auto activeComp = dynamic_cast<GISAssetInputWidget*>(mainAssetWidget->getComponent(it));

	  if(activeComp == nullptr)
	    continue;
        }
    }
    
    this->copyFile(geoJsonFileName, destDir);
    return this->copyFile(compLineEditText, destDir);
}


bool InpFileWaterInputWidget::outputAppDataToJSON(QJsonObject &jsonObject)
{
    qDebug()<<"--------------\n"<<"Sina in outputAppDataToJSON\n";
    //jsonObject["Application"]="GEOJSON_TO_ASSET";
    jsonObject["Application"]="INP_FILE";

    QJsonObject data;

    QFileInfo inpFile(inpFileLineEdit->text());
    if (inpFile.exists()){
        data.insert("inpFile", inpFile.absoluteFilePath());
    } else {
        this->errorMessage("Cannot find inp file" + inpFileLineEdit->text());
	return false;
    }

    QFileInfo componentFile(geoJsonFileName);
    if (componentFile.exists()){
        data.insert("assetSourceFile", componentFile.absoluteFilePath());
    } else {
        this->errorMessage("Cannot find the tmp GeoJSON file created: " + geoJsonFileName + " This is a bug contact SimCenter");
    }
    
    QList<QString> activeComponents = mainAssetWidget->getActiveComponents();

    if (!activeComponents.isEmpty()){
        for(QString it : activeComponents)
        {
            auto activeComp = dynamic_cast<GISAssetInputWidget*>(mainAssetWidget->getComponent(it));

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

    for (auto&& it : theAssetInputWidgetList)
    {
        auto assetType = it->getAssetType();

        QJsonObject assetData;

        // The file containing the network bridges

        if(!it->outputAppDataToJSON(assetData))
        {
            this->errorMessage("Error, could not get the .json output from the "+assetType+" widget in GIS_to_TransportNETWORK");
            return false;
        }

        data[assetType] = assetData;

    }


    jsonObject["ApplicationData"] = data;

    return true;
}


bool InpFileWaterInputWidget::inputAppDataFromJSON(QJsonObject &jsonObject)
{
    // Check the app type
    qDebug() << "------------ sina -------------\n"<<jsonObject["Application"].toString() <<"\n";
    if (jsonObject.contains("Application")) {
        if ("INP_FILE" != jsonObject["Application"].toString()) {
            this->errorMessage("InpFileWaterInputWidget::inputFRommJSON app name conflict");
            return false;
        }
    }


    if (!jsonObject.contains("ApplicationData"))
    {
        this->errorMessage("Could not find the 'ApplicationData' key in 'InpFileWaterInputWidget' input");
        return false;
    }

    QJsonObject appData = jsonObject["ApplicationData"].toObject();

    QString fileName;

    //qDebug() << "------------ alani -------------\n"<<jsonObject["ApplicationData"] <<"\n";
    if (appData.contains("inpFile")){
        fileName = appData["inpFile"].toString();
    }
    //qDebug() << "------------ alani -------------\n"<<fileName <<"\n";
    QFileInfo fileInfo(fileName);

    if (fileInfo.exists()) {

        inpFileLineEdit->setText(fileInfo.absoluteFilePath());

        if(this->loadAssetData() == false)
            return false;

    } else {
        QString pathToComponentInputFile = QDir::currentPath() + QDir::separator() +
            "input_data" + QDir::separator() + fileName;
        if (fileInfo.exists(pathToComponentInputFile)) {
            inpFileLineEdit->setText(pathToComponentInputFile);

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
            auto activeComp = dynamic_cast<GISAssetInputWidget*>(mainAssetWidget->getComponent(it));

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


void InpFileWaterInputWidget::clear()
{
    theAssetInputWidgetList.clear();
    theAssetLayerList.clear();
    mainAssetWidget->removeAllComponents();
    inpFileLineEdit->clear();
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


void InpFileWaterInputWidget::chooseAssetFileDialog(void)
{
    auto newPathToComponentInputFile = QFileDialog::getOpenFileName(this,tr("Geojson Asset File"));

    // Return if the user cancels
    if(newPathToComponentInputFile.isEmpty())
        return;

    // Clear the existing widget data
    mainAssetWidget->removeAllComponents();

    // Set file name & entry in qLine edit
    inpFileLineEdit->setText(newPathToComponentInputFile);

    this->loadAssetData();

    return;
}

extern "C" int createJSON(const char *, const char *, const char *, const char *);

bool InpFileWaterInputWidget::loadAssetData(void)
{
    QString pathInpFileWater = inpFileLineEdit->text();

    //
    // fmk - create a tmp geoJSON file and then use that file subequently in Steves code
    //

    //    QFile inpFile(pathInpFileWater);
    
    QString writableLocation = QStandardPaths::writableLocation(QStandardPaths::StandardLocation::AppLocalDataLocation);
    QDir writableDir(writableLocation);
    if(!writableDir.exists())
        writableDir.mkpath(".");
    
    geoJsonFileName = writableDir.filePath("sc_inpFileGeoJSON.json");
    QString tmp1 = writableDir.filePath("SimCenter.thing1");
    QString tmp2 = writableDir.filePath("SimCenter.thing2");
    
    qDebug() << pathInpFileWater;
    qDebug() << tmp1;
    qDebug() << tmp2;
    qDebug() << geoJsonFileName;    
    
    createJSON(pathInpFileWater.toStdString().c_str(),
	       tmp1.toStdString().c_str(),
	       tmp2.toStdString().c_str(),
	       geoJsonFileName.toStdString().c_str());
	qDebug() << "SINA JSON FILE: "<< geoJsonFileName<<"\n";
    QFile jsonFile(geoJsonFileName);
    // back to Stevan    

    QMap<QString, QList<QJsonObject>> assetDictionary;

    QJsonObject crs;
    if (jsonFile.exists() && jsonFile.open(QFile::ReadOnly)) {
      
      QJsonDocument exDoc = QJsonDocument::fromJson(jsonFile.readAll());
      QJsonObject jsonObject = exDoc.object();
      
      if(jsonObject.contains("crs"))
	crs = jsonObject["crs"].toObject();
      QString crsString = crs["properties"].toObject()["name"].toString();
      QgsCoordinateReferenceSystem qgsCRS = QgsCoordinateReferenceSystem(crsString);
      if (!qgsCRS.isValid()){
	qgsCRS.createFromOgcWmsCrs(crsString);
      }
      if (!qgsCRS.isValid()){
	QString msg = "INP File does not have a CRS defined. Choose an existing CRS. ";
	//QString msg = "The CRS: " + crsString + " defined in " + geoJsonFileName + " is invalid and ignored.";
	errorMessage(msg);
      }
	
        crsSelectorWidget->setCRS(qgsCRS);
        QJsonArray features = jsonObject["features"].toArray();

        for (const QJsonValue& valueIt : features) {

            QJsonObject value = valueIt.toObject();

            // Get the type
            QJsonObject assetProperties = value["properties"].toObject();

            if (!jsonObject.contains("type")) {
                this->errorMessage("The Json object is missing the 'type' key that defines the asset type");
                return false;
            }

            QString assetType = assetProperties["type"].toString();

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
        this->errorMessage("Failed to open file at location: "+ geoJsonFileName);
        return false;
    }

    for (auto it = assetDictionary.begin(); it != assetDictionary.end(); ++it)
    {
        QString assetType = it.key();
        QList<QJsonObject> features = it.value();

        QJsonArray featuresArray;
        for (const auto& obj : features) {
            featuresArray.append(obj);
        }

        QJsonObject assetDictionary;
        assetDictionary["type"]="FeatureCollection";
        assetDictionary["features"]=featuresArray;

        if(!crs.isEmpty())
            assetDictionary["crs"]=crs;

        //QDir tempDir(QDir::tempPath());
		QTemporaryFile tempFile;
		
		tempFile.setFileTemplate(tempFile.fileTemplate() + "_" + assetType + ".geojson");
        //QString outputFile = tempDir.absolutePath() + QDir::separator() + assetType + ".geojson";

        //QFile file(outputFile);
        qDebug() << "SINA: "<<tempFile<<"\n";
		if (tempFile.open())
        {

            QString outputFile = tempFile.fileName();

            // Write the file to the folderd
            QJsonDocument doc(assetDictionary);
            tempFile.write(doc.toJson());

			// Close the file
            tempFile.close();

            this->statusMessage("Loading asset type "+assetType+" with "+ QString::number(features.size())+" features");

            GISAssetInputWidget *thisAssetWidget = new GISAssetInputWidget(nullptr, theVisualizationWidget, assetType);

            // Hide the first label
            /*
        thisAssetWidget->getLabel1()->hide();
        thisAssetWidget->getCRSSelectorWidget()->hide();

        // Hide the file input widgets in the asset file path layout
        auto pathLayout = thisAssetWidget->getAssetFilePathLayout();
        for (int i = 0; i < pathLayout->count(); ++i) {
            QWidget *widget = pathLayout->itemAt(i)->widget();
            if (widget) {
                widget->hide();
            }
        }
        */

            thisAssetWidget->hideCRS_Selection();
            thisAssetWidget->hideAssetFilePath();

            thisAssetWidget->setPathToComponentInputFile(outputFile);
            if (!thisAssetWidget->loadAssetData(false)) {
                this->errorMessage("Failed to load asset data for asset type" + assetType);
                return false;
            }

            theAssetLayerList.append(thisAssetWidget->getMainLayer());

            mainAssetWidget->addComponent(assetType, thisAssetWidget);


            // Automatically delete the temporary file when it goes out of scope or when the application exits
            tempFile.setAutoRemove(true);
        }
        else {
            this->errorMessage("Failed to create temporary file for " + assetType);
        }


        if (ComponentTypeToAdditionalWidget.contains(assetType)){
            for (QWidget* it:ComponentTypeToAdditionalWidget[assetType]){
                it->show();
            }
        }

    }



    return true;


}