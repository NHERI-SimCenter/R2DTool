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

// Written by: fmk

#include "PyrecodesResults.h"
#include "VisualizationWidget.h"
#include "QGISVisualizationWidget.h"

#include <QMap>
#include <QComboBox>
#include <QLineSeries>
#include <QDir>
#include <QFileInfo>
#include <QApplication>
#include <SC_MovieWidget.h>
#include <QPen>


#include <SC_MultipleLineChart.h>

using namespace QtCharts;

#include <QGridLayout>

PyrecodesResults::PyrecodesResults(QWidget * parent, bool dockable)
  :SC_ResultsWidget(parent) {

  //
  // demand-supply
  //
  
  QWidget *supplyWidget = new QWidget();
  QGridLayout *supplyLayout = new QGridLayout();
  supplyWidget->setLayout(supplyLayout);

  supplyChart = new SC_MultipleLineChart(this);  
  supplyLayout->addWidget(supplyChart, 0, 0);

  //
  // recovery gifs
  //

  QWidget *gifWidget = new QWidget();
  QGridLayout *gifLayout = new QGridLayout();
  gifWidget->setLayout(gifLayout);
  gifComboBox = new QComboBox();
  movieWidget = new SC_MovieWidget(this,"", true);
  
  gifLayout->addWidget(new QLabel("Select Realization:"),0,0);
  gifLayout->addWidget(gifComboBox,0,1);
  gifLayout->addWidget(movieWidget,1,0,1,3);
  gifLayout->setColumnStretch(2,1);

  connect(gifComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
    if (index >= 0 && index < gifFilenames.size()) // the -1 for when combo is invoked with clear(), sets index to -1
      movieWidget->updateGif(gifFilenames[index]);  
  });

  //
  // supply demand
  //

  QWidget *sdWidget = new QWidget();
  QGridLayout *sdLayout = new QGridLayout();
  sdWidget->setLayout(sdLayout);
  sdComboBox = new QComboBox();
  supplyDemandChart = new SC_MultipleLineChart(this); 
  
  sdLayout->addWidget(new QLabel("Select Realization:"),0,0);
  sdLayout->addWidget(sdComboBox,0,1);
  sdLayout->addWidget(supplyDemandChart,1,0,1,3);
  sdLayout->setRowStretch(1,1);

  connect(sdComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
    if (index >= 0 && index < sdWorkDirs.size()) { // the -1 for when combo is invoked with clear(), sets index to -1
      //      qDebug() << "PROCESS SUPPLY DEMAND: " << sdWorkDirs[index];
      this->processSupplyDemandUpdate(sdWorkDirs[index]);
    }
  });  

  
  if (dockable == true) {

    //
    // use QDockWidgets for interface
    //

    layout = new QVBoxLayout(this);
    mainWindow = new QMainWindow(parent);
    mainWindow->show();
    layout->addWidget(mainWindow);

    QTabWidget *tabWidget = new QTabWidget();
    QVBoxLayout *tabLayout = new QVBoxLayout();
    tabLayout->addWidget(tabWidget);

    tabWidget->addTab(supplyWidget, "Supply Curves");
    tabWidget->addTab(sdWidget, "Supply Demand Curves");    
    tabWidget->addTab(gifWidget, "Recovery GIFs");
    
    QDockWidget *tabDockWidget = new QDockWidget("Recovery", mainWindow);
    tabDockWidget->setWidget(tabWidget);    
    
    mainWindow->addDockWidget(Qt::LeftDockWidgetArea, tabDockWidget);
    this->setLayout(layout);

    /* ************** two dockable widgets ******************************
    layout = new QVBoxLayout(this);
    mainWindow = new QMainWindow(parent);
    mainWindow->show();
    layout->addWidget(mainWindow);
    
    curveDockWidget = new QDockWidget("Supply Curves", mainWindow);
    gifDockWidget = new QDockWidget("Recovery GIFs", mainWindow);

    gifDockWidget->setWidget(gifWidget);    
    curveDockWidget->setWidget(supplyWidget);
    
    mainWindow->addDockWidget(Qt::LeftDockWidgetArea, curveDockWidget);
    mainWindow->addDockWidget(Qt::RightDockWidgetArea, gifDockWidget);
    
    this->setLayout(layout);
    ******************************************************************** */
    
  } else {

    //
    // use QTabWidget for interface
    //
  
    QTabWidget *tabWidget = new QTabWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addWidget(tabWidget);

    tabWidget->addTab(supplyWidget, "Supply Curves");
    tabWidget->addTab(sdWidget, "Supply Demand Curves");    
    tabWidget->addTab(gifWidget, "Recovery GIFs");
    
    this->setLayout(mainLayout);
  }
}

