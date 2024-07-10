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

#include "CSVReaderWriter.h"
#include "ComponentDatabaseManager.h"
#include "GeneralInformationWidgetR2D.h"
#include "MainWindowWorkflowApp.h"
#include "Pelicun3PostProcessor.h"
#include "REmpiricalProbabilityDistribution.h"
#include "TablePrinter.h"
#include "TableNumberItem.h"
#include "VisualizationWidget.h"
#include "WorkflowAppR2D.h"
#include "Utils/ProgramOutputDialog.h"
#include "PelicunPostProcessor.h"
#include <QBarCategoryAxis>
#include <QBarSeries>
#include <QBarSet>
#include <QChart>
#include <QChartView>
#include <QComboBox>
#include <QDir>
#include <QDockWidget>
#include <QFileInfo>
#include <QFontMetrics>
#include <QGraphicsLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QLineSeries>
#include <QMenuBar>
#include <QPixmap>
#include <QPrinter>
#include <QStackedBarSeries>
#include <QStringList>
#include <QTabWidget>
#include <QTableWidget>
#include <QTextCursor>
#include <QTextTable>
#include <QValueAxis>

#include "QGISVisualizationWidget.h"

#include <qgsattributes.h>
#include <qgsmapcanvas.h>

// Test to remove start
// #include <chrono>
// using namespace std::chrono;
// Test to remove end

using namespace QtCharts;

Pelicun3PostProcessor::Pelicun3PostProcessor(QWidget * parent) : SC_ResultsWidget(parent){
    layout = new QVBoxLayout(this);
    mainWindow = new QMainWindow(parent);

    // Total widget
//    totalAndFootNoteWidget = new QWidget(mainWindow);
//    totalAndFootNoteLayout = new QVBoxLayout(totalAndFootNoteWidget);
//    totalsWidget = new QWidget(totalAndFootNoteWidget);
//    totalsWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
//    QGridLayout* totalsLayout = new QGridLayout(totalsWidget);

//    totalRepairCostLabel = new QLabel("Repair cost*: ", totalsWidget);
//    totalRepairTimeSequentialLabel = new QLabel("Repair Time (Sequential) [days]", totalsWidget);
//    totalRepairTimeParallelLabel = new QLabel("Repair Time (Parallel) [days]", totalsWidget);

//    totalRepairCostValueLabel = new QLabel("", totalsWidget);
//    totalRepairTimeSequentialValueLabel = new QLabel("", totalsWidget);
//    totalRepairTimeParallelValueLabel = new QLabel("", totalsWidget);

//    totalCostNoteLabel = new QLabel("* The unit of repair cost is the same with the units defined in ASD.", totalAndFootNoteWidget);
//    totalCostNoteLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

//    totalsLayout->addWidget(totalRepairCostLabel,0,0);
//    totalsLayout->addWidget(totalRepairCostValueLabel,0,1,1,1,Qt::AlignLeft);
//    totalsLayout->addWidget(totalRepairTimeSequentialLabel,0,2);
//    totalsLayout->addWidget(totalRepairTimeSequentialValueLabel,0,3,1,1,Qt::AlignLeft);
//    totalsLayout->addWidget(totalRepairTimeParallelLabel,1,0);
//    totalsLayout->addWidget(totalRepairTimeParallelValueLabel,1,1,1,1,Qt::AlignLeft);
//    totalAndFootNoteLayout->addWidget(totalsWidget);
//    totalAndFootNoteLayout->addWidget(totalCostNoteLabel);
//    totalAndFootNoteLayout->addStretch();
//    totalAndFootNoteWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

//    summaryDock = new QDockWidget("Estimated Regional Totals",this);
//    summaryDock->setObjectName("SummaryDock");
//    summaryDock->setWidget(totalAndFootNoteWidget);
//    summaryDock->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
//    summaryDock->setMaximumHeight(200);
//    mainWindow->addDockWidget(Qt::RightDockWidgetArea, summaryDock);

    mainWindow->show();
    layout->addWidget(mainWindow);

    mapViewDock = new QDockWidget("Regional Map",mainWindow);

}


//Constructor
Pelicun3PostProcessor::Pelicun3PostProcessor(QWidget *parent, ResultsWidget *resWidget, QMap<QString, QList<QString>> assetTypeToType)
    : SC_ResultsWidget(parent,resWidget,assetTypeToType){};
// AddResultTab
int Pelicun3PostProcessor::addResultTab(QString tabName){
    repeat current processResults;

 }
