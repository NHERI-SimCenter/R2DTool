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
#include "GeneralInformationWidget.h"
#include "MainWindowWorkflowApp.h"
#include "PelicunPostProcessor.h"
#include "REmpiricalProbabilityDistribution.h"
#include "TablePrinter.h"
#include "TableNumberItem.h"
#include "VisualizationWidget.h"
#include "WorkflowAppR2D.h"
#include "Utils/PythonProgressDialog.h"

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

#ifdef ARC_GIS
#include "Basemap.h"
#include "FeatureTable.h"
#include "FeatureCollectionLayer.h"
#include "Map.h"
#include "MapGraphicsView.h"
#include "ArcGISVisualizationWidget.h"
#include "SimCenterMapGraphicsView.h"
#endif


#ifdef Q_GIS
#include "QGISVisualizationWidget.h"

#include <qgsmapcanvas.h>
#endif

// Test to remove
#include <chrono>
using namespace std::chrono;

using namespace QtCharts;

PelicunPostProcessor::PelicunPostProcessor(QWidget *parent, VisualizationWidget* visWidget) : QMainWindow(parent), theVisualizationWidget(visWidget)
{
    casualtiesChart = nullptr;
    RFDiagChart = nullptr;
    Losseschart = nullptr;
    viewMenu = nullptr;

    // Create a view menu for the dockable windows
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
        viewMenu = resultsMenu->addMenu(tr("&View"));
        viewMenu->addAction(tr("&Restore"), this, &PelicunPostProcessor::restoreUI);
    }
    else
    {
        PythonProgressDialog::getInstance()->appendErrorMessage("Could not find the results menu bar in PelicunPostProcessor::");
        return;
    }

    connect(theVisualizationWidget,&VisualizationWidget::emitScreenshot,this,&PelicunPostProcessor::assemblePDF);

    // Summary group box
    QWidget* totalsWidget = new QWidget(this);
    totalsWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
    QGridLayout* totalsLayout = new QGridLayout(totalsWidget);

    totalCasLabel = new QLabel("Casualties:", this);
    totalFatalitiesLabel = new QLabel("Fatalities:", this);
    totalLossLabel = new QLabel("Losses:", this);
    totalRepairTimeLabel = new QLabel("Repair Time [days]:", this);
    structLossLabel = new QLabel("Structural Losses:", this);
    nonStructLossLabel = new QLabel("Non-structural Losses:", this);

    totalCasValueLabel = new QLabel("", this);
    totalLossValueLabel = new QLabel("", this);
    totalRepairTimeValueLabel = new QLabel("", this);
    totalFatalitiesValueLabel = new QLabel("", this);
    structLossValueLabel = new QLabel("", this);
    nonStructLossValueLabel = new QLabel("", this);

    totalsLayout->addWidget(totalCasLabel,0,0);
    totalsLayout->addWidget(totalCasValueLabel,0,1,1,1,Qt::AlignLeft);
    totalsLayout->addWidget(totalFatalitiesLabel,0,2);
    totalsLayout->addWidget(totalFatalitiesValueLabel,0,3,1,1,Qt::AlignLeft);
    totalsLayout->addWidget(totalLossLabel,1,0);
    totalsLayout->addWidget(totalLossValueLabel,1,1,1,1,Qt::AlignLeft);
    totalsLayout->addWidget(totalRepairTimeLabel,1,2);
    totalsLayout->addWidget(totalRepairTimeValueLabel,1,3,1,1,Qt::AlignLeft);
    totalsLayout->addWidget(structLossLabel,2,0);
    totalsLayout->addWidget(structLossValueLabel,2,1,1,1,Qt::AlignLeft);
    totalsLayout->addWidget(nonStructLossLabel,2,2);
    totalsLayout->addWidget(nonStructLossValueLabel,2,3,1,1,Qt::AlignLeft);

    QDockWidget* summaryDock = new QDockWidget("Estimated Regional Totals",this);
    summaryDock->setObjectName("SummaryDock");
    summaryDock->setWidget(totalsWidget);
    addDockWidget(Qt::RightDockWidgetArea, summaryDock);

    // Charts
    chartsDock1 = new QDockWidget(tr("Casualties"), this);
    chartsDock1->setObjectName("Casualties");
    chartsDock1->setContentsMargins(5,5,5,5);

    chartsDock2 = new QDockWidget(tr("Economic Losses"), this);
    chartsDock2->setObjectName("Economic Losses");
    chartsDock2->setContentsMargins(5,5,5,5);

    chartsDock3 = new QDockWidget(tr("Relative Freq. Losses"), this);
    chartsDock3->setObjectName("Relative Freq. Losses");
    chartsDock3->setContentsMargins(5,5,5,5);

    this->addDockWidget(Qt::RightDockWidgetArea,chartsDock1);

    this->tabifyDockWidget(chartsDock1,chartsDock2);
    this->tabifyDockWidget(chartsDock1,chartsDock3);

    chartsDock1->setFocus();

    // Create the table that will show the Component information
    tableWidget = new QWidget(this);

    auto tableWidgetLayout = new QVBoxLayout(tableWidget);

    pelicunResultsTableWidget = new QTableWidget(this);
    pelicunResultsTableWidget->verticalHeader()->setVisible(false);
    pelicunResultsTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    pelicunResultsTableWidget->setSizeAdjustPolicy(QAbstractScrollArea::SizeAdjustPolicy::AdjustToContents);
    pelicunResultsTableWidget->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum);

    pelicunResultsTableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    pelicunResultsTableWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    pelicunResultsTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    pelicunResultsTableWidget->setItemDelegate(new DoubleDelegate(this,3));

    // Combo box to select how to sort the table
    QHBoxLayout *comboLayout = new QHBoxLayout();

    QStringList comboBoxHeadings = {"Asset ID","Repair Cost","Repair Time","Replacement Probability","Fatalities","Loss Ratio"};
    sortComboBox = new QComboBox();
    sortComboBox->insertItems(0,comboBoxHeadings);
    sortComboBox->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);

    connect(sortComboBox,QOverload<int>::of(&QComboBox::currentIndexChanged),this, &PelicunPostProcessor::sortTable);

    auto comboBoxLabel = new QLabel("Sorting Filter:", this);

    comboLayout->addWidget(comboBoxLabel);
    comboLayout->addWidget(sortComboBox);
    comboLayout->addStretch(0);

    tableWidgetLayout->addLayout(comboLayout);
    tableWidgetLayout->addWidget(pelicunResultsTableWidget);
    tableWidgetLayout->addStretch(0);

    QDockWidget* tableDock = new QDockWidget("Detailed Results",this);
    tableDock->setObjectName("TableDock");
    tableDock->setWidget(tableWidget);
    tableDock->setMinimumWidth(475);
    addDockWidget(Qt::RightDockWidgetArea, tableDock);