int
PyrecodesResults::processSupplyDemandUpdate(QString &workdirPath) {

  QDir workDir(workdirPath);
  QMap<QString, SC_MLC_ChartData *> *chartData = new QMap<QString, SC_MLC_ChartData *>;

  int resourceCounter = 0;  
  for (const QString &resource : resourceList) {

    QString fileName = workDir.filePath(resource + QString("_supply_demand_consumption.json"));
    SC_MLC_ChartData *resourceChartData = new SC_MLC_ChartData();    

    resourceChartData->xLabel = "Days";
    resourceChartData->yLabel = resource + QString(" ");
    resourceChartData->title =  resource + QString(" Supply Demand Consumption Curves");
    resourceChartData->showLegend = true;    
    chartData->insert(resource, resourceChartData);
    
    // add curves for resource
    QLineSeries *supplyLineSeries = new QLineSeries();
    QLineSeries *demandLineSeries = new QLineSeries();
    QLineSeries *consumptionLineSeries = new QLineSeries();      
    
    readDemandSupplyJSON(fileName, supplyLineSeries, demandLineSeries, consumptionLineSeries);
    supplyLineSeries->setName("Supply");
    demandLineSeries->setName("Demand");
    consumptionLineSeries->setName("Consumption");            

    consumptionLineSeries->setPen(QPen(Qt::yellow));
    resourceChartData->theLines.append(consumptionLineSeries);

    demandLineSeries->setPen(QPen(Qt::red));
    resourceChartData->theLines.append(demandLineSeries);
    
    supplyLineSeries->setPen(QPen(Qt::blue));
    resourceChartData->theLines.append(supplyLineSeries);

  }
  
  supplyDemandChart->setData(chartData);
}

int PyrecodesResults::processResults(QString &outputFile, QString &outputDirPath)
{
  //
  // clear old results
  //
  
  resourceList.clear();  
  gifFilenames.clear();  
  gifComboBox->clear();
  sdWorkDirs.clear();  
  sdComboBox->clear();  

  // cd to RecoveryResults dir

  QDir workDir(outputDirPath);
  workDir.cd("RecoverySimulation");

  //
  // get a list of all workdir there
  //   - for first dir returned get list of files ending in _supply_demand_consumption.json .. this is resources plotted
  //   - loop through all workdir building the data needed to plot all the results
  //   - plot them

  // list of workdir

  QStringList work_directories = workDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

  // get list of resources to be plotted & create list of QMap<Qstring, QMap<QString, QLineSeries *>>
  // & then to map add a new SC_MLC_ChartData for each resource

  workDir.cd(work_directories[0]);
  QStringList filters;
  filters << "*_supply_demand_consumption.json";
  QStringList fileList = workDir.entryList(filters, QDir::Files);

  QMap<QString, SC_MLC_ChartData *>multipleLineChartData;
    
  for (const QString &fileName : fileList) {
    
    // Remove the suffix to get resources list
    if (fileName.endsWith("_supply_demand_consumption.json")) {
      QString resource = fileName.left(fileName.length() - QString("_supply_demand_consumption.json").length());
      resourceList << resource;
      SC_MLC_ChartData *resourceLineData = new SC_MLC_ChartData();
      resourceLineData->xLabel = "Days";
      resourceLineData->yLabel = resource + QString(" Supply");
      resourceLineData->title =  resource + QString(" Supply Curve");
      multipleLineChartData.insert(resource, resourceLineData);
    }
  }
  
  workDir.cdUp();

  // loop foreach working dir

  int counter = 0;
  for (const QString &work_directory : work_directories) {

    workDir.cd(work_directory);
    
    //
    // foreach resource add supply line data
    //

    int resourceCounter = 0;
    for (const QString &resource: resourceList) {
	
      // add supply curve for resource
      QLineSeries *supplyLineSeries = new QLineSeries();
      //QLineSeries *demandLineSeries = new QLineSeries();
      //QLineSeries *consumptionLineSeries = new QLineSeries();      
      QString fileName = workDir.absoluteFilePath(fileList[resourceCounter]);

      readDemandSupplyJSON(fileName, supplyLineSeries);
      supplyLineSeries->setName(work_directory);

      SC_MLC_ChartData *resourceChartData = multipleLineChartData[resource];
      supplyLineSeries->setPen(QPen(Qt::blue));
      resourceChartData->theLines.append(supplyLineSeries);
      
      resourceCounter++;
    }

    //
    // create a gif
    //

    gifComboBox->addItem(work_directory);
    gifFilenames.append(workDir.absoluteFilePath("system_recovery.gif"));

    //
    // store info for workdirs
    //
    
    sdComboBox->addItem(work_directory);
    sdWorkDirs.append(workDir.absolutePath());    

    counter++;
    workDir.cdUp();
    
  }

  supplyChart->setData(&multipleLineChartData);

  /*
  if (counter > 5) {
    chart->addMean("Mean", QColor("Black"), Qt::SolidLine, 3);
    chart->addPercentile("90th Percentile", 0.95, QColor("red"), Qt::CustomDashLine, 3);
  }
  */

  if (gifFilenames.size() > 0) {
    movieWidget->updateGif(gifFilenames[0]);    
    gifComboBox->setCurrentIndex(0); // to get gif 0 updated
  }

  if (sdWorkDirs.size() >  0) {
    sdComboBox->setCurrentIndex(0);
    this->processSupplyDemandUpdate(sdWorkDirs[0]);
  }
  
  return 0;
}


