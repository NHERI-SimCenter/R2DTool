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

#include "OpenQuakeSelectionWidget.h"
#include "QGISVisualizationWidget.h"
#include "SimCenterMapcanvasWidget.h"

#include <qgsmapcanvas.h>
#include <qgsvectorlayer.h>
#include <qgslinesymbol.h>
#include <qgsfillsymbol.h>
#include <qgsrenderer.h>
#include <qgslayertreegroup.h>

#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QProgressBar>
#include <QPushButton>
#include <QFileDialog>
#include <QStandardPaths>
#include <QGroupBox>

OpenQuakeSelectionWidget::OpenQuakeSelectionWidget(VisualizationWidget* visWidget, QWidget *parent) : SimCenterAppWidget(parent)
{
    theVisualizationWidget = dynamic_cast<QGISVisualizationWidget*>(visWidget);

    assert(visWidget);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(5,0,0,0);

    layout->addWidget(this->getOpenQuakeSelectionWidget());

    // For testing start
    //    xmlImportPathLineEdit->setText("/Users/steve/Desktop/SimCenter/Examples/OpenQuakeSourceMaps/NSHM_source-model.xml");
    //    xmlImportPathLineEdit->setText("/Users/steve/Desktop/SimCenter/Examples/OpenQuakeSourceMaps/faults_backg_source_model.xml");
    //    xmlImportPathLineEdit->setText("/Users/steve/Desktop/SimCenter/Examples/OpenQuakeSourceMaps/test_source_model.xml");
    //    xmlImportPathLineEdit->setText("/Users/steve/Desktop/SimCenter/Examples/OpenQuakeSourceMaps/area_source_model.xml");

    //xmlExportPathLineEdit->setText("/Users/steve/Desktop/SimCenter/Examples/OpenQuakeSourceMaps/Test.xml");
    // this->loadOpenQuakeXMLData();
    // For testing end
}


OpenQuakeSelectionWidget::~OpenQuakeSelectionWidget()
{

}


QStackedWidget* OpenQuakeSelectionWidget::getOpenQuakeSelectionWidget(void)
{
    theStackedWidget = new QStackedWidget();
    theStackedWidget->setContentsMargins(0,0,0,0);

    theStackedWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    //
    // file and dir input
    //

    fileInputWidget = new QWidget();
    fileInputWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    fileInputWidget->setContentsMargins(0,0,0,0);

    QVBoxLayout *mainLayout = new QVBoxLayout(fileInputWidget);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(4);

    QHBoxLayout* xmlLoadLayout = new QHBoxLayout();
    QHBoxLayout* xmlexportLayout = new QHBoxLayout();

    auto inputText = new QLabel("Load xml OpenQuake file");

    auto exportText = new QLabel("Export xml OpenQuake file");

    xmlImportPathLineEdit = new QLineEdit();
    xmlImportPathLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    xmlExportPathLineEdit = new QLineEdit();
    xmlExportPathLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QPushButton *importBrowseFileButton = new QPushButton();
    importBrowseFileButton->setText(tr("Browse"));
    importBrowseFileButton->setMaximumWidth(150);

    xmlLoadLayout->addWidget(inputText);
    xmlLoadLayout->addWidget(xmlImportPathLineEdit);
    xmlLoadLayout->addWidget(importBrowseFileButton);

    QPushButton *exportBrowseFileButton = new QPushButton();
    exportBrowseFileButton->setText(tr("Browse"));
    exportBrowseFileButton->setMaximumWidth(150);

    QPushButton *exportButton = new QPushButton();
    exportButton->setText(tr("Export"));
    exportButton->setMaximumWidth(150);

    connect(exportButton,SIGNAL(clicked()),this,SLOT(handleSelectionDone()));

    xmlexportLayout->addWidget(exportText);
    xmlexportLayout->addWidget(xmlExportPathLineEdit);
    xmlexportLayout->addWidget(exportBrowseFileButton);
    xmlexportLayout->addWidget(exportButton);

    mainLayout->addLayout(xmlLoadLayout);
    mainLayout->addLayout(xmlexportLayout);

    connect(exportBrowseFileButton,SIGNAL(clicked()),this,SLOT(chooseExportFileDialog()));

    connect(importBrowseFileButton,SIGNAL(clicked()),this,SLOT(chooseImportFileDialog()));

    QGroupBox* selectionGB = new QGroupBox("Selection Method");
    QHBoxLayout* selectionLayout = new QHBoxLayout(selectionGB);

    selectionGB->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

    QPushButton *radiusSelectButton = new QPushButton(tr("Radius"));
    QPushButton *rectangleSelectButton = new QPushButton(tr("Rectangle"));
    QPushButton *freehandSelectButton = new QPushButton(tr("Freehand"));
    QPushButton *polygonSelectButton = new QPushButton(tr("Polygon"));

    QPushButton *noneSelectButton = new QPushButton(tr("None (Pan)"));

    connect(radiusSelectButton,SIGNAL(clicked()),this,SLOT(handleRadiusSelect()));
    connect(rectangleSelectButton,SIGNAL(clicked()),this,SLOT(handleRectangleSelect()));
    connect(freehandSelectButton,SIGNAL(clicked()),this,SLOT(handleFreehandSelect()));
    connect(noneSelectButton,SIGNAL(clicked()),this,SLOT(handleNoneSelect()));
    connect(polygonSelectButton,SIGNAL(clicked()),this,SLOT(handlePolygonSelect()));

    selectionLayout->addWidget(polygonSelectButton);
    selectionLayout->addWidget(rectangleSelectButton);
    selectionLayout->addWidget(radiusSelectButton);
    selectionLayout->addWidget(freehandSelectButton);
    selectionLayout->addWidget(noneSelectButton);

    QWidget* selectionWidget = new QWidget();
    QHBoxLayout* selectionWidgetLayout = new QHBoxLayout(selectionWidget);

    QPushButton *clearAllButton = new QPushButton(tr("Clear All"));
    clearAllButton->setMaximumWidth(150);
    connect(clearAllButton,SIGNAL(clicked()),this,SLOT(clear()));


    QPushButton *clearSelection = new QPushButton(tr("Clear Selection"));
    clearSelection->setMaximumWidth(150);
    connect(clearSelection,SIGNAL(clicked()),this,SLOT(clearSelection()));

    selectionWidgetLayout->addWidget(clearSelection);
    selectionWidgetLayout->addWidget(clearAllButton);

    QHBoxLayout* topLayout = new QHBoxLayout();

    topLayout->addWidget(selectionGB);
    topLayout->addWidget(selectionWidget);

    mainLayout->addLayout(topLayout);

    auto mapView = theVisualizationWidget->getMapViewWidget("OpenQuakeSelectionWidget");
    mapViewSubWidget = std::unique_ptr<SimCenterMapcanvasWidget>(mapView);

    // Enable the selection tool
    mapViewSubWidget->setShowPopUpOnSelection(false); // Do not show popup
    mapViewSubWidget->enableSelectionTool();

    mainLayout->addWidget(mapViewSubWidget.get());

    //
    // progress bar
    //

    progressBarWidget = new QWidget();
    auto progressBarLayout = new QVBoxLayout(progressBarWidget);

    auto progressText = new QLabel("Loading OpenQuake .xml file. This may take a while.");
    progressLabel =  new QLabel("");
    progressBar = new QProgressBar();

    auto vspacer = new QSpacerItem(0,0,QSizePolicy::Minimum, QSizePolicy::Expanding);
    auto vspacer2 = new QSpacerItem(0,0,QSizePolicy::Minimum, QSizePolicy::Expanding);
    progressBarLayout->addItem(vspacer);
    progressBarLayout->addWidget(progressText,1, Qt::AlignCenter);
    progressBarLayout->addWidget(progressLabel,1, Qt::AlignCenter);
    progressBarLayout->addWidget(progressBar);
    progressBarLayout->addItem(vspacer2);
    //    progressBarLayout->addStretch(1);

    //
    // add file and progress widgets to stacked widgets, then set defaults
    //

    theStackedWidget->addWidget(fileInputWidget);
    theStackedWidget->addWidget(progressBarWidget);

    theStackedWidget->setCurrentWidget(fileInputWidget);

    return theStackedWidget;
}