#ifdef Q_GIS
    // Get the map view widget
    auto mapView = theVisualizationWidget->getMapViewWidget("ResultsWidget");
    mapViewSubWidget = std::unique_ptr<SimCenterMapcanvasWidget>(mapView);

    // Enable the selection tool
    mapViewSubWidget->enableSelectionTool();
#endif

#ifdef ARC_GIS
    mapViewSubWidget = std::make_unique<EmbeddedMapViewWidget>(mapViewMainWidget);

    // Popup stuff Once map is set, connect to MapQuickView mouse clicked signal
    connect(mapViewSubWidget.get(), &EmbeddedMapViewWidget::mouseClick, theVisualizationWidget, &VisualizationWidget::onMouseClickedGlobal);
#endif

    QDockWidget* mapViewDock = new QDockWidget("Regional Map",this);
    mapViewDock->setObjectName("MapViewDock");
    mapViewDock->setAllowedAreas(Qt::LeftDockWidgetArea);
    mapViewDock->setWidget(mapViewSubWidget.get());
    addDockWidget(Qt::LeftDockWidgetArea, mapViewDock);

    if(viewMenu)
    {
        viewMenu->addAction(summaryDock->toggleViewAction());
        viewMenu->addAction(chartsDock1->toggleViewAction());
        viewMenu->addAction(chartsDock2->toggleViewAction());
        viewMenu->addAction(chartsDock3->toggleViewAction());
        viewMenu->addAction(tableDock->toggleViewAction());
        viewMenu->addAction(mapViewDock->toggleViewAction());
    }

    uiState = this->saveState();

    // The number of header rows in the Pelicun results file
    numHeaderRows = 4;
}


void PelicunPostProcessor::importResults(const QString& pathToResults)
{
    qDebug() << "PelicunPostProcessor: " << pathToResults;

    // Remove old csv files in the output pathToResults
    QDir resultsDir(pathToResults);

    const QFileInfo existingFilesInfo(pathToResults);

    // Get the existing files in the folder to see if we already have the record

    QStringList acceptableFileExtensions = {"*.csv"};
    QStringList existingCSVFiles = resultsDir.entryList(acceptableFileExtensions, QDir::Files);

    QString errMsg;

    if(existingCSVFiles.empty())
    {
        QStringList acceptableFileExtensions = {"*.*"};
        QStringList existingFiles = existingFilesInfo.dir().entryList(acceptableFileExtensions, QDir::Files);
        qDebug() << "FILES IN FOLDER: " << existingFiles;
        errMsg = "The results folder is empty";
        throw errMsg;
    }

    QString DMResultsSheet;
    QString DVResultsSheet;
    QString EDPreultsSheet;

    for(auto&& it : existingCSVFiles)
    {
        if(it.startsWith("DM_"))
            DMResultsSheet = it;
        else if(it.startsWith("DV_"))
            DVResultsSheet = it;
        else if(it.startsWith("EDP_"))
            EDPreultsSheet = it;
    }

    CSVReaderWriter csvTool;

    DMdata = csvTool.parseCSVFile(pathToResults + QDir::separator() + DMResultsSheet,errMsg);
    if(!errMsg.isEmpty())
        throw errMsg;

    DVdata = csvTool.parseCSVFile(pathToResults + QDir::separator() + DVResultsSheet,errMsg);
    if(!errMsg.isEmpty())
        throw errMsg;

    EDPdata = csvTool.parseCSVFile(pathToResults + QDir::separator() + EDPreultsSheet,errMsg);
    if(!errMsg.isEmpty())
        throw errMsg;

    if(!DVdata.empty())
        this->processDVResults(DVdata);
    else
    {
        errMsg = "The DV results are empty";
        throw errMsg;
    }

}


void PelicunPostProcessor::showEvent(QShowEvent *e)
{
    auto mainCanvas = mapViewSubWidget->getMainCanvas();

    auto mainExtent = mainCanvas->extent();

    mapViewSubWidget->mapCanvas()->zoomToFeatureExtent(mainExtent);
    QMainWindow::showEvent(e);
}




