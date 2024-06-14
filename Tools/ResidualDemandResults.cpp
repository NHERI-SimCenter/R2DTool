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



#include <qgsattributes.h>
#include <qgsmapcanvas.h>

// Test to remove start
// #include <chrono>
// using namespace std::chrono;
// Test to remove end

using namespace QtCharts;

ResidualDemandResults::ResidualDemandResults(QWidget * parent) : SC_ResultsWidget(parent){

}

int ResidualDemandResults::processResults(QString &outputFile, QString &dirName, QString &assetType,
                                          QList<QString> typesInAssetType){

    // If no results tab for Transportation exists
    // Initiate the qmainwindow
    // Create visualization
    // Create table widget
    // See Pelicun3PostProcessor as an example
}

int ResidualDemandResults::addResults(SC_ResultsWidget* resultsTab, QString &outputFile, QString &dirName,
                                      QString &assetType, QList<QString> typesInAssetType){
    //Initiate pointers from resultsTab
    mainWindow = resultsTab->getMainWindow();
    dockList = resultsTab->getDockList();
    mapViewSubWidget = resultsTab->getMapViewSubWidget();
    uiState = resultsTab->getUiState();
    neededLayers = resultsTab->getNeededLayers();
    // Add visualization
//    theVisualizationWidget = dynamic_cast<QGISVisualizationWidget*> (theVizWidget);
//    if (theVisualizationWidget == nullptr || theVisualizationWidget==0){
//        this->errorMessage("Can't convert to the visualization widget");
//        return -1;
//    }
//    // Create new GIS vis below:
//    theVisualizationWidget->addVectorLayer();
//    // Add the newly added layers to map displayed on the current resultsTab
//    QgsMapCanvas* mapCanvas = mapViewSubWidget->mapCanvas();
//    neededLayers.append();
//    resultsTab->neededLayers = neededLayers;
//    mapCanvas->setLayers(neededLayers);
//    // Create new GIS vis below
//    QDockWidget* residualDemandDockWidget = new QDockWidget("ResidualDemandResults",resultsTab->mainWindow);
//    residualDemandDockWidget->setMinimumWidth(475);
//    residualDemandDockWidget->setMaximumWidth(575);
//    residualDemandDockWidget->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Maximum);
//    mainWindow->addDockWidget(Qt::RightDockWidgetArea, residualDemandDockWidget);
//    dockList.append(residualDemandDockWidget);
//    if (dockList->count()>1){
//        QDockWidget* base = dockList->at(0);
//        for (int dock_i = 1; dock_i<dockList->count(); dock_i++){
//            mainWindow->tabifyDockWidget(base,dockList->at(dock_i));
//        }
//    }
//    // resize docks
//    QDockWidget* mapViewDock = mainWindow->findChild<QDockWidget*>("MapViewDock");
//    if(mapViewDock){
//        QList<QDockWidget*> alldocks = {mapViewDock};
//        float windowWidth = mainWindow->size().width();
//        float windowHeight = mainWindow->size().height();
//        QList<int> dockWidthes = {int(0.7*windowWidth)};
//        QList<int> dockHeights = {int(windowHeight)};
//        for (int dock_i = 0; dock_i<dockList->count(); dock_i++){
//            alldocks.append(dockList.at(dock_i));
//            dockWidthes.append(int(0.3*windowWidth));
//            dockHeights.append(int(0.7*windowHeight));
//        }
//        mainWindow->resizeDocks(alldocks, dockWidthes, Qt::Horizontal);
//        mainWindow->resizeDocks(alldocks, dockHeights, Qt::Vertical);
//    }
//    if(viewMenu){
//        viewMenu->addAction(residualDemandDockWidget->toggleViewAction());
//    }

//    uiState = mainWindow->saveState();
//    resultsTab->setUiState(uiState);
    return true;

}


void ResidualDemandResults::restoreUI(void)
{
    mainWindow->restoreState(uiState);
}




void ResidualDemandResults::clear(void)
{
    // See Pelicun3PostProcessor as an example
}

