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

// Written by: Stevan Gavrilovic

#include "AssetInputDelegate.h"
#include "DLWidget.h"
#include "GeneralInformationWidget.h"
#include "PelicunPostProcessor.h"
#include "CBCitiesPostProcessor.h"
#include "ResultsWidget.h"
#include "SimCenterPreferences.h"
#include <WorkflowAppR2D.h>
#include "sectiontitle.h"

#include <QCheckBox>
#include <QApplication>
#include <QTabWidget>
#include <QDebug>
#include <QDir>
#include <QMenu>
#include <QGridLayout>
#include <QGroupBox>
#include <QJsonObject>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPaintEngine>
#include <QPushButton>
#include <QStandardPaths>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QFileDialog>

ResultsWidget::ResultsWidget(QWidget *parent, VisualizationWidget* visWidget) : SimCenterAppWidget(parent)
{
    theVisualizationWidget = static_cast<QGISVisualizationWidget*>(visWidget);

    theParent = parent;

    DVApp = "Pelicun";

    resultsMainLabel = new QLabel("No results to display", this);

    mainStackedWidget = new QStackedWidget(this);

    mainStackedWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(5,0,0,0);

    // Header layout and objects
    QHBoxLayout *theHeaderLayout = new QHBoxLayout();
    SectionTitle *label = new SectionTitle();
    label->setText(tr("Regional Results Summary"));
    label->setMinimumWidth(150);

    auto disclaimerLabel = new QLabel("Disclaimer: The presented simulation results are not representative of any individual building’s response. To understand the response of any individual building, "
                                      "please consult with a professional structural engineer. The presented tool does not assert the known condition of the building. Just as it cannot be used to predict the negative outcome of an individual "
                                      "building, prediction of safety or an undamaged state is not assured for an individual building. Any opinions, findings, and conclusions or recommendations expressed in this material are "
                                      "those of the author(s) and do not necessarily reflect the views of the National Science Foundation.",this);
    disclaimerLabel->setStyleSheet("font: 10pt;");
    disclaimerLabel->setWordWrap(true);

    theHeaderLayout->addWidget(label);
    QSpacerItem *spacer = new QSpacerItem(10,10);
    theHeaderLayout->addItem(spacer);
    theHeaderLayout->addWidget(disclaimerLabel,Qt::AlignLeft);

    //    theHeaderLayout->addStretch(1);

    // Layout to display the results
    resultsPageWidget = new QWidget();

    mainStackedWidget->addWidget(resultsPageWidget);

    QVBoxLayout* resultsPlaceHolderLayout = new QVBoxLayout(resultsPageWidget);
    resultsPlaceHolderLayout->addStretch();
    resultsPlaceHolderLayout->addWidget(resultsMainLabel,0,Qt::AlignCenter);
    resultsPlaceHolderLayout->addStretch();

    thePelicunPostProcessor = std::make_unique<PelicunPostProcessor>(parent,theVisualizationWidget);

    theCBCitiesPostProcessor = std::make_unique<CBCitiesPostProcessor>(parent,theVisualizationWidget);

    resTabWidget = new QTabWidget();

    mainStackedWidget->addWidget(resTabWidget);

    //    // Export layout and objects
    //    QGridLayout *theExportLayout = new QGridLayout();

    //    exportLabel = new QLabel("Export folder:", this);
    //    exportPathLineEdit = new QLineEdit(this);
    //    //exportPathLineEdit->setMaximumWidth(1000);
    //    // exportPathLineEdit->setMinimumWidth(400);
    //    exportPathLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    //    QString defaultOutput = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + QDir::separator() + QString("Results.pdf");
    //    exportPathLineEdit->setText(defaultOutput);

    //    exportBrowseFileButton = new QPushButton(this);
    //    exportBrowseFileButton->setText(tr("Browse"));
    //    exportBrowseFileButton->setMaximumWidth(150);

    //    connect(exportBrowseFileButton,&QPushButton::clicked,this,&ResultsWidget::chooseResultsDirDialog);

    //    exportPDFFileButton = new QPushButton(this);
    //    exportPDFFileButton->setText(tr("Export to PDF"));

    //    connect(exportPDFFileButton,&QPushButton::clicked,this,&ResultsWidget::printToPDF);

    //    selectComponentsText = new QLabel("Select a subset of buildings to display the results:",this);
    //    selectComponentsLineEdit = new AssetInputDelegate();

    //    connect(selectComponentsLineEdit,&AssetInputDelegate::componentSelectionComplete,this,&ResultsWidget::handleComponentSelection);

    //    selectComponentsButton = new QPushButton();
    //    selectComponentsButton->setText(tr("Select"));
    //    selectComponentsButton->setMaximumWidth(150);

    //    connect(selectComponentsButton,SIGNAL(clicked()),this,SLOT(selectComponents()));

    //    // theExportLayout->addStretch();
    //    theExportLayout->addWidget(selectComponentsText,     0,0);
    //    theExportLayout->addWidget(selectComponentsLineEdit, 0,1);
    //    theExportLayout->addWidget(selectComponentsButton,   0,2);
    //    // theExportLayout->addStretch();
    //    theExportLayout->addWidget(exportLabel,            1,0);
    //    theExportLayout->addWidget(exportPathLineEdit,     1,1);
    //    theExportLayout->addWidget(exportBrowseFileButton, 1,2);
    //    theExportLayout->addWidget(exportPDFFileButton,       2,0,1,3);

    // theExportLayout->addStretch();
    //    theExportLayout->setRowStretch(3,1);

    mainLayout->addLayout(theHeaderLayout);
    mainLayout->addWidget(mainStackedWidget);
    //    mainLayout->addLayout(theExportLayout,1);
    //    mainLayout->addStretch(1);

    this->resultsShow(false);

    this->setMinimumWidth(640);
}