void OpenQuakeSelectionWidget::showEvent(QShowEvent *e)
{
    auto mainCanvas = mapViewSubWidget->getMainCanvas();

    auto mainExtent = mainCanvas->extent();

    mapViewSubWidget->mapCanvas()->zoomToFeatureExtent(mainExtent);
    SimCenterAppWidget::showEvent(e);
}


void OpenQuakeSelectionWidget::loadOpenQuakeXMLData(void)
{

    auto filePath = xmlImportPathLineEdit->text();

    // Load xml file
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly ))
    {
        // Error while loading file
        this->errorMessage("Error while loading file "+filePath);
        return;
    }

    if(!xmlGMs.isNull())
    {
        this->clear();
        xmlImportPathLineEdit->setText(filePath);
    }

    theStackedWidget->setCurrentWidget(progressBarWidget);
    progressBarWidget->setVisible(true);

    // QDomDocument used to import XML data
    // Set raw XML content into the QDomDocument
    xmlGMs.setContent(&file);

    // Close the file now that we are done with it
    file.close();

    // Extract the root element
    auto root = xmlGMs.documentElement();

    // Get root names and attributes
    //QString Type = root.tagName();


    auto sourceModelList = root.elementsByTagName("sourceModel");

    if(sourceModelList.size() == 0)
    {
        this->errorMessage("Could not find sourceModel tag in .xml file");
        // Reset the widget back to the input pane and close
        theStackedWidget->setCurrentWidget(fileInputWidget);
        fileInputWidget->setVisible(true);
        return;
    }

    auto sourceModelRoot = sourceModelList.at(0);

    auto sourceList = sourceModelRoot.childNodes();

    auto numSources = sourceList.size();

    if(numSources==0)
    {
        this->errorMessage("Number of sources in the source model is zero");
        // Reset the widget back to the input pane and close
        theStackedWidget->setCurrentWidget(fileInputWidget);
        fileInputWidget->setVisible(true);
        return;
    }

    auto sourceName = sourceModelRoot.attributes().namedItem("name").nodeValue();
    this->statusMessage("Importing sources "+sourceName);

    // For testing start
    //    QSet<QString> sourceTypes;

    //    for(int i = 0; i<numSources; ++i)
    //    {
    //        auto sourceType = sourceList.at(i).nodeName();

    //        sourceTypes.insert(sourceType);
    //    }

    //    for(auto&& it: sourceTypes)
    //        qDebug()<<it;
    // For testing end

    QDomNodeList pointSources = root.elementsByTagName("pointSource");

    auto res = this->getPointSources(&pointSources);

    if(res == -1)
    {
        this->errorMessage("Failed to import point sources. Something went wrong");
        // Reset the widget back to the input pane and close
        theStackedWidget->setCurrentWidget(fileInputWidget);
        fileInputWidget->setVisible(true);
        return;
    }

    QDomNodeList characLineSources = root.elementsByTagName("characteristicFaultSource");

    res = this->getCharacteristicComplexLineSources(&characLineSources,true);

    if(res == -1)
    {
        this->errorMessage("Failed to import line sources. Something went wrong");
        // Reset the widget back to the input pane and close
        theStackedWidget->setCurrentWidget(fileInputWidget);
        fileInputWidget->setVisible(true);
        return;
    }


    QDomNodeList compLineSources = root.elementsByTagName("complexFaultSource");

    res = this->getCharacteristicComplexLineSources(&compLineSources,false);

    if(res == -1)
    {
        this->errorMessage("Failed to import line sources. Something went wrong");
        // Reset the widget back to the input pane and close
        theStackedWidget->setCurrentWidget(fileInputWidget);
        fileInputWidget->setVisible(true);
        return;
    }


    QDomNodeList simpleFaultSources = root.elementsByTagName("simpleFaultSource");

    res = this->getSimpleFaultSources(&simpleFaultSources);

    if(res == -1)
    {
        this->errorMessage("Failed to import area sources. Something went wrong");
        // Reset the widget back to the input pane and close
        theStackedWidget->setCurrentWidget(fileInputWidget);
        fileInputWidget->setVisible(true);
        return;
    }

    QDomNodeList areaSources = root.elementsByTagName("areaSource");

    res = this->getAreaSources(&areaSources);

    if(res == -1)
    {
        this->errorMessage("Failed to import area sources. Something went wrong");
        // Reset the widget back to the input pane and close
        theStackedWidget->setCurrentWidget(fileInputWidget);
        fileInputWidget->setVisible(true);
        return;
    }

    progressLabel->setVisible(false);

    // Reset the widget back to the input pane and close
    theStackedWidget->setCurrentWidget(fileInputWidget);
    fileInputWidget->setVisible(true);

    if(theStackedWidget->isModal())
        theStackedWidget->close();

    if(!referenceLayerGroup.empty())
    {
        if(referenceLayerGroup.size() > 1)
            theVisualizationWidget->createLayerGroup(referenceLayerGroup, "OpenQuake Sources");
    }
    else
    {
        this->errorMessage("Failed to load OpenQuake sources");
    }

    auto numImportedSources = 0;

    if(pointReferenceLayer)
        numImportedSources += pointReferenceLayer->featureCount();

    if(lineReferenceLayer)

        numImportedSources += lineReferenceLayer->featureCount();

    if(areaReferenceLayer)
        numImportedSources += areaReferenceLayer->featureCount();

    if(numSources != numImportedSources)
    {
        this->errorMessage("Failed to import all of the sources "+QString::number(numSources)+" in input file vs. "+QString::number(numImportedSources)+" imported");
    }

    emit loadingComplete(true);

    return;
}


