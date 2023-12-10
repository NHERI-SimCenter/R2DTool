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
    totalAndFootNoteWidget = new QWidget(mainWindow);
    totalAndFootNoteLayout = new QVBoxLayout(totalAndFootNoteWidget);
    totalsWidget = new QWidget(totalAndFootNoteWidget);
    totalsWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    QGridLayout* totalsLayout = new QGridLayout(totalsWidget);

    totalRepairCostLabel = new QLabel("Repair cost*: ", totalsWidget);
    totalRepairTimeSequentialLabel = new QLabel("Repair Time (Sequential) [days]", totalsWidget);
    totalRepairTimeParallelLabel = new QLabel("Repair Time (Parallel) [days]", totalsWidget);

    totalRepairCostValueLabel = new QLabel("", totalsWidget);
    totalRepairTimeSequentialValueLabel = new QLabel("", totalsWidget);
    totalRepairTimeParallelValueLabel = new QLabel("", totalsWidget);

    totalCostNoteLabel = new QLabel("* The unit of repair cost is the same with the units defined in ASD.", totalAndFootNoteWidget);
    totalCostNoteLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    totalsLayout->addWidget(totalRepairCostLabel,0,0);
    totalsLayout->addWidget(totalRepairCostValueLabel,0,1,1,1,Qt::AlignLeft);
    totalsLayout->addWidget(totalRepairTimeSequentialLabel,0,2);
    totalsLayout->addWidget(totalRepairTimeSequentialValueLabel,0,3,1,1,Qt::AlignLeft);
    totalsLayout->addWidget(totalRepairTimeParallelLabel,1,0);
    totalsLayout->addWidget(totalRepairTimeParallelValueLabel,1,1,1,1,Qt::AlignLeft);
//    totalsLayout->addWidget(totalCostNoteLabel, 2, 0, 3, 1, Qt::AlignLeft);
    totalAndFootNoteLayout->addWidget(totalsWidget);
    totalAndFootNoteLayout->addWidget(totalCostNoteLabel);
    totalAndFootNoteLayout->addStretch();
    totalAndFootNoteWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    QDockWidget* summaryDock = new QDockWidget("Estimated Regional Totals",this);
    summaryDock->setObjectName("SummaryDock");
    summaryDock->setWidget(totalAndFootNoteWidget);
    summaryDock->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    mainWindow->addDockWidget(Qt::RightDockWidgetArea, summaryDock);

//    layout->addWidget(totalsWidget);
//    QTabWidget* MainWindownContainer = new QTabWidget();
//    MainWindownContainer->addTab(mainWindow, "test");
    mainWindow->show();
    layout->addWidget(mainWindow);

    mapViewDock = new QDockWidget("Regional Map",mainWindow);

}

