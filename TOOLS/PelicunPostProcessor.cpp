#include "PelicunPostProcessor.h"
#include "CSVReaderWriter.h"
#include "VisualizationWidget.h"

#include <QHeaderView>
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
    componentTableWidget = new QTableWidget();
    componentTableWidget->verticalHeader()->setVisible(false);
    componentTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    componentTableWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    componentTableWidget->hide();

    // Layout to display the results
    resultsGridLayout = new QGridLayout();
    resultsGridLayout->setContentsMargins(20,0,0,0);

    QLabel* estCasLabel = new QLabel("Estimated Casualties", this);

    aggCasLabel = new QLabel("Aggregate casualties:", this);

    QLabel* estLossLabel = new QLabel("Estimated Economic Losses", this);

    aggLossLabel = new QLabel("Aggregate losses:", this);

    // Create a map view that will be used for selecting the grid points
    mapViewMainWidget = theVisualizationWidget->getMapViewWidget();

    mapViewSubWidget = std::make_unique<ResultsMapViewWidget>(nullptr,mapViewMainWidget);

    mapViewSubWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    mapViewSubWidget->setMaximumWidth(990);


    resultsGridLayout->addWidget(estCasLabel,0,0,1,2,Qt::AlignCenter);
    resultsGridLayout->addWidget(estLossLabel,0,2,1,2,Qt::AlignCenter);
    // Charts go here->
    resultsGridLayout->addWidget(aggCasLabel,2,0,1,2,Qt::AlignCenter);
    resultsGridLayout->addWidget(aggLossLabel,2,2,1,2,Qt::AlignCenter);

    resultsGridLayout->addWidget(mapViewSubWidget.get(),3,0,1,7);
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


int PelicunPostProcessor::processDVResults(const QVector<QStringList>& DMdata)
{

    this->createCasualtiesChart();

    this->createLossesChart();


    return 0;
}


int PelicunPostProcessor::createCasualtiesChart()
{
    QBarSet *set0 = new QBarSet("Casulaties");

    *set0 << 2.1 << 4.2 << 6.3 << 2.4 << 1.5;

    auto aggCas = aggCasLabel->text().append(" "+QString::number(set0->sum()));
    aggCasLabel->setText(aggCas);

    QBarSeries *series = new QBarSeries();
    series->append(set0);
    series->setBarWidth(1.0);
    series->setLabelsVisible(true);
    series->setLabelsPosition(QAbstractBarSeries::LabelsCenter);

    QChart *chart = new QChart();
    chart->setDropShadowEnabled(false);
    chart->addSeries(series);
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

    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setContentsMargins(0,0,0,0);
    chartView->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    chartView->setMinimumHeight(300);
    chartView->setMaximumWidth(500);

    resultsGridLayout->addWidget(chartView,1,0,1,2);

    return 0;
}


int PelicunPostProcessor::createLossesChart()
{
    QBarSet *set0 = new QBarSet("Structural");
    QBarSet *set1 = new QBarSet("Non-structural Acc.");
    QBarSet *set2 = new QBarSet("Non-structural Drift");

    *set0 << 1.7 << 2.4 << 3.1 << 4.2 ;
    *set1 << 5.1 << 0.1 << 0.3 << 4.3 ;
    *set2 << 3.2 << 5.4 << 8.4 << 13.4;

    auto sumLosses = set0->sum() + set1->sum() + set2->sum();

    auto aggLoss = aggLossLabel->text().append(" "+QString::number(sumLosses));
    aggLossLabel->setText(aggLoss);

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
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Millions of dollars [$]");
    //    axisY->setRange(0,25);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    chart->legend()->setVisible(true);

    chartView2 = new QChartView(chart);
    chartView2->setRenderHint(QPainter::Antialiasing);
    chartView2->setContentsMargins(0,0,0,0);
    chartView2->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    chartView2->setMinimumHeight(300);
    chartView2->setMaximumWidth(500);

    resultsGridLayout->addWidget(chartView2,1,2,1,2);

    return 0;
}



QGridLayout *PelicunPostProcessor::getResultsGridLayout() const
{
    return resultsGridLayout;
}