void OpenQuakeSelectionWidget::chooseImportFileDialog(void)
{
    QFileDialog dialog(this);

    dialog.setFileMode(QFileDialog::ExistingFile);

    QString oldPath;

    if(!xmlImportPathLineEdit->text().isEmpty())
        oldPath = xmlImportPathLineEdit->text();
    else
        oldPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

    auto pathToXmlFile = dialog.getOpenFileName(this, tr("Select the xml file"), oldPath);

    dialog.close();

    // Set file name & entry in line edit
    xmlImportPathLineEdit->setText(pathToXmlFile);

    this->loadOpenQuakeXMLData();

    return;
}


void OpenQuakeSelectionWidget::chooseExportFileDialog(void)
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::AnyFile);

    QString oldPath;

    if(!xmlExportPathLineEdit->text().isEmpty())
        oldPath = xmlExportPathLineEdit->text();
    else
        oldPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

    auto exportPathfile = dialog.getSaveFileName(this, tr("Where to export new .xml file"), oldPath,".xml");

    if(!exportPathfile.endsWith(".xml"))
        exportPathfile += ".xml";

    dialog.close();

    // Set file name & entry in line edit
    xmlExportPathLineEdit->setText(exportPathfile);

    return;
}


void OpenQuakeSelectionWidget::clear(void)
{
    xmlImportPathLineEdit->clear();
    xmlExportPathLineEdit->clear();

    xmlGMs.clear();

    selectedLayerGroup.clear();
    referenceLayerGroup.clear();

    if(pointReferenceLayer != nullptr)
        theVisualizationWidget->removeLayer(pointReferenceLayer);

    if(areaReferenceLayer != nullptr)
        theVisualizationWidget->removeLayer(areaReferenceLayer);

    if(lineReferenceLayer != nullptr)
        theVisualizationWidget->removeLayer(lineReferenceLayer);

    if(pointSelectedLayer != nullptr)
        theVisualizationWidget->removeLayer(pointSelectedLayer);

    if(lineSelectedLayer != nullptr)
        theVisualizationWidget->removeLayer(lineSelectedLayer);

    if(areaSelectedLayer != nullptr)
        theVisualizationWidget->removeLayer(areaSelectedLayer);

    pointReferenceLayer = nullptr;
    lineReferenceLayer = nullptr;
    areaReferenceLayer = nullptr;
    pointSelectedLayer = nullptr;
    lineSelectedLayer = nullptr;
    areaSelectedLayer = nullptr;
}