// AddResultSubtab
int Pelicun3PostProcessor::addResultSubtab(QString name, QWidget* existTab){
    if dynamic_cast<QWidget> to SC_ResultsWidget
        create the new sub tab widget
                existTab->addResultSubtab()
    else
        add existTab to docklist...
}



int Pelicun3PostProcessor::processResults(QString &outputFile, QString &dirName, QString &assetType,
                                          QList<QString> typesInAssetType){

    theVisualizationWidget = dynamic_cast<VisualizationWidget*> (theVizWidget);
    if (theVisualizationWidget == nullptr || theVisualizationWidget==0){
        this->errorMessage("Can't convert to the visualization widget");
        return -1;
    }
    // AssemblePDF is not implemented
//    connect(theVisualizationWidget,&VisualizationWidget::emitScreenshot,this,&Pelicun3PostProcessor::assemblePDF);


    // Get the map view widget
    auto mapView = theVisualizationWidget->getMapViewWidget("ResultsWidget");
    mapViewSubWidget = std::shared_ptr<SimCenterMapcanvasWidget>(mapView);
    QgsMapCanvas* mapCanvas = mapViewSubWidget->mapCanvas();
    QList<QgsMapLayer*> allLayers= mapCanvas->layers();
    for (int map_i = 0; map_i < allLayers.count(); ++map_i) {
        QString layerName = allLayers.at(map_i)->name();
        for (int type_i = 0; type_i < typesInAssetType.count(); ++type_i){
            if (layerName.compare(typesInAssetType.at(type_i)) == 0){
                neededLayers->append(allLayers.at(map_i));
            }
        }
    }
    mapCanvas->setLayers(*neededLayers);

    mapViewDock->setObjectName("MapViewDock");
    mapViewDock->setAllowedAreas(Qt::LeftDockWidgetArea);
    mapViewDock->setWidget(mapViewSubWidget.get());
    mainWindow->addDockWidget(Qt::LeftDockWidgetArea, mapViewDock);

//    double totalRepairCostValue = 0.0;
//    double totalRepairTimeSequentialValue = 0.0;
//    double totalRepairTimeParallelValue = 0.0;
    for (int type_i=0; type_i<typesInAssetType.count(); type_i++){
        QString type = typesInAssetType.at(type_i);
        QString pathGeojson = dirName + QDir::separator() +  type + QString(".geojson");
        QFile jsonFile(pathGeojson);
        QJsonArray features;
        if (jsonFile.exists() && jsonFile.open(QFile::ReadOnly)){
            QJsonDocument exDoc = QJsonDocument::fromJson(jsonFile.readAll());
            QJsonObject jsonObject = exDoc.object();
            features = jsonObject["features"].toArray();
        }

        QStringList extractAttributes = {"AIM_id"};
        QStringList comboBoxHeadings = {"AIM_id"};
        bool hasR2DresToShow = false;
        for (int i = 0; i < features.size(); i++){
            QJsonObject ft = features.at(i).toObject();
            QJsonObject properties = ft["properties"].toObject();
            QStringList keys = properties.keys();
            foreach (const QString &key, keys) {
                if (key.startsWith("R2Dres_")){
                    QString resultName = key.section('_', 1);
                    if (!resultName.startsWith("MostLikelyDamageState") &&
                        !extractAttributes.contains(key)){
                        extractAttributes.append(key);
                        comboBoxHeadings.append(key.section('_', 1));
                        hasR2DresToShow = true;
                    }
                }
            }
        }
        if (!hasR2DresToShow) {
            continue;
        }

//        totalRepairCostValue += calculateTotal(features, "mean repair_cost-");
//        totalRepairTimeSequentialValue += calculateTotal(features, "mean repair_time-sequential");
//        totalRepairTimeParallelValue += calculateTotal(features, "mean repair_time-parallel");

        //Dock widget for this type
        QDockWidget* typeDockWidget = new QDockWidget(type,mainWindow);
        typeDockWidget->setObjectName(type + "TableDock");
        // Create the table that will show the Component information

        QWidget* typetableWidget = new QWidget(typeDockWidget);

        QVBoxLayout* typetableWidgetLayout = new QVBoxLayout(typetableWidget);

        QTableWidget* typeResultsTableWidget = new QTableWidget(typeDockWidget);
        typeResultsTableWidget->verticalHeader()->setVisible(false);
        typeResultsTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

        typeResultsTableWidget->setSizeAdjustPolicy(QAbstractScrollArea::SizeAdjustPolicy::AdjustToContents);
        typeResultsTableWidget->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum);

        typeResultsTableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        typeResultsTableWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

        typeResultsTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

        typeResultsTableWidget->setItemDelegate(new DoubleDelegate(typeDockWidget,3));
        tableList.append(typeResultsTableWidget);
        // Combo box to select how to sort the table
        QHBoxLayout* comboLayout = new QHBoxLayout();


//        QStringList comboBoxHeadings = {"Asset ID","Mean Repair Cost","Mean Repair Time, Parallel [days]","Mean Repair Time, Sequential [days]", "Most Likely Critical Damage State"};
        QComboBox* sortComboBox = new QComboBox(typeDockWidget);
        sortComboBox->insertItems(0,comboBoxHeadings);
        sortComboBox->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);

        connect(sortComboBox,QOverload<int>::of(&QComboBox::currentIndexChanged),this, &Pelicun3PostProcessor::sortTable);

        auto comboBoxLabel = new QLabel("Sorting Filter:", typeDockWidget);

        comboLayout->addWidget(comboBoxLabel);
        comboLayout->addWidget(sortComboBox);
        comboLayout->addStretch(0);
        typetableWidgetLayout->addLayout(comboLayout);
        typetableWidgetLayout->addWidget(typeResultsTableWidget);
        typetableWidgetLayout->addStretch(0);

