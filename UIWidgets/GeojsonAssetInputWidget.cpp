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
#include "GISAssetInputWidget.h"
#include "MultiComponentR2D.h"

#include <qgslinesymbol.h>
#include <qgsmarkersymbol.h>

#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QSplitter>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QJsonArray>

GeojsonAssetInputWidget::GeojsonAssetInputWidget(QWidget *parent, VisualizationWidget* visWidget, QString componentType, QString appType) : SimCenterAppWidget(parent), componentType(componentType), appType(appType)
{
    theVisualizationWidget = static_cast<QGISVisualizationWidget*>(visWidget);
    assert(theVisualizationWidget);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

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


bool GeojsonAssetInputWidget::copyFiles(QString &destName)
{

    // Copy the files over for the various widgets
//    for (auto&& it : theAssetInputWidgetList)
//    {
//        if(!it->copyFiles(destName))
//            return false;
//    }

    return true;
}


bool GeojsonAssetInputWidget::outputAppDataToJSON(QJsonObject &jsonObject)
{

    jsonObject["Application"]="GEOJSON_TO_ASSET";

    QJsonObject data;

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


//    // Input the
//    res = theRoadwaysWidget->inputAppDataFromJSON(roadwaysData);

//    if(!res)
//        return res;


    return true;
}


void GeojsonAssetInputWidget::clear()
{
    theAssetInputWidgetList.clear();
    theAssetLayerList.clear();
    mainAssetWidget->removeAllComponents();
    componentFileLineEdit->clear();
}


void GeojsonAssetInputWidget::chooseAssetFileDialog(void)
{
    auto newPathToComponentInputFile = QFileDialog::getOpenFileName(this,tr("Geojson Asset File"));

    // Return if the user cancels
    if(newPathToComponentInputFile.isEmpty())
        return;

    // Clear the existing widget data
    mainAssetWidget->removeAllComponents();

    // Set file name & entry in qLine edit
    componentFileLineEdit->setText(newPathToComponentInputFile);

    this->loadAssetData();

    return;
}



void GeojsonAssetInputWidget::loadAssetData(void)
{
    QString pathGeojson = componentFileLineEdit->text();
      QFile jsonFile(pathGeojson);

    QMap<QString, QList<QJsonObject>> assetDictionary;

    QJsonObject crs;
    if (jsonFile.exists() && jsonFile.open(QFile::ReadOnly)) {

        QJsonDocument exDoc = QJsonDocument::fromJson(jsonFile.readAll());
        QJsonObject jsonObject = exDoc.object();

        if(jsonObject.contains("crs"))
            crs = jsonObject["crs"].toObject();

        QJsonArray features = jsonObject["features"].toArray();

        for (const QJsonValue& valueIt : features) {

            QJsonObject value = valueIt.toObject();

            // Get the type
            QJsonObject assetProperties = value["properties"].toObject();

            if (!jsonObject.contains("type")) {
                this->errorMessage("The Json object is missing the 'type' key that defines the asset type");
                return;
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
        this->errorMessage("Failed to open file at location: "+pathGeojson);
        return;
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

        QDir tempDir(QDir::tempPath());
        QString outputFile = tempDir.absolutePath() + QDir::separator() + assetType + ".geojson";

        QFile file(outputFile);
        if (!file.open(QFile::WriteOnly | QFile::Text))
        {
            this->errorMessage("Error creating the asset output json file in GeojsonAssetInputWidget");
            return;
        }

        // Write the file to the folder
        QJsonDocument doc(assetDictionary);
        file.write(doc.toJson());
        file.close();

        this->statusMessage("Loading asset type "+assetType+" with "+ QString::number(features.size())+" features");

        auto thisAssetWidget = new GISAssetInputWidget(nullptr, theVisualizationWidget, assetType);

        // Hide the first label
        thisAssetWidget->getLabel1()->hide();

        // Hide the file input widgets in the asset file path layout
        auto pathLayout = thisAssetWidget->getAssetFilePathLayout();
        for (int i = 0; i < pathLayout->count(); ++i) {
            QWidget *widget = pathLayout->itemAt(i)->widget();
            if (widget) {
                widget->hide();
            }
        }

        thisAssetWidget->setPathToComponentInputFile(outputFile);
        if (!thisAssetWidget->loadAssetData()) {
            this->errorMessage("Failed to load asset data for asset type" + assetType);
            return;
        }

        theAssetLayerList.append(thisAssetWidget->getMainLayer());

        mainAssetWidget->addComponent(assetType, thisAssetWidget);

        if (!file.remove()) {
            this->errorMessage("Warning, failed to remove the temporary file "+outputFile);
        }

    }


}

