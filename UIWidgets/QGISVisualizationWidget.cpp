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

#include "QGISVisualizationWidget.h"
#include "PopUpWidget.h"
#include "SimCenterMapcanvasWidget.h"
#include "GISSelectable.h"

// Test to delete
#include "RectangleGrid.h"
#include "NodeHandle.h"
// Test to delete end

#include <qgsuserprofilemanager.h>
#include <qgslayertreeview.h>
#include <qgisapp.h>
#include <qgsmaplayer.h>
#include <qgslayertreemodel.h>
#include <qgslayertreegroup.h>
#include <qgslayertreeviewdefaultactions.h>
#include <qgsmapcanvas.h>
#include <qgsrubberband.h>
#include <qgsvertexmarker.h>
#include <qgsmaptoolregularpolygoncenterpoint.h>
#include <qgslayertreemapcanvasbridge.h>
#include <qgsmaptooladdfeature.h>
#include <qgsmapcanvas.h>
#include <qgsfield.h>
#include <qgsvectorlayer.h>
#include <qgscategorizedsymbolrenderer.h>
#include <qgsmarkersymbol.h>
#include <qgsfillsymbol.h>
#include <qgscolorramp.h>
#include <qgsexpressioncontextutils.h>
#include <qgsrulebasedrenderer.h>
#include <qgsclassificationmethod.h>
#include <qgssymbollayerutils.h>
#include <qgsrendererrange.h>
#include <qgsgraduatedsymbolrenderer.h>
#include <qgsclassificationmethodregistry.h>
#include <qgsapplication.h>
#include <qgssinglesymbolrenderer.h>
#include <qgslayertreenode.h>
#include <qgssinglesymbolrenderer.h>
#include <qgsrasterlayer.h>

#include <QStandardPaths>
#include <QSplitter>

QGISVisualizationWidget::QGISVisualizationWidget(QMainWindow *parent) : VisualizationWidget(parent)
{

    QSplitter* mainWidget = new QSplitter(this);
    mainWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(0);

    mainLayout->addWidget(mainWidget);

    bool mySkipVersionCheck = true;
    bool myRestorePlugins = true;
    auto profileName = QLatin1String( "default" );
    auto configLocalStorageLocation = QStandardPaths::standardLocations( QStandardPaths::AppDataLocation ).value( 0 );
    QString rootProfileFolder = QgsUserProfileManager::resolveProfilesFolder( configLocalStorageLocation );

    qgis = new QgisApp(parent, myRestorePlugins, mySkipVersionCheck, rootProfileFolder, profileName, nullptr, Qt::Widget);
    qgis->setObjectName( QStringLiteral( "QgisApp" ) );
    qgis->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    //    QThread *thread = new QThread();
    //    qgis->moveToThread(thread);
    //    QWidget::connect(thread, SIGNAL(finished()), qgis, SLOT(deleteLater()));

    //    thread->start();

    layerTreeView = qgis->layerTreeView();

    qgis->mapCanvas()->setCenter(QgsPointXY(37.8717450069,-122.2609607382));

    auto crs = QgsProject::instance()->crs();
    qgis->mapCanvas()->setDestinationCrs(crs);

    auto leftHandWidget = new QWidget();
    leftHandWidget->setContentsMargins(0,0,0,0);

    QVBoxLayout* leftHandLayout = new QVBoxLayout(leftHandWidget);
    leftHandLayout->setMargin(0);

    QLabel* basemapText = new QLabel(this);
    basemapText->setText("Add a basemap:");

    baseMapCombo = new QComboBox();
    baseMapCombo->addItem("OpenStreetMap");
    baseMapCombo->addItem("OpenStreetMap Topo");
    baseMapCombo->addItem("Google Satellite");
    baseMapCombo->addItem("Google Road");
    baseMapCombo->addItem("Google Road & Satellite");
    baseMapCombo->addItem("Bing Aerial");

    connect(baseMapCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &QGISVisualizationWidget::handleBasemapSelection);


    QPushButton *identifyAssets = new QPushButton("Identify",this);
    connect(identifyAssets,&QPushButton::pressed,this,&QGISVisualizationWidget::handleIdentifyButton);
    QPushButton *selectAssets = new QPushButton("Select",this);
    connect(selectAssets,&QPushButton::pressed,this,&QGISVisualizationWidget::handleSelectButton);

    auto lhbLayout = new QHBoxLayout();
    lhbLayout->addWidget(identifyAssets);
    lhbLayout->addWidget(selectAssets);

    QLabel* topText = new QLabel(this);
    topText->setText("Add selected assets to analysis list");
    topText->setStyleSheet("font-weight: bold; color: black; text-align: center");

    QPushButton *addAssetsButton = new QPushButton(this);
    addAssetsButton->setText(tr("Add Assets"));
    //    addAssetsButton->setMaximumWidth(150);


    QLabel* bottomText = new QLabel(this);
    bottomText->setText("Clear all selected assets");
    bottomText->setStyleSheet("font-weight: bold; color: black; text-align: center");

    QPushButton *clearButton = new QPushButton(this);
    clearButton->setText(tr("Clear"));
    //    clearButton->setMaximumWidth(150);

    connect(addAssetsButton,&QPushButton::clicked,this,&QGISVisualizationWidget::handleSelectAssetsMap);
    connect(clearButton,&QPushButton::clicked,this,&QGISVisualizationWidget::handleClearAssetsMap);

    leftHandLayout->addWidget(basemapText);
    leftHandLayout->addWidget(baseMapCombo);
    leftHandLayout->addWidget(layerTreeView);
    leftHandLayout->addLayout(lhbLayout);
    leftHandLayout->addWidget(topText);
    leftHandLayout->addWidget(addAssetsButton);
    leftHandLayout->addWidget(clearButton);
    leftHandLayout->addWidget(bottomText);
    leftHandLayout->addWidget(clearButton);

    mainWidget->addWidget(leftHandWidget);
    mainWidget->addWidget(qgis);

    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    handleBasemapSelection(0);

    // Set the map tool to select
    handleClearAssetsMap();

    //        testVectorLayer();

    //    testNewMapCanvas();

    //    testNewMapCanvas2();
}