//        QStringList extractAttributes = {"AIM_id","mean repair_cost-","mean repair_time-parallel","mean repair_time-sequential", "highest_DMG"};
        extractDataAddToTable(features, extractAttributes,typeResultsTableWidget, comboBoxHeadings);
        dockList->append(typeDockWidget);

        typeDockWidget->setWidget(typetableWidget);
        typeDockWidget->setMinimumWidth(475);
        typeDockWidget->setMaximumWidth(575);
        typeDockWidget->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Maximum);
        mainWindow->addDockWidget(Qt::RightDockWidgetArea, typeDockWidget);

    }
//    totalRepairCostValueLabel->setText(QString::number(totalRepairCostValue));
//    totalRepairTimeSequentialValueLabel->setText(QString::number(totalRepairTimeSequentialValue));
//    totalRepairTimeParallelValueLabel->setText(QString::number(totalRepairTimeParallelValue));
    // tabify dock widgets
    if (dockList->count()>1){
        QDockWidget* base = dockList->at(0);
        for (int dock_i = 1; dock_i<dockList->count(); dock_i++){
            mainWindow->tabifyDockWidget(base,dockList->at(dock_i));
        }
    }
    // resize docks
    QList<QDockWidget*> alldocks = {mapViewDock};
    float windowWidth = mainWindow->size().width();
    float windowHeight = mainWindow->size().height();
    QList<int> dockWidthes = {int(0.7*windowWidth)};
    QList<int> dockHeights = {int(windowHeight)};
    for (int dock_i = 0; dock_i<dockList->count(); dock_i++){
        alldocks.append(dockList->at(dock_i));
        dockWidthes.append(int(0.3*windowWidth));
        dockHeights.append(int(0.7*windowHeight));
    }
    mainWindow->resizeDocks(alldocks, dockWidthes, Qt::Horizontal);
    mainWindow->resizeDocks(alldocks, dockHeights, Qt::Vertical);


    auto menuBar = WorkflowAppR2D::getInstance()->getTheMainWindow()->menuBar();
    QMenu* resultsMenu = nullptr;
    foreach (QAction *action, menuBar->actions()) {
        auto actionMenu = action->menu();
        if(actionMenu)
        {
            if(action->text().compare("&Results") == 0)
                resultsMenu = actionMenu;
        }
    }
    if(resultsMenu)
    {
        foreach(QAction* action, resultsMenu->actions()){
            if (action->text().compare("&" + assetType) == 0){
                auto actionMenu = action->menu();
                actionMenu->clear();
                resultsMenu->removeAction(action);
            }
        }
        viewMenu = resultsMenu->addMenu("&" + assetType);
        viewMenu->addAction(tr("&Restore"), this, &Pelicun3PostProcessor::restoreUI);
//        viewMenu->addAction(summaryDock->toggleViewAction());
        viewMenu->addAction(mapViewDock->toggleViewAction());
        for (int dock_i = 0; dock_i<dockList->count(); dock_i++){
            viewMenu->addAction(dockList->at(dock_i)->toggleViewAction());
        }
        uiState = mainWindow->saveState();

    }
    else
    {
        ProgramOutputDialog::getInstance()->appendErrorMessage("Could not find the results menu bar in Pelicun3PostProcessor::");
        return 1;
    }

}