ResultsWidget::~ResultsWidget()
{

}


void ResultsWidget::resultsShow(bool value)
{
    if(!value)
    {
//        resTabWidget->setVisible(false);
        mainStackedWidget->setCurrentWidget(resultsPageWidget);
        //        selectComponentsButton->hide();
        //        selectComponentsLineEdit->hide();
        //        selectComponentsText->hide();
        //        exportPDFFileButton->hide();
        //        exportBrowseFileButton->hide();
        //        exportPathLineEdit->hide();
        //        exportLabel->hide();
    }
    else
    {
//        resTabWidget->setVisible(true);
        mainStackedWidget->setCurrentWidget(resTabWidget);
        //        selectComponentsButton->show();
        //        selectComponentsLineEdit->show();
        //        selectComponentsText->show();
        //        exportPDFFileButton->show();
        //        exportBrowseFileButton->show();
        //        exportPathLineEdit->show();
        //        exportLabel->show();
    }
}


bool ResultsWidget::outputToJSON(QJsonObject &/*jsonObject*/)
{
    return true;
}


bool ResultsWidget::inputFromJSON(QJsonObject &/*jsonObject*/)
{
    return true;
}


int ResultsWidget::processResults(QString resultsDirectory)
{
    //auto resultsDirectory = SCPrefs->getLocalWorkDir() + QDir::separator() + "tmp.SimCenter" + QDir::separator() + "Results";
    int tabCount = resTabWidget->count();
    for (int ind = 0; ind < tabCount; ind++){
        SC_ResultsWidget* tab = static_cast<SC_ResultsWidget*>(resTabWidget->widget(ind));
        if (tab !=0){
            tab->clear();
        }
    }
    resTabWidget->clear();
    auto resultLyr = theVisualizationWidget->getLayerGroup("Results");
    if (resultLyr){
        theVisualizationWidget->removeLayerGroup("Results");
    }
    auto DMGLyr = theVisualizationWidget->getLayerGroup("Most Likely Critical Damage State");
    if (DMGLyr){
        theVisualizationWidget->removeLayerGroup("Most Likely Damage State");
    }
    QgsProject *project = QgsProject::instance();
    for (QgsMapLayer *layer : project->mapLayers().values()) {
//        QString
        if ((layer->name() == "Results")||(layer->name() == "Most Likely Critical Damage State")) {
            theVisualizationWidget->removeLayer(layer);
        }
    }

    QString pathGeojson = resultsDirectory + QDir::separator() +  QString("R2D_results.geojson");
    QFile jsonFile(pathGeojson);
    QMap<QString, QList<QJsonObject>> assetDictionary;
    QMap<QString, QList<QString>> assetTypeToType;
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
            QString msg = "The CRS defined in " + pathGeojson + "is invalid and ignored";
            errorMessage(msg);
        }
        QJsonArray features = jsonObject["features"].toArray();
        for (const QJsonValue& valueIt : features) {
            QJsonObject value = valueIt.toObject();
            // Get the type
            QJsonObject assetProperties = value["properties"].toObject();
            if (!jsonObject.contains("type")) {
                this->errorMessage("The Json object is missing the 'type' key that defines the asset type");
                return false;
            }
            // type is Bridge/Tunnel/Road
            QString type = assetProperties["type"].toString();
            if (!assetDictionary.contains(type))
            {
                assetDictionary[type] = QList<QJsonObject>({value});
            }
            else
            {
                QList<QJsonObject>& featList = assetDictionary[type];
                featList.append(value);
            }
            // assetType is Transportation Network
            QString assetType = assetProperties["assetType"].toString();
            if (!assetTypeToType.contains(assetType))
            {
                assetTypeToType[assetType] = QList<QString>({type});
            }
            else
            {
                QList<QString>& typesList = assetTypeToType[assetType];
                bool typeExists = false;
                for (QString it : typesList){
                    if (it.compare(type)==0){
                        typeExists = true;
                    }
                }
                if (!typeExists){
                    typesList.append(type);
                }
            }
        }
    }
    else{
        // for legacy pelicun 2 results
//        this->errorMessage("Failed to open file at location: "+pathGeojson);
//        return false;
    }

    if (jsonFile.exists()){
    QVector<QgsMapLayer*> mapLayers;
    QVector<QgsMapLayer*> DMGLayers;
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

        QString outputFile = resultsDirectory + QDir::separator() + assetType + ".geojson";

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

        QgsVectorLayer* assetLayer;
        assetLayer = theVisualizationWidget->addVectorLayer(outputFile, assetType + QString("_results"), "ogr");
        if(assetLayer == nullptr)
        {
            this->errorMessage("Error, failed to add GIS layer");
            return false;
        }
        QgsVectorLayer* DMGlayer;
        DMGlayer = theVisualizationWidget->duplicateExistingLayer(assetLayer);
        DMGlayer->setName(assetType + QString("_DMG"));
        QgsSymbol* markerSymbol = nullptr;
        // Get the layer type
        QString layerType;
        auto geomType = DMGlayer->geometryType();
        if(geomType == QgsWkbTypes::PolygonGeometry){
            layerType = "polygon";
            markerSymbol = new QgsFillSymbol();
        }
        else if (geomType == QgsWkbTypes::PointGeometry){
            layerType = "point";
            markerSymbol = new QgsMarkerSymbol();
        }
        else if (geomType == QgsWkbTypes::LineGeometry){
            layerType = "linestring";
            markerSymbol = new QgsLineSymbol();
        }
        else
        {
            this->errorMessage("Could not parse the layer type for layer "+DMGlayer->name());
            return -1;
        }
        theVisualizationWidget->createCategoryRenderer("R2Dres_MostLikelyCriticalDamageState", DMGlayer, markerSymbol);
        DMGLayers.append(DMGlayer);
        mapLayers.append(assetLayer);
    }
        if (mapLayers.count()>1){
            theVisualizationWidget->createLayerGroup(mapLayers,"Results");
        } else {
            mapLayers.at(0)->setName("Results");
        }
        if (DMGLayers.count()>1){
            theVisualizationWidget->createLayerGroup(DMGLayers,"Most Likely Critical Damage State");
        } else {
            DMGLayers.at(0)->setName("Most Likely Critical Damage State");
        }
    }

    auto activeComponents = WorkflowAppR2D::getInstance()->getTheDamageAndLossWidget()->getActiveDLApps();
    auto activeAssetDLappMap = WorkflowAppR2D::getInstance()->getTheDamageAndLossWidget()->getActiveAssetDLMap();
    if(activeComponents.isEmpty())
        return -1;

    qDebug() << resultsDirectory;

    QMap<QString, SC_ResultsWidget*> activeDLResultsWidgets = WorkflowAppR2D::getInstance()->getTheDamageAndLossWidget()->getActiveDLResultsWidgets(theParent);


    try {
        for (QString assetType : activeDLResultsWidgets.keys()){
            activeDLResultsWidgets[assetType]->setVisualizationWidget(theVisualizationWidget);
            resTabWidget->addTab(activeDLResultsWidgets[assetType], assetType);
            QString resultFile = assetType + QString(".geojson");
            QString assetTypeSimplified = assetType.simplified().replace( " ", "" );
            activeDLResultsWidgets[assetType]->processResults(resultFile, resultsDirectory, assetType, assetTypeToType[assetTypeSimplified]);

        }
    } catch (const QString msg)
    {
        this->errorMessage(msg);

        return -1;
    }
    try
    {
        if (activeAssetDLappMap.contains("Buildings") && activeAssetDLappMap["Buildings"].compare("pelicun")==0){
            resTabWidget->addTab(thePelicunPostProcessor.get(),"Buildings");
            thePelicunPostProcessor->importResults(resultsDirectory);
        }
        if(activeAssetDLappMap.contains("Water Network") && activeAssetDLappMap["Water Network"].compare("CBCitiesDL")==0)
        {
            resTabWidget->addTab(theCBCitiesPostProcessor.get(),"Water Network");
            theCBCitiesPostProcessor->importResults(resultsDirectory);
        }

        this->resultsShow(true);

    }
    catch (const QString msg)
    {
        this->errorMessage(msg);

        return -1;
    }


    return 0;
}