void OpenQuakeSelectionWidget::clearSelection(void)
{
    if(pointReferenceLayer)
        pointReferenceLayer->removeSelection();

    if(lineReferenceLayer)
        lineReferenceLayer->removeSelection();

    if(areaReferenceLayer)
        areaReferenceLayer->removeSelection();
}

int OpenQuakeSelectionWidget::getPointSources(QDomNodeList* pointSources)
{

    auto numSources = pointSources->size();
    if(numSources == 0)
        return 0;

    QDomNode firstItem = pointSources->item(0);

    //auto nodeName = firstItem.nodeName();

    QDomNamedNodeMap xmlAtrb = firstItem.attributes();

    // Create the fields
    QList<QgsField> attribFields;
    for(int i = 0; i< xmlAtrb.size(); ++i)
    {
        QDomNode item = xmlAtrb.item(i);

        auto name = item.nodeName();

        attribFields.push_back(QgsField(name, QVariant::String));
    }

    // auto numFields = attribFields.size();

    QgsFeatureList featureList;
    featureList.reserve(numSources);
    for(int i = 0; i < numSources; ++i)
    {
        QDomNode item = pointSources->item(i);

        QDomNamedNodeMap xmlAtrb = item.attributes();

        auto numAtributes = xmlAtrb.size();

        // create the feature attributes
        QgsAttributes featAttributes(numAtributes);

        // Get the feature attibutes
        for(int i = 0; i< numAtributes; ++i)
        {
            QDomNode item = xmlAtrb.item(i);
            auto val = item.nodeValue();

            featAttributes[i] = val;
        }

        QDomNode geom = item.namedItem("pointGeometry").namedItem("gml:Point").namedItem("gml:pos");

        if(geom.isNull())
        {
            this->errorMessage("Could not get geometry for node "+item.nodeName());
            return -1;
        }

        auto val = geom.toElement().text();

        // Split according to space
        auto pointsList = val.split(" ",Qt::SkipEmptyParts);

        // First number is lon, second is lat
        bool OK = true;

        auto longitude = pointsList[0].toDouble(&OK);

        if(!OK)
        {
            this->errorMessage("Error converting longitude to double for node "+item.nodeName());
            return -1;
        }

        auto latitude = pointsList[1].toDouble(&OK);

        if(!OK)
        {
            this->errorMessage("Error converting latitude to double for node "+item.nodeName());
            return -1;
        }

        if(longitude == 0.0 || latitude == 0.0)
        {
            this->errorMessage("Error, zero lat lon values for node "+item.nodeName());
            return -1;
        }

        // Create the feature
        QgsFeature feature;
        feature.setGeometry(QgsGeometry::fromPointXY(QgsPointXY(longitude,latitude)));
        feature.setAttributes(featAttributes);
        featureList.append(feature);
    }

    pointReferenceLayer = theVisualizationWidget->addVectorLayer("Point", "Point Sources");

    if(pointReferenceLayer == nullptr)
    {
        this->errorMessage("Error creating a layer");
        return -1;
    }

    referenceLayerGroup.append(pointReferenceLayer);

    auto dProvider = pointReferenceLayer->dataProvider();
    auto res = dProvider->addAttributes(attribFields);

    if(!res)
    {
        this->errorMessage("Error adding attribute fields to layer");
        theVisualizationWidget->removeLayer(pointReferenceLayer);
        return -1;
    }

    pointReferenceLayer->updateFields(); // tell the vector layer to fetch changes from the provider

    dProvider->addFeatures(featureList);
    pointReferenceLayer->updateExtents();

    theVisualizationWidget->createSymbolRenderer(QgsSimpleMarkerSymbolLayerBase::Cross,Qt::black,2.0,pointReferenceLayer);


    return 0;
}


