/* *****************************************************************************
Copyright (c) 2016-2017, The Regents of the University of California (Regents).
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

// Written: fmckenna

#include "DL_TableWidget.h"
#include <QMouseEvent>
#include <QDebug>
#include <QHeaderView>
#include <QAction>
#include <QMenu>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QVBoxLayout>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QFileDialog>

DL_TableWidget::DL_TableWidget(QWidget *parent)
    :QTableWidget(parent),mLeft(true)
{

  this->setSelectionBehavior(QAbstractItemView::SelectColumns);
  connect(this,SIGNAL(cellPressed(int,int)),this,SLOT(onSpreadsheetCellClicked(int,int)));  
}

DL_TableWidget::~DL_TableWidget()
{

}

void DL_TableWidget::mousePressEvent(QMouseEvent *event)
{
    // keep track of which button pressed
    if(event->button() == Qt::LeftButton)
        mLeft = true;                                // bool m_isLeftClick; is class member
    else if (event->button() == Qt::RightButton)
        mLeft = false;

    // call base class
    this->QTableWidget::mousePressEvent(event);
}

bool
DL_TableWidget::wasLeftKeyPressed(void)
{
    return mLeft;
}

void DL_TableWidget::onSpreadsheetCellClicked(int row, int col) {
  QMenu menu;

  QAction *action1 = menu.addAction("Summary Statistics");
  QAction *action2 = menu.addAction("Show Histogram");
  menu.addSeparator();
  QAction *action3 = menu.addAction("Order (Descending)");
  QAction *action4 = menu.addAction("Order (Ascending)");
  menu.addSeparator();  
  QAction *action5 = menu.addAction("Save Table as CSV");  
  
  QAction *selectedAction = menu.exec(QCursor::pos());
  if (selectedAction == action1) {
    showStatistics(col);
  } else if (selectedAction == action2) {
    showHistogram(col);
  } else if (selectedAction == action3) {
    this->sortItems(col, Qt::DescendingOrder);
  } else if (selectedAction == action4) {
    this->sortItems(col, Qt::AscendingOrder);    
  } else if (selectedAction == action5) {
    saveTableToCSV(col);
  }
}

void DL_TableWidget::showHistogram(int col)
{
    using namespace QtCharts;
    
    int rowCount = this->rowCount();
    if (rowCount <= 0)
      return;

    double NUM_DIVISIONS_FOR_DIVISION = ceil(sqrt(rowCount));
    if (NUM_DIVISIONS_FOR_DIVISION < 10)
      NUM_DIVISIONS_FOR_DIVISION = 10;
    else if (NUM_DIVISIONS_FOR_DIVISION > 20)
      NUM_DIVISIONS_FOR_DIVISION = 20;
    
    int NUM_DIVISIONS = NUM_DIVISIONS_FOR_DIVISION;

    double *dataValues = new double[rowCount];
    double histogram[20];
    for (int i=0; i<NUM_DIVISIONS; i++)
      histogram[i] = 0;    

    double min = 0;
    double max = 0;
    int n_eff = 0;    
    for (int i=0; i<rowCount; i++) {
      QTableWidgetItem *itemX = this->item(i,col);      
      double value = itemX->text().toDouble();
      dataValues[i] =  value;
      if (i == 0) {
	min = value;
	max = value;
      } else if (value < min) {
	min = value;
      } else if (value > max) {
	max = value;
      }
      
      n_eff+=1;
    }
    // if constant
    if (min==max) {
      double axisMargin=abs(min)*0.1;
      min=min-axisMargin;
      max=max+axisMargin;
    }

    if (col == 1) {
      min = -.5;
      max = 4.5;
      NUM_DIVISIONS_FOR_DIVISION = 5;
      NUM_DIVISIONS=5;
    }
    
    double range = max-min;
    double dRange = range/NUM_DIVISIONS_FOR_DIVISION;    

    for (int i=0; i<rowCount; i++) {
      // compute block belongs to, watch under and overflow due to numerics
      int block = floor((dataValues[i]-min)/dRange);
      if (block < 0) block = 0;
      if (block > NUM_DIVISIONS-1) block = NUM_DIVISIONS-1;
      histogram[block] += 1;
    }
    QLineSeries *series = new QLineSeries();    
    for (int i=0; i<NUM_DIVISIONS; i++) {
      series->append(min+i*dRange, 0);
      series->append(min+i*dRange, histogram[i]);
      series->append(min+(i+1)*dRange, histogram[i]);
      series->append(min+(i+1)*dRange, 0);
    }    

    // Create the chart and add the series
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->createDefaultAxes();
    //chart->setTitle("Histogram!");

    if (col == 1) { // for Damage State
      
      // Set explicit x-axis as ValueAxis (not CategoryAxis)
      QValueAxis *axisX = new QValueAxis();
      axisX->setRange(-0.5, 4.5);  // So you can see full width for 0 and 4
      axisX->setTickType(QValueAxis::TicksDynamic);
      axisX->setTickAnchor(0.0);   // anchor at 0
      axisX->setTickInterval(1.0); // tick every integer
      axisX->setLabelFormat("%d"); // integer labels
      
      // Optionally fine tune
      axisX->setMinorTickCount(0);
      
      QValueAxis *axisY = new QValueAxis();
      axisY->setTitleText("Frequency");
      axisY->applyNiceNumbers();
      axisY->setLabelFormat("%d");
      axisY->setMinorTickCount(0);
      
      // Attach axes
      chart->setAxisX(axisX, series);
      chart->setAxisY(axisY, series);
    }
    
    // Create a ChartView and show it in a window
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    // Create a window to display the chart
    QDialog *dialog = new QDialog(this); // Pass 'this' to parent it properly
    dialog->setWindowTitle("Histogram");
 
    QVBoxLayout *layout = new QVBoxLayout(dialog);
    layout->addWidget(chartView);
    dialog->resize(400, 300);
    dialog->exec();

    delete [] dataValues;
}

void DL_TableWidget::showStatistics(int col)
{
    int rowCount = this->rowCount();
    if (rowCount <= 0)
      return;

    double min = 0;
    double max = 0;
    double sum = 0;
    double *dataValues = new double[rowCount];    
    int n_eff = 0;    
    for (int i=0; i<rowCount; i++) {
      QTableWidgetItem *itemX = this->item(i,col);
      bool ok;
      double value = itemX->text().toDouble(&ok);
      // it not int or double, return as cannot do stats
      if (!ok) {
	delete [] dataValues;
	return;
      }
	
      dataValues[i] =  value;
      if (i == 0) {
	min = value;
	max = value;
      } else if (value < min) {
	min = value;
      } else if (value > max) {
	max = value;
      }
      sum += value;
    }

    // mean
    double mean = sum/rowCount;

    // stdev    
    double stdDev = 0.0;
    for (int i=0; i<rowCount; i++)
      stdDev += (dataValues[i] - mean) * (dataValues[i] - mean);
    
    stdDev = sqrt(stdDev/(rowCount-1));
      
    // Create a window to display the stats
    
    QDialog *dialog = new QDialog(this); // Pass 'this' to parent it properly
    dialog->setWindowTitle("Statistics");
 
    QGridLayout *layout = new QGridLayout(dialog);
    int numRow = 0;
    layout->addWidget(new QLabel("Num Entries: "), numRow,0);
    QLineEdit *numEdit = new QLineEdit();
    numEdit->setText(QString::number(rowCount));
    numEdit->setReadOnly(true);
    layout->addWidget(numEdit, numRow,1);
    
    layout->addWidget(new QLabel("Min: "),  ++numRow,0);
    QLineEdit *minEdit = new QLineEdit();
    minEdit->setText(QString::number(min));
    minEdit->setReadOnly(true);
    layout->addWidget(minEdit, numRow, 1);
    
    layout->addWidget(new QLabel("Max: "),  ++numRow,0);
    QLineEdit *maxEdit = new QLineEdit();
    maxEdit->setText(QString::number(max));
    maxEdit->setReadOnly(true);
    layout->addWidget(maxEdit, numRow, 1);

    layout->addWidget(new QLabel("Mean: "), ++numRow,0);
    QLineEdit *meanEdit = new QLineEdit();
    meanEdit->setText(QString::number(mean));
    meanEdit->setReadOnly(true);
    layout->addWidget(meanEdit, numRow, 1);

    layout->addWidget(new QLabel("std Deviation: "), ++numRow,0);
    QLineEdit *devEdit = new QLineEdit();
    devEdit->setText(QString::number(stdDev));
    devEdit->setReadOnly(true);
    layout->addWidget(devEdit, numRow, 1);    
    
    layout->addWidget(new QLabel("Sum: "),  ++numRow,0);
    QLineEdit *sumEdit = new QLineEdit();
    sumEdit->setText(QString::number(sum));
    sumEdit->setReadOnly(true);
    layout->addWidget(sumEdit, numRow, 1);    
		      
    dialog->resize(400, 300);
    dialog->exec();

    delete [] dataValues;
    
}

void DL_TableWidget::saveTableToCSV(int col)
{
  //
  // get name of a file to save data to
  //
  
  QString fileName = QFileDialog::getSaveFileName(this, "Save CSV", "", "CSV Files (*.csv)");
  if (fileName.isEmpty())
    return;

  //
  // open file
  //
  
  QFile file(fileName);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    QMessageBox::warning(this, "Error", "Cannot open file for writing");
    return;
  }

  //
  // write to it
  //
  
  QTextStream out(&file);
  
  // Write headers
  QStringList headers;
  for (int col = 0; col < this->columnCount(); ++col) {
    headers << this->horizontalHeaderItem(col)->text();
  }
  out << headers.join(",") << "\n";
  
  // Write rows
  for (int row = 0; row < this->rowCount(); ++row) {
    QStringList rowValues;
    for (int col = 0; col < this->columnCount(); ++col) {
      QTableWidgetItem *item = this->item(row, col);
      rowValues << (item ? item->text() : "");
    }
    out << rowValues.join(",") << "\n";
  }
  
  file.close();
}


DL_TableWidget::ColType DL_TableWidget::detectColType(int col) {
  
  uniqueValues.clear();
  
  bool allInt = true;
  bool allFloat = true;
  bool allStrings = true;
  
  for (int row = 0; col < this->rowCount(); row++) {
    QTableWidgetItem *item = this->item(row, col);
    if (!item) return ColType::Mixed;  // Treat missing as mixed & just return
    
    QString text = item->text().trimmed();
    uniqueValues.insert(text);
    
    // Check for int
    bool okInt = false;
    text.toInt(&okInt);
    allInt = allInt && okInt;
    
    // Check for float
    bool okFloat = false;
    text.toDouble(&okFloat);
    allFloat = allFloat && okFloat;
    
    if (!okFloat && !okInt)
      return ColType::Strings;  // early out for non-numeric
  }
  
  if (allInt) return ColType::Ints;
  if (allFloat) return ColType::Floats;
  
  return ColType::Mixed;
}
