#include "PelicunPostProcessor.h"
#include "CSVReaderWriter.h"
#include "VisualizationWidget.h"

#include <QHeaderView>
#include <QTableWidget>
#include <QFileInfo>
#include <QDir>
#include <QStringList>
#include <QGridLayout>

// GIS headers
#include "Basemap.h"
#include "Map.h"
#include "MapGraphicsView.h"

PelicunPostProcessor::PelicunPostProcessor(VisualizationWidget* visWidget) : theVisualizationWidget(visWidget)
{
    // Create the table that will show the Component information
    componentTableWidget = new QTableWidget();
    componentTableWidget->verticalHeader()->setVisible(false);
    componentTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    componentTableWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    componentTableWidget->hide();

    // Layout to display the results
    QGridLayout* resultsGridLayout = new QGridLayout();
    resultsGridLayout->setContentsMargins(0,0,0,0);

    // Create a map view that will be used for selecting the grid points
    mapViewMainWidget = theVisualizationWidget->getMapViewWidget();

    mapViewSubWidget = std::make_unique<ResultsMapViewWidget>(nullptr,mapViewMainWidget);
    mapViewSubWidget->setFixedSize(QSize(700,700));

    resultsGridLayout->addWidget(mapViewSubWidget.get(),0,0);


    // Create a map view that will be used for selecting the grid points
    mapViewMainWidget = theVisualizationWidget->getMapViewWidget();
    mapViewSubWidget = std::make_unique<ResultsMapViewWidget>(nullptr,mapViewMainWidget);
    mapViewSubWidget->setFixedSize(QSize(700,700));

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


    return 0;
}


int PelicunPostProcessor::processDVResults(const QVector<QStringList>& DMdata)
{



    return 0;
}