int OpenQuakeSelectionWidget::getCharacteristicComplexLineSources(QDomNodeList* lineSources, bool isCharacteristic)
{

    auto numSources = lineSources->size();
    if(numSources == 0)
        return 0;

    QDomNode firstItem = lineSources->item(0);

    // auto nodeName = firstItem.nodeName();

    QDomNamedNodeMap xmlAtrb = firstItem.attributes();

    // Create the fields
    QList<QgsField> attribFields;
    for(int i = 0; i< xmlAtrb.size(); ++i)
    {
        QDomNode item = xmlAtrb.item(i);

        auto name = item.nodeName();

        attribFields.push_back(QgsField(name, QVariant::String));
    }

    // auto numFields = attribFields.size();

    auto getFeature = [=](const QStringList& pointsList, const QgsAttributes& featAttributes, const QDomNode& item) -> QgsFeature
    {

        QgsPolylineXY lineGeom;
        for(int i = 0; i<pointsList.size()-1; i+=3)
        {
            // First number is lon, second is lat
            bool OK = true;

            auto longitude = pointsList[i].toDouble(&OK);

            if(!OK)
            {
                this->errorMessage("Error converting longitude to double for node "+item.nodeName());
                return -1;
            }

            auto latitude = pointsList[i+1].toDouble(&OK);

            if(!OK)
            {
                this->errorMessage("Error converting latitude to double for node "+item.nodeName());
                return -1;
            }

            if(longitude == 0.0 || latitude == 0.0)
            {
                continue;
                //this->errorMessage("Error, zero lat lon values for node "+item.nodeName());
                //return -1;
            }

            lineGeom.append(QgsPointXY(longitude,latitude));
        }

        // Create the feature
        QgsFeature feature;
        feature.setGeometry(QgsGeometry::fromPolylineXY(lineGeom));
        feature.setAttributes(featAttributes);

        return feature;
    };

    QgsFeatureList featureList;
    featureList.reserve(numSources);
    for(int i = 0; i < numSources; ++i)
    {
        QDomNode item = lineSources->item(i);

        QDomNamedNodeMap xmlAtrb = item.attributes();

        auto numAtributes = xmlAtrb.size();

        // create the feature attributes
        QgsAttributes featAttributes(numAtributes);

        // Get the feature attibutes
        for(int i = 0; i< numAtributes; ++i)
        {
            QDomNode item = xmlAtrb.item(i);
            auto val = item.nodeValue();

            featAttributes[i] = val;
        }

        // The top edge of the fault
        QDomNode topEdgeGeom;

        if(isCharacteristic)
            topEdgeGeom = item.namedItem("surface");
        else
            topEdgeGeom = item;

        topEdgeGeom = topEdgeGeom.namedItem("complexFaultGeometry").namedItem("faultTopEdge").namedItem("gml:LineString").namedItem("gml:posList");

        if(topEdgeGeom.isNull())
        {
            this->errorMessage("Could not get geometry for node "+item.nodeName());
            return -1;
        }

        auto topVal = topEdgeGeom.toElement().text();

        // Split according to space
        auto topEdgepointsList = topVal.split(" ",Qt::SkipEmptyParts);

        auto topEdgefeature = getFeature(topEdgepointsList,featAttributes,item);

        featureList.append(topEdgefeature);

        // The bottom edge of the fault
        //        QDomNode bottomEdgeGeom;

        //        if(isCharacteristic)
        //            bottomEdgeGeom = item.namedItem("surface");
        //        else
        //            bottomEdgeGeom = item;

        //        bottomEdgeGeom = bottomEdgeGeom.namedItem("complexFaultGeometry").namedItem("faultBottomEdge").namedItem("gml:LineString").namedItem("gml:posList");


        //        if(bottomEdgeGeom.isNull())
        //        {
        //            this->errorMessage("Could not get geometry for node "+item.nodeName());
        //            return -1;
        //        }

        //        auto botVal = bottomEdgeGeom.toElement().text();

        //        // Split according to space
        //        auto botEdgepointsList = botVal.split(" ",Qt::SkipEmptyParts);

        //        auto botEdgefeature = getFeature(botEdgepointsList,featAttributes,item);

        //        featureList.append(botEdgefeature);
    }

    if(lineReferenceLayer == nullptr)
    {
        lineReferenceLayer = theVisualizationWidget->addVectorLayer("linestring", "Line Sources");

        if(lineReferenceLayer == nullptr)
        {
            this->errorMessage("Error creating a layer");
            return -1;
        }

        referenceLayerGroup.append(lineReferenceLayer);

        auto dProvider = lineReferenceLayer->dataProvider();
        auto res = dProvider->addAttributes(attribFields);

        if(!res)
        {
            this->errorMessage("Error adding attribute fields to layer");
            theVisualizationWidget->removeLayer(lineReferenceLayer);
            return -1;
        }

        lineReferenceLayer->updateFields(); // tell the vector layer to fetch changes from the provider

        dProvider->addFeatures(featureList);
        lineReferenceLayer->updateExtents();

        auto lineSymbol = new QgsLineSymbol();

        lineSymbol->setWidth(0.75);

        theVisualizationWidget->createSimpleRenderer(lineSymbol,lineReferenceLayer);
    }
    else
    {
        auto dProvider = lineReferenceLayer->dataProvider();

        auto res = dProvider->addFeatures(featureList);

        if(!res)
        {
            this->errorMessage("Error adding features to layer");
            theVisualizationWidget->removeLayer(lineReferenceLayer);
            return -1;
        }

        lineReferenceLayer->updateExtents();
    }

    return 0;
}


