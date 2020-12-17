#include "PelicunPostProcessor.h"
#include "CSVReaderWriter.h"
#include "VisualizationWidget.h"

#include <QHeaderView>
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
#include <QBarCategoryAxis>
#include <QStackedBarSeries>
#include <QValueAxis>
#include <QChartView>
#include <QGraphicsLayout>

// GIS headers
#include "Basemap.h"
#include "Map.h"
#include "MapGraphicsView.h"

using namespace QtCharts;

PelicunPostProcessor::PelicunPostProcessor(QWidget *parent, VisualizationWidget* visWidget) : SimCenterAppWidget(parent), theVisualizationWidget(visWidget)
{
    // Create the table that will show the Component information
    pelicunResultsTableWidget = new QTableWidget(this);
    pelicunResultsTableWidget->verticalHeader()->setVisible(false);
    pelicunResultsTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    pelicunResultsTableWidget->setSizeAdjustPolicy(QAbstractScrollArea::SizeAdjustPolicy::AdjustToContents);
    pelicunResultsTableWidget->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum);

    QStringList tableHeadings = {"Asset ID","Repair\nCost","Repair\nTime","Replacement\nProbability","Fatalities","Loss\nRatio"};

    pelicunResultsTableWidget->setColumnCount(tableHeadings.size());
    pelicunResultsTableWidget->setHorizontalHeaderLabels(tableHeadings);

    // Layout to display the results
    resultsGridLayout = new QGridLayout();
    resultsGridLayout->setContentsMargins(10,0,0,0);

    QGroupBox* totalsWidget = new QGroupBox("Total Estimates",this);
    totalsWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    QGridLayout* totalsLayout = new QGridLayout(totalsWidget);

    QLabel* estCasLabel = new QLabel("Estimated Casualties", this);
    QLabel* estLossLabel = new QLabel("Estimated Economic Losses - Millions of USD", this);

    estCasLabel->setStyleSheet("font-weight: bold");
    estLossLabel->setStyleSheet("font-weight: bold");

    totalCasLabel = new QLabel("Casualties:", this);
    totalLossLabel = new QLabel("Losses:", this);
    totalRepairTimeLabel = new QLabel("Repair Time: N/A", this);
    totalFatalitiesLabel = new QLabel("Fatalities: N/A", this);

    totalsLayout->addWidget(totalCasLabel,0,0);
    totalsLayout->addWidget(totalLossLabel,0,1);
    totalsLayout->addWidget(totalRepairTimeLabel,1,0);
    totalsLayout->addWidget(totalFatalitiesLabel,1,1);


    // Create a map view that will be used for selecting the grid points
    mapViewMainWidget = theVisualizationWidget->getMapViewWidget();

    mapViewSubWidget = std::make_unique<ResultsMapViewWidget>(nullptr,mapViewMainWidget);

    mapViewSubWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    mapViewSubWidget->setMinimumHeight(400);

    resultsGridLayout->addWidget(estCasLabel,0,0,Qt::AlignCenter);
    resultsGridLayout->addWidget(estLossLabel,0,1,Qt::AlignCenter);
    resultsGridLayout->addWidget(totalsWidget,0,2,2,1,Qt::AlignLeft);

    // Charts go here->

    resultsGridLayout->addWidget(mapViewSubWidget.get(),4,0,1,2);

    resultsGridLayout->setColumnStretch(4, 1);

}