QGISVisualizationWidget::~QGISVisualizationWidget()
{

}


SimCenterMapcanvasWidget* QGISVisualizationWidget::getMapViewWidget(const QString& name)
{
    SimCenterMapcanvasWidget* mapCanvasWidget = new SimCenterMapcanvasWidget(name, this);

    QgsMapCanvas *mapCanvas = mapCanvasWidget->mapCanvas();
    mapCanvas->freeze( true );
    mapCanvas->setObjectName( name );
    mapCanvas->setProject(QgsProject::instance());

    connect(qgis->layerTreeCanvasBridge(), &QgsLayerTreeMapCanvasBridge::canvasLayersChanged, mapCanvas, &QgsMapCanvas::setLayers );

    mapCanvas->setCanvasColor( QgsProject::instance()->backgroundColor() );
    mapCanvas->setSelectionColor( QgsProject::instance()->selectionColor() );

    QgsSettings settings;
    mapCanvas->enableAntiAliasing( settings.value( QStringLiteral( "qgis/enable_anti_aliasing" ), true ).toBool() );
    double zoomFactor = settings.value( QStringLiteral( "qgis/zoom_factor" ), 1.5 ).toDouble();
    mapCanvas->setWheelFactor( zoomFactor );
    mapCanvas->setCachingEnabled( settings.value( QStringLiteral( "qgis/enable_render_caching" ), true ).toBool() );
    mapCanvas->setParallelRenderingEnabled( settings.value( QStringLiteral( "qgis/parallel_rendering" ), true ).toBool() );
    mapCanvas->setMapUpdateInterval( settings.value( QStringLiteral( "qgis/map_update_interval" ), 125 ).toInt() );
    mapCanvas->setSegmentationTolerance( settings.value( QStringLiteral( "qgis/segmentationTolerance" ), "0.01745" ).toDouble() );
    mapCanvas->setSegmentationToleranceType( QgsAbstractGeometry::SegmentationToleranceType( settings.enumValue( QStringLiteral( "qgis/segmentationToleranceType" ), QgsAbstractGeometry::MaximumAngle ) ) );

    //    // add existing annotations to canvas
    //    const auto constAnnotations = QgsProject::instance()->annotationManager()->annotations();
    //    for ( QgsAnnotation *annotation : constAnnotations )
    //    {
    //        QgsMapCanvasAnnotationItem *canvasItem = new QgsMapCanvasAnnotationItem( annotation, mapCanvas );
    //        Q_UNUSED( canvasItem ) //item is already added automatically to canvas scene
    //    }

    qgis->markDirty();
    connect( mapCanvasWidget, &QWidget::close, qgis, &QgisApp::markDirty );

    mapCanvas->setInteractive(true);

    mapCanvas->setLayers( qgis->mapCanvas()->layers() );
    mapCanvas->setExtent( qgis->mapCanvas()->extent() );
    QgsDebugMsgLevel( QStringLiteral( "QgisApp::createNewMapCanvas -2- : QgsProject::instance()->crs().description[%1]ellipsoid[%2]" ).arg( QgsProject::instance()->crs().description(), QgsProject::instance()->crs().ellipsoidAcronym() ), 3 );
    //    mapCanvas->setDestinationCrs( QgsProject::instance()->crs() );
    mapCanvas->setDestinationCrs(QgsCoordinateReferenceSystem(QStringLiteral("EPSG:3857")));
    mapCanvas->freeze(false);

    return mapCanvasWidget;
}


