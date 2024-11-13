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

// Written by: Jinyan Zhao, ...
#include "ResidualDemandResults.h"
#include "VisualizationWidget.h"
#include "QGISVisualizationWidget.h"
#include "SimCenterMapcanvasWidget.h"

#include "CSVReaderWriter.h"
#include "ComponentDatabaseManager.h"
#include "GeneralInformationWidgetR2D.h"
#include "MainWindowWorkflowApp.h"
#include "REmpiricalProbabilityDistribution.h"
#include "TablePrinter.h"
#include "TableNumberItem.h"
#include "VisualizationWidget.h"
#include "WorkflowAppR2D.h"
#include "Utils/ProgramOutputDialog.h"
#include "SC_MovieWidget.h"

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
#include <QMovie>


#include <qgsattributes.h>
#include <qgsmapcanvas.h>

// Test to remove start
// #include <chrono>
// using namespace std::chrono;
// Test to remove end

using namespace QtCharts;

ResidualDemandResults::ResidualDemandResults(QWidget * parent) : SC_ResultsWidget(parent){

}

int ResidualDemandResults::addResultTab(QString tabName, QString &dirName){
    QString resultFile = tabName + QString(".geojson");
    QString assetTypeSimplified = tabName.simplified().replace( " ", "" );
    R2DresWidget->getTabWidget()->addTab(this, tabName);
    return 0;
    // Do the results processing here
//    this->processResults(resultFile, dirName, tabName, theAssetTypeToType[assetTypeSimplified]);

}

int ResidualDemandResults::addResultSubtab(QString name, QWidget* existTab, QString &dirName){

    SC_ResultsWidget* existingResult = dynamic_cast<SC_ResultsWidget*>(existTab);
    if (existingResult){ // Make a subtab and add to existing result tab
        // Do the visualizations
        // Make the sub
//        QDockWidget* gifWidget = new QDockWidget(name, this);
//        gifWidget->setObjectName(name + "Congestion");

        // Add the gif
        QDockWidget* gifWidget = createGIFWidget(this, name, dirName);
        existingResult->addResultSubtab(QString("ResidualDemand"), gifWidget, dirName);
    }
    else{ //Add the subtab to docklist
        QDockWidget* subTabToAdd = dynamic_cast<QDockWidget*>(existTab);
        dockList->append(subTabToAdd);
        if (dockList->count()>1){
            QDockWidget* base = dockList->at(0);
            for (int dock_i = 1; dock_i<dockList->count(); dock_i++){
                mainWindow->tabifyDockWidget(base,dockList->at(dock_i));
            }
        }
    }
    return 0;

}