int ResultsWidget::printToPDF(void)
{
    auto outputFileName = exportPathLineEdit->text();

    if(outputFileName.isEmpty())
    {
        QString errMsg = "The file name is empty";
        this->errorMessage(errMsg);
        return -1;
    }

    QDir theDir(outputFileName);
    if (theDir.exists()) {
        QMessageBox msgBox;
        msgBox.setText("Output file is a directory. No file will be written.");
        msgBox.exec();
        return 0;
    }

    QFile theFile(outputFileName);
    if (theFile.exists()) {
        QMessageBox::StandardButton reply = QMessageBox::question(this,
                                                                  "File Exists", "File Exists .. Do you want to overwrite?",
                                                                  QMessageBox::Yes | QMessageBox::No);
        if(reply == QMessageBox::No) {
            return 0;
        }
    }


    if(DVApp.compare("Pelicun") == 0) {

        int res = thePelicunPostProcessor->printToPDF(outputFileName);

        if(res != 0) {
            QString err = "Error printing the PDF";
            this->errorMessage(err);
            return -1;
        }
    }

    return 0;
}


void ResultsWidget::selectComponents(void)
{
    try
    {
        selectComponentsLineEdit->selectComponents();
    }
    catch (const QString msg)
    {
        this->errorMessage(msg);
    }
}