void QGISVisualizationWidget::testNewMapCanvas2()
{
    auto mapCanvas = qgis->createNewMapCanvas("Test2");

    QWidget* testWidget = new QWidget();

    QVBoxLayout* newBoxLayout = new QVBoxLayout(testWidget);

    newBoxLayout->addWidget(mapCanvas);

    testWidget->show();

    mapCanvas->zoomToFullExtent();
}


void QGISVisualizationWidget::markDirty()
{
    qgis->markDirty();
}


void QGISVisualizationWidget::turnOnSelectionTool()
{
    qgis->selectFeatures();
}


SimCenterMapcanvasWidget* QGISVisualizationWidget::testNewMapCanvas()
{
    auto mapCanvasWidget = this->getMapViewWidget("Test");

    mapCanvasWidget->setWindowFlag(Qt::Window);

    auto mapCanvas = mapCanvasWidget->mapCanvas();

    mapCanvasWidget->show();

    //    QGraphicsScene* scene = mapCanvas->scene();

    //    auto grid = new RectangleGrid(mapCanvas);

    //    auto sceneRect = scene->sceneRect();

    //    auto centerScene = sceneRect.center();

    //    auto sceneWidth = sceneRect.width();
    //    auto sceneHeight = sceneRect.height();

    //    // Set the initial grid size if it has not already been set
    //    if(grid->getBottomLeftNode()->pos().isNull() || grid->getTopRightNode()->pos().isNull() || grid->getTopLeftNode()->pos().isNull() || grid->getBottomRightNode()->pos().isNull() )
    //    {
    //        grid->setWidth(0.5*sceneWidth);
    //        grid->setHeight(0.5*sceneHeight);
    //        grid->setPos(centerScene.toPoint());

    //        scene->addItem(grid);
    //    }

    //    grid->show();

    //    mapCanvasWidget->mapCanvas()->zoomToFullExtent();

    return mapCanvasWidget;
}


void QGISVisualizationWidget::handleBasemapSelection(int index)
{
    QgsMapLayer* baseMapLayer = nullptr;

    if(index == 0)
    {
        auto uri = "tilePixelRatio=2&type=xyz&url=http://tile.openstreetmap.org/%7Bz%7D/%7Bx%7D/%7By%7D.png&zmax=19&zmin=0";
        auto baseName = "Open Street Map";
        auto key = "wms";

        baseMapLayer = qgis->addRasterLayer(uri,baseName,key);
    }
    else if(index == 1)
    {
        auto uri = "tilePixelRatio=2&type=xyz&url=http://tile.opentopomap.org/%7Bz%7D/%7Bx%7D/%7By%7D.png&zmax=19&zmin=0";
        auto baseName = "Open Street Map Topo";
        auto key = "wms";

        baseMapLayer = qgis->addRasterLayer(uri,baseName,key);
    }
    else if(index == 2)
    {
        auto uri = "tilePixelRatio=2&type=xyz&url=https://mt1.google.com/vt/lyrs%3Ds%26x%3D%7Bx%7D%26y%3D%7By%7D%26z%3D%7Bz%7D&zmax=19&zmin=0";
        auto baseName = "Google Satellite";
        auto key = "wms";

        baseMapLayer = qgis->addRasterLayer(uri,baseName,key);
    }
    else if(index == 3)
    {
        auto uri = "tilePixelRatio=2&type=xyz&url=https://mt1.google.com/vt/lyrs%3Dm%26x%3D%7Bx%7D%26y%3D%7By%7D%26z%3D%7Bz%7D&zmax=19&zmin=0";
        auto baseName = "Google Road";
        auto key = "wms";

        baseMapLayer = qgis->addRasterLayer(uri,baseName,key);
    }
    else if(index == 4)
    {
        auto uri = "tilePixelRatio=2&type=xyz&url=https://mt1.google.com/vt/lyrs%3Dy%26x%3D%7Bx%7D%26y%3D%7By%7D%26z%3D%7Bz%7D&zmax=19&zmin=0";
        auto baseName = "Google Road and Satellite";
        auto key = "wms";

        baseMapLayer = qgis->addRasterLayer(uri,baseName,key);
    }
    else if(index == 5)
    {
        auto uri = "tilePixelRatio=2&type=xyz&url=http://ecn.t3.tiles.virtualearth.net/tiles/a%7Bq%7D.jpeg?g%3D1&zmax=18&zmin=0";
        auto baseName = "Bing Aerial";
        auto key = "wms";

        baseMapLayer = qgis->addRasterLayer(uri,baseName,key);
    }

    // Make the basemap non-removable so that it does not get deleted when the project is cleared
    QgsMapLayer::LayerFlags flags = baseMapLayer->flags();

    //    flags |= QgsMapLayer::Identifiable; // yes flag
    //    flags &= ~QgsMapLayer::Identifiable; // no flag

    //    flags |= QgsMapLayer::Removable;
    flags &= ~QgsMapLayer::Removable;

    //    flags |= QgsMapLayer::Searchable;
    //    flags &= ~QgsMapLayer::Searchable;

    //    flags |= QgsMapLayer::Private;
    //    flags &= ~QgsMapLayer::Private;

    baseMapLayer->setFlags( flags );
}