int PelicunPostProcessor::processDVResults(const QVector<QStringList>& DVResults)
{
    if(DVResults.size() < numHeaderRows)
    {
        QString msg = "No results to import!";
        throw msg;
    }

    auto numHeaderColumns = DVResults.at(0).size();

    QStringList headerStrings;

    for(int i = 0; i<numHeaderColumns; ++i)
    {
        QString headerStr = DVResults.at(0).at(i) +"-"+ DVResults.at(1).at(i) +"-"+ DVResults.at(2).at(i) +"-"+ DVResults.at(3).at(i);

        headerStrings.append(headerStr);
    }


    auto indexRCagg = headerStrings.indexOf("Repair Cost-aggregate--mean");
    auto indexRepairImpracProb = headerStrings.indexOf("Repair Impractical-probability--");

    if(indexRCagg == -1 || indexRepairImpracProb==-1)
    {
        QString msg = "Could not find the required header keys in the Pelicun DV results file.";
        throw msg;
    }

    // Decipher the results file

    // Structural - seismic
    auto indexSRCagg = headerStrings.indexOf("Repair Cost-S-aggregate-mean");
    auto indexNSRCagg = headerStrings.indexOf("Repair Cost-NS-aggregate-mean");

    auto indexSRC1_1 = headerStrings.indexOf("Repair Cost-S-1_1-mean");

    // Non-structural - seismic
    // auto indexNSRC1_1 = headerStrings.indexOf("Repair Cost-NS-1_1-mean");

    // Non-structural - acceleration sensitive - seismic
    auto indexNSARC1_1 = headerStrings.indexOf("Repair Cost-NSA-1_1-mean");

    // Non-structural - drift sensitive - seismic
    auto indexNSDRC1_1 = headerStrings.indexOf("Repair Cost-NSD-1_1-mean");

    // Repair times
    auto indexRepairTime = headerStrings.indexOf("Repair Time--aggregate-mean");

    // Injuries
    auto indexInjuriesSev1 = headerStrings.indexOf("Injuries-sev1-aggregate-mean");

    //    // Wind repair cost
    //    auto indexWindRCagg = headerStrings.indexOf("Repair Cost-Wind-aggregate");
    //    auto indexWindRC1_1 = headerStrings.indexOf("Repair Cost-Wind-1_1-mean");

    //    // Flood repair cost
    //    auto indexFloodRCagg = headerStrings.indexOf("Repair Cost-Flood-aggregate");
    //    auto indexFloodRC1_1 = headerStrings.indexOf("Repair Cost-Flood-1_1-mean");

    QStringList tableHeadings = {"Asset ID","Repair\nCost","Repair\nTime","Replacement\nProbability","Fatalities","Loss\nRatio"};

    pelicunResultsTableWidget->setColumnCount(tableHeadings.size());
    pelicunResultsTableWidget->setHorizontalHeaderLabels(tableHeadings);
    pelicunResultsTableWidget->setRowCount(DVResults.size()-numHeaderRows);

    auto cumulativeSagg = 0.0;
    auto cumulativeNSagg = 0.0;

    auto cumulativeStructDS1 = 0.0;
    auto cumulativeStructDS2 = 0.0;
    auto cumulativeStructDS3 = 0.0;
    auto cumulativeStructDS4 = 0.0;

    auto cumulativeNSAccDS1 = 0.0;
    auto cumulativeNSAccDS2 = 0.0;
    auto cumulativeNSAccDS3 = 0.0;
    auto cumulativeNSAccDS4 = 0.0;

    auto cumulativeNSDriftDS1 = 0.0;
    auto cumulativeNSDriftDS2 = 0.0;
    auto cumulativeNSDriftDS3 = 0.0;
    auto cumulativeNSDriftDS4 = 0.0;

    auto cumulativeinjSevLvl1 = 0.0;
    auto cumulativeinjSevLvl2 = 0.0;
    auto cumulativeinjSevLvl3 = 0.0;
    auto cumulativeinjSevLvl4 = 0.0;

    auto cumulativeRepairTime = 0.0;
    auto cumulativeRepairCost = 0.0;

    REmpiricalProbabilityDistribution theProbDist;

    // Get the buildings database
    auto theBuildingDB = ComponentDatabaseManager::getInstance()->getBuildingComponentDb();

    if(theBuildingDB == nullptr)
    {
        QString msg = "Error getting the building database from the input widget!";
        throw msg;
    }

    if(theBuildingDB->isEmpty())
    {
        QString msg = "Building database is empty";
        throw msg;
    }

    QVector<QVariant> attributes(DVResults.size()-numHeaderRows);

    // 4 rows of headers in the results file
    for(int i = numHeaderRows, count = 0; i<DVResults.size(); ++i, ++count)
    {
        auto inputRow = DVResults.at(i);

        auto buildingID = objectToInt(inputRow.at(0));

        // Defaults to 1.0 if no replacement cost is given, i.e., it assumes the repair cost is the loss ratio
        auto replacementCostVar = theBuildingDB->getAttributeValue(buildingID,"ReplacementCost",QVariant(1.0));

        auto replacementCost = replacementCostVar.toDouble();

        // This assumes that the output from pelicun will not change
        auto IDStr = inputRow.at(0);                                // ID
        auto totalRepairCost = inputRow.at(indexRCagg);             // Aggregate repair cost (mean)
        auto replaceMentProb = inputRow.at(indexRepairImpracProb);  // Replacement probability, i.e., repair impractical probability

        auto repairTime = 0.0;

        auto injSevLvl1 = 0.0;
        auto injSevLvl2 = 0.0;
        auto injSevLvl3 = 0.0;
        auto fatalities = 0.0; // injSevLvl4

        // Aggregate repair time (mean)
        if(indexRepairTime != -1)
            repairTime = objectToDouble(inputRow.at(indexRepairTime));

        cumulativeRepairTime += repairTime;

        if(indexSRC1_1 != -1)
        {
            auto StructDS1 = objectToDouble(inputRow.at(indexSRC1_1));    // Structural losses damage state 1 (mean)
            auto StructDS2 = objectToDouble(inputRow.at(indexSRC1_1+1));  // Structural losses damage state 2 (mean)
            auto StructDS3 = objectToDouble(inputRow.at(indexSRC1_1+2));  // Structural losses damage state 3 (mean)
            auto StructDS4 = objectToDouble(inputRow.at(indexSRC1_1+3));  // Structural losses damage state 4 (mean)
            StructDS4 += objectToDouble(inputRow.at(indexSRC1_1+4));      // Structural losses damage state 4_2 (mean)

            cumulativeStructDS1 += StructDS1;
            cumulativeStructDS2 += StructDS2;
            cumulativeStructDS3 += StructDS3;
            cumulativeStructDS4 += StructDS4;
        }

        if(indexNSARC1_1 != -1)
        {
            auto NSAccDS1 = objectToDouble(inputRow.at(indexNSARC1_1));    // Non-structural acceleration sensitive losses damage state 1 (mean)
            auto NSAccDS2 = objectToDouble(inputRow.at(indexNSARC1_1+1));  // Non-structural acceleration sensitive losses damage state 2 (mean)
            auto NSAccDS3 = objectToDouble(inputRow.at(indexNSARC1_1+2));  // Non-structural acceleration sensitive losses damage state 3 (mean)
            auto NSAccDS4 = objectToDouble(inputRow.at(indexNSARC1_1+3));  // Non-structural acceleration sensitive losses damage state 4 (mean)

            cumulativeNSAccDS1 += NSAccDS1;
            cumulativeNSAccDS2 += NSAccDS2;
            cumulativeNSAccDS3 += NSAccDS3;
            cumulativeNSAccDS4 += NSAccDS4;
        }

        if(indexNSDRC1_1 != -1)
        {
            auto NSDriftDS1 = objectToDouble(inputRow.at(24));  // Non-structural drift sensitive losses damage state 1 (mean)
            auto NSDriftDS2 = objectToDouble(inputRow.at(25));  // Non-structural drift sensitive losses damage state 2 (mean)
            auto NSDriftDS3 = objectToDouble(inputRow.at(26));  // Non-structural drift sensitive losses damage state 3 (mean)
            auto NSDriftDS4 = objectToDouble(inputRow.at(27));  // Non-structural drift sensitive losses damage state 4 (mean)

            cumulativeNSDriftDS1 += NSDriftDS1;
            cumulativeNSDriftDS2 += NSDriftDS2;
            cumulativeNSDriftDS3 += NSDriftDS3;
            cumulativeNSDriftDS4 += NSDriftDS4;
        }

        if(indexInjuriesSev1 != -1)
        {
            injSevLvl1 = objectToDouble(inputRow.at(indexInjuriesSev1));    // Injuries severity level 1 (mean)
            injSevLvl2 = objectToDouble(inputRow.at(indexInjuriesSev1+1));  // Injuries severity level 2 (mean)
            injSevLvl3 = objectToDouble(inputRow.at(indexInjuriesSev1+2));  // Injuries severity level 3 (mean)
            fatalities = objectToDouble(inputRow.at(indexInjuriesSev1+3));  // Injuries severity level 4 (mean)

            cumulativeinjSevLvl1 += injSevLvl1;
            cumulativeinjSevLvl2 += injSevLvl2;
            cumulativeinjSevLvl3 += injSevLvl3;
            cumulativeinjSevLvl4 += fatalities;
        }

        if(indexSRCagg != -1)
            cumulativeSagg += objectToDouble(inputRow.at(indexSRCagg));

        if(indexNSRCagg != -1)
            cumulativeNSagg += objectToDouble(inputRow.at(indexNSRCagg));

        auto repairCost = objectToDouble(totalRepairCost);
        auto lossRatio = repairCost/replacementCost;

        cumulativeRepairCost += repairCost;

        theProbDist.addSample(repairCost);

        auto IDItem = new TableNumberItem(IDStr);
        auto RepCostItem = new TableNumberItem(totalRepairCost);
        auto RepProbItem = new TableNumberItem(replaceMentProb);
        auto RepairTimeItem = new TableNumberItem(QString::number(repairTime));
        auto fatalitiesItem = new TableNumberItem(QString::number(fatalities));
        auto lossRatioItem = new TableNumberItem(QString::number(lossRatio));

        pelicunResultsTableWidget->setItem(count,0, IDItem);
        pelicunResultsTableWidget->setItem(count,1, RepCostItem);
        pelicunResultsTableWidget->setItem(count,2, RepairTimeItem);
        pelicunResultsTableWidget->setItem(count,3, RepProbItem);
        pelicunResultsTableWidget->setItem(count,4, fatalitiesItem);
        pelicunResultsTableWidget->setItem(count,5, lossRatioItem);

        attributes[count] = lossRatio;
    }

    // Test to remove
     auto start = high_resolution_clock::now();

    // Starting editing
    theBuildingDB->startEditing();

    auto res = theBuildingDB->updateComponentAttributes("LossRatio",attributes);
    if(!res)
    {
        QString msg = "Error updating component attribute: Loss Ratio";
        throw msg;
    }

    // Commit the changes
    theBuildingDB->commitChanges();

    // Test to remove
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(stop - start);
        PythonProgressDialog::getInstance()->appendText("Done processing results "+QString::number(duration.count()));


    QGISVisualizationWidget* QGISVisWidget = static_cast<QGISVisualizationWidget*>(theVisualizationWidget);

    // Apply the default renderer
    QGISVisWidget->createPrettyGraduatedRenderer("LossRatio",Qt::yellow,Qt::red,5,theBuildingDB->getSelectedLayer());

    //  CASUALTIES
    QBarSet *casualtiesSet = new QBarSet("Casualties");

    *casualtiesSet << cumulativeinjSevLvl1 << cumulativeinjSevLvl2 << cumulativeinjSevLvl3 << cumulativeinjSevLvl4;

    this->createCasualtiesChart(casualtiesSet);

    totalCasValueLabel->setText(QString::number(casualtiesSet->sum(),'f',2));
    totalFatalitiesValueLabel->setText(QString::number(casualtiesSet->at(3),'f',2));

    //  LOSSES
    QBarSet *structLossSet = new QBarSet("Structural");
    QBarSet *NSAccLossSet = new QBarSet("Non-structural Acc.");
    QBarSet *NSDriftLossSet = new QBarSet("Non-structural Drift");

    *structLossSet << cumulativeStructDS1 << cumulativeStructDS2 << cumulativeStructDS3 << cumulativeStructDS4 ;
    *NSAccLossSet << cumulativeNSAccDS1 << cumulativeNSAccDS2 << cumulativeNSAccDS3 << cumulativeNSAccDS4 ;
    *NSDriftLossSet << cumulativeNSDriftDS1 << cumulativeNSDriftDS2 << cumulativeNSDriftDS3 << cumulativeNSDriftDS4;

    this->createLossesChart(structLossSet, NSAccLossSet, NSDriftLossSet);

    totalLossValueLabel->setText(QString::number(cumulativeRepairCost,'g',3));

    structLossValueLabel->setText(QString::number(cumulativeSagg,'g',3));
    nonStructLossValueLabel->setText(QString::number(cumulativeNSagg,'g',3));

    // Repair time
    totalRepairTimeValueLabel->setText(QString::number(cumulativeRepairTime,'g',3));

    this->createHistogramChart(&theProbDist);

    if(theProbDist.getNumberSamples() < 2)
        lossesRFDiagram->setProperty("ToPlot",false);
    else
        lossesRFDiagram->setProperty("ToPlot",true);

    // Set a default size to the charts
    chartsDock1->setWidget(casualtiesChartView);
    chartsDock2->setWidget(lossesChartView);
    chartsDock3->setWidget(lossesRFDiagram);

    return 0;
}