int Pelicun3PostProcessor::processResults(QString &outputFile, QString &dirName, QString &assetType,
                                          QList<QString> typesInAssetType){

    theVisualizationWidget = dynamic_cast<VisualizationWidget*> (theVizWidget);
    if (theVisualizationWidget == nullptr || theVisualizationWidget==0){
        this->errorMessage("Can't convert to the visualization widget");
        return -1;
    }
    connect(theVisualizationWidget,&VisualizationWidget::emitScreenshot,this,&Pelicun3PostProcessor::assemblePDF);
//    std::unique_ptr<PelicunPostProcessor> thePelicunPostProcessor = std::make_unique<PelicunPostProcessor>(this,theVisualizationWidget);
//    thePelicunPostProcessor->show();
//    layout->addWidget(thePelicunPostProcessor.get());
    // Create a view menu for the dockable windows
    // Get the map view widget
    auto mapView = theVisualizationWidget->getMapViewWidget("ResultsWidget");
//    QGISVisWidget = static_cast<QGISVisualizationWidget*>(theVisualizationWidget);
//    QGISVisWidget->createPrettyGraduatedRenderer("LossRatio",Qt::yellow,Qt::red,5,theBuildingDB->getSelectedLayer());

    mapViewSubWidget = std::unique_ptr<SimCenterMapcanvasWidget>(mapView);



    QgsMapCanvas* mapCanvas = mapViewSubWidget->mapCanvas();
    QList<QgsMapLayer*> allLayers= mapCanvas->layers();
    QList<QgsMapLayer*> neededLayers;
    for (int map_i = 0; map_i < allLayers.count(); ++map_i) {
        QString layerName = allLayers.at(map_i)->name();
        for (int type_i = 0; type_i < typesInAssetType.count(); ++type_i){
            if (layerName.compare(typesInAssetType.at(type_i)) == 0){
                neededLayers.append(allLayers.at(map_i));
            }
        }
    }
    bool isf = mapCanvas->isFrozen();
    mapCanvas->setLayers(neededLayers);

    mapViewDock->setObjectName("MapViewDock");
    mapViewDock->setAllowedAreas(Qt::LeftDockWidgetArea);
    mapViewDock->setWidget(mapViewSubWidget.get());
    mainWindow->addDockWidget(Qt::LeftDockWidgetArea, mapViewDock);

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
//        viewMenu = resultsMenu->addMenu(tr("&Pelicun3" + assetType));
//        QAction* resultsMenuAction = resultsMenu->menuAction();
        foreach(QAction* action, resultsMenu->actions()){
            if (action->text().compare("&" + assetType) == 0){
                auto actionMenu = action->menu();
                actionMenu->clear();
                resultsMenu->removeAction(action);
            }
        }
        viewMenu = resultsMenu->addMenu("&" + assetType);
        viewMenu->addAction(tr("&Restore"), this, &Pelicun3PostProcessor::restoreUI);
    }
    else
    {
        ProgramOutputDialog::getInstance()->appendErrorMessage("Could not find the results menu bar in Pelicun3PostProcessor::");
        return 1;
    }

    double totalRepairCostValue = 0.0;
    double totalRepairTimeSequentialValue = 0.0;
    double totalRepairTimeParallelValue = 0.0;
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

        totalRepairCostValue += calculateTotal(features, "mean repair_cost-");
        totalRepairTimeSequentialValue += calculateTotal(features, "mean repair_time-sequential");
        totalRepairTimeParallelValue += calculateTotal(features, "mean repair_time-parallel");

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
        ResultsTableWidgetList.append(typeResultsTableWidget);
        // Combo box to select how to sort the table
        QHBoxLayout* comboLayout = new QHBoxLayout();
        QStringList comboBoxHeadings = {"Asset ID","Mean Repair Cost","Mean Repair Time, Parallel [days]","Mean Repair Time, Sequential [days]", "Most Likely Critical Damage State"};
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

        QStringList extractAttributes = {"AIM_id","mean repair_cost-","mean repair_time-parallel","mean repair_time-sequential", "highest_DMG"};
        extractDataAddToTable(features, extractAttributes,typeResultsTableWidget, comboBoxHeadings);