void QGISVisualizationWidget::handleSelectAssetsMap(void)
{
    auto mapCanvas = qgis->mapCanvas();

    auto layers = mapCanvas->layers();

    for(auto&& layer : layers)
    {

        auto currVecLayer = static_cast<QgsVectorLayer*>(layer);

        if(currVecLayer == nullptr)
            continue;

        auto layerId = layer->id();

        auto selectWidget = mapSelectableAssetWidgets.value(layerId,nullptr);
        if(selectWidget == nullptr)
            continue;

        QgsFeatureIds selectedAssets = currVecLayer->selectedFeatureIds();

        if(selectedAssets.isEmpty())
            continue;

        selectWidget->insertSelectedAssets(selectedAssets);
    }

    qgis->deselectAll();
}


void QGISVisualizationWidget::handleClearAssetsMap(void)
{
    // Iterate through the map and clear all selected assets
    QMapIterator<QString,GISSelectable*> it(mapSelectableAssetWidgets);
    while (it.hasNext()) {
        it.next();
        it.value()->clearSelectedAssets();
    }

}


QgsRasterLayer* QGISVisualizationWidget::addRasterLayer(const QString &layerPath, const QString &name, const QString &providerKey)
{
    auto layer = qgis->addRasterLayer(layerPath, name, providerKey);

    //    if(layer != nullptr)
    //        layer->setCrs(QgsCoordinateReferenceSystem("EPSG:4326"));

    return layer;
}


QgsVectorLayer* QGISVisualizationWidget::addVectorLayer(const QString &layerPath, const QString &name, const QString &providerKey)
{
    auto layer = qgis->addVectorLayer(layerPath, name, providerKey);

    if(layer != nullptr)
        layer->setCrs(QgsCoordinateReferenceSystem("EPSG:4326"));

    return layer;
}