void PelicunPostProcessor::setIsVisible(const bool value)
{
    if(viewMenu)
        viewMenu->menuAction()->setVisible(value);
}


int PelicunPostProcessor::createCasualtiesChart(QBarSet *casualtiesSet)
{
    QBarSeries *series = new QBarSeries();
    series->append(casualtiesSet);
    series->setBarWidth(1.0);
    series->setLabelsVisible(true);
    series->setLabelsPosition(QAbstractBarSeries::LabelsCenter);

    if(casualtiesChart == nullptr)
    {
        casualtiesChart = new QChart();
        casualtiesChart->setDropShadowEnabled(false);
        casualtiesChart->setMargins(QMargins(5,5,5,5));
        casualtiesChart->layout()->setContentsMargins(0, 0, 0, 0);
        casualtiesChart->legend()->setVisible(false);

        casualtiesChartView = new QChartView(casualtiesChart);
        casualtiesChartView->setRenderHint(QPainter::Antialiasing);
        casualtiesChartView->setContentsMargins(0,0,0,0);
        casualtiesChartView->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    }
    else
    {
        casualtiesChart->removeAllSeries();

        auto axes = casualtiesChart->axes();

        for(auto&& it : axes)
            casualtiesChart->removeAxis(it);
    }

    casualtiesChart->addSeries(series);

    QStringList categories;
    categories << "Level 1" << "Level 2" << "Level 3" << "Level 4";
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    casualtiesChart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    return 0;
}


