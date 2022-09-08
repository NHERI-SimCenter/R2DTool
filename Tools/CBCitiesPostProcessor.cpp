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
#include "CBCitiesPostProcessor.h"
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

#include "QGISVisualizationWidget.h"

#include <qgsattributes.h>
#include <qgsmapcanvas.h>
#include <qgslinesymbol.h>

// Test to remove start
// #include <chrono>
// using namespace std::chrono;
// Test to remove end

using namespace QtCharts;

CBCitiesPostProcessor::CBCitiesPostProcessor(QWidget *parent, VisualizationWidget* visWidget) : QMainWindow(parent), theVisualizationWidget(visWidget)
{
    RFDiagChart = nullptr;
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
        viewMenu->addAction(tr("&Restore"), this, &CBCitiesPostProcessor::restoreUI);
    }
    else
    {
        PythonProgressDialog::getInstance()->appendErrorMessage("Could not find the results menu bar in CBCitiesPostProcessor::");
        return;
    }


    // Charts
    chartsDock3 = new QDockWidget(tr("Relative Freq.  Repair Rates"), this);
    chartsDock3->setObjectName("Relative Freq. Failure Probabilities");
    chartsDock3->setContentsMargins(5,5,5,5);

    this->addDockWidget(Qt::RightDockWidgetArea,chartsDock3);
    chartsDock3->setFocus();

    // Create the table that will show the Component information
    tableWidget = new QWidget(this);

    auto tableWidgetLayout = new QVBoxLayout(tableWidget);

    resultsTableWidget = new QTableWidget(this);
    resultsTableWidget->verticalHeader()->setVisible(false);
    resultsTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    resultsTableWidget->setSizeAdjustPolicy(QAbstractScrollArea::SizeAdjustPolicy::AdjustToContents);
    resultsTableWidget->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum);

    resultsTableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    resultsTableWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    resultsTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    resultsTableWidget->setItemDelegate(new DoubleDelegate(this,3));

    // Combo box to select how to sort the table
    QHBoxLayout *comboLayout = new QHBoxLayout();

    QStringList comboBoxHeadings = {"Asset ID","Repair Cost","Repair Time","Replacement Probability","Fatalities","Loss Ratio"};
    sortComboBox = new QComboBox();
    sortComboBox->insertItems(0,comboBoxHeadings);
    sortComboBox->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);

    connect(sortComboBox,QOverload<int>::of(&QComboBox::currentIndexChanged),this, &CBCitiesPostProcessor::sortTable);

    auto comboBoxLabel = new QLabel("Sorting Filter:", this);

    comboLayout->addWidget(comboBoxLabel);
    comboLayout->addWidget(sortComboBox);
    comboLayout->addStretch(0);

    tableWidgetLayout->addLayout(comboLayout);
    tableWidgetLayout->addWidget(resultsTableWidget);
    tableWidgetLayout->addStretch(0);

    //QDockWidget* tableDock = new QDockWidget("Detailed Results",this);
    tableDock = new QDockWidget("Detailed Results",this);
    tableDock->setObjectName("TableDock");
    tableDock->setWidget(tableWidget);
    tableDock->setMinimumWidth(475);
    addDockWidget(Qt::RightDockWidgetArea, tableDock);

    tableDock2 = new QDockWidget(tr("Site Responses"), this);
    tableDock2->setObjectName("Site Responses");

    // Get the map view widget
    auto mapView = theVisualizationWidget->getMapViewWidget("ResultsCBWidget");
    mapViewSubWidget = std::unique_ptr<SimCenterMapcanvasWidget>(mapView);

    QDockWidget* mapViewDock = new QDockWidget("Regional Map",this);
    mapViewDock->setObjectName("MapViewDock");
    mapViewDock->setAllowedAreas(Qt::LeftDockWidgetArea);
    mapViewDock->setWidget(mapViewSubWidget.get());
    addDockWidget(Qt::LeftDockWidgetArea, mapViewDock);

    if(viewMenu)
    {
        viewMenu->addAction(chartsDock3->toggleViewAction());
        viewMenu->addAction(tableDock->toggleViewAction());
        viewMenu->addAction(mapViewDock->toggleViewAction());
    }

    uiState = this->saveState();

    // The number of header rows in the results file
    numHeaderRows = 1;

    assetType = "Water Network Pipelines";
}