void QGISVisualizationWidget::testVectorLayer()
{

    // QgsVectorLayer( const QString &path = QString(), const QString &baseName = QString(), const QString &providerLib = "ogr", const QgsVectorLayer::LayerOptions &options = QgsVectorLayer::LayerOptions() );

    auto vl = this->addVectorLayer("Point", "Feature Layer");

    auto vl2 = this->addVectorLayer("Point", "Feature Layer2");

    connect(vl,&QgsVectorLayer::selectionChanged,this, &QGISVisualizationWidget::selectionChanged);

    auto pr = vl->dataProvider();

    auto pr2 = vl2->dataProvider();

    // add fields
    QList<QgsField> attrib = {QgsField("name", QVariant::String),
                              QgsField("age",  QVariant::Int),
                              QgsField("size", QVariant::Double),
                              QgsField("repairRatio", QVariant::Double)};

    QVector<QString> buildTypes = {"Res 1", "Res 2", "Comm 1", "Comm 2", "Ind 1", "Ind 2"};

    auto res = pr->addAttributes(attrib);

    if(!res)
        qDebug()<<"Error adding attributes";

    pr2->addAttributes(attrib);

    vl->updateFields(); // tell the vector layer to fetch changes from the provider

    vl2->updateFields(); // tell the vector layer to fetch changes from the provider

    int numFeat = 1000;
    QgsFeatureList featList;
    QgsFeatureList featList2;

    int numTypes = buildTypes.size()-1;

    auto minX =  -120;
    auto minY = 30;

    float maxD = 5.5;
    float minD = 3.2;

    for(int i = 0; i < numFeat; ++i)
    {

        auto x = minX + minD + static_cast <float> (rand()) / ( static_cast <float> (RAND_MAX/(maxD-minD)));
        auto y = minY + minD + static_cast <float> (rand()) / ( static_cast <float> (RAND_MAX/(maxD-minD)));

        // add a feature
        QgsFeature feature;
        feature.setGeometry(QgsGeometry::fromPointXY(QgsPointXY(x,y)));

        QgsAttributes featAttrb(attrib.size());

        featAttrb[0] = QVariant(buildTypes[(rand() % numTypes) + 1]);
        featAttrb[1] = QVariant(2);
        featAttrb[2] = QVariant(0.3);
        featAttrb[3] = QVariant((double) rand()/RAND_MAX);

        feature.setAttributes(featAttrb);

        if(i%2 == 0)
            featList.append(feature);
        else
            featList2.append(feature);
    }


    pr->addFeatures(featList);
    pr2->addFeatures(featList2);

    vl->updateExtents();
    vl2->updateExtents();

    //  createPrettyGraduatedRenderer("repairRatio",Qt::yellow, Qt::red,5,vl);
    //    createCategoryRenderer("name",vl);

    QVector<QPair<double,double>>  classBreaks;
    classBreaks.append(QPair<double,double>(0.0,0.4));
    classBreaks.append(QPair<double,double>(0.4,0.6));
    classBreaks.append(QPair<double,double>(0.6,0.8));
    classBreaks.append(QPair<double,double>(0.8,1.0));

    QVector<QColor> colors;
    colors.push_back(Qt::black);
    colors.push_back(Qt::blue);
    colors.push_back(Qt::green);
    colors.push_back(Qt::red);

    createCustomClassBreakRenderer("repairRatio",classBreaks,colors,vl);

    createCustomClassBreakRenderer("repairRatio",classBreaks,colors,vl2);

    QVector<QgsMapLayer*> vecLayers= {vl,vl2};

    this->createLayerGroup(vecLayers, "Test layer group");
}


void QGISVisualizationWidget::selectLayersInTree(const QVector<QgsMapLayer*>& layers)
{
    // First get all of the indexes
    QVector<QModelIndex> vecIndexes;
    vecIndexes.reserve(layers.size());
    for(auto&& it: layers)
    {
        auto index = this->getLayerIndex(it);

        if(!index.isValid())
            this->errorMessage("Invalid index in select layers");

        vecIndexes.push_back(index);
    }

    // Then do the selection
    auto selectModel = layerTreeView->selectionModel();
    for(auto&& it : vecIndexes)
    {
        selectModel->select(it , QItemSelectionModel::Select);
    }
}


void QGISVisualizationWidget::createLayerGroup(const QVector<QgsMapLayer*>& layers, const QString groupName)
{
    if(layers.empty())
    {
        this->errorMessage("No layers given in create group");
        return;
    }

    // First select the layers
    this->selectLayersInTree(layers);

    if(layerTreeView->selectedLayers().empty())
    {
        this->errorMessage("Layer selection failed in create group");
        return;
    }

    // Add the group
    layerTreeView->defaultActions()->addGroup();

    // Rename the group
    auto group = layerTreeView->currentGroupNode();
    group->setName(groupName);

    // Clear the selection otherwise future layers added to the map will automatically be part of the group
    this->deselectAllTreeItems();
}


void QGISVisualizationWidget::deselectAllTreeItems(void)
{
    auto selectModel = layerTreeView->selectionModel();
    selectModel->clearSelection();
    selectModel->clearCurrentIndex();
}


