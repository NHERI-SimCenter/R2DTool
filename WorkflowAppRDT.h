#ifndef WORKFLOW_APP_RDT_H
#define WORKFLOW_APP_RDT_H

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

// Written by: Stevan Gavrilovic
// Latest revision: 10.08.2020

#include "WorkflowAppWidget.h"
#include "UQ_EngineSelection.h"

class AssetsWidget;
class HazardsWidget;
class EngDemandParameterWidget;
class DamageMeasureWidget;
class DecisionVariableWidget;
class VisualizationWidget;
class GeneralInformationWidget;
class AssetsModelWidget;
class Application;
class InputWidgetUQ;
class QNetworkAccessManager;
class QNetworkReply;
class QStackedWidget;
class RandomVariablesContainer;
class RemoteJobManager;
class RemoteService;
class ResultsWidget;
class RunLocalWidget;
class RunWidget;
class RegionalMappingWidget;
class SimCenterComponentSelection;
class UQOptions;
class UQ_Results;

class WorkflowAppRDT : public WorkflowAppWidget
{
    Q_OBJECT
public:
    explicit WorkflowAppRDT(RemoteService *theService, QWidget *parent = 0);
    ~WorkflowAppRDT();
    static WorkflowAppRDT *theInstance;

    static WorkflowAppRDT *getInstance(void);

    bool outputToJSON(QJsonObject &rvObject);
    bool inputFromJSON(QJsonObject &rvObject);
    void clear(void);

    void onRunButtonClicked();
    void onRemoteRunButtonClicked();
    void onRemoteGetButtonClicked();
    void onExitButtonClicked();
    int getMaxNumParallelTasks();
    
    void setActiveWidget(SimCenterAppWidget* widget);

    VisualizationWidget *getVisualizationWidget() const;
    AssetsWidget *getAssetsWidget() const;
    HazardsWidget *getHazardsWidget() const;
    GeneralInformationWidget *getGeneralInformationWidget() const;
    RegionalMappingWidget *getTheRegionalMappingWidget() const;
    DamageMeasureWidget *getTheDamageMeasureWidget() const;

signals:
    void setUpForApplicationRunDone(QString &tmpDirectory, QString &inputFile);
    void sendLoadFile(QString filename);

public slots:  

    void setUpForApplicationRun(QString &, QString &);
    void processResults(QString dakotaOut, QString dakotaTab, QString inputFile);
    void loadFile(QString filename);
    void replyFinished(QNetworkReply*);

private:

    // Sidebar container selection
    SimCenterComponentSelection *theComponentSelection;

    // Widgets that go in sidebar
    AssetsWidget* theAssetsWidget;
    HazardsWidget* theHazardsWidget;
    RegionalMappingWidget* theRegionalMappingWidget;
    VisualizationWidget* theVisualizationWidget;
    GeneralInformationWidget* theGeneralInformationWidget;
    AssetsModelWidget* theModelingWidget;
    EngDemandParameterWidget* theEngDemandParamWidget;
    DamageMeasureWidget* theDamageMeasureWidget;
    DecisionVariableWidget* theDecisionVariableWidget;
    RandomVariablesContainer* theRVs;
    UQ_EngineSelection* theUQWidget;
    UQ_Results* theUQResultsWidget;


    // Objects for running the workflow and obtaining results
    RunWidget *theRunWidget;
    Application *localApp;
    Application *remoteApp;
    RemoteJobManager *theJobManager;

    QJsonObject *jsonObjOrig;
    QNetworkAccessManager *manager;
};

#endif // WORKFLOW_APP_RDT_H