int OpenQuakeSelectionWidget::getSimpleFaultSources(QDomNodeList* lineSources)
{

    auto numSources = lineSources->size();
    if(numSources == 0)
        return 0;

    QDomNode firstItem = lineSources->item(0);

    // auto nodeName = firstItem.nodeName();

    QDomNamedNodeMap xmlAtrb = firstItem.attributes();

    // Create the fields
    QList<QgsField> attribFields;
    for(int i = 0; i< xmlAtrb.size(); ++i)
    {
        QDomNode item = xmlAtrb.item(i);

        auto name = item.nodeName();

        attribFields.push_back(QgsField(name, QVariant::String));
    }

    // auto numFields = attribFields.size();

    auto getFeature = [=](const QStringList& pointsList, const QgsAttributes& featAttributes, const QDomNode& item) -> QgsFeature
    {

        QgsPolylineXY lineGeom;
        for(int i = 0; i<pointsList.size()-1; i+=2)
        {
            // First number is lon, second is lat
            bool OK = true;

            auto longitude = pointsList[i].toDouble(&OK);

            if(!OK)
            {
                this->errorMessage("Error converting longitude to double for node "+item.nodeName());
                return -1;
            }

            auto latitude = pointsList[i+1].toDouble(&OK);

            if(!OK)
            {
                this->errorMessage("Error converting latitude to double for node "+item.nodeName());
                return -1;
            }

            if(longitude == 0.0 || latitude == 0.0)
            {
                continue;
                //this->errorMessage("Error, zero lat lon values for node "+item.nodeName());
                //return -1;
            }

            lineGeom.append(QgsPointXY(longitude,latitude));
        }

        // Create the feature
        QgsFeature feature;
        feature.setGeometry(QgsGeometry::fromPolylineXY(lineGeom));
        feature.setAttributes(featAttributes);

        return feature;
    };

    QgsFeatureList featureList;
    featureList.reserve(numSources);
    for(int i = 0; i < numSources; ++i)
    {
        QDomNode item = lineSources->item(i);

        QDomNamedNodeMap xmlAtrb = item.attributes();

        auto numAtributes = xmlAtrb.size();

        // create the feature attributes
        QgsAttributes featAttributes(numAtributes);

        // Get the feature attibutes
        for(int i = 0; i< numAtributes; ++i)
        {
            QDomNode item = xmlAtrb.item(i);
            auto val = item.nodeValue();

            featAttributes[i] = val;
        }

        // The top edge of the fault
        QDomNode edgeGeom = item.namedItem("simpleFaultGeometry").namedItem("gml:LineString").namedItem("gml:posList");

        if(edgeGeom.isNull())
        {
            this->errorMessage("Could not get geometry for node "+item.nodeName());
            return -1;
        }

        auto lineVal = edgeGeom.toElement().text();

        // Split according to space
        auto linePointsList = lineVal.split(" ",Qt::SkipEmptyParts);

        auto linefeature = getFeature(linePointsList,featAttributes,item);

        featureList.append(linefeature);

    }

    if(lineReferenceLayer == nullptr)
    {
        lineReferenceLayer = theVisualizationWidget->addVectorLayer("linestring", "Line Fault Sources");

        if(lineReferenceLayer == nullptr)
        {
            this->errorMessage("Error creating a layer");
            return -1;
        }

        referenceLayerGroup.append(lineReferenceLayer);

        auto dProvider = lineReferenceLayer->dataProvider();
        auto res = dProvider->addAttributes(attribFields);

        if(!res)
        {
            this->errorMessage("Error adding attribute fields to layer");
            theVisualizationWidget->removeLayer(lineReferenceLayer);
            return -1;
        }

        lineReferenceLayer->updateFields(); // tell the vector layer to fetch changes from the provider

        dProvider->addFeatures(featureList);
        lineReferenceLayer->updateExtents();

        auto lineSymbol = new QgsLineSymbol();

        lineSymbol->setWidth(0.75);

        theVisualizationWidget->createSimpleRenderer(lineSymbol,lineReferenceLayer);
    }
    else
    {
        auto dProvider = lineReferenceLayer->dataProvider();

        auto res = dProvider->addFeatures(featureList);

        if(!res)
        {
            this->errorMessage("Error adding features to layer");
            theVisualizationWidget->removeLayer(lineReferenceLayer);
            return -1;
        }

        lineReferenceLayer->updateExtents();
    }

    return 0;
}