void QGISVisualizationWidget::showFeaturePopUp(QgsFeatureIterator& features)
{

    if(!features.isValid())
        return;

    std::unique_ptr<PopUpWidget> popUp = std::make_unique<PopUpWidget>(this);

    QgsFeature feat;
    while (features.nextFeature(feat))
    {
        auto fields = feat.fields();
        auto attributes = feat.attributes();

        if(fields.size() != attributes.size())
        {
            this->errorMessage("Error getting the fields and attributes in information popup");
            return;
        }

        QStringList attrbKeyList;
        QStringList attrbValList;

        for(int i = 0; i<attributes.size(); ++i)
        {
            auto atrb = attributes.at(i).toString();
            auto field = fields.at(i).name();

            // Do not list empty attributes in the popup
            if(atrb.isEmpty())
                continue;

            if(QString::compare(atrb,"ObjectID") == 0 || QString::compare(atrb,"AssetType") == 0 || QString::compare(atrb,"TabName") == 0)
                continue;

            attrbKeyList.append(field);
            attrbValList.append(atrb);
        }

        // Create a table to display the attributes of this element
        auto attributeTableWidget = new QTableWidget(popUp.get());
        attributeTableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        attributeTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        attributeTableWidget->horizontalHeader()->hide();
        attributeTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

        auto numRows = attrbValList.size();

        attributeTableWidget->setColumnCount(1);
        attributeTableWidget->setRowCount(numRows);

        attributeTableWidget->setVerticalHeaderLabels(attrbKeyList);

        for(int i = 0; i<numRows; ++i)
        {
            auto item = new QTableWidgetItem(attrbValList[i]);

            attributeTableWidget->setItem(i,0, item);
        }

        QString label = feat.attribute("TabName").toString();

        // If the label is empty, append the layer name
        if(label.isEmpty())
        {
            label = "Feature Information";
        }

        popUp->addTab(attributeTableWidget,label);
    }

    popUp->exec();

}



QModelIndex QGISVisualizationWidget::getLayerIndex(QgsMapLayer* layer)
{
    layerTreeView->setCurrentLayer(layer);
    auto node = layerTreeView->currentNode();

    auto index = layerTreeView->node2index(node);

    return index;
}


void QGISVisualizationWidget::createCustomClassBreakRenderer(const QString attrName, const QVector<QPair<double,double>>& classBreaks, const QVector<QColor>& colors, QgsVectorLayer * vlayer)
{

    if(vlayer == nullptr)
        return;

    if(colors.size() != classBreaks.size())
    {
        qDebug()<<"Mismatch between number of class breaks and colors, they have to be the same size";
        return;
    }

    // qgsclassificationcustom
    QString methodId = "Custom";
    QgsClassificationMethod *method = QgsApplication::classificationMethodRegistry()->method(methodId);

    QgsRangeList rngList;

    for(int i = 0; i<classBreaks.size(); ++i)
    {
        QgsMarkerSymbol* mMarkerSymbol = new QgsMarkerSymbol();
        mMarkerSymbol->setColor(colors[i]);
        auto cBreak = classBreaks.at(i);

        auto lowEnd = cBreak.first;
        auto highEnd = cBreak.second;

        QString label = QString::number(lowEnd) + " - " + QString::number(highEnd);

        QgsRendererRange newRange(lowEnd,highEnd, mMarkerSymbol,label);

        rngList.append(newRange);
    }

    QgsGraduatedSymbolRenderer* renderer = new QgsGraduatedSymbolRenderer(attrName,rngList);

    // set method to renderer
    renderer->setClassificationMethod(method);

    // create and set new renderer
    renderer->setClassAttribute(attrName);

    vlayer->setRenderer(renderer);
}


void QGISVisualizationWidget::createPrettyGraduatedRenderer(const QString attrName, const QColor color1, const QColor color2, const int nclasses, QgsVectorLayer * vlayer)
{

    if(vlayer == nullptr)
        return;

    // *** Graduated renderer

    // qgsclassificationcustom
    // qgsclassificationequalinterval
    // qgsclassificationquantile
    // qgsclassificationjenks
    // qgsclassificationstandarddeviation
    // qgsclassificationprettybreaks
    // qgsclassificationlogarithmic

    QString methodId = "Pretty";
    QgsClassificationMethod *method = QgsApplication::classificationMethodRegistry()->method(methodId);

    QgsGraduatedSymbolRenderer* renderer = new QgsGraduatedSymbolRenderer(attrName);

    // set method to renderer
    renderer->setClassificationMethod(method);

    // create and set new renderer
    renderer->setClassAttribute(attrName);

    QgsGradientColorRamp* ramp = new QgsGradientColorRamp(color1, color2);

    renderer->setSourceColorRamp(ramp);

    renderer->updateClasses(vlayer, nclasses);

    vlayer->setRenderer(renderer);
}


