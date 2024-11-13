#ifndef RESIDUALDEMANDRESULTS_H
#define RESIDUALDEMANDRESULTS_H
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

#include "ComponentDatabase.h"
#include "SC_ResultsWidget.h"
#include "SimCenterMapcanvasWidget.h"

#include <QString>
#include <QMainWindow>
#include <QJsonArray>

class QVBoxLayout;
class QGISVisualizationWidget;
class QDockWidget;
class SC_MovieWidget;
class QComboBox;


class ResidualDemandResults : public SC_ResultsWidget
{
    Q_OBJECT

public:
    ResidualDemandResults(QWidget *parent);

    void clear(void);

    int processResults(QString &outputFile, QString &dirName, QString &assetType,
                       QList<QString> typesInAssetType);

    int addResultTab(QString tabName, QString &dirName);
    int addResultSubtab(QString name, QWidget* existTab, QString &dirName);


private slots:

    void restoreUI(void);
    void congestionRlzSelectChanged(const QString &text);

protected:

private:
    QDockWidget* createGIFWidget(QWidget* parent, QString name, QString &dirName);
    QGISVisualizationWidget* theVisualizationWidget;
    QComboBox* rlzSelectionComboBox;
    SC_MovieWidget* gifDisplay;
    QString residualDemandResultsFolder;
    QWidget* summaryDisplay;
    QLabel* averageTravelTimeIncreaseLabel;
    QLabel* averageTravelTimeIncreaseValue;
    QLabel* averageTravelTimeIncreaseRatioLabel;
    QLabel* averageTravelTimeIncreaseRatioValue;
};

#endif // RESIDUALDEMANDRESULTS_H
