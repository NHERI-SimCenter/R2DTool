#include "PelicunPostProcessor.h"
#include "CSVReaderWriter.h"
#include "VisualizationWidget.h"
#include "TablePrinter.h"
#include "WorkflowAppRDT.h"
#include "MainWindowWorkflowApp.h"
#include "GeneralInformationWidget.h"
#include "REmpiricalProbabilityDistribution.h"

#include <QHeaderView>
#include <QTabWidget>
#include <QGroupBox>
#include <QTableWidget>
#include <QFileInfo>
#include <QLabel>
#include <QDir>
#include <QStringList>
#include <QGridLayout>
#include <QBarSet>
#include <QBarSeries>
#include <QChart>
#include <QDockWidget>
#include <QBarCategoryAxis>
#include <QStackedBarSeries>
#include <QValueAxis>
#include <QChartView>
#include <QGraphicsLayout>
#include <QPrinter>
#include <QTextCursor>
#include <QPixmap>
#include <QFontMetrics>
#include <QTextTable>
#include <QComboBox>
#include <QLineSeries>
#include <QMenuBar>

// GIS headers
#include "Basemap.h"
#include "Map.h"
#include "MapGraphicsView.h"
#include "FeatureTable.h"

using namespace QtCharts;

PelicunPostProcessor::PelicunPostProcessor(QWidget *parent, VisualizationWidget* visWidget) : QMainWindow(parent), theVisualizationWidget(visWidget)
{
    casualtiesChart = nullptr;
    RFDiagChart = nullptr;
    Losseschart = nullptr;

    // Create a view menu for the dockable windows
    auto mainWindow = WorkflowAppRDT::getInstance()->getTheMainWindow();
    QMenu *viewMenu = mainWindow->menuBar()->addMenu(tr("&View"));

    viewMenu->addAction(tr("&Restore"), this, &PelicunPostProcessor::restoreUI);

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
    viewMenu->addAction(summaryDock->toggleViewAction());
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

    viewMenu->addAction(chartsDock1->toggleViewAction());
    viewMenu->addAction(chartsDock2->toggleViewAction());
    viewMenu->addAction(chartsDock3->toggleViewAction());

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

    QStringList tableHeadings = {"Asset ID","Repair\nCost","Repair\nTime","Replacement\nProbability","Fatalities","Loss\nRatio"};

    pelicunResultsTableWidget->setColumnCount(tableHeadings.size());
    pelicunResultsTableWidget->setHorizontalHeaderLabels(tableHeadings);
    pelicunResultsTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

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

    viewMenu->addAction(tableDock->toggleViewAction());

    // Create a map view that will be used for selecting the grid points
    mapViewMainWidget = theVisualizationWidget->getMapViewWidget();

    mapViewSubWidget = std::make_unique<ResultsMapViewWidget>(nullptr, mapViewMainWidget);

    // Popup stuff
    // Once map is set, connect to MapQuickView mouse clicked signal
    connect(mapViewSubWidget.get(), &ResultsMapViewWidget::mouseClick, theVisualizationWidget, &VisualizationWidget::onMouseClickedGlobal);

    mapViewSubWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    QDockWidget* mapViewDock = new QDockWidget("Regional Map",this);
    mapViewDock->setObjectName("MapViewDock");
    mapViewDock->setAllowedAreas(Qt::LeftDockWidgetArea);
    mapViewDock->setWidget(mapViewSubWidget.get());
    addDockWidget(Qt::LeftDockWidgetArea, mapViewDock);

    viewMenu->addAction(mapViewDock->toggleViewAction());

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


int PelicunPostProcessor::processDVResults(const QVector<QStringList>& DVResults)
{
    if(DVResults.size() < numHeaderRows)
    {
        QString msg = "No results to import!";
        throw msg;
    }

    auto numHeaderColumns = DVResults.at(0).size();

    QVector<QString> headerStrings(numHeaderColumns);

    for(int i = 0; i<numHeaderColumns; ++i)
    {
        QString headerStr =  DVResults.at(0).at(i)  +"-"+ DVResults.at(1).at(i)  +"-"+  DVResults.at(2).at(i)  +"-"+  DVResults.at(3).at(i);

        headerStrings[i] = headerStr;
    }

    pelicunResultsTableWidget->setRowCount(DVResults.size()-numHeaderRows);

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

    REmpiricalProbabilityDistribution theProbDist;

    // Get the buildings database
    auto theBuildingDB = theVisualizationWidget->getBuildingDatabase();

    // 4 rows of headers in the results file
    for(int i = numHeaderRows, count = 0; i<DVResults.size(); ++i, ++count)
    {
        auto inputRow = DVResults.at(i);

        auto buildingID = objectToInt(inputRow.at(0));

        auto building = theBuildingDB->getBuilding(buildingID);

        if(building.ID == -1)
            throw QString("Could not find the building ID " + QString::number(buildingID) + " in the database");

        for(int j = 1; j<numHeaderColumns; ++j)
        {
            building.ResultsValues.insert(headerStrings.at(j),inputRow.at(j).toDouble());
        }

        auto replacementCostVar = building.buildingAttributes.value("replacementCost",QVariant(0.0));

        auto replacementCost = objectToDouble(replacementCostVar);

        building.ID = buildingID;

        buildingsVec.push_back(building);

        // This assumes that the output from pelicun will not change
        auto IDStr = inputRow.at(0);            // ID
        auto totalRepairCost = inputRow.at(1);  // Aggregate repair cost (mean)
        auto replaceMentProb = inputRow.at(6);  // Replacement probability, i.e., repair impractical probability
        auto repairTime = inputRow.at(28);      // Aggregate repair time (mean)

        cumulativeRepairTime += objectToDouble(repairTime);

        auto StructDS1 = objectToDouble(inputRow.at(8));    // Structural losses damage state 1 (mean)
        auto StructDS2 = objectToDouble(inputRow.at(9));    // Structural losses damage state 2 (mean)
        auto StructDS3 = objectToDouble(inputRow.at(10));   // Structural losses damage state 3 (mean)
        auto StructDS4 = objectToDouble(inputRow.at(11));   // Structural losses damage state 4 (mean)

        cumulativeStructDS1 += StructDS1;
        cumulativeStructDS2 += StructDS2;
        cumulativeStructDS3 += StructDS3;
        cumulativeStructDS4 += StructDS4;

        auto NSAccDS1 = objectToDouble(inputRow.at(19));    // Non-structural acceleration sensitive losses damage state 1 (mean)
        auto NSAccDS2 = objectToDouble(inputRow.at(20));    // Non-structural acceleration sensitive losses damage state 2 (mean)
        auto NSAccDS3 = objectToDouble(inputRow.at(21));    // Non-structural acceleration sensitive losses damage state 3 (mean)
        auto NSAccDS4 = objectToDouble(inputRow.at(22));    // Non-structural acceleration sensitive losses damage state 4 (mean)

        cumulativeNSAccDS1 += NSAccDS1;
        cumulativeNSAccDS2 += NSAccDS2;
        cumulativeNSAccDS3 += NSAccDS3;
        cumulativeNSAccDS4 += NSAccDS4;

        auto NSDriftDS1 = objectToDouble(inputRow.at(24));  // Non-structural drift sensitive losses damage state 1 (mean)
        auto NSDriftDS2 = objectToDouble(inputRow.at(25));  // Non-structural drift sensitive losses damage state 2 (mean)
        auto NSDriftDS3 = objectToDouble(inputRow.at(26));  // Non-structural drift sensitive losses damage state 3 (mean)
        auto NSDriftDS4 = objectToDouble(inputRow.at(27));  // Non-structural drift sensitive losses damage state 4 (mean)

        cumulativeNSDriftDS1 += NSDriftDS1;
        cumulativeNSDriftDS2 += NSDriftDS2;
        cumulativeNSDriftDS3 += NSDriftDS3;
        cumulativeNSDriftDS4 += NSDriftDS4;

        auto injSevLvl1 = objectToDouble(inputRow.at(33));  // Injuries severity level 1 (mean)
        auto injSevLvl2 = objectToDouble(inputRow.at(38));  // Injuries severity level 2 (mean)
        auto injSevLvl3 = objectToDouble(inputRow.at(43));  // Injuries severity level 3 (mean)
        auto fatalities = objectToDouble(inputRow.at(48));  // Injuries severity level 4 (mean)

        cumulativeinjSevLvl1 += injSevLvl1;
        cumulativeinjSevLvl2 += injSevLvl2;
        cumulativeinjSevLvl3 += injSevLvl3;
        cumulativeinjSevLvl4 += fatalities;

        auto repairCost = objectToDouble(totalRepairCost);
        auto lossRatio = repairCost/replacementCost;

        theProbDist.addSample(repairCost);

        auto IDItem = new QTableWidgetItem(IDStr);
        auto RepCostItem = new QTableWidgetItem(totalRepairCost);
        auto RepProbItem = new QTableWidgetItem(replaceMentProb);
        auto RepairTimeItem = new QTableWidgetItem(repairTime);
        auto fatalitiesItem = new QTableWidgetItem(inputRow.at(48));
        auto lossRatioItem = new QTableWidgetItem(QString::number(lossRatio));

        pelicunResultsTableWidget->setItem(count,0, IDItem);
        pelicunResultsTableWidget->setItem(count,1, RepCostItem);
        pelicunResultsTableWidget->setItem(count,2, RepairTimeItem);
        pelicunResultsTableWidget->setItem(count,3, RepProbItem);
        pelicunResultsTableWidget->setItem(count,4, fatalitiesItem);
        pelicunResultsTableWidget->setItem(count,5, lossRatioItem);

        auto buildingFeature = building.buildingFeature;

        buildingFeature->attributes()->replaceAttribute("LossRatio",lossRatio);

        buildingFeature->featureTable()->updateFeature(buildingFeature);
    }

    //  CASUALTIES
    QBarSet *casualtiesSet = new QBarSet("Casualties");

    *casualtiesSet << cumulativeinjSevLvl1 << cumulativeinjSevLvl2 << cumulativeinjSevLvl3 << cumulativeinjSevLvl4;

    this->createCasualtiesChart(casualtiesSet);

    totalCasValueLabel->setText(QString::number(casualtiesSet->sum()));
    totalFatalitiesValueLabel->setText(QString::number(casualtiesSet->at(3)));

    //  LOSSES
    QBarSet *structLossSet = new QBarSet("Structural");
    QBarSet *NSAccLossSet = new QBarSet("Non-structural Acc.");
    QBarSet *NSDriftLossSet = new QBarSet("Non-structural Drift");

    *structLossSet << cumulativeStructDS1 << cumulativeStructDS2 << cumulativeStructDS3 << cumulativeStructDS4 ;
    *NSAccLossSet << cumulativeNSAccDS1 << cumulativeNSAccDS2 << cumulativeNSAccDS3 << cumulativeNSAccDS4 ;
    *NSDriftLossSet << cumulativeNSDriftDS1 << cumulativeNSDriftDS2 << cumulativeNSDriftDS3 << cumulativeNSDriftDS4;

    this->createLossesChart(structLossSet, NSAccLossSet, NSDriftLossSet);

    auto sumStruct = structLossSet->sum();
    auto sumNonStruct = NSAccLossSet->sum() + NSDriftLossSet->sum();

    auto sumLosses = sumStruct + sumNonStruct;
    totalLossValueLabel->setText(QString::number(sumLosses));

    structLossValueLabel->setText(QString::number(sumStruct));
    nonStructLossValueLabel->setText(QString::number(sumNonStruct));

    // Repair time
    totalRepairTimeValueLabel->setText(QString::number(cumulativeRepairTime));

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
    normalFormat.setFontWeight(QFont::Normal);

    QTextCharFormat titleFormat;
    titleFormat.setFontWeight(QFont::Bold);
    titleFormat.setFontCapitalization(QFont::AllUppercase);
    titleFormat.setFontPointSize(normalFormat.fontPointSize() * 2.0);

    QTextCharFormat captionFormat;
    captionFormat.setFontWeight(QFont::Light);
    captionFormat.setFontPointSize(normalFormat.fontPointSize() / 2.0);
    captionFormat.setFontItalic(true);

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

    cursor.insertText("\nRegional Resilience Determination Tool (RDT)\n",titleFormat);

    cursor.insertText("Results Summary\n",boldFormat);

    cursor.setBlockFormat(alignLeft);

    cursor.insertText("Employing Pelicun loss methodology to calculate seismic losses.\n",normalFormat);

    QString currentDT = "Timestamp: " + QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") + "\n";
    cursor.insertText(currentDT,normalFormat);

    auto workflowApp = WorkflowAppRDT::getInstance();
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