int PelicunPostProcessor::createHistogramChart(REmpiricalProbabilityDistribution* probDist)
{

    QVector<double> xValues;
    QVector<double> yValues;

    // Handle the special case where there is only one sample
    if(probDist->getNumberSamples() < 2)
    {
        xValues = probDist->getValues();
        yValues.push_back(1.0);

    }
    else
    {
        xValues = probDist->getHistogramTicks();
        yValues = probDist->getRelativeFrequencyDiagram();
    }

    QLineSeries *series = new QLineSeries();

    for(int i = 0; i<yValues.size(); ++i)
    {
        series->append(xValues.at(i),yValues.at(i));
    }

    if(RFDiagChart == nullptr)
    {
        RFDiagChart = new QChart();
        RFDiagChart->setDropShadowEnabled(false);
        RFDiagChart->setMargins(QMargins(5,5,5,5));
        RFDiagChart->layout()->setContentsMargins(0, 0, 0, 0);
        RFDiagChart->legend()->setVisible(false);

        lossesRFDiagram = new QChartView(RFDiagChart);
        lossesRFDiagram->setRenderHint(QPainter::Antialiasing);
        lossesRFDiagram->setContentsMargins(0,0,0,0);
        lossesRFDiagram->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    }
    else
    {
        RFDiagChart->removeAllSeries();

        auto axes = RFDiagChart->axes(Qt::Horizontal);

        for(auto&& it : axes)
            RFDiagChart->removeAxis(it);
    }

    RFDiagChart->addSeries(series);

    QValueAxis *axisX = new QValueAxis();
    axisX->setGridLineVisible(false);
    axisX->setLabelsVisible(true);
    RFDiagChart->addAxis(axisX, Qt::AlignBottom);

    axisX->setMin(0.0);

    series->attachAxis(axisX);

    return 0;
}