//        dataList.append(typedata);
//        typeDockWidget->setAllowedAreas(Qt::RightDockWidgetArea);
        dockList.append(typeDockWidget);

        typeDockWidget->setWidget(typetableWidget);
        typeDockWidget->setMinimumWidth(475);
        typeDockWidget->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Maximum);
        mainWindow->addDockWidget(Qt::RightDockWidgetArea, typeDockWidget);

    }
    totalRepairCostValueLabel->setText(QString::number(totalRepairCostValue));
    totalRepairTimeSequentialValueLabel->setText(QString::number(totalRepairTimeSequentialValue));
    totalRepairTimeParallelValueLabel->setText(QString::number(totalRepairTimeParallelValue));

    // tabify dock widgets
    if (dockList.count()>1){
        QDockWidget* base = dockList.at(0);
        for (int dock_i = 1; dock_i<dockList.count(); dock_i++){
            mainWindow->tabifyDockWidget(base,dockList.at(dock_i));
        }
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
                this->errorMessage(attri + QString(" dose not exist in R2D_results.geojson"));
                return -1;
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

//void Pelicun3PostProcessor::addDataToTable(QVector<QVector<double>>* data, QTableWidget* table, QStringList headings){
//    table->setColumnCount(headings.size());
//    table->setHorizontalHeaderLabels(headings);
//    table->setRowCount(data->count());



//}

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

//Pelicun3PostProcessor::Pelicun3PostProcessor(QWidget *parent, VisualizationWidget* visWidget) : QMainWindow(parent), theVisualizationWidget(visWidget)
//{
//    casualtiesChart = nullptr;
//    RFDiagChart = nullptr;
//    Losseschart = nullptr;
//    viewMenu = nullptr;



//    connect(theVisualizationWidget,&VisualizationWidget::emitScreenshot,this,&Pelicun3PostProcessor::assemblePDF);

//    // Summary group box
//    QWidget* totalsWidget = new QWidget(this);
//    totalsWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
//    QGridLayout* totalsLayout = new QGridLayout(totalsWidget);

//    totalCasLabel = new QLabel("Casualties:", this);
//    totalFatalitiesLabel = new QLabel("Fatalities:", this);
//    totalLossLabel = new QLabel("Losses:", this);
//    totalRepairTimeLabel = new QLabel("Repair Time [days]:", this);
//    structLossLabel = new QLabel("Structural Losses:", this);
//    nonStructLossLabel = new QLabel("Non-structural Losses:", this);

//    totalCasValueLabel = new QLabel("", this);
//    totalLossValueLabel = new QLabel("", this);
//    totalRepairTimeValueLabel = new QLabel("", this);
//    totalFatalitiesValueLabel = new QLabel("", this);
//    structLossValueLabel = new QLabel("", this);
//    nonStructLossValueLabel = new QLabel("", this);

//    totalsLayout->addWidget(totalCasLabel,0,0);
//    totalsLayout->addWidget(totalCasValueLabel,0,1,1,1,Qt::AlignLeft);
//    totalsLayout->addWidget(totalFatalitiesLabel,0,2);
//    totalsLayout->addWidget(totalFatalitiesValueLabel,0,3,1,1,Qt::AlignLeft);
//    totalsLayout->addWidget(totalLossLabel,1,0);
//    totalsLayout->addWidget(totalLossValueLabel,1,1,1,1,Qt::AlignLeft);
//    totalsLayout->addWidget(totalRepairTimeLabel,1,2);
//    totalsLayout->addWidget(totalRepairTimeValueLabel,1,3,1,1,Qt::AlignLeft);
//    totalsLayout->addWidget(structLossLabel,2,0);
//    totalsLayout->addWidget(structLossValueLabel,2,1,1,1,Qt::AlignLeft);
//    totalsLayout->addWidget(nonStructLossLabel,2,2);
//    totalsLayout->addWidget(nonStructLossValueLabel,2,3,1,1,Qt::AlignLeft);

//    QDockWidget* summaryDock = new QDockWidget("Estimated Regional Totals",this);
//    summaryDock->setObjectName("SummaryDock");
//    summaryDock->setWidget(totalsWidget);
//    addDockWidget(Qt::RightDockWidgetArea, summaryDock);

//    // Charts
//    chartsDock1 = new QDockWidget(tr("Casualties"), this);
//    chartsDock1->setObjectName("Casualties");
//    chartsDock1->setContentsMargins(5,5,5,5);

//    chartsDock2 = new QDockWidget(tr("Economic Losses"), this);
//    chartsDock2->setObjectName("Economic Losses");
//    chartsDock2->setContentsMargins(5,5,5,5);

//    chartsDock3 = new QDockWidget(tr("Relative Freq. Losses"), this);
//    chartsDock3->setObjectName("Relative Freq. Losses");
//    chartsDock3->setContentsMargins(5,5,5,5);

//    this->addDockWidget(Qt::RightDockWidgetArea,chartsDock1);

//    this->tabifyDockWidget(chartsDock1,chartsDock2);
//    this->tabifyDockWidget(chartsDock1,chartsDock3);

//    chartsDock1->setFocus();

//    // Create the table that will show the Component information
//    tableWidget = new QWidget(this);

//    auto tableWidgetLayout = new QVBoxLayout(tableWidget);

//    pelicunResultsTableWidget = new QTableWidget(this);
//    pelicunResultsTableWidget->verticalHeader()->setVisible(false);
//    pelicunResultsTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

//    pelicunResultsTableWidget->setSizeAdjustPolicy(QAbstractScrollArea::SizeAdjustPolicy::AdjustToContents);
//    pelicunResultsTableWidget->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum);

//    pelicunResultsTableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
//    pelicunResultsTableWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

//    pelicunResultsTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

//    pelicunResultsTableWidget->setItemDelegate(new DoubleDelegate(this,3));

//    // Combo box to select how to sort the table
//    QHBoxLayout *comboLayout = new QHBoxLayout();

//    QStringList comboBoxHeadings = {"Asset ID","Repair Cost","Repair Time","Replacement Probability","Fatalities","Loss Ratio"};
//    sortComboBox = new QComboBox();
//    sortComboBox->insertItems(0,comboBoxHeadings);
//    sortComboBox->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);

//    connect(sortComboBox,QOverload<int>::of(&QComboBox::currentIndexChanged),this, &Pelicun3PostProcessor::sortTable);

//    auto comboBoxLabel = new QLabel("Sorting Filter:", this);

//    comboLayout->addWidget(comboBoxLabel);
//    comboLayout->addWidget(sortComboBox);
//    comboLayout->addStretch(0);

//    tableWidgetLayout->addLayout(comboLayout);
//    tableWidgetLayout->addWidget(pelicunResultsTableWidget);
//    tableWidgetLayout->addStretch(0);

//    //QDockWidget* tableDock = new QDockWidget("Detailed Results",this);
//    tableDock = new QDockWidget("Detailed Results",this);
//    tableDock->setObjectName("TableDock");
//    tableDock->setWidget(tableWidget);
//    tableDock->setMinimumWidth(475);
//    addDockWidget(Qt::RightDockWidgetArea, tableDock);

//    tableDock2 = new QDockWidget(tr("Site Responses"), this);
//    tableDock2->setObjectName("Site Responses");

//    // Get the map view widget
//    auto mapView = theVisualizationWidget->getMapViewWidget("ResultsWidget");
//    mapViewSubWidget = std::unique_ptr<SimCenterMapcanvasWidget>(mapView);


//    QDockWidget* mapViewDock = new QDockWidget("Regional Map",this);
//    mapViewDock->setObjectName("MapViewDock");
//    mapViewDock->setAllowedAreas(Qt::LeftDockWidgetArea);
//    mapViewDock->setWidget(mapViewSubWidget.get());
//    addDockWidget(Qt::LeftDockWidgetArea, mapViewDock);

//    if(viewMenu)
//    {
//        viewMenu->addAction(summaryDock->toggleViewAction());
//        viewMenu->addAction(chartsDock1->toggleViewAction());
//        viewMenu->addAction(chartsDock2->toggleViewAction());
//        viewMenu->addAction(chartsDock3->toggleViewAction());
//        viewMenu->addAction(tableDock->toggleViewAction());
//        viewMenu->addAction(mapViewDock->toggleViewAction());
//    }

//    uiState = this->saveState();

//    // The number of header rows in the Pelicun results file
//    numHeaderRows = 4;
//}




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


//void Pelicun3PostProcessor::processResultsSubset(const std::set<int>& selectedComponentIDs)
//{

//    if(selectedComponentIDs.empty())
//        return;

//    if(DVdata.size() < numHeaderRows)
//    {
//        QString msg = "No results to import!";
//        throw msg;
//    }

//    if(DVdata.at(numHeaderRows).isEmpty() || DVdata.last().isEmpty())
//    {
//        QString msg = "No values in the cells";
//        throw msg;
//    }

//    auto firstID = objectToInt(DVdata.at(numHeaderRows).at(0));

//    auto lastID = objectToInt(DVdata.last().at(0));

//    QVector<QStringList> DVsubset(&DVdata[0],&DVdata[numHeaderRows]);

//    for(auto&& id : selectedComponentIDs)
//    {
//        // Check that the ID falls within the bounds of the data
//        if(id<firstID || id>lastID)
//        {
//            QString msg = "ID " + QString::number(id) + " is out of bounds of the results";
//            throw msg;
//        }

//        auto found = false;
//        for(int i = numHeaderRows; i<DVdata.size(); ++i)
//        {
//            auto inputRow = DVdata.at(i);

//            auto buildingID = objectToInt(inputRow.at(0));

//            if(id == buildingID)
//            {
//                DVsubset << inputRow;
//                found = true;
//                break;
//            }
//        }

//        if(!found)
//        {
//            QString msg = "ID " + QString::number(id) + " cannot be found in the results";
//            throw msg;
//        }
//    }

//    this->processDVResults(DVsubset);
//}


int Pelicun3PostProcessor::assemblePDF(QImage screenShot)
{
//    // The printer
//    QPrinter printer(QPrinter::HighResolution);
//    printer.setOutputFormat(QPrinter::PdfFormat);
//    printer.setPaperSize(QPrinter::Letter);
//    printer.setPageMargins(25.4, 25.4, 25.4, 25.4, QPrinter::Millimeter);
//    printer.setFullPage(true);
//    qreal leftMargin, topMargin;
//    printer.getPageMargins(&leftMargin,&topMargin,nullptr,nullptr,QPrinter::Point);
//    printer.setOutputFileName(outputFilePath);

//    // Create a new document
//    QTextDocument* document = new QTextDocument();
//    QTextCursor cursor(document);
//    document->setDocumentMargin(25.4);
//    document->setDefaultFont(QFont("Helvetica"));

//    // Define font styles
//    QTextCharFormat normalFormat;
//    normalFormat.setFontPointSize(12);
//    normalFormat.setFontWeight(QFont::Normal);

//    QTextCharFormat titleFormat;
//    titleFormat.setFontWeight(QFont::Bold);
//    titleFormat.setFontCapitalization(QFont::AllUppercase);
//    titleFormat.setFontPointSize(normalFormat.fontPointSize() * 2.0);

//    QTextCharFormat captionFormat;
//    captionFormat.setFontWeight(QFont::Light);
//    captionFormat.setFontPointSize(normalFormat.fontPointSize());
//    captionFormat.setFontItalic(true);

//    QTextCharFormat disclaimerFormat;
//    disclaimerFormat.setFontWeight(QFont::Light);
//    disclaimerFormat.setFontPointSize(normalFormat.fontPointSize() / 1.5);

//    QTextCharFormat boldFormat;
//    boldFormat.setFontWeight(QFont::Bold);

//    QFontMetrics normMetrics(normalFormat.font());
//    auto lineSpacing = normMetrics.lineSpacing();

//    // Define alignment formats
//    QTextBlockFormat alignCenter;
//    alignCenter.setLineHeight(lineSpacing, QTextBlockFormat::LineDistanceHeight) ;
//    alignCenter.setAlignment(Qt::AlignCenter);

//    QTextBlockFormat alignLeft;
//    alignLeft.setAlignment(Qt::AlignLeft);
//    alignLeft.setLineHeight(lineSpacing, QTextBlockFormat::LineDistanceHeight) ;

//    cursor.movePosition(QTextCursor::Start);

//    cursor.insertBlock(alignCenter);

//    // Insert the simcenter logo at the top
//    QImage simCenterLogo(":resources/SimCenter@1x.png");
//    document->addResource(QTextDocument::ImageResource, QUrl("Logo"), simCenterLogo);
//    QTextImageFormat imageFormatSimCenterLogo;
//    imageFormatSimCenterLogo.setName("Logo");
//    imageFormatSimCenterLogo.setWidth(250);
//    imageFormatSimCenterLogo.setQuality(600);

//    cursor.insertImage(imageFormatSimCenterLogo);

//    cursor.insertText("\nRegional Resilience Determination (R2D) Tool\n",titleFormat);

//    cursor.insertText("Results Summary\n",boldFormat);

//    alignLeft.setLineHeight(1, QTextBlockFormat::SingleHeight) ;

//    cursor.setBlockFormat(alignLeft);

//    QString disclaimerText = "Disclaimer: The presented simulation results are not representative of any individual building’s response. To understand the response of any individual building, "
//                             "please consult with a professional structural engineer. The presented tool does not assert the known condition of the building. Just as it cannot be used to predict the negative outcome of an individual "
//                             "building, prediction of safety or an undamaged state is not assured for an individual building. Any opinions, findings, and conclusions or recommendations expressed in this material are "
//                             "those of the author(s) and do not necessarily reflect the views of the National Science Foundation.\n";
//    cursor.insertText(disclaimerText,disclaimerFormat);

//    alignLeft.setLineHeight(lineSpacing, QTextBlockFormat::LineDistanceHeight);

//    cursor.setBlockFormat(alignLeft);

//    cursor.insertText("\nEmploying Pelicun loss methodology to calculate seismic losses.\n",normalFormat);

//    QString currentDT = "Timestamp: " + QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") + "\n";
//    cursor.insertText(currentDT,normalFormat);

//    auto workflowApp = WorkflowAppR2D::getInstance();
//    auto analysisName = workflowApp->getGeneralInformationWidget()->getAnalysisName();

//    QString analysisNameLabel = "Analysis name: " + analysisName + "\n";
//    cursor.insertText(analysisNameLabel,normalFormat);

//    cursor.insertText("Estimated Regional Totals\n",boldFormat);

//    QTextTableFormat tableFormat;
//    tableFormat.setPadding(5.0);
//    tableFormat.setCellPadding(5.0);
//    tableFormat.setBorder(0.0);
//    tableFormat.setAlignment(Qt::AlignVCenter);

//    tableFormat.setBackground(QColor("#f0f0f0"));
//    QVector<QTextLength> constraints;
//    constraints << QTextLength(QTextLength::PercentageLength, 25);
//    constraints << QTextLength(QTextLength::PercentageLength, 25);
//    constraints << QTextLength(QTextLength::PercentageLength, 25);
//    constraints << QTextLength(QTextLength::PercentageLength, 25);
//    tableFormat.setColumnWidthConstraints(constraints);

//    // rows, columns, tableFormat
//    QTextTable *table = cursor.insertTable(3, 4, tableFormat);

//    {
//        QTextTableCell cell = table->cellAt(0, 0);
//        cell.setFormat(normalFormat);
//        QTextCursor cellCursor = cell.firstCursorPosition();
//        cellCursor.insertText(totalCasLabel->text());
//    }

//    {
//        QTextTableCell cell = table->cellAt(0, 1);
//        cell.setFormat(normalFormat);
//        QTextCursor cellCursor = cell.firstCursorPosition();
//        cellCursor.insertText(totalCasValueLabel->text());
//    }

//    {
//        QTextTableCell cell = table->cellAt(0, 2);
//        cell.setFormat(normalFormat);
//        QTextCursor cellCursor = cell.firstCursorPosition();
//        cellCursor.insertText(totalFatalitiesLabel->text());
//    }

//    {
//        QTextTableCell cell = table->cellAt(0, 3);
//        cell.setFormat(normalFormat);
//        QTextCursor cellCursor = cell.firstCursorPosition();
//        cellCursor.insertText(totalFatalitiesValueLabel->text());
//    }

//    {
//        QTextTableCell cell = table->cellAt(1, 0);
//        cell.setFormat(normalFormat);
//        QTextCursor cellCursor = cell.firstCursorPosition();
//        cellCursor.insertText(totalLossLabel->text());
//    }

//    {
//        QTextTableCell cell = table->cellAt(1, 1);
//        cell.setFormat(normalFormat);
//        QTextCursor cellCursor = cell.firstCursorPosition();
//        cellCursor.insertText(totalLossValueLabel->text());
//    }

//    {
//        QTextTableCell cell = table->cellAt(1, 2);
//        cell.setFormat(normalFormat);
//        QTextCursor cellCursor = cell.firstCursorPosition();
//        cellCursor.insertText(totalRepairTimeLabel->text());
//    }

//    {
//        QTextTableCell cell = table->cellAt(1, 3);
//        cell.setFormat(normalFormat);
//        QTextCursor cellCursor = cell.firstCursorPosition();
//        cellCursor.insertText(totalRepairTimeValueLabel->text());
//    }

//    {
//        QTextTableCell cell = table->cellAt(2, 0);
//        cell.setFormat(normalFormat);
//        QTextCursor cellCursor = cell.firstCursorPosition();
//        cellCursor.insertText(structLossLabel->text());
//    }

//    {
//        QTextTableCell cell = table->cellAt(2, 1);
//        cell.setFormat(normalFormat);
//        QTextCursor cellCursor = cell.firstCursorPosition();
//        cellCursor.insertText(structLossValueLabel->text());
//    }

//    {
//        QTextTableCell cell = table->cellAt(2, 2);
//        cell.setFormat(normalFormat);
//        QTextCursor cellCursor = cell.firstCursorPosition();
//        cellCursor.insertText(nonStructLossLabel->text());
//    }

//    {
//        QTextTableCell cell = table->cellAt(2, 3);
//        cell.setFormat(normalFormat);
//        QTextCursor cellCursor = cell.firstCursorPosition();
//        cellCursor.insertText(nonStructLossValueLabel->text());
//    }

//    cursor.movePosition( QTextCursor::End );

//    cursor.insertText("\n\n",normalFormat);

//    // Ratio of the page width that is printable
//    auto useablePageWidth = printer.pageRect(QPrinter::Point).width()-(1.5*leftMargin);

//    QRect viewPortRect(0, mapViewMainWidget->height() - mapViewSubWidget->height(), mapViewSubWidget->width(), mapViewSubWidget->height());
//    QImage cropped = screenShot.copy(viewPortRect);
//    document->addResource(QTextDocument::ImageResource,QUrl("Figure1"),cropped);
//    QTextImageFormat imageFormatFig1;
//    imageFormatFig1.setName("Figure1");
//    imageFormatFig1.setQuality(600);
//    imageFormatFig1.setWidth(useablePageWidth);

//    cursor.setBlockFormat(alignCenter);

//    cursor.insertImage(imageFormatFig1);

//    cursor.insertText("Regional map visualization.\n",captionFormat);

//    auto origSize = casualtiesChartView->size();

//    casualtiesChartView->resize(QSize(640,480));

//    casualtiesChartView->setVisible(true);
//    auto rectFig2 = casualtiesChartView->viewport()->rect();
//    QPixmap pixmapFig2(rectFig2.size());
//    QPainter painterFig2(&pixmapFig2);
//    painterFig2.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
//    casualtiesChartView->render(&painterFig2, pixmapFig2.rect(), rectFig2);
//    auto figure2 = pixmapFig2.toImage();

//    casualtiesChartView->resize(origSize);

//    QTextImageFormat imageFormatFig2;
//    imageFormatFig2.setName("Figure2");
//    imageFormatFig2.setQuality(600);
//    imageFormatFig2.setWidth(400);

//    document->addResource(QTextDocument::ImageResource,QUrl("Figure2"),figure2);

//    cursor.insertImage(imageFormatFig2,QTextFrameFormat::InFlow);

//    cursor.insertText("\nEstimated casualties.\n",captionFormat);

//    auto origSize2 = lossesChartView->size();
//    lossesChartView->resize(QSize(640,480));
//    auto rectFig3 = lossesChartView->viewport()->rect();
//    QPixmap pixmapFig3(rectFig3.size());
//    QPainter painterFig3(&pixmapFig3);
//    painterFig3.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
//    lossesChartView->render(&painterFig3, pixmapFig3.rect(), rectFig3);
//    auto figure3 = pixmapFig3.toImage();
//    lossesChartView->resize(origSize2);

//    QTextImageFormat imageFormatFig3;
//    imageFormatFig3.setName("Figure3");
//    imageFormatFig3.setQuality(600);
//    imageFormatFig3.setWidth(400);

//    document->addResource(QTextDocument::ImageResource,QUrl("Figure3"),figure3);
//    cursor.insertImage(imageFormatFig3,QTextFrameFormat::InFlow);

//    cursor.insertText("\nEstimated economic losses.\n",captionFormat);

//    if(lossesRFDiagram->property("ToPlot").toBool())
//    {
//        auto origSize3 = lossesRFDiagram->size();
//        lossesRFDiagram->resize(QSize(640,480));
//        auto rectFig4 = lossesRFDiagram->viewport()->rect();
//        QPixmap pixmapFig4(rectFig4.size());
//        QPainter painterFig4(&pixmapFig4);
//        painterFig4.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
//        lossesRFDiagram->render(&painterFig4, pixmapFig4.rect(), rectFig4);
//        auto figure4 = pixmapFig4.toImage();
//        lossesRFDiagram->resize(origSize3);

//        QTextImageFormat imageFormatFig4;
//        imageFormatFig4.setName("Figure4");
//        imageFormatFig4.setQuality(600);
//        imageFormatFig4.setWidth(400);

//        document->addResource(QTextDocument::ImageResource,QUrl("Figure4"),figure4);
//        cursor.insertImage(imageFormatFig4,QTextFrameFormat::InFlow);

//        cursor.insertText("\nRelative frequency diagram of expected losses.\n",captionFormat);
//    }

//    cursor.insertText("Individual Asset Results - Sorted According to the " + sortComboBox->currentText() + "\n",boldFormat);

//    TablePrinter prettyTablePrinter;
//    prettyTablePrinter.printToTable(&cursor, pelicunResultsTableWidget,"Asset Results");

//    if(!IMdata.isEmpty())
//    {
//        cursor.insertText("Individual Site Responses\n",boldFormat);
//        TablePrinter prettyTablePrinter;
//        prettyTablePrinter.printToTable(&cursor, siteResponseTableWidget,"Site Response Results");
//    }

//    document->print(&printer);

    return 0;
}


void Pelicun3PostProcessor::sortTable(int index)
{
    if(index == 0){
        for(int i = 0; i < ResultsTableWidgetList.count(); i++){
            ResultsTableWidgetList.at(i)->sortByColumn(index, Qt::AscendingOrder);
        }
    }
    else{
        for(int i = 0; i < ResultsTableWidgetList.count(); i++){
            ResultsTableWidgetList.at(i)->sortByColumn(index, Qt::DescendingOrder);
        }
    }
}


void Pelicun3PostProcessor::restoreUI(void)
{
//    this->restoreState(uiState);
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
    for (int i = 0; i < dockList.count(); i++){
        QDockWidget* parentWidget = dockList.at(i);
        qDeleteAll(parentWidget->findChildren<QWidget*>("", Qt::FindDirectChildrenOnly));
        delete dockList.at(i);
    }
    dockList.clear();



//    outputFilePath.clear();

//    totalCasValueLabel->clear();
//    totalLossValueLabel->clear();
//    totalRepairTimeValueLabel->clear();
//    totalFatalitiesValueLabel->clear();
//    structLossValueLabel->clear();
//    nonStructLossValueLabel->clear();

//    pelicunResultsTableWidget->clear();

//    sortComboBox->setCurrentIndex(0);

    mapViewSubWidget->clear();
}

