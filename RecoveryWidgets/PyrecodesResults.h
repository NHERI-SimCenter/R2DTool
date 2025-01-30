#ifndef PYRECODES_RESULTS_H
#define PYRECODES_RESULTS_H

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

#include "SC_ResultsWidget.h"
#include "SimCenterMapcanvasWidget.h"

#include <QString>
#include <QMainWindow>
#include <QJsonArray>
#include <QJsonObject>

class QVBoxLayout;
class QGISVisualizationWidget;
class SC_MultipleLineChart;
class SC_MovieWidget;
class QComboBox;
class QStackedWidget;
class QPixmap;


namespace QtCharts
{
    class QChartView;
    class QBarSet;
    class QChart;
    class QLineSeries;
}


class PyrecodesResults : public SC_ResultsWidget
{
    Q_OBJECT

public:
    PyrecodesResults(QWidget *parent);

    void clear(void);

    int processResults(QString &outputFile, QString &outputDirPath);

    int addResultTab(QString tabName, QString &dirName);
    int addResultSubtab(QString name, QWidget* existTab, QString &dirName);

private slots:

    void restoreUI(void);

protected:

private:

  // methods
  int readDemandSupplyJSON(QString &filename, QtCharts::QLineSeries *lineSeries);

  // data
  
  QGISVisualizationWidget* theVisualizationWidget;

  // for supply curves
  SC_MultipleLineChart *chart;

  // for recovery gifs
  QComboBox *gifComboBox;
  SC_MovieWidget *movieWidget;
  QStringList gifFilenames;

  // for supply-demand curves  
  QComboBox *sdComboBox;
  QPixmap *sdPixmapWidget;
  QStringList sdFilenames;  
  
};

#endif // PYRECODES_RESULTS_H