int Pelicun3PostProcessor::extractDataAddToTable(QJsonArray& features, QStringList& attributes, QTableWidget* table, QStringList headings){
    QVector<QVector<double>> result(features.count(), QVector<double> (attributes.count(), 0.0));
    table->setColumnCount(headings.size());
    table->setHorizontalHeaderLabels(headings);
    table->setRowCount(features.count());
    for (int m = 0; m < features.count(); m++){
        QJsonObject ft = features.at(m).toObject();
        QJsonObject properties = ft["properties"].toObject();
        for (int n = 0; n < attributes.count(); n++){
            QString attri = attributes.at(n);
            if (!properties.contains(attri)){
                if (!properties.contains("type")) {
//                    this->errorMessage(attri + QString("dose not exist in R2D_results.geojson"));
                    return -1;
                } else {
//                    this->errorMessage(attri + QString("for") + properties["type"].toString() + QString(" ") + ft["id"].toString() + QString(" dose not exist in R2D_results.geojson"));
                    if (attri.compare("AIM_id")==0){
                        auto item = new TableNumberItem(QString::number(m));
                        table->setItem(m, n, item);
                    } else {
                        auto item = new TableNumberItem("");
                        table->setItem(m, n, item);
                    }
                }
            }
            else{
                if (attri.compare("AIM_id")==0){
                    auto item = new TableNumberItem(properties[attri].toString());
                    table->setItem(m, n, item);
                } else {
                    result[m][n] = properties[attri].toDouble();
                    auto item = new TableNumberItem(QString::number(result[m][n]));
                    table->setItem(m, n, item);
                }
            }
        }
    }
    return 0;
}

double Pelicun3PostProcessor::calculateTotal(QJsonArray& featArray, QString field){
    double result = 0.0;
    for (auto ft : featArray){
        QJsonObject properties = ft.toObject()["properties"].toObject();
        if (!properties.contains(field)){
            this->errorMessage(field + QString(" dose not exist in R2D_results.geojson"));
            return 0.0;
        }
        else{
            result += properties[field].toDouble();
        }
    }
    return result;
}


void Pelicun3PostProcessor::showEvent(QShowEvent *e)
{
//    auto mainCanvas = mapViewSubWidget->getMainCanvas();

//    auto mainExtent = mainCanvas->extent();

//    mapViewSubWidget->mapCanvas()->zoomToFeatureExtent(mainExtent);
//    QMainWindow::showEvent(e);
}


void Pelicun3PostProcessor::setIsVisible(const bool value)
{
    if(viewMenu)
        viewMenu->menuAction()->setVisible(value);
}




//int Pelicun3PostProcessor::printToPDF(const QString& outputPath)
//{
//    outputFilePath = outputPath;

//    theVisualizationWidget->takeScreenShot();

//    return 0;
//}


int Pelicun3PostProcessor::assemblePDF(QImage screenShot)
{

    return 0;
}


void Pelicun3PostProcessor::sortTable(int index)
{
    if(index == 0){
        for(int i = 0; i < tableList.count(); i++){
            tableList.at(i)->sortByColumn(index, Qt::AscendingOrder);
        }
    }
    else{
        for(int i = 0; i < tableList.count(); i++){
            tableList.at(i)->sortByColumn(index, Qt::DescendingOrder);
        }
    }
}


void Pelicun3PostProcessor::restoreUI(void)
{
    mainWindow->restoreState(uiState);
}


void Pelicun3PostProcessor::setCurrentlyViewable(bool status){

    Q_UNUSED(status);

}


//void Pelicun3PostProcessor::addSiteResponseTable(void)
//{
//    // kz: adding a dock layer for different assets
//    // site table
//    this->tabifyDockWidget(tableDock,tableDock2);
//    tableDock->setFocus();
//    if(viewMenu)
//    {
//        viewMenu->addAction(tableDock2->toggleViewAction());
//    }
//    tableWidget2 = new QWidget(static_cast<QMainWindow*>(this));
//    auto tableWidgetLayout = new QVBoxLayout(tableWidget2);
//    siteResponseTableWidget = new QTableWidget(static_cast<QMainWindow*>(this));
//    siteResponseTableWidget->verticalHeader()->setVisible(false);
//    siteResponseTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
//    siteResponseTableWidget->setSizeAdjustPolicy(QAbstractScrollArea::SizeAdjustPolicy::AdjustToContents);
//    siteResponseTableWidget->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum);
//    siteResponseTableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
//    siteResponseTableWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
//    siteResponseTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
//    siteResponseTableWidget->setItemDelegate(new DoubleDelegate(static_cast<QMainWindow*>(this),3));
//    tableWidgetLayout->addWidget(siteResponseTableWidget);
//    tableDock2->setWidget(tableWidget2);
//}

