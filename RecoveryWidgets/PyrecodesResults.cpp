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

PyrecodesResults::PyrecodesResults(QWidget * parent)
  :SC_ResultsWidget(parent) {

  // basic widget is a QTabbedWidget
  
  QTabWidget *tabWidget = new QTabWidget();
  QVBoxLayout *mainLayout = new QVBoxLayout();
  mainLayout->addWidget(tabWidget);
  this->setLayout(mainLayout);
  
  //
  // demand-supply
  //
  
  QWidget *supplyWidget = new QWidget();
  QGridLayout *supplyLayout = new QGridLayout();
  supplyWidget->setLayout(supplyLayout);

  //FMK  chart = new SC_TimeSeriesResultChart(&allSeries, this);
  chart = new SC_MultipleLineChart(this);  
  supplyLayout->addWidget(chart, 0, 0);
  tabWidget->addTab(supplyWidget, "Supply Curves");

  //
  // recovery gifs
  //

  QWidget *gifWidget = new QWidget();
  QGridLayout *gifLayout = new QGridLayout();
  gifWidget->setLayout(gifLayout);
  gifComboBox = new QComboBox();
  //  gifStackedWidget = new QStackedWidget();
  movieWidget = new SC_MovieWidget(this,"", true);
  
  gifLayout->addWidget(new QLabel("Select Realization:"),0,0);
  gifLayout->addWidget(gifComboBox,0,1);
  gifLayout->addWidget(movieWidget,1,0,1,3);
  gifLayout->setColumnStretch(2,1);
  tabWidget->addTab(gifWidget, "Recovery GIFs");  

  /* ORIGINAL
  gifWidget->setMinimumWidth(475);
  gifWidget->setMaximumWidth(575);
  gifWidget->setMinimumHeight(475);
  gifWidget->setMaximumHeight(575);    
  gifWidget->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
  // Connect QComboBox to change the current index of QStackedWidget
  // connect(gifComboBox, SIGNAL(currentIndexChanged(int)),
  //	  gifStackedWidget, SLOT(setCurrentIndex(int)) );
  */

  connect(gifComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
    if (index >= 0 && index < gifFilenames.size()) // the -1 for when combo is invoked with clear(), sets index to -1
      movieWidget->updateGif(gifFilenames[index]);  
  });
}

int PyrecodesResults::processResults(QString &outputDirPath)
{
  //
  // clear old results
  //
  
  // FMK allSeries.clear();
  gifFilenames.clear();  
  gifComboBox->clear();

  // gifStackedWidget->clear(); // imagine no clear method at leats in 5.15 !!
  /*
  while (gifStackedWidget->count() > 0) {
    QWidget *widget = gifStackedWidget->widget(0); // Get the first widget
    gifStackedWidget->removeWidget(widget);        // Remove it from QStackedWidget
    delete widget;                              // Delete the widget to free memory
  }
  qDebug() << "STACKED WIDGETS DONE";
  */
  
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
  QStringList resourceList;
  
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
  
  qDebug() << "PyrecodesResults:: work_directories: " << work_directories;
  qDebug() << "PyrecodesResults:: resourceList: " << resourceList;

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
      QLineSeries *lineSeries = new QLineSeries();
      QString fileName = workDir.absoluteFilePath(fileList[resourceCounter]);

      readDemandSupplyJSON(fileName, lineSeries);

      SC_MLC_ChartData *resourceChartData = multipleLineChartData[resource];
      lineSeries->setPen(QPen(Qt::blue));
      resourceChartData->theLines.append(lineSeries);

      
      resourceCounter++;
    }

    //
    // create a gif
    //

    gifComboBox->addItem(work_directory);
    gifFilenames.append(workDir.absoluteFilePath("system_recovery.gif"));

    /*
    QLabel *gifLabel = new QLabel("No system_recovery.gif file found in workdir");
    gifLabel->setWindowTitle("Animated GIF Viewer");
    gifLabel->resize(400, 300);
    gifLabel->setAlignment(Qt::AlignCenter);
    QString gifFilename = workDir.absoluteFilePath("system_recovery.gif");
    SC_MovieWidget *movie = new SC_QMovieWidget(this, gifFilename);
    gifLabel->setMovie(movie);

    QMovie *movie = new QMovie(gifFilename);
    movie->start();
    
    if (!movie->isValid()) {
      qDebug() << "Failed to load the GIF file!" << gifFilename;
    } else {
      qDebug() << "LOADED AND DISPLAYING  GIF file!" << gifFilename;      
      gifLabel->setMovie(movie);
    }
    gifStackedWidget->addWidget(gifLabel);    
    */
    
    counter++;
    workDir.cdUp();
    
  }

  chart->setData(&multipleLineChartData);

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
  
  return 0;
}


int
PyrecodesResults::readDemandSupplyJSON(QString &filename, QtCharts::QLineSeries *lineSeries) {

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

  // Check for matching array sizes
  if (timeSteps.size() != supply.size() || timeSteps.size() != consumption.size()) {
    qDebug() << "PyrecodesResults:: readDataFROmJSON: Array sizes do not match for file: " << filename;
    return -1;
  }

  // loop over arrays and append to line series
  for (int i = 0; i < timeSteps.size(); ++i) {
    int timeStep = timeSteps[i].toInt();
    double supplyValue = supply[i].toDouble();
    // double consumptionValue = consumption[i].toDouble();
    if (i == 0)
      lineSeries->append(-1, supplyValue); // add a point on line to indicate day before event same as day 0
    lineSeries->append(timeStep, supplyValue);    
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

	this->processResults(dirName);
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