int PelicunPostProcessor::createLossesChart(QBarSet *structLossSet, QBarSet *NSAccLossSet, QBarSet *NSDriftLossSet)
{
    QStackedBarSeries *series = new QStackedBarSeries();
    series->append(structLossSet);
    series->append(NSAccLossSet);
    series->append(NSDriftLossSet);
    series->setBarWidth(1.0);
    //    series->setLabelsVisible(true);
    //    series->setLabelsPrecision(3);
    //    series->setLabelsPosition(QAbstractBarSeries::LabelsCenter);

    if(Losseschart == nullptr)
    {
        Losseschart = new QChart();
        Losseschart->setDropShadowEnabled(false);
        Losseschart->setMargins(QMargins(5,5,5,5));
        Losseschart->layout()->setContentsMargins(0, 0, 0, 0);
        Losseschart->legend()->setVisible(true);

        lossesChartView = new QChartView(Losseschart);
        lossesChartView->setRenderHint(QPainter::Antialiasing);
        lossesChartView->setContentsMargins(0,0,0,0);
        lossesChartView->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    }
    else
    {
        Losseschart->removeAllSeries();

        auto axes = Losseschart->axes();

        for(auto&& it : axes)
            Losseschart->removeAxis(it);
    }

    Losseschart->addSeries(series);

    QValueAxis *axisY = new QValueAxis();
    axisY->setGridLineVisible(false);
    Losseschart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    QStringList categories;
    categories << "DS = 1" << "DS = 2" << "DS = 3" << "DS = 4";
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->setMinorGridLineVisible(false);
    axisX->append(categories);
    Losseschart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    return 0;
}


int PelicunPostProcessor::printToPDF(const QString& outputPath)
{
    outputFilePath = outputPath;

    theVisualizationWidget->takeScreenShot();

    return 0;
}


void PelicunPostProcessor::processResultsSubset(const std::set<int>& selectedComponentIDs)
{

    if(selectedComponentIDs.empty())
        return;

    if(DVdata.size() < numHeaderRows)
    {
        QString msg = "No results to import!";
        throw msg;
    }

    if(DVdata.at(numHeaderRows).isEmpty() || DVdata.last().isEmpty())
    {
        QString msg = "No values in the cells";
        throw msg;
    }

    auto firstID = objectToInt(DVdata.at(numHeaderRows).at(0));

    auto lastID = objectToInt(DVdata.last().at(0));

    QVector<QStringList> DVsubset(&DVdata[0],&DVdata[numHeaderRows]);

    for(auto&& id : selectedComponentIDs)
    {
        // Check that the ID falls within the bounds of the data
        if(id<firstID || id>lastID)
        {
            QString msg = "ID " + QString::number(id) + " is out of bounds of the results";
            throw msg;
        }

        auto found = false;
        for(int i = numHeaderRows; i<DVdata.size(); ++i)
        {
            auto inputRow = DVdata.at(i);

            auto buildingID = objectToInt(inputRow.at(0));

            if(id == buildingID)
            {
                DVsubset << inputRow;
                found = true;
                break;
            }
        }

        if(!found)
        {
            QString msg = "ID " + QString::number(id) + " cannot be found in the results";
            throw msg;
        }
    }

    this->processDVResults(DVsubset);
}