void ResultsWidget::handleComponentSelection(void)
{

    try
    {
        if(DVApp.compare("Pelicun") == 0)
        {
            auto IDSet = selectComponentsLineEdit->getSelectedComponentIDs();
            thePelicunPostProcessor->processResultsSubset(IDSet);
        }

    }
    catch (const QString msg)
    {
        this->errorMessage(msg);
    }
}


void ResultsWidget::chooseResultsDirDialog(void)
{

    QFileDialog dialog(this);

    dialog.setFileMode(QFileDialog::Directory);
    QString newPath = dialog.getExistingDirectory(this, tr("Directory to Save Results PDF"));
    dialog.close();

    // Return if the user cancels or enters same dir
    if(newPath.isEmpty())
    {
        return;
    }

    newPath += QDir::separator() + QString("Results.pdf");

    exportPathLineEdit->setText(newPath);

    return;
}


void ResultsWidget::clear(void)
{
    //    QString defaultOutput = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + QDir::separator() + QString("Results.pdf");
    //    exportPathLineEdit->setText(defaultOutput);
    //    selectComponentsLineEdit->clear();

    thePelicunPostProcessor->clear();
    theCBCitiesPostProcessor->clear();
    int tabCount = resTabWidget->count();
    for (int ind = 0; ind < tabCount; ind++){
        SC_ResultsWidget* tab = dynamic_cast<SC_ResultsWidget*>(resTabWidget->widget(ind));
        if (tab !=0){
            tab->clear();
        }
    }
    resTabWidget->clear();

    resultsShow(false);
}