int OpenQuakeSelectionWidget::getAreaSources(QDomNodeList* lineSources)
{

    auto numSources = lineSources->size();
    if(numSources == 0)
        return 0;

    QDomNode firstItem = lineSources->item(0);

    // auto nodeName = firstItem.nodeName();

    QDomNamedNodeMap xmlAtrb = firstItem.attributes();

    // Create the fields
    QList<QgsField> attribFields;
    for(int i = 0; i< xmlAtrb.size(); ++i)
    {
        QDomNode item = xmlAtrb.item(i);

        auto name = item.nodeName();

        attribFields.push_back(QgsField(name, QVariant::String));
    }

    // auto numFields = attribFields.size();

    auto getFeature = [=](const QStringList& pointsList, const QgsAttributes& featAttributes, const QDomNode& item) -> QgsFeature
    {

        QgsPolygonXY polygon;
        QgsPolylineXY polyLineRing;
        for(int i = 0; i<pointsList.size()-1; i+=2)
        {
            // First number is lon, second is lat
            bool OK = true;

            auto longitude = pointsList[i].toDouble(&OK);

            if(!OK)
            {
                this->errorMessage("Error converting longitude to double for node "+item.nodeName());
                return -1;
            }

            auto latitude = pointsList[i+1].toDouble(&OK);

            if(!OK)
            {
                this->errorMessage("Error converting latitude to double for node "+item.nodeName());
                return -1;
            }

            if(longitude == 0.0 || latitude == 0.0)
                continue;


            polyLineRing.append(QgsPointXY(longitude,latitude));
        }

        polygon.append(polyLineRing);

        // Create the feature
        QgsFeature feature;
        feature.setGeometry(QgsGeometry::fromPolygonXY(polygon));
        feature.setAttributes(featAttributes);

        return feature;
    };

    QgsFeatureList featureList;
    featureList.reserve(numSources);
    for(int i = 0; i < numSources; ++i)
    {
        QDomNode item = lineSources->item(i);

        QDomNamedNodeMap xmlAtrb = item.attributes();

        auto numAtributes = xmlAtrb.size();

        // create the feature attributes
        QgsAttributes featAttributes(numAtributes);

        // Get the feature attibutes
        for(int i = 0; i< numAtributes; ++i)
        {
            QDomNode item = xmlAtrb.item(i);
            auto val = item.nodeValue();

            featAttributes[i] = val;
        }

        // The top edge of the fault
        QDomNode polygonGeom = item.namedItem("areaGeometry").namedItem("gml:Polygon").namedItem("gml:exterior").namedItem("gml:LinearRing").namedItem("gml:posList");

        if(polygonGeom.isNull())
        {
            this->errorMessage("Could not get geometry for node "+item.nodeName());
            return -1;
        }

        auto posList = polygonGeom.toElement().text();

        // Split according to space
        auto polygonPointsList = posList.split(" ",Qt::SkipEmptyParts);

        auto polygonFeature = getFeature(polygonPointsList,featAttributes,item);

        featureList.append(polygonFeature);
    }

    areaReferenceLayer = theVisualizationWidget->addVectorLayer("polygon", "Area Sources");

    if(areaReferenceLayer == nullptr)
    {
        this->errorMessage("Error creating a layer");
        return -1;
    }

    referenceLayerGroup.append(areaReferenceLayer);

    auto dProvider = areaReferenceLayer->dataProvider();
    auto res = dProvider->addAttributes(attribFields);

    if(!res)
    {
        this->errorMessage("Error adding attribute fields to layer");
        theVisualizationWidget->removeLayer(areaReferenceLayer);
        return -1;
    }

    areaReferenceLayer->updateFields(); // tell the vector layer to fetch changes from the provider

    dProvider->addFeatures(featureList);
    areaReferenceLayer->updateExtents();

    auto markerSymbol = new QgsFillSymbol();

    markerSymbol->setColor(Qt::darkGray);
    markerSymbol->setOpacity(0.30);

    theVisualizationWidget->createSimpleRenderer(markerSymbol,areaReferenceLayer);

    return 0;
}


void OpenQuakeSelectionWidget::handleRectangleSelect(void)
{
    mapViewSubWidget->enableSelectionTool();
}


void OpenQuakeSelectionWidget::handleRadiusSelect(void)
{
    mapViewSubWidget->enableRadiusSelectionTool();
}


void OpenQuakeSelectionWidget::handleFreehandSelect(void)
{
    mapViewSubWidget->enableFreehandSelectionTool();
}


void OpenQuakeSelectionWidget::handleNoneSelect(void)
{
    mapViewSubWidget->enablePanTool();
}


void OpenQuakeSelectionWidget::handlePolygonSelect(void)
{
    mapViewSubWidget->enablePolygonSelectionTool();
}