//int Pelicun3PostProcessor::processIMResults(const QVector<QStringList>& IMResults)
//{
//    qDebug() << "Starting processing IM results";
//    if(IMResults.size() < numHeaderRows)
//    {
//        QString msg = "No IM results to import!";
//        throw msg;
//    }

//    auto numHeaderColumnsFull = IMResults.at(0).size();

//    QStringList headerStringsFull = {"Site ID"};
//    QStringList headerStrings = {"Site ID"};
//    for(int i = 1; i<numHeaderColumnsFull; ++i)
//    {
//        QString headerStr = IMResults.at(0).at(i) +"-"+ IMResults.at(2).at(i) +"-"+ IMResults.at(3).at(i);
//        headerStringsFull.append(headerStr);
//        if (headerStr.contains("median"))
//        {
//            if (headerStr.contains("PG") || headerStr.contains("SA(1.0s)"))
//                headerStrings.append(headerStr);
//        }
//    }
//    auto numHeaderColumns = headerStrings.size();
//    siteResponseTableWidget->setColumnCount(headerStrings.size());
//    siteResponseTableWidget->setHorizontalHeaderLabels(headerStrings);
//    siteResponseTableWidget->setRowCount(IMResults.size()-numHeaderRows);

//    // Get the site database
//    auto theSiteDB = ComponentDatabaseManager::getInstance()->getAssetDb("SiteSoilColumn");
//    if(theSiteDB == nullptr)
//    {
//        QString msg = "Error getting the site database from the input widget!";
//        throw msg;
//    }
//    if(theSiteDB->isEmpty())
//    {
//        QString msg = "Site database is empty";
//        throw msg;
//    }

//    auto selFeatLayer = theSiteDB->getSelectedLayer();
//    mapViewSubWidget->setCurrentLayer(selFeatLayer);

//    // Vector to hold the attributes
//    QVector< QgsAttributes > fieldAttributes2(IMResults.size()-numHeaderRows, QgsAttributes(numHeaderColumns));

//    // Loop over all sites
//    for(int i = numHeaderRows, count = 0; i<IMResults.size(); ++i, ++count)
//    {
//        auto inputRow = IMResults.at(i);
//        auto siteID = new TableNumberItem(QString::number(objectToInt(inputRow.at(0))));
//        siteResponseTableWidget->setItem(count, 0, siteID);
//        // Loop over all IMs
//        for(int  j = 1; j < headerStrings.size(); j++)
//        {
//            auto curItem = new TableNumberItem(QString::number(objectToDouble(inputRow.at(headerStringsFull.indexOf(headerStrings.at(j))))));
//            siteResponseTableWidget->setItem(count, j, curItem);
//        }
//        auto& rowData = fieldAttributes2[count];
//        // Populate the attributes vector with the results
//        //for(int k = 0; k<inputRow.size(); ++k)
//        for(int  k = 0; k < headerStrings.size(); k++)
//        {
//            // Add the result to the database
//            auto value = inputRow.at(headerStringsFull.indexOf(headerStrings.at(k)));
//            rowData[k] = QVariant(value.toDouble());
//        }
//    }

//    // Starting editing
//    theSiteDB->startEditing();
//    QString errMsg;
//    auto res = theSiteDB->addNewComponentAttributes(headerStrings,fieldAttributes2,errMsg);
//    if(!res)
//        throw errMsg;

//    // Commit the changes
//    theSiteDB->commitChanges();

//    ProgramOutputDialog::getInstance()->appendText("Done processing site response results "/*+QString::number(duration.count())*/);
//    // Apply the default renderer
//    QGISVisWidget->createPrettyGraduatedRenderer("PGA-1-median",Qt::yellow,Qt::red,5,theSiteDB->getSelectedLayer());
//    theSiteDB->getSelectedLayer()->setName("Site Response (PGA in Dir.1, g)");
//}


void Pelicun3PostProcessor::clear(void)
{

    for (int i = 0; i < tableList.count(); i++){
        QTableWidget* parentWidget = tableList.at(i);
        parentWidget->clear();
    }
    tableList.clear();
    for (int i = 0; i < dockList->count(); i++){
        QDockWidget* parentWidget = dockList->at(i);
        qDeleteAll(parentWidget->findChildren<QWidget*>("", Qt::FindDirectChildrenOnly));
        delete dockList->at(i);
    }
    dockList->clear();

    mapViewSubWidget->clear();
}