int PelicunPostProcessor::importResults(const QString& pathToResults)
{

    // Remove old csv files in the output pathToResults
    const QFileInfo existingFilesInfo(pathToResults);

    // Get the existing files in the folder to see if we already have the record
    QStringList acceptableFileExtensions = {"*.csv"};
    QStringList existingCSVFiles = existingFilesInfo.dir().entryList(acceptableFileExtensions, QDir::Files);

    if(existingCSVFiles.empty())
    {
        QString errMessage = "The results folder is empty";
        return -1;
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

    QString err;
    QVector<QStringList> DMdata = csvTool.parseCSVFile(pathToResults + DMResultsSheet,err);

    if(!err.isEmpty() || DMdata.empty())
        return -1;


    QVector<QStringList> DVdata = csvTool.parseCSVFile(pathToResults + DVResultsSheet,err);

    if(!err.isEmpty() || DVdata.empty())
        return -1;


    QVector<QStringList> EDPdata = csvTool.parseCSVFile(pathToResults + EDPreultsSheet,err);

    if(!err.isEmpty() || EDPdata.empty())
        return -1;


    auto res1 = this->processDVResults(DVdata);


    return 0;
}


int PelicunPostProcessor::processDVResults(const QVector<QStringList>& DVdata)
{
    if(DVdata.size() < 4)
        return -1;

    auto numHeaders = DVdata.at(0).size();

    QVector<QString> headerStrings(numHeaders);

    for(int i = 0; i<numHeaders; ++i)
    {
        QString headerStr =  DVdata.at(0).at(i)  +"-"+ DVdata.at(1).at(i)  +"-"+  DVdata.at(2).at(i)  +"-"+  DVdata.at(3).at(i);

        headerStrings[i] = headerStr;
    }

    pelicunResultsTableWidget->setRowCount(DVdata.size()-4);

    int count = 0;
    for(int i = 4; i<DVdata.size(); ++i, ++count)
    {
        auto inputRow = DVdata.at(i);

        Building building;

        for(int j = 1; j<numHeaders; ++j)
        {
            building.values.insert(headerStrings.at(j),inputRow.at(j).toDouble());
        }

        building.ID = inputRow.at(0).toInt();

        buildingsVec.push_back(building);

        auto IDStr = inputRow.at(0);
        auto totalRepairCost = inputRow.at(1);
        auto replaceMentProb = inputRow.at(6);
        auto repairTime = inputRow.at(28);
        auto fatalities = inputRow.at(48);
        auto lossRatio = totalRepairCost;

        auto IDItem = new QTableWidgetItem(IDStr);
        auto RepCostItem = new QTableWidgetItem(totalRepairCost);
        auto RepProbItem = new QTableWidgetItem(replaceMentProb);
        auto RepairTimeItem = new QTableWidgetItem(repairTime);
        auto fatalitiesItem = new QTableWidgetItem(fatalities);
        auto lossRatioItem = new QTableWidgetItem(lossRatio);


        pelicunResultsTableWidget->setItem(count,0, IDItem);
        pelicunResultsTableWidget->setItem(count,1, RepCostItem);
        pelicunResultsTableWidget->setItem(count,2, RepairTimeItem);
        pelicunResultsTableWidget->setItem(count,3, RepProbItem);
        pelicunResultsTableWidget->setItem(count,4, fatalitiesItem);
        pelicunResultsTableWidget->setItem(count,5, lossRatioItem);
    }


    this->createCasualtiesChart();

    this->createLossesChart();

    casualtiesChartView->setMinimumHeight(250);
    casualtiesChartView->setMaximumWidth(500);

    lossesChartView->setMinimumHeight(250);
    lossesChartView->setMaximumWidth(500);

    resultsGridLayout->addWidget(casualtiesChartView,1,0,3,1);
    resultsGridLayout->addWidget(lossesChartView,1,1,3,1);
    resultsGridLayout->addWidget(pelicunResultsTableWidget,2,2,3,1);


    return 0;
}


int PelicunPostProcessor::createCasualtiesChart()
{
    QBarSet *set0 = new QBarSet("Casualties");

    *set0 << 2.1 << 4.2 << 6.3 << 2.4 << 1.5;

    auto aggCas = totalCasLabel->text().append(" "+QString::number(set0->sum()));
    totalCasLabel->setText(aggCas);

    QBarSeries *series = new QBarSeries();
    series->append(set0);
    series->setBarWidth(1.0);
    series->setLabelsVisible(true);
    series->setLabelsPosition(QAbstractBarSeries::LabelsCenter);

    QChart *chart = new QChart();
    chart->setDropShadowEnabled(false);
    chart->addSeries(series);
    chart->setMargins(QMargins(5,5,5,5));
    chart->layout()->setContentsMargins(0, 0, 0, 0);


    QStringList categories;
    categories << "Level 1" << "Level 2" << "Level 3" << "Level 4";
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    //    QValueAxis *axisY = new QValueAxis();
    //    //    axisY->setRange(0,15);
    //    chart->addAxis(axisY, Qt::AlignLeft);
    //    series->attachAxis(axisY);

    chart->legend()->setVisible(false);

    casualtiesChartView = new QChartView(chart);
    casualtiesChartView->setRenderHint(QPainter::Antialiasing);
    casualtiesChartView->setContentsMargins(0,0,0,0);
    casualtiesChartView->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    return 0;
}


int PelicunPostProcessor::createLossesChart()
{
    QBarSet *set0 = new QBarSet("Structural");
    QBarSet *set1 = new QBarSet("Non-structural Acc.");
    QBarSet *set2 = new QBarSet("Non-structural Drift");

    *set0 << 1.7 << 2.4 << 3.1 << 4.2 ;
    *set1 << 5.1 << 4.2 << 11.3 << 4.3 ;
    *set2 << 3.2 << 5.4 << 8.4 << 13.4;

    auto sumLosses = set0->sum() + set1->sum() + set2->sum();

    auto aggLoss = totalLossLabel->text().append(" "+QString::number(sumLosses));
    totalLossLabel->setText(aggLoss);

    QStackedBarSeries *series = new QStackedBarSeries();
    series->append(set0);
    series->append(set1);
    series->append(set2);
    series->setBarWidth(1.0);
    series->setLabelsVisible(true);
    series->setLabelsPosition(QAbstractBarSeries::LabelsCenter);

    QChart *chart = new QChart();
    chart->setDropShadowEnabled(false);
    chart->addSeries(series);
    chart->setMargins(QMargins(5,5,5,5));
    chart->layout()->setContentsMargins(0, 0, 0, 0);


    QStringList categories;
    categories << "DS = 1" << "DS = 2" << "DS = 3" << "DS = 4";
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->setMinorGridLineVisible(false);
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    //    QValueAxis *axisY = new QValueAxis();
    //    axisY->setTickCount(2);
    //    axisY->setLabelsVisible(false);
    //    axisY->setGridLineVisible(false);
    //    chart->addAxis(axisY, Qt::AlignLeft);
    //    series->attachAxis(axisY);

    chart->legend()->setVisible(true);

    lossesChartView = new QChartView(chart);
    lossesChartView->setRenderHint(QPainter::Antialiasing);
    lossesChartView->setContentsMargins(0,0,0,0);
    lossesChartView->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    return 0;
}



QGridLayout *PelicunPostProcessor::getResultsGridLayout() const
{
    return resultsGridLayout;
}

