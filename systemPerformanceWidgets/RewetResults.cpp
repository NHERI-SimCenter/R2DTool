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

// Written by: Sina Naeimi, Jinyan Zhao

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
#include <QJsonObject>
#include <QtCharts/QChart>
#include <QToolTip>
#include <QColor>


#include <qgsattributes.h>
#include <qgsmapcanvas.h>
#include <SC_Chart.h>
#include <SC_TimeSeriesResultChart.h>


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
        QDockWidget* rewetDockWidget = new QDockWidget(name, this);
        rewetDockWidget->setObjectName(name + "REWETDock");
        // Create the table that will show the Component information
        
        existingResult->addResultSubtab(QString("rewetDock"), rewetDockWidget, dirName);\

        //  Assign the layout to the dock widget
        QWidget *rewetResultWidget = new QWidget(rewetDockWidget);
        QVBoxLayout* rewetWidgetLayout = new QVBoxLayout();
        //rewetWidgetLayout->addStretch(0);
        rewetResultWidget->setLayout(rewetWidgetLayout);
        rewetDockWidget->setWidget(rewetResultWidget);

        // Read timeseries data
        QString subtypeName = name.replace(" ", "");
        QString fileName = dirName + QDir::separator() + subtypeName + QDir::separator() + subtypeName + "_timeseries.json";
        // Read the JSON file
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            qDebug() << "rewetResults - Failed to open file: " << fileName;
            return -1;
        }

        QByteArray jsonData = file.readAll();
        file.close();

        // Parse the JSON data
        QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
        if (jsonDoc.isNull()) {
            qDebug() << "rewetResults - Failed to parse " + fileName;
            return -1;
        }

        // Create JSON object and read data from JSON file

        // Read the JSON data
        QJsonObject jsonObj = jsonDoc.object();
        

        // Create a map to store the series. The first map is for the metrics, the second map is for the realizations
        allSeiries = new QMap<QString, QMap<QString, QLineSeries *>>();
        // Read series data and store them in the map
        int ret_value =  extractDataFramJSON(jsonObj, allSeiries);

        // Set titles for the chart
        // QMap<QString, QString> titles;
        // titles.insert("title", "Network Performace");
        // titles.insert("hAxis", "time");
        // titles.insert("vAxis", "Percent");    

        // sCreate SC_TimeSeries Widget
        chart = new SC_TimeSeriesResultChart(allSeiries, titles, rewetResultWidget);
        // Add mean and 90th percentile lines
        chart->addMean("Mean", QColor("Black"), Qt::SolidLine, 3);
        chart->addPercentile("90th Percentile", 0.95, QColor("red"), Qt::CustomDashLine, 3);
        // Add the chart to the layout
        rewetWidgetLayout->addWidget(chart);

        return ret_value;
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

int RewetResults::extractDataFramJSON(QJsonObject jsonObj, QMap<QString, QMap<QString, QLineSeries *>> *allSeiries){

    // Check if the JSON object contains the key "Result"
    if (!jsonObj.contains("Result") || !jsonObj["Result"].isObject()) {
        qDebug() << "Result key does not exist.";
        return -1;
    }
    // Read the value of the key "Result"
    QJsonObject resultObj = jsonObj["Result"].toObject();
    
    for (auto metricsIT = resultObj.begin(); metricsIT != resultObj.end(); ++metricsIT){

        QString metricKey = metricsIT.key();
        // Check if the QN exists in
        if (!resultObj.contains(metricKey) || !resultObj[metricKey].isObject()) {
            qDebug() << metricKey + " key does not exist.";
            return -1;
            
        }
            // Read the value of the MetricKey
        QJsonObject metricObject = resultObj[metricKey].toObject();
        if (!metricObject.contains("Data") || !metricObject["Data"].isObject()) {
            qDebug() << "Data does not exist in " + metricKey + " key does not exist.";
            return -1;  
        }

        QString metricName;

        // Check if the metricObject contains the key "Name"
        if (metricObject.contains("Name")){
            // Read the value of the key "Name"
            metricName = metricObject["Name"].toString();
        }
        // If the key "Name" does not exist, use the metricKey as the name
        else{
            metricName = metricKey;
        }
        
        metricObject = metricObject["Data"].toObject();

        // QMap<QString, QMap<QString, QLineSeries *>> *allSeiries = new QMap<QString, QMap<QString, QLineSeries *>>();

        QMap<QString, QLineSeries *> seriesMap;

        // Iterate over the QN values
        for (auto realizationIT = metricObject.begin(); realizationIT != metricObject.end(); ++realizationIT) {

            // Create a new QLineSeries        
            QLineSeries *series = new QLineSeries();
            
            // Get the key of the realization
            QString realizationKey = realizationIT.key();
            bool ConversionStatus;
            int realizationCounter = realizationKey.toInt(&ConversionStatus, 10);
            // Check if the conversion was successful
            if (!ConversionStatus){
                return -1;
            }
                                        
            // Get the value of the realization
            QJsonObject RealizationObject = metricObject[realizationKey].toObject();

            // Collect the keys into a list
            QStringList timeKeys = RealizationObject.keys();
            // Convert the keys to floats and store them in a list of pairs (float, QString)
            QList<QPair<float, QString>> timePairs;
            for (const QString &key : timeKeys) {
                bool ConversionStatus;
                float time = key.toFloat(&ConversionStatus);
                if (!ConversionStatus) {
                    qDebug() << "rewetResults - Failed to convert time to float: " + key;
                    return -1;
                }
                timePairs.append(qMakePair(time, key));
            }

            // Sort the list of pairs in descending order based on the float values
            std::sort(timePairs.begin(), timePairs.end(), [](const QPair<float, QString> &a, const QPair<float, QString> &b) {
                return a.first < b.first;
            });

            // Iterate over the sorted list of pairs
            for (const QPair<float, QString> &pair : timePairs) {
                float time = pair.first;
                QString timeValue = pair.second;

                // Get the value of the time
                QJsonValue metricValue = RealizationObject[timeValue];
            
                double metric;
                if (metricValue.isDouble()) {
                    metric = metricValue.toDouble();
                } else {
                    qDebug() << "rewetResults - metricValue is not double: " + QString::number(metric);
                    return -1;
                }

                series->append(time, metric * 100); // Sina added here to convert ratio to percent.
            }
            seriesMap[QString(realizationIT.key())] = series;
        }
        allSeiries->insert(metricName, seriesMap);

    }

    return 0;

}

void RewetResults::restoreUI(void)
{
    mainWindow->restoreState(uiState);
}


void RewetResults::clear(void)
{
    delete allSeiries;
    qDebug() << "RewetResults::clear()";
}