int PelicunPostProcessor::assemblePDF(QImage screenShot)
{
    // The printer
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPaperSize(QPrinter::Letter);
    printer.setPageMargins(25.4, 25.4, 25.4, 25.4, QPrinter::Millimeter);
    printer.setFullPage(true);
    qreal leftMargin, topMargin;
    printer.getPageMargins(&leftMargin,&topMargin,nullptr,nullptr,QPrinter::Point);
    printer.setOutputFileName(outputFilePath);

    // Create a new document
    QTextDocument* document = new QTextDocument();
    QTextCursor cursor(document);
    document->setDocumentMargin(25.4);
    document->setDefaultFont(QFont("Helvetica"));

    // Define font styles
    QTextCharFormat normalFormat;
    normalFormat.setFontPointSize(12);
    normalFormat.setFontWeight(QFont::Normal);

    QTextCharFormat titleFormat;
    titleFormat.setFontWeight(QFont::Bold);
    titleFormat.setFontCapitalization(QFont::AllUppercase);
    titleFormat.setFontPointSize(normalFormat.fontPointSize() * 2.0);

    QTextCharFormat captionFormat;
    captionFormat.setFontWeight(QFont::Light);
    captionFormat.setFontPointSize(normalFormat.fontPointSize());
    captionFormat.setFontItalic(true);

    QTextCharFormat disclaimerFormat;
    disclaimerFormat.setFontWeight(QFont::Light);
    disclaimerFormat.setFontPointSize(normalFormat.fontPointSize() / 1.5);

    QTextCharFormat boldFormat;
    boldFormat.setFontWeight(QFont::Bold);

    QFontMetrics normMetrics(normalFormat.font());
    auto lineSpacing = normMetrics.lineSpacing();

    // Define alignment formats
    QTextBlockFormat alignCenter;
    alignCenter.setLineHeight(lineSpacing, QTextBlockFormat::LineDistanceHeight) ;
    alignCenter.setAlignment(Qt::AlignCenter);

    QTextBlockFormat alignLeft;
    alignLeft.setAlignment(Qt::AlignLeft);
    alignLeft.setLineHeight(lineSpacing, QTextBlockFormat::LineDistanceHeight) ;

    cursor.movePosition(QTextCursor::Start);

    cursor.insertBlock(alignCenter);

    // Insert the simcenter logo at the top
    QImage simCenterLogo(":resources/SimCenter@1x.png");
    document->addResource(QTextDocument::ImageResource, QUrl("Logo"), simCenterLogo);
    QTextImageFormat imageFormatSimCenterLogo;
    imageFormatSimCenterLogo.setName("Logo");
    imageFormatSimCenterLogo.setWidth(250);
    imageFormatSimCenterLogo.setQuality(600);

    cursor.insertImage(imageFormatSimCenterLogo);

    cursor.insertText("\nRegional Resilience Determination (R2D) Tool\n",titleFormat);

    cursor.insertText("Results Summary\n",boldFormat);

    alignLeft.setLineHeight(1, QTextBlockFormat::SingleHeight) ;

    cursor.setBlockFormat(alignLeft);

    QString disclaimerText = "Disclaimer: The presented simulation results are not representative of any individual building’s response. To understand the response of any individual building, "
                                "please consult with a professional structural engineer. The presented tool does not assert the known condition of the building. Just as it cannot be used to predict the negative outcome of an individual "
                                "building, prediction of safety or an undamaged state is not assured for an individual building. Any opinions, findings, and conclusions or recommendations expressed in this material are "
                                "those of the author(s) and do not necessarily reflect the views of the National Science Foundation.\n";
    cursor.insertText(disclaimerText,disclaimerFormat);

    alignLeft.setLineHeight(lineSpacing, QTextBlockFormat::LineDistanceHeight);

    cursor.setBlockFormat(alignLeft);

    cursor.insertText("\nEmploying Pelicun loss methodology to calculate seismic losses.\n",normalFormat);

    QString currentDT = "Timestamp: " + QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") + "\n";
    cursor.insertText(currentDT,normalFormat);

    auto workflowApp = WorkflowAppR2D::getInstance();
    auto analysisName = workflowApp->getGeneralInformationWidget()->getAnalysisName();

    QString analysisNameLabel = "Analysis name: " + analysisName + "\n";
    cursor.insertText(analysisNameLabel,normalFormat);

    cursor.insertText("Estimated Regional Totals\n",boldFormat);

    QTextTableFormat tableFormat;
    tableFormat.setPadding(5.0);
    tableFormat.setCellPadding(5.0);
    tableFormat.setBorder(0.0);
    tableFormat.setAlignment(Qt::AlignVCenter);

    tableFormat.setBackground(QColor("#f0f0f0"));
    QVector<QTextLength> constraints;
    constraints << QTextLength(QTextLength::PercentageLength, 25);
    constraints << QTextLength(QTextLength::PercentageLength, 25);
    constraints << QTextLength(QTextLength::PercentageLength, 25);
    constraints << QTextLength(QTextLength::PercentageLength, 25);
    tableFormat.setColumnWidthConstraints(constraints);

    // rows, columns, tableFormat
    QTextTable *table = cursor.insertTable(3, 4, tableFormat);

    {
        QTextTableCell cell = table->cellAt(0, 0);
        cell.setFormat(normalFormat);
        QTextCursor cellCursor = cell.firstCursorPosition();
        cellCursor.insertText(totalCasLabel->text());
    }

    {
        QTextTableCell cell = table->cellAt(0, 1);
        cell.setFormat(normalFormat);
        QTextCursor cellCursor = cell.firstCursorPosition();
        cellCursor.insertText(totalCasValueLabel->text());
    }

    {
        QTextTableCell cell = table->cellAt(0, 2);
        cell.setFormat(normalFormat);
        QTextCursor cellCursor = cell.firstCursorPosition();
        cellCursor.insertText(totalFatalitiesLabel->text());
    }

    {
        QTextTableCell cell = table->cellAt(0, 3);
        cell.setFormat(normalFormat);
        QTextCursor cellCursor = cell.firstCursorPosition();
        cellCursor.insertText(totalFatalitiesValueLabel->text());
    }

    {
        QTextTableCell cell = table->cellAt(1, 0);
        cell.setFormat(normalFormat);
        QTextCursor cellCursor = cell.firstCursorPosition();
        cellCursor.insertText(totalLossLabel->text());
    }

    {
        QTextTableCell cell = table->cellAt(1, 1);
        cell.setFormat(normalFormat);
        QTextCursor cellCursor = cell.firstCursorPosition();
        cellCursor.insertText(totalLossValueLabel->text());
    }

    {
        QTextTableCell cell = table->cellAt(1, 2);
        cell.setFormat(normalFormat);
        QTextCursor cellCursor = cell.firstCursorPosition();
        cellCursor.insertText(totalRepairTimeLabel->text());
    }

    {
        QTextTableCell cell = table->cellAt(1, 3);
        cell.setFormat(normalFormat);
        QTextCursor cellCursor = cell.firstCursorPosition();
        cellCursor.insertText(totalRepairTimeValueLabel->text());
    }

    {
        QTextTableCell cell = table->cellAt(2, 0);
        cell.setFormat(normalFormat);
        QTextCursor cellCursor = cell.firstCursorPosition();
        cellCursor.insertText(structLossLabel->text());
    }

    {
        QTextTableCell cell = table->cellAt(2, 1);
        cell.setFormat(normalFormat);
        QTextCursor cellCursor = cell.firstCursorPosition();
        cellCursor.insertText(structLossValueLabel->text());
    }

    {
        QTextTableCell cell = table->cellAt(2, 2);
        cell.setFormat(normalFormat);
        QTextCursor cellCursor = cell.firstCursorPosition();
        cellCursor.insertText(nonStructLossLabel->text());
    }

    {
        QTextTableCell cell = table->cellAt(2, 3);
        cell.setFormat(normalFormat);
        QTextCursor cellCursor = cell.firstCursorPosition();
        cellCursor.insertText(nonStructLossValueLabel->text());
    }

    cursor.movePosition( QTextCursor::End );

    cursor.insertText("\n\n",normalFormat);

    // Ratio of the page width that is printable
    auto useablePageWidth = printer.pageRect(QPrinter::Point).width()-(1.5*leftMargin);

    QRect viewPortRect(0, mapViewMainWidget->height() - mapViewSubWidget->height(), mapViewSubWidget->width(), mapViewSubWidget->height());
    QImage cropped = screenShot.copy(viewPortRect);
    document->addResource(QTextDocument::ImageResource,QUrl("Figure1"),cropped);
    QTextImageFormat imageFormatFig1;
    imageFormatFig1.setName("Figure1");
    imageFormatFig1.setQuality(600);
    imageFormatFig1.setWidth(useablePageWidth);

    cursor.setBlockFormat(alignCenter);

    cursor.insertImage(imageFormatFig1);

    cursor.insertText("Regional map visualization.\n",captionFormat);

    auto origSize = casualtiesChartView->size();

    casualtiesChartView->resize(QSize(640,480));

    casualtiesChartView->setVisible(true);
    auto rectFig2 = casualtiesChartView->viewport()->rect();
    QPixmap pixmapFig2(rectFig2.size());
    QPainter painterFig2(&pixmapFig2);
    painterFig2.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    casualtiesChartView->render(&painterFig2, pixmapFig2.rect(), rectFig2);
    auto figure2 = pixmapFig2.toImage();

    casualtiesChartView->resize(origSize);

    QTextImageFormat imageFormatFig2;
    imageFormatFig2.setName("Figure2");
    imageFormatFig2.setQuality(600);
    imageFormatFig2.setWidth(400);

    document->addResource(QTextDocument::ImageResource,QUrl("Figure2"),figure2);

    cursor.insertImage(imageFormatFig2,QTextFrameFormat::InFlow);

    cursor.insertText("\nEstimated casualties.\n",captionFormat);

    auto origSize2 = lossesChartView->size();
    lossesChartView->resize(QSize(640,480));
    auto rectFig3 = lossesChartView->viewport()->rect();
    QPixmap pixmapFig3(rectFig3.size());
    QPainter painterFig3(&pixmapFig3);
    painterFig3.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    lossesChartView->render(&painterFig3, pixmapFig3.rect(), rectFig3);
    auto figure3 = pixmapFig3.toImage();
    lossesChartView->resize(origSize2);

    QTextImageFormat imageFormatFig3;
    imageFormatFig3.setName("Figure3");
    imageFormatFig3.setQuality(600);
    imageFormatFig3.setWidth(400);

    document->addResource(QTextDocument::ImageResource,QUrl("Figure3"),figure3);
    cursor.insertImage(imageFormatFig3,QTextFrameFormat::InFlow);

    cursor.insertText("\nEstimated economic losses.\n",captionFormat);

    if(lossesRFDiagram->property("ToPlot").toBool())
    {
        auto origSize3 = lossesRFDiagram->size();
        lossesRFDiagram->resize(QSize(640,480));
        auto rectFig4 = lossesRFDiagram->viewport()->rect();
        QPixmap pixmapFig4(rectFig4.size());
        QPainter painterFig4(&pixmapFig4);
        painterFig4.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        lossesRFDiagram->render(&painterFig4, pixmapFig4.rect(), rectFig4);
        auto figure4 = pixmapFig4.toImage();
        lossesRFDiagram->resize(origSize3);

        QTextImageFormat imageFormatFig4;
        imageFormatFig4.setName("Figure4");
        imageFormatFig4.setQuality(600);
        imageFormatFig4.setWidth(400);

        document->addResource(QTextDocument::ImageResource,QUrl("Figure4"),figure4);
        cursor.insertImage(imageFormatFig4,QTextFrameFormat::InFlow);

        cursor.insertText("\nRelative frequency diagram of expected losses.\n",captionFormat);
    }

    cursor.insertText("Individual Asset Results - Sorted According to the " + sortComboBox->currentText() + "\n",boldFormat);

    TablePrinter prettyTablePrinter;
    prettyTablePrinter.printToTable(&cursor, pelicunResultsTableWidget,"Asset Results");

    document->print(&printer);

    return 0;
}