QDockWidget* ResidualDemandResults::createGIFWidget(QWidget* parent, QString name, QString &dirName){
    QDockWidget* gifWidget = new QDockWidget("Congestion Level", parent);

    QWidget *congestionResultWidget = new QWidget(gifWidget);

    QGridLayout *layout = new QGridLayout();
    congestionResultWidget->setLayout(layout);

    QLabel* selectRlzLabel = new QLabel("Realization to plot: ");

    rlzSelectionComboBox = new QComboBox(this);

    residualDemandResultsFolder = dirName + QDir::separator() + "ResidualDemand";
    QDir dir(residualDemandResultsFolder);

    QString undamaged_gif_path = residualDemandResultsFolder + QDir::separator() + "undamaged" + QDir::separator() + "congestion.gif";
    QFileInfo undamaged_gif_fileInfo = QFileInfo(undamaged_gif_path);
    if (undamaged_gif_fileInfo.exists()){
        rlzSelectionComboBox->addItem("Undamaged");
    }
    // Set the filter to look only for directories
    dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    // Set the name filter to match folders starting with "workdir"
    QStringList nameFilter("workdir*");
    dir.setNameFilters(nameFilter);
    QStringList directories = dir.entryList();
    QStringList availableRlzs;
    for (const QString &directory : directories) {
        QStringList parts = directory.split('.');
        if (parts.count()!=2){
            this->errorMessage(QString("Can not add ") + directory + QString(" to ResidualDemand Results."));
        } else {
            availableRlzs.append(parts.at(1));
            rlzSelectionComboBox->addItem(parts.at(1));
        }
    }

    connect(rlzSelectionComboBox,&QComboBox::currentTextChanged, this, &ResidualDemandResults::congestionRlzSelectChanged);
    rlzSelectionComboBox->setCurrentIndex(0);

    // For testing add the first realization
    if (availableRlzs.count()==0){
        this->errorMessage(QString("No ResidualDemand realization results exist."));
        return gifWidget;
    }

    summaryDisplay = new QWidget(gifWidget);
    QGridLayout *summaryLayout = new QGridLayout();
    summaryDisplay->setLayout(summaryLayout);
    averageTravelTimeIncreaseLabel = new QLabel("Average travel time increase (s): ");
    averageTravelTimeIncreaseValue = new QLabel("-");
    averageTravelTimeIncreaseRatioLabel = new QLabel("Average travel time increase ratio: ");
    averageTravelTimeIncreaseRatioValue = new QLabel("-");
    summaryLayout->addWidget(averageTravelTimeIncreaseLabel, 0,0,1,1);
    summaryLayout->addWidget(averageTravelTimeIncreaseValue, 0,1,1,1);
    summaryLayout->addWidget(averageTravelTimeIncreaseRatioLabel, 0,2,1,1);
    summaryLayout->addWidget(averageTravelTimeIncreaseRatioValue, 0,3,1,1);

//    QMovie *gif = new QMovie(gifPath);
//    gif->start();
//    QLabel *gifDisplay = new QLabel;
//    gifDisplay->setMovie(gif);
//    gifDisplay->setScaledContents(true);

    gifDisplay = new SC_MovieWidget(this, undamaged_gif_path);


    layout->addWidget(selectRlzLabel, 0, 0, 1, 1);
    layout->addWidget(rlzSelectionComboBox, 0, 1, 1, 1);
    layout->addWidget(summaryDisplay, 1, 0, 1, 2);
    layout->addWidget(gifDisplay, 2, 0, 1, 2);
    layout->setColumnStretch(3,1);
    layout->setRowStretch(3,1);

    gifWidget->setMinimumWidth(475);
    gifWidget->setMaximumWidth(575);
    gifWidget->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);

    gifWidget->setWidget(congestionResultWidget);

    return gifWidget;

}

//int ResidualDemandResults::addResults(SC_ResultsWidget* resultsTab, QString &outputFile, QString &dirName,
//                                      QString &assetType, QList<QString> typesInAssetType){
//    //Initiate pointers from resultsTab
//    mainWindow = resultsTab->getMainWindow();
//    dockList = resultsTab->getDockList();
//    mapViewSubWidget = resultsTab->getMapViewSubWidget();
//    uiState = resultsTab->getUiState();
//    neededLayers = resultsTab->getNeededLayers();
//    // Add visualization
////    theVisualizationWidget = dynamic_cast<QGISVisualizationWidget*> (theVizWidget);
////    if (theVisualizationWidget == nullptr || theVisualizationWidget==0){
////        this->errorMessage("Can't convert to the visualization widget");
////        return -1;
////    }
////    // Create new GIS vis below:
////    theVisualizationWidget->addVectorLayer();
////    // Add the newly added layers to map displayed on the current resultsTab
////    QgsMapCanvas* mapCanvas = mapViewSubWidget->mapCanvas();
////    neededLayers.append();
////    resultsTab->neededLayers = neededLayers;
////    mapCanvas->setLayers(neededLayers);
////    // Create new GIS vis below
////    QDockWidget* residualDemandDockWidget = new QDockWidget("ResidualDemandResults",resultsTab->mainWindow);
////    residualDemandDockWidget->setMinimumWidth(475);
////    residualDemandDockWidget->setMaximumWidth(575);
////    residualDemandDockWidget->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Maximum);
////    mainWindow->addDockWidget(Qt::RightDockWidgetArea, residualDemandDockWidget);
////    dockList.append(residualDemandDockWidget);
////    if (dockList->count()>1){
////        QDockWidget* base = dockList->at(0);
////        for (int dock_i = 1; dock_i<dockList->count(); dock_i++){
////            mainWindow->tabifyDockWidget(base,dockList->at(dock_i));
////        }
////    }
////    // resize docks
////    QDockWidget* mapViewDock = mainWindow->findChild<QDockWidget*>("MapViewDock");
////    if(mapViewDock){
////        QList<QDockWidget*> alldocks = {mapViewDock};
////        float windowWidth = mainWindow->size().width();
////        float windowHeight = mainWindow->size().height();
////        QList<int> dockWidthes = {int(0.7*windowWidth)};
////        QList<int> dockHeights = {int(windowHeight)};
////        for (int dock_i = 0; dock_i<dockList->count(); dock_i++){
////            alldocks.append(dockList.at(dock_i));
////            dockWidthes.append(int(0.3*windowWidth));
////            dockHeights.append(int(0.7*windowHeight));
////        }
////        mainWindow->resizeDocks(alldocks, dockWidthes, Qt::Horizontal);
////        mainWindow->resizeDocks(alldocks, dockHeights, Qt::Vertical);
////    }
////    if(viewMenu){
////        viewMenu->addAction(residualDemandDockWidget->toggleViewAction());
////    }