void QGISVisualizationWidget::createCategoryRenderer(const QString attrName, QgsVectorLayer * vlayer, QgsSymbol* symbol, QVector<QColor> colors)
{
    QList<QVariant> uniqueValues;

    const int idx = vlayer->fields().lookupField(attrName);
    if (idx == -1)
    {
        // Lets assume it's an expression
        std::unique_ptr<QgsExpression> expression = std::make_unique<QgsExpression>(attrName);
        QgsExpressionContext context;
        context << QgsExpressionContextUtils::globalScope()
                << QgsExpressionContextUtils::projectScope(QgsProject::instance())
                << QgsExpressionContextUtils::atlasScope(nullptr)
                << QgsExpressionContextUtils::layerScope(vlayer);

        expression->prepare(&context);
        QgsFeatureIterator fit = vlayer->getFeatures();
        QgsFeature feature;
        while (fit.nextFeature(feature))
        {
            context.setFeature(feature);
            const QVariant value = expression->evaluate(&context);
            if (uniqueValues.contains(value))
                continue;
            uniqueValues << value;
        }
    }
    else
    {
        uniqueValues = qgis::setToList(vlayer->uniqueValues(idx));
    }

    if(uniqueValues.isEmpty())
    {
        qDebug()<<"No unique values found for the attribute " + attrName + " so no categories will be created";
        return;
    }

    QgsCategoryList cats = QgsCategorizedSymbolRenderer::createCategories(uniqueValues, symbol, vlayer, attrName);

    auto numCats = cats.size();

    if(!colors.empty())
    {
        if(colors.size() != numCats)
        {
            qDebug()<<"Warning, not using the provided colors as there is a size mismatch between the number of colors provided and number of categories. Assigning random colors to the categories instead";

            QgsRandomColorRamp ramp;
            ramp.setTotalColorCount(numCats);

            for (int i = 0; i < numCats; ++i)
                cats.at(i).symbol()->setColor(ramp.color(i));
        }
        else
        {
            for (int i = 0; i < numCats; ++i)
                cats.at(i).symbol()->setColor(colors[i]);
        }
    }
    else
    {
        QgsRandomColorRamp ramp;
        ramp.setTotalColorCount(numCats);

        for (int i = 0; i < numCats; ++i)
            cats.at(i).symbol()->setColor(ramp.color(i));
    }

    QgsCategorizedSymbolRenderer* renderer = new QgsCategorizedSymbolRenderer(attrName, cats);
    renderer->setSourceSymbol(symbol);

    vlayer->setRenderer(renderer);

}


void QGISVisualizationWidget::createSimpleRenderer(QgsSymbol* symbol, QgsVectorLayer * layer)
{
    QgsSingleSymbolRenderer* renderer = new QgsSingleSymbolRenderer(symbol);

    layer->setRenderer(renderer);
}


void QGISVisualizationWidget::createSymbolRenderer(QgsSimpleMarkerSymbolLayerBase::Shape symbolShape, QColor color, double size, QgsVectorLayer * layer)
{
    QgsSimpleMarkerSymbolLayer *mSimpleMarkerLayer = new QgsSimpleMarkerSymbolLayer();
    mSimpleMarkerLayer->setColor(color);
    mSimpleMarkerLayer->setStrokeColor(Qt::black);
    mSimpleMarkerLayer->setShape(symbolShape);
    mSimpleMarkerLayer->setSize(size);

    QgsMarkerSymbol* mMarkerSymbol = new QgsMarkerSymbol();
    mMarkerSymbol->changeSymbolLayer(0, mSimpleMarkerLayer);
    mMarkerSymbol->setColor(color);

    QgsSingleSymbolRenderer *mSymbolRenderer = mSymbolRenderer = new QgsSingleSymbolRenderer(mMarkerSymbol);

    layer->setRenderer(mSymbolRenderer);
}


QWidget* QGISVisualizationWidget::getVisWidget()
{
    return this;
}


double QGISVisualizationWidget::getLatFromScreenPoint(const QPointF& point)
{
    return 0.0;
}


double QGISVisualizationWidget::getLongFromScreenPoint(const QPointF& point)
{
    return 0.0;
}


QPointF QGISVisualizationWidget::getScreenPointFromLatLong(const double& latitude, const double& longitude)
{
    return QPointF();
}


void QGISVisualizationWidget::clear(void)
{
    qgis->layerTreeView()->selectAll();

    const QList<QgsMapLayer *> selectedLayers = qgis->layerTreeView()->selectedLayersRecursive();

    for(auto&& it: selectedLayers)
        this->removeLayer(it);


    mapSelectableAssetWidgets.clear();
}