void PelicunPostProcessor::sortTable(int index)
{
    if(index == 0)
        pelicunResultsTableWidget->sortByColumn(index,Qt::AscendingOrder);
    else
        pelicunResultsTableWidget->sortByColumn(index,Qt::DescendingOrder);

}


void PelicunPostProcessor::restoreUI(void)
{
    this->restoreState(uiState);
}


void PelicunPostProcessor::setCurrentlyViewable(bool status){


#ifdef ARC_GIS    
    // Set the legend to display the selected building layer
    auto buildingsWidget = theVisualizationWidget->getComponentWidget("BUILDINGS");

    if(buildingsWidget)
    {
        if (status == true)
            mapViewSubWidget->setCurrentlyViewable(status);

        auto selectedLayer = buildingsWidget->getSelectedFeatureLayer();

        auto arcVizWidget = static_cast<ArcGISVisualizationWidget*>(theVisualizationWidget);

        if(arcVizWidget == nullptr)
        {
            qDebug()<<"Failed to cast to ArcGISVisualizationWidget";
            return;
        }

        arcVizWidget->handleLegendChange(selectedLayer);
    }
#else
    Q_UNUSED(status);
#endif

}


void PelicunPostProcessor::clear(void)
{
    DMdata.clear();
    DVdata.clear();
    EDPdata.clear();

    outputFilePath.clear();

    totalCasValueLabel->clear();
    totalLossValueLabel->clear();
    totalRepairTimeValueLabel->clear();
    totalFatalitiesValueLabel->clear();
    structLossValueLabel->clear();
    nonStructLossValueLabel->clear();

    pelicunResultsTableWidget->clear();

    sortComboBox->setCurrentIndex(0);
}