void OpenQuakeSelectionWidget::handleSelectionDone(void)
{

    auto filePathToSave = xmlExportPathLineEdit->text();

    if(filePathToSave.isEmpty())
    {
        this->statusMessage("Please select a file name/location to save new .xml file");
        return;
    }

    QStringList selectedIds;

    if(pointReferenceLayer != nullptr)
    {
        auto numeFeatSel = pointReferenceLayer->selectedFeatureCount();

        if(pointSelectedLayer != nullptr)
            pointSelectedLayer->dataProvider()->truncate();

        if(numeFeatSel != 0)
        {
            if(pointSelectedLayer == nullptr)
            {
                pointSelectedLayer = pointReferenceLayer->clone();

                pointSelectedLayer->setName("Selected Point Sources");

                // Render the selected layer as dark yellow
                theVisualizationWidget->createSymbolRenderer(QgsSimpleMarkerSymbolLayerBase::Cross,Qt::darkYellow,2.0,pointSelectedLayer);

                theVisualizationWidget->addMapLayer(pointSelectedLayer);

                selectedLayerGroup.append(pointSelectedLayer);
            }

            QgsFeatureIterator selectedFeatures = pointReferenceLayer->getSelectedFeatures();

            QgsFeatureList featList;

            featList.reserve(numeFeatSel);

            QgsFeature feature;
            while (selectedFeatures.nextFeature(feature))
            {
                auto featId = feature.attribute("id").toString();

                selectedIds.push_back(featId);

                featList.append(feature);
            }

            pointSelectedLayer->dataProvider()->addFeatures(featList,QgsFeatureSink::FastInsert);

            pointSelectedLayer->dataProvider()->updateExtents();

            pointReferenceLayer->removeSelection();
        }
    }

    if(lineReferenceLayer != nullptr)
    {
        auto numeFeatSel = lineReferenceLayer->selectedFeatureCount();

        if(lineSelectedLayer != nullptr)
            lineSelectedLayer->dataProvider()->truncate();

        if(numeFeatSel != 0)
        {
            if(lineSelectedLayer == nullptr)
            {
                lineSelectedLayer = lineReferenceLayer->clone();

                lineSelectedLayer->setName("Selected Line Sources");

                auto lineSymbol = new QgsLineSymbol();

                lineSymbol->setColor(Qt::darkYellow);

                lineSymbol->setWidth(0.75);

                // Render the selected layer as dark yellow
                theVisualizationWidget->createSimpleRenderer(lineSymbol,lineSelectedLayer);

                theVisualizationWidget->addMapLayer(lineSelectedLayer);

                selectedLayerGroup.append(lineSelectedLayer);
            }

            lineSelectedLayer->dataProvider()->truncate();

            QgsFeatureIterator selectedFeatures = lineReferenceLayer->getSelectedFeatures();

            QgsFeatureList featList;

            featList.reserve(numeFeatSel);

            QgsFeature feature;
            while (selectedFeatures.nextFeature(feature))
            {
                auto featId = feature.attribute("id").toString();

                selectedIds.push_back(featId);

                featList.append(feature);
            }

            lineSelectedLayer->dataProvider()->addFeatures(featList,QgsFeatureSink::FastInsert);

            lineSelectedLayer->dataProvider()->updateExtents();

            lineReferenceLayer->removeSelection();
        }
    }

    if(areaReferenceLayer != nullptr)
    {
        auto numeFeatSel = areaReferenceLayer->selectedFeatureCount();

        if(areaSelectedLayer != nullptr)
            areaSelectedLayer->dataProvider()->truncate();

        if(numeFeatSel != 0)
        {
            if(areaSelectedLayer == nullptr)
            {
                areaSelectedLayer = areaReferenceLayer->clone();

                areaSelectedLayer->setName("Selected Area Sources");

                auto fillSymbol = new QgsFillSymbol();

                fillSymbol->setColor(Qt::darkYellow);
                fillSymbol->setOpacity(0.30);

                // Render the selected layer as dark yellow
                theVisualizationWidget->createSimpleRenderer(fillSymbol,areaSelectedLayer);

                theVisualizationWidget->addMapLayer(areaSelectedLayer);

                selectedLayerGroup.append(areaSelectedLayer);
            }

            areaSelectedLayer->dataProvider()->truncate();

            QgsFeatureIterator selectedFeatures = areaReferenceLayer->getSelectedFeatures();

            QgsFeatureList featList;

            featList.reserve(numeFeatSel);

            QgsFeature feature;
            while (selectedFeatures.nextFeature(feature))
            {
                auto featId = feature.attribute("id").toString();

                selectedIds.push_back(featId);

                featList.append(feature);
            }

            areaSelectedLayer->dataProvider()->addFeatures(featList,QgsFeatureSink::FastInsert);

            areaSelectedLayer->dataProvider()->updateExtents();

            areaReferenceLayer->removeSelection();
        }
    }

    theVisualizationWidget->markDirty();

    if(selectedIds.size() == 0)
    {
        this->infoMessage("Select items to export");
        return;
    }

    auto exportDoc = QDomDocument();

    exportDoc.setContent(xmlGMs.toString());

    auto root = exportDoc.documentElement();

    QDomNodeList sourceModelList = root.elementsByTagName("sourceModel");

    QVector<QDomNode> nodesToKeep;

    // Lambda to do a one deep nested search for nodes
    std::function<void(QDomNodeList&,bool)> nestedSearch = [&](QDomNodeList& list, bool checkChildren){

        for(int i = 0; i<list.size(); ++i)
        {
            auto it = list.at(i);

            auto nodeAttb = it.attributes();

            auto id = nodeAttb.namedItem("id").nodeValue();

            if(!id.isEmpty())
            {
                if(selectedIds.contains(id))
                    nodesToKeep.push_back(it);
            }
            else
            {
                auto childNodes = it.childNodes();

                if(!childNodes.isEmpty() && checkChildren == true)
                    nestedSearch(childNodes,false);
            }
        }
    };


    nestedSearch(sourceModelList,true);

    if(nodesToKeep.size() != selectedIds.size())
    {
        this->errorMessage("Failed to find all of the selected nodes. Export failed!");
        return;
    }

    auto sourceModelElem = root.firstChildElement("sourceModel");

    auto nodes = sourceModelElem.childNodes();

    // Remove the old nodes from the source model
    while(!nodes.isEmpty())
    {
        QDomNode node = nodes.at(0);
        node.parentNode().removeChild(node);
    }

    // Put back the nodes we want to keep
    for(auto&& it : nodesToKeep)
        sourceModelElem.appendChild(it);


    // Save the new xml file
    QFile file(filePathToSave);
    if( !file.open( QIODevice::WriteOnly | QIODevice::Text ) )
    {
        this->errorMessage("Failed to open file for writing.");
        return;
    }

    QTextStream stream( &file );
    stream << exportDoc.toString();
    file.close();

    this->statusMessage("Successfully saved file to: "+filePathToSave);

    auto grp = theVisualizationWidget->getLayerGroup("OpenQuake Selected Sources");

    if(grp == nullptr)
    {
        if(selectedLayerGroup.size() > 1)
            theVisualizationWidget->createLayerGroup(selectedLayerGroup, "OpenQuake Selected Sources");
    }
    else
    {
        for(auto&& it : selectedLayerGroup)
            if(grp->findLayer(it) == nullptr)
                grp->addLayer(it);
    }


}