void CBCitiesPostProcessor::importResults(const QString& pathToResults)
{
    qDebug() << "CBCitiesPostProcessor: " << pathToResults;

    auto pathToAssets = pathToResults + QDir::separator() + QString(assetType).remove(" ");

    // Remove old csv files in the output pathToResults
    QDir resultsDir(pathToAssets);

    const QFileInfo existingFilesInfo(pathToAssets);

    // Get the existing files in the folder to see if we already have the record

    QStringList acceptableFileExtensions = {"*.csv"};
    QStringList existingCSVFiles = resultsDir.entryList(acceptableFileExtensions, QDir::Files);

    QString errMsg;

    if(existingCSVFiles.empty())
    {
        QStringList acceptableFileExtensions = {"*.*"};
        QStringList existingFiles = existingFilesInfo.dir().entryList(acceptableFileExtensions, QDir::Files);
        qDebug() << "FILES IN FOLDER: " << existingFiles;
        errMsg = "The results folder "+pathToAssets+" is empty";
        throw errMsg;
    }

    QString DVResultsSheet;

    for(auto&& it : existingCSVFiles)
    {
         if(it.startsWith("DV_"))
            DVResultsSheet = it;

    }

    CSVReaderWriter csvTool;


    DVdata = csvTool.parseCSVFile(pathToAssets + QDir::separator() + DVResultsSheet,errMsg);
    if(!errMsg.isEmpty())
        throw errMsg;


    if(!DVdata.empty())
        this->processDVResults(DVdata);
    else
    {
        errMsg = "The DV results are empty";
        throw errMsg;
    }

    // Enable the selection tool
    mapViewSubWidget->enableSelectionTool();

}


void CBCitiesPostProcessor::showEvent(QShowEvent *e)
{
    auto mainCanvas = mapViewSubWidget->getMainCanvas();

    auto mainExtent = mainCanvas->extent();

    mapViewSubWidget->mapCanvas()->zoomToFeatureExtent(mainExtent);
    QMainWindow::showEvent(e);
}