void QGISVisualizationWidget::removeLayer(QgsMapLayer* layer)
{
    if(layer == nullptr)
        return;

    auto layerTreeView = qgis->layerTreeView();

    layerTreeView->setCurrentLayer(layer);

    // False to not show prompt asking user
    qgis->removeLayer(false);

    layer = nullptr;
}


void QGISVisualizationWidget::handleLegendChange(const QString layerUID)
{

}


void QGISVisualizationWidget::clearSelection(void)
{

    return;
}


void QGISVisualizationWidget::selectionChanged( const QgsFeatureIds &selected, const QgsFeatureIds &deselected, bool clearAndSelect )
{
    if(clearAndSelect == true)
        qDebug()<<"Clear and select true";
    else
        qDebug()<<"Clear and select false";
}


QgsGeometry QGISVisualizationWidget::getPolygonGeometryFromJson(const QString& geoJson)
{
    QRegularExpression rx("[^\\[\\]]+(?=\\])");

    QRegularExpressionMatchIterator i = rx.globalMatch(geoJson);

    QStringList pointsList;
    while (i.hasNext())
    {
        QRegularExpressionMatch match = i.next();

        if(!match.hasMatch())
            continue;

        QString word = match.captured(0);
        pointsList << word;
    }

    if(pointsList.empty())
        return QgsGeometry();

    //    typedef QVector<QgsPolylineXY> QgsPolygonXY;
    QgsPolygonXY polygon;
    QgsPolylineXY polyLineRing;
    for(auto&& it : pointsList)
    {
        auto points = it.split(",");

        if(points.size() != 2)
            return QgsGeometry();

        bool OK = false;
        double lat = points.at(0).toDouble(&OK);

        if(!OK)
            return QgsGeometry();

        double lon = points.at(1).toDouble(&OK);
        if(!OK)
            return QgsGeometry();

        polyLineRing.append(QgsPointXY(lat,lon));
    }

    polygon.append(polyLineRing);

    QgsGeometry geom = QgsGeometry::fromPolygonXY(polygon);

    return geom;
}


void QGISVisualizationWidget::selectLayerInTree(QgsMapLayer* layer)
{
    auto layerIndex = this->getLayerIndex(layer);

    auto selectModel = layerTreeView->selectionModel();

    selectModel->select(layerIndex , QItemSelectionModel::Select);
}


void QGISVisualizationWidget::setActiveLayer(QgsMapLayer* layer)
{
    qgis->setActiveLayer(layer);
}


void QGISVisualizationWidget::zoomToLayer(QgsMapLayer* layer)
{

    this->selectLayerInTree(layer);

    qgis->zoomToLayerExtent();

    return;
}


QgisApp* QGISVisualizationWidget::getQgis(void)
{
    return qgis;
}


QgsMapCanvas* QGISVisualizationWidget::getMainCanvas(void)
{
    return qgis->mapCanvas();
}


void QGISVisualizationWidget::zoomToLayer(const QString /*layerID*/)
{

    this->statusMessage("Implement me QGISVisualizationWidget::zoomToLayer");

    return;
}


void QGISVisualizationWidget::setLayerVisibility(QgsMapLayer* layer, bool value)
{
    layerTreeView->setLayerVisible(layer, value);
}


QgsGeometry QGISVisualizationWidget::getPolygonGeometryFromJson(const QJsonArray& geoJson)
{

    if(geoJson.size() == 0)
        return QgsGeometry();


    QgsPolygonXY polygon;
    QgsPolylineXY polyLineRing;
    for(auto&& it : geoJson)
    {

        auto points = it.toArray();

        if(points.size() != 2)
            return QgsGeometry();

        double lat = points.at(1).toDouble(360.0);
        double lon = points.at(0).toDouble(360.0);

        if(lat == 360.0 || lon == 360.0)
            return QgsGeometry();

        polyLineRing.append(QgsPointXY(lat,lon));
    }

    polygon.append(polyLineRing);

    QgsGeometry geom = QgsGeometry::fromPolygonXY(polygon);

    return geom;
}


void QGISVisualizationWidget::registerLayerForSelection(const QString layerId, GISSelectable* widget)
{
    mapSelectableAssetWidgets.insert(layerId,widget);
}


void QGISVisualizationWidget::handleSelectButton(void)
{
    qgis->selectFeatures();
}


void QGISVisualizationWidget::handleIdentifyButton(void)
{
    qgis->identify();
}
