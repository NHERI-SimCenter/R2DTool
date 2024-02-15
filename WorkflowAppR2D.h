#ifndef WORKFLOW_APP_R2D_H
#define WORKFLOW_APP_R2D_H
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

// Written by: Stevan Gavrilovic, Frank McKenna

#include "WorkflowAppWidget.h"

#include <QMap>

class AnalysisWidget;
class Application;
class LocalApplication;
class AssetsWidget;
class DLWidget;
class EngDemandParameterWidget;
class GeneralInformationWidgetR2D;
class HazardToAssetWidget;
class RandomVariablesWidget;
class HazardsWidget;
class UQWidget;
class ModelWidget;
class QNetworkAccessManager;
class QNetworkReply;
class QStackedWidget;
class RandomVariablesContainer;
class RemoteJobManager;
class RemoteService;
class ResultsWidget;
class LoadResultsDialog;
class RunWidget;
class SimCenterAppWidget;
class SimCenterAppEventSelection;
class SimCenterComponentSelection;
class UQOptions;
class UQWidget;
class RVWidget;
class UQ_Results;
class QGISVisualizationWidget;
class PerformanceWidget;
class LocalMappingWidget;
class ToolDialog;

class WorkflowAppR2D : public WorkflowAppWidget
{
    Q_OBJECT
public:
    explicit WorkflowAppR2D(RemoteService *theService, QWidget *parent = 0);
    ~WorkflowAppR2D();

    static WorkflowAppR2D *getInstance(void);

    bool outputToJSON(QJsonObject &rvObject);
    bool inputFromJSON(QJsonObject &rvObject);

    void onRunButtonClicked();
    void onRemoteRunButtonClicked();
    void onRemoteGetButtonClicked();
    void onExitButtonClicked();
    int getMaxNumParallelTasks();

    void initialize(void);
    
    void setActiveWidget(SimCenterAppWidget* widget);

    QGISVisualizationWidget *getVisualizationWidget() const;
    AssetsWidget *getAssetsWidget() const;
    GeneralInformationWidgetR2D *getGeneralInformationWidget() const;
    DLWidget *getTheDamageAndLossWidget() const;

    LocalApplication *getLocalApp() const;
    RandomVariablesWidget *getTheRandomVariableWidget() const;

signals:

public slots:  
    void clear(void);
    void loadResults(void);
    void setUpForApplicationRun(QString &, QString &);
    void processResults(QString &dirResults);
    int loadFile(QString &filename);
    void replyFinished(QNetworkReply*);
    void assetSelectionChanged(QString, bool);

    void statusMessage(QString message);
    void infoMessage(QString message);
    void errorMessage(QString message);
    void fatalMessage(QString message);
    void runComplete();

private:

    // Sidebar container selection
    SimCenterComponentSelection *theComponentSelection;

    //
    // Widgets that go in sidebar
    //

    ToolDialog* theToolDialog = nullptr;

    QGISVisualizationWidget* theVisualizationWidget = nullptr;
    GeneralInformationWidgetR2D* theGeneralInformationWidgetR2D = nullptr;
    HazardsWidget* theHazardsWidget = nullptr;
    AssetsWidget* theAssetsWidget = nullptr;
    HazardToAssetWidget* theHazardToAssetWidget = nullptr;
    ModelWidget* theModelingWidget = nullptr;
    AnalysisWidget *theAnalysisWidget = nullptr;
    DLWidget* theDamageAndLossWidget = nullptr;
    UQWidget* theUQWidget = nullptr;
    RVWidget* theRVWidget = nullptr;
    ResultsWidget* theResultsWidget = nullptr;
    LoadResultsDialog* resultsDialog = nullptr;
    PerformanceWidget* thePerformanceWidget = nullptr;
    //LocalMappingWidget* theLocalMappingWidget;  
    SimCenterAppEventSelection* theLocalEvent = nullptr;
    //
    // Objects for running the workflow and obtaining results
    //

    RunWidget *theRunWidget = nullptr;
    LocalApplication *localApp = nullptr;
    Application *remoteApp = nullptr;
    RemoteJobManager *theJobManager = nullptr;

    QJsonObject *jsonObjOrig = nullptr;
    QNetworkAccessManager *manager = nullptr;
    QString commonFilePath = nullptr;
    static WorkflowAppR2D *theInstance;

};

#endif // WORKFLOW_APP_R2D_H
