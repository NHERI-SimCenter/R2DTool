#ifndef RESIDUALDEMAND_H
#define RESIDUALDEMAND_H

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
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
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

// Written by: Jinyan Zhao, Sina Naeimi
#include "SimCenterAppWidget.h"

class VisualizationWidget;
class SC_DoubleLineEdit;
class SC_FileEdit;
class SC_DirEdit;
class SC_CheckBox;
class SC_ComboBox;
class SC_CheckBox;
class AssetInputDelegate;
class QPushButton;
class QLineEdit;
class SC_ComboBox;
class QLabel;
class QGroupBox;
class ResidualDemandResults;
class ResultsWidget;


class ResidualDemandWidget : public SimCenterAppWidget
{
    Q_OBJECT

public:
    ResidualDemandWidget(QWidget *parent = nullptr);
    ~ResidualDemandWidget();
//    bool inputFromJSON(QJsonObject &rvObject);
//    bool outputToJSON(QJsonObject &rvObject);
    bool outputAppDataToJSON(QJsonObject &jsonObject);
    bool inputAppDataFromJSON(QJsonObject &jsonObject);
    bool copyFiles(QString &dirName);
    bool outputCitation(QJsonObject &citation);
    SC_ResultsWidget* getResultsWidget(QWidget *parent, QWidget *R2DresWidget, QMap<QString, QList<QString>> assetTypeToType);

public slots:
    void clear(void);

signals:

protected:

private slots:
  void handleInputTypeChanged(void);
    void togglePostEventODFileEdit(int state);
    void selectComponents(void);
private:

//SC_DirEdit *damageStateDataSrource;
AssetInputDelegate *realizationInputWidget;
SC_DirEdit *resultsDir;
SC_FileEdit *pathEdgesFile;
SC_FileEdit *pathNodesFile;
SC_FileEdit *pathCapacityMapFile;
SC_FileEdit *pathODFilePre;
SC_FileEdit *pathODFilePost;
QLineEdit *simulationHourList;
SC_CheckBox *postEventODCheckBox;
SC_CheckBox *twoWayEdgeCheckbox;
SC_CheckBox *createAnimationCheckbox;
SC_ComboBox *damageInputMethodComboBox;
QPushButton *runButton;
QLabel *realizationToAnalyzeLabel;
QGroupBox* theGroupBox;
QString appInputPath;
QString appOutputPath;
ResidualDemandResults* theResidualDemandResultsWidget;
ResultsWidget* theR2DResultsWidget;

QLabel *postEventFileEditLable;

QString pathCapacityMap = "";
QString pathEdges = "";
QString pathNodes= "";
QString pathPreOD = "";
QString pathPostOD = "";

ResidualDemandResults *resultWidget;


};

#endif // RESIDUALDEMAND_H