int CBCitiesPostProcessor::processDVResults(const QVector<QStringList>& DVResults)
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
        QString headerStr = DVResults.at(0).at(i);

        headerStrings.append(headerStr);
    }

    // Decipher the results file

    // Structural - seismic
    auto indexMeanRR = headerStrings.indexOf("RepairRate");

    // Aggregate repair time (mean)
    if(indexMeanRR == -1)
    {
        QString msg = "Error getting the header index for RepairRate!";
        throw msg;
    }

    QStringList tableHeadings = {"Asset ID","RepairRate"};

    resultsTableWidget->setColumnCount(tableHeadings.size());
    resultsTableWidget->setHorizontalHeaderLabels(tableHeadings);
    resultsTableWidget->setRowCount(DVResults.size()-numHeaderRows);

    REmpiricalProbabilityDistribution theProbDist;

    // Get the buildings database
    auto theAssetDB = ComponentDatabaseManager::getInstance()->getAssetDb(assetType);

    if(theAssetDB == nullptr)
    {
        QString msg = "Error getting the water network pipelines database from the input widget!";
        throw msg;
    }

    if(theAssetDB->isEmpty())
    {
        QString msg = "Water network pipelines database is empty";
        throw msg;
    }

    auto selFeatLayer = theAssetDB->getSelectedLayer();
    mapViewSubWidget->setCurrentLayer(selFeatLayer);

    // Vector to hold the attributes
    QVector< QgsAttributes > fieldAttributes(DVResults.size()-numHeaderRows, QgsAttributes(numHeaderColumns));


    for(int i = numHeaderRows, count = 0; i<DVResults.size(); ++i, ++count)
    {
        auto inputRow = DVResults.at(i);

        // This assumes that the output from pelicun will not change
        auto IDStr = inputRow.at(0);

        // ID
        auto meanRR = inputRow.at(indexMeanRR);  // Mean failure probability

        auto repairRate = objectToDouble(meanRR);

        theProbDist.addSample(repairRate);

        auto IDItem = new TableNumberItem(IDStr);
        auto failureProbItem = new TableNumberItem(meanRR);

        resultsTableWidget->setItem(count,0, IDItem);
        resultsTableWidget->setItem(count,1, failureProbItem);

        auto& rowData = fieldAttributes[count];

        // Populate the attributes vector with the results
        for(int k = 0; k<inputRow.size(); ++k)
        {
            // Add the result to the database
            auto value = inputRow.at(k);

            rowData[k] = QVariant(value.toDouble());
        }
    }

    // Test to remove start
    // auto start = high_resolution_clock::now();
    // Test to remove end

    // Starting editing
    theAssetDB->startEditing();

    QString errMsg;
    auto res = theAssetDB->addNewComponentAttributes(headerStrings,fieldAttributes,errMsg);
    if(!res)
        throw errMsg;

    // Commit the changes
    theAssetDB->commitChanges();

    // Test to remove start
    // auto stop = high_resolution_clock::now();
    // auto duration = duration_cast<milliseconds>(stop - start);
    // Test to remove end

    PythonProgressDialog::getInstance()->appendText("Done processing "+assetType+" results "/*+QString::number(duration.count())*/);

    //QGISVisualizationWidget* QGISVisWidget = static_cast<QGISVisualizationWidget*>(theVisualizationWidget);
    QGISVisWidget = static_cast<QGISVisualizationWidget*>(theVisualizationWidget);

    auto selLayer = theAssetDB->getSelectedLayer();

    // Apply the graduated renderer
    QgsLineSymbol* selectedLayerMarkerSymbol = new QgsLineSymbol();

    selectedLayerMarkerSymbol->setWidth(1.25);

    QGISVisWidget->createPrettyGraduatedRenderer("RepairRate",Qt::yellow,Qt::red,5,selLayer,selectedLayerMarkerSymbol);

    // Change the name to say loss ratio
    theAssetDB->getSelectedLayer()->setName("Repair Rate");

    this->createHistogramChart(&theProbDist);

    if(theProbDist.getNumberSamples() < 2)
        lossesRFDiagram->setProperty("ToPlot",false);
    else
        lossesRFDiagram->setProperty("ToPlot",true);

    // Set a default size to the charts
    chartsDock3->setWidget(lossesRFDiagram);

    return 0;
}


void CBCitiesPostProcessor::setIsVisible(const bool value)
{
    if(viewMenu)
        viewMenu->menuAction()->setVisible(value);
}



int CBCitiesPostProcessor::createHistogramChart(REmpiricalProbabilityDistribution* probDist)
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




void CBCitiesPostProcessor::processResultsSubset(const std::set<int>& selectedComponentIDs)
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



void CBCitiesPostProcessor::sortTable(int index)
{
    if(index == 0)
        resultsTableWidget->sortByColumn(index,Qt::AscendingOrder);
    else
        resultsTableWidget->sortByColumn(index,Qt::DescendingOrder);

}


void CBCitiesPostProcessor::restoreUI(void)
{
    this->restoreState(uiState);
}


void CBCitiesPostProcessor::setCurrentlyViewable(bool status){

    Q_UNUSED(status);

}



void CBCitiesPostProcessor::clear(void)
{
    DVdata.clear();

    outputFilePath.clear();

    resultsTableWidget->clear();

    sortComboBox->setCurrentIndex(0);

    mapViewSubWidget->clear();
}

