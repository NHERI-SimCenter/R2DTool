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
#include "RewetResults.h"
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

#include <QGridLayout>

RewetResults::RewetResults(QWidget * parent)
  :SC_ResultsWidget(parent) {

  QGridLayout *layout = new QGridLayout();
  layout->addWidget(new QLabel("Hello World"), 0, 0);
  this->setLayout(layout);
}

int RewetResults::processResults(QString &outputFile, QString &dirName, QString &assetType,
				 QList<QString> typesInAssetType){
  
  qDebug() << "RewetResults::processResults() - should never be called!!";
  return -1;
}


int RewetResults::addResultTab(QString tabName, QString &dirName){
  
    QString resultFile = tabName + QString(".geojson");
    QString assetTypeSimplified = tabName.simplified().replace( " ", "" );
    R2DresWidget->getTabWidget()->addTab(this, tabName);
    return 0;
    // Do the results processing here
    //    this->processResults(resultFile, dirName, tabName, theAssetTypeToType[assetTypeSimplified]);
}

int RewetResults::addResultSubtab(QString name, QWidget* existTab, QString &dirName){

    SC_ResultsWidget* existingResult = dynamic_cast<SC_ResultsWidget*>(existTab);
    if (existingResult){ // Make a subtab and add to existing result tab
        // Do the visualizations
        // Make the sub
        // DL is always the first to be called, so no need to implement this
        QWidget* subTab = new QWidget(this);
        existingResult->addResultSubtab(QString("subTabName"), subTab, dirName);
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


/*
int RewetResults::addResults(SC_ResultsWidget* resultsTab,
			     QString &outputFile,
			     QString &dirName,
			     QString &assetType,
			     QList<QString> typesInAssetType){

  //
  // Initiate pointers from resultsTab
  //
  
  mainWindow = resultsTab->getMainWindow();
  dockList = resultsTab->getDockList();
  mapViewSubWidget = resultsTab->getMapViewSubWidget();
  uiState = resultsTab->getUiState();
  neededLayers = resultsTab->getNeededLayers();
  
  qDebug() << "outputFile: " << outputFile;
  qDebug() << "dirName: " << dirName;    
  qDebug() << "assetType: " << assetType;
  qDebug() << "typesInAssetType: " << typesInAssetType;    
  
  return true;
}
*/


void RewetResults::restoreUI(void)
{
    mainWindow->restoreState(uiState);
}




void RewetResults::clear(void)
{

}