////    uiState = mainWindow->saveState();
////    resultsTab->setUiState(uiState);
  //    return 0;

//}


void ResidualDemandResults::restoreUI(void)
{
    mainWindow->restoreState(uiState);
}




void ResidualDemandResults::clear(void)
{
    // See Pelicun3PostProcessor as an example
}

void ResidualDemandResults::congestionRlzSelectChanged(const QString &text){
    QString gifPath;
    QString meanTravelTimeIncrease = "-";
    QString meanTravelTimeIncreaseRatio = "-";
    if (text.compare("Undamaged")==0){
        gifPath = residualDemandResultsFolder + QDir::separator() + "undamaged" + QDir::separator() + "congestion.gif";
    } else {
        gifPath = residualDemandResultsFolder + QDir::separator() + "workdir." + text + QDir::separator() +
                          "damaged" + QDir::separator() + "congestion.gif";
        // Get the mean travel time increase
        QString tripInfoFile = residualDemandResultsFolder + QDir::separator() + "workdir." + text + QDir::separator() +
                                "trip_info_compare.csv";
        QFile file(tripInfoFile);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            this->errorMessage( "Unable to open file: " + tripInfoFile);
        } else {
            QTextStream in(&file);
            QVector<double> delayDurations;
            QVector<double> delayRatios;
            // Read the header to find column indices
            QString headerLine = in.readLine();
            QStringList headers = headerLine.split(",");
            int delayDurationIndex = headers.indexOf("delay_duration");
            int delayRatioIndex = headers.indexOf("delay_ratio");
            if (delayDurationIndex == -1 || delayRatioIndex == -1) {
                this->errorMessage( "Columns named \"delay_duration\" or \"delay_ratio\" not found in the CSV file");
            } else {
                // Read data lines
                while (!in.atEnd()) {
                    QString line = in.readLine();
                    QStringList values = line.split(",");

                    // Check if the line has enough columns
                    if (values.size() > qMax(delayDurationIndex, delayRatioIndex)) {
                        bool ok1, ok2;
                        double delayDuration = values[delayDurationIndex].toDouble(&ok1);
                        double delayRatio = values[delayRatioIndex].toDouble(&ok2);

                        // Add values to vectors if they are valid numbers
                        if (ok1) delayDurations.append(delayDuration);
                        if (ok2) delayRatios.append(delayRatio);
                    }
                }
                // Calculate mean values
                double meanDelayDuration = std::accumulate(delayDurations.begin(), delayDurations.end(), 0.0) / delayDurations.size();
                double meanDelayRatio = std::accumulate(delayRatios.begin(), delayRatios.end(), 0.0) / delayRatios.size();
                meanTravelTimeIncrease = QString::number(meanDelayDuration);
                meanTravelTimeIncreaseRatio = QString::number(meanDelayRatio);
            }
        }
    }
    bool updateSuccess = gifDisplay->updateGif(gifPath);
    if (!updateSuccess){
        this->errorMessage("Failed to display "+gifPath+".\n Check if the file exists");
    }

    this->averageTravelTimeIncreaseValue->setText(meanTravelTimeIncrease);
    this->averageTravelTimeIncreaseRatioValue->setText(meanTravelTimeIncreaseRatio);
}