int
PyrecodesResults::readDemandSupplyJSON(QString &filename,
				       QtCharts::QLineSeries *supplySeries,
				       QtCharts::QLineSeries *demandSeries,
				       QtCharts::QLineSeries *consumptionSeries) {

  //
  // Open the file in read-only mode & get JSON object
  //

  // open file
  QFile file(filename);
  if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
    QString message("PyrecodesResults Failed to open file specified"); message += filename;
    errorMessage(message);
    return -1;
  }
  
  // Read the file contents
  QByteArray fileData = file.readAll();
  
  // Parse the JSON document and get a JSON object
  QJsonDocument jsonDoc = QJsonDocument::fromJson(fileData);
  if (jsonDoc.isNull() || !jsonDoc.isObject()) { 
    errorMessage("PyrecodesResults: file specified is not in JSON format");
    return -1;
  }
  QJsonObject jsonObj = jsonDoc.object();

  //
  // now lets parse the JSON object for time and supply values & add to the QLineSeries
  //
  
  QJsonArray timeSteps = jsonObj["TimeStep"].toArray();
  QJsonArray supply = jsonObj["Supply"].toArray();
  QJsonArray consumption = jsonObj["Consumption"].toArray();
  QJsonArray demand = jsonObj["Demand"].toArray();  

  // Check for matching array sizes
  if (timeSteps.size() != supply.size() ||
      timeSteps.size() != consumption.size() ||
      timeSteps.size() != demand.size()) {
    qDebug() << "PyrecodesResults:: readDataFROmJSON: Array sizes do not match for file: " << filename;
    return -1;
  }

  // loop over arrays and append to line series
  for (int i = 0; i < timeSteps.size(); ++i) {
    int timeStep = timeSteps[i].toInt();
    double supplyValue = supply[i].toDouble();
    double consumptionValue = consumption[i].toDouble();
    double demandValue = demand[i].toDouble();    
    if (i == 0) {
      // add a point on line to indicate day before event same as day 0      
      supplySeries->append(-1, supplyValue);
      if (demandSeries != 0)
	demandSeries->append(-1, demandValue);
      if (consumptionSeries != 0)
	consumptionSeries->append(-1, consumptionValue);
    }
    supplySeries->append(timeStep, supplyValue);
    if (demandSeries != 0)    
      demandSeries->append(timeStep, demandValue);
    if (consumptionSeries != 0)    
      consumptionSeries->append(timeStep, consumptionValue); 
  }
  
  return 0;
}
      

int PyrecodesResults::addResultTab(QString tabName, QString &dirName){
  
    QString resultFile = tabName + QString(".geojson");
    QString assetTypeSimplified = tabName.simplified().replace( " ", "" );
    R2DresWidget->getTabWidget()->addTab(this, tabName);
    return 0;
}

int PyrecodesResults::addResultSubtab(QString name, QWidget* existTab, QString &dirName){

    SC_ResultsWidget* existingResult = dynamic_cast<SC_ResultsWidget*>(existTab);
    
    if (existingResult){ // Make a subtab and add to existing result tab
        // Do the visualizations
        // Make the sub
        // DL is always the first to be called, so no need to implement this
        QDockWidget* pyrecodesDockWidget = new QDockWidget(name, this);
        pyrecodesDockWidget->setObjectName(name + "PYRECODESDock");
        // Create the table that will show the Component information
        
        existingResult->addResultSubtab(QString("PyrecodesDock"), pyrecodesDockWidget, dirName);\

        //  Assign the layout to the dock widget
        QWidget *pyrecodesResultWidget = new QWidget(pyrecodesDockWidget);
        QVBoxLayout* pyrecodesWidgetLayout = new QVBoxLayout();
        //pyrecodesWidgetLayout->addStretch(0);
        pyrecodesResultWidget->setLayout(pyrecodesWidgetLayout);
        pyrecodesDockWidget->setWidget(pyrecodesResultWidget);

	QString blank;
	this->processResults(blank, dirName);
    }
    
    else{ //Add the subtab to docklist
      
        QDockWidget* subTabToAdd = dynamic_cast<QDockWidget*>(existTab);
        if (!subTabToAdd){
            qDebug() << "Adding a tab named " + name +" which is not a QDockWidget";
            return -1;
        }
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


void PyrecodesResults::restoreUI(void)
{
    mainWindow->restoreState(uiState);
}


void PyrecodesResults::clear(void)
{
  // allSeries.clear();
  qDebug() << "PyrecodesResults::clear()";
}



