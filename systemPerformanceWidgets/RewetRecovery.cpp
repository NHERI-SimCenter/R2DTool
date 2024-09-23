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
// Written: fmk, Sina Naeimi

#include "RewetRecovery.h"
#include <QScrollArea>
#include <QLineEdit>
#include <QTabWidget>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QJsonObject>
#include <QDoubleValidator>

#include <SC_QRadioButton.h>
#include <SC_DoubleLineEdit.h>
#include <SC_FileEdit.h>
#include <SC_IntLineEdit.h>
#include <SC_ComboBox.h>
#include <SC_CheckBox.h>
#include <SC_TableEdit.h>
//#include <SC_intLineEdit.h>
#include <RewetResults.h>


RewetRecovery::RewetRecovery(QWidget *parent)
  : SimCenterAppWidget(parent), resultWidget(0)
{
    int windowWidth = 800;

    QGridLayout *mainLayout = new QGridLayout();

    QTabWidget *theTabWidget = new QTabWidget();
    mainLayout->addWidget(theTabWidget);
   
    //
    // simulation widget
    //
   
    QWidget *simulationWidget = new QWidget();
    QGridLayout *simLayout = new QGridLayout();
    simulationWidget->setLayout(simLayout);

    
    int numRow = 0;

    // 1.0 Event Time
    eventTime = new SC_IntLineEdit("eventTime", 3600*2, 0, 3600*24*10000);
    simLayout->addWidget(new QLabel("Event Time"), numRow, 0);
    simLayout->addWidget(eventTime, numRow, 1);
    simLayout->addWidget(new QLabel("Seconds"), numRow++, 2);
    
    // 1.1 Simulation End Time
    simulationTime = new SC_IntLineEdit("simulationTime", 3600*24*10, 0, 3600*24*10000);
    simLayout->addWidget(new QLabel("Simulation End Time"), numRow, 0);
    simLayout->addWidget(simulationTime, numRow, 1);
    simLayout->addWidget(new QLabel("Seconds"), numRow++, 2);

    // 1.2 Simulation Time Step
    //int numRow = 1;
    simulationTimeStep = new SC_IntLineEdit("simulationTimeStep",3600 , 0,3600*24*10000);
    simLayout->addWidget(new QLabel("Simulation Time Step"), numRow, 0);
    simLayout->addWidget(simulationTimeStep, numRow, 1);
    simLayout->addWidget(new QLabel("Seconds"), numRow++, 2);

    // 1.3 condition 1 status for simulation termination 
    //int numRow = 2;
    lastTerminationCheckBox = new SC_CheckBox("last_sequence_termination", false);
    QLabel *terminate_1_label = new QLabel("Terminate simulation After the last job sequence is done");
    terminate_1_label->setWordWrap(true);
    simLayout->addWidget(terminate_1_label, numRow, 0);
    simLayout->addWidget(lastTerminationCheckBox, numRow++, 1);

    // 1.4 condition 2 status for simulation termination
    //int numRow = 3;
    demandMetTerminationCheckBox = new SC_CheckBox("node_demand_temination", false);
    QLabel *terminate_2_label = new QLabel("Terminate simulation After the demand is met");
    terminate_2_label->setWordWrap(true);
    simLayout->addWidget(terminate_2_label, numRow, 0);
    simLayout->addWidget(demandMetTerminationCheckBox, numRow++, 1);

    // 1.5 time window for condition 2
    demandMetTerminationTimeWindow = new SC_IntLineEdit("node_demand_termination_time", 3600*3, 1 , 3600*24*10000);
    QLabel *terminateTimeWindowLabel = new QLabel("Demand checking time window");
    terminateTimeWindowLabel->setWordWrap(true);
    simLayout->addWidget(terminateTimeWindowLabel, numRow, 0);
    simLayout->addWidget(demandMetTerminationTimeWindow, numRow, 1);
    simLayout->addWidget(new QLabel("Seconds"), numRow++, 2);

    // 1.5 time window for condition 2
    demandMetCriteriaRatio = new SC_DoubleLineEdit("node_demand_termination_ratio", 0.95, 0, 1, 2);
    //QDoubleValidator *demandMetCriteriaRatioValidator = new QDoubleValidator(0.001, 1, 3);
    //demandMetCriteriaRatio->setValidator(demandMetCriteriaRatioValidator);
    QLabel *terminateCriteriaLabel = new QLabel("Demand checking Criteria");
    terminateTimeWindowLabel->setWordWrap(true);
    simLayout->addWidget(terminateCriteriaLabel, numRow, 0);
    simLayout->addWidget(demandMetCriteriaRatio, numRow++, 1);
    
    // Stretch the last row, leave the strech of the remaining as minimum,
    // so the last row is at teh minimum  
    
    simLayout->setRowStretch(numRow, 1);
    
    //
    // hydraulics widget
    //
    
    // make 2 main group box. Layout is VBox.
    // Create the hydraulic widget
    QWidget *hydraulicsWidget = new QWidget();
    
    // create the widget's layout
    QBoxLayout *hydroLayout = new QBoxLayout(QBoxLayout::TopToBottom);
    hydraulicsWidget->setLayout(hydroLayout);
    
    // create group box 1 and assign a grid layout to it
    QGroupBox *hydraulicSolverGroupBox = new QGroupBox("Solver");
    QGridLayout *hydraulicSolverLayout = new QGridLayout();
    hydraulicSolverGroupBox->setLayout(hydraulicSolverLayout);

    // create group box 2 and assign a grid layout to it
    QGroupBox *hydraulicDamageModelingGroupBox = new QGroupBox("Damage Modeling");
    QGridLayout *hydraulicDamageModleingLayout = new QGridLayout();
    hydraulicDamageModelingGroupBox->setLayout(hydraulicDamageModleingLayout);

    // assign group box 1 and 2 to the hydraulic widget
    hydroLayout->addWidget(hydraulicSolverGroupBox);
    hydroLayout->addWidget(hydraulicDamageModelingGroupBox);

    // This streches the Vbox to the end, compacting two group boxes at teh top
    hydroLayout->addStretch();

    // Now we need to populate each group boxes with requried widgets

    // 2.1.1 Solver
    QLabel *solverTypeLabel = new QLabel("Solver Type"); 
    solverSelection = new SC_ComboBox("Solver", {"ModifiedEPANETV2.2"});
    
    // 2.1.2 PDA Min
    QLabel *solverPDAMinLabel = new QLabel("Min Pressure Override (-1 for ignore)"); 
    solverPDAMin = new SC_DoubleLineEdit("minimum_pressure", -1);
    QLabel *solverPDAMinUnitLabel = new QLabel("Meter");

    // 2.1.3 PDA Required
    QLabel *solverPDARequiredLabel = new QLabel("Required Pressure Override (-1 for ignore)");
    solverPDARequired = new SC_DoubleLineEdit("required_pressure", -1);
    QLabel *solverPDARequiredUnitLabel = new QLabel("Meter");

    // (2.1) add Solver widgets to the hydraulic solver layout
    numRow = 0;
    hydraulicSolverLayout->addWidget(solverTypeLabel, numRow, 0);
    hydraulicSolverLayout->addWidget(solverSelection, numRow++, 1);
    hydraulicSolverLayout->addWidget(solverPDAMinLabel, numRow, 0);
    hydraulicSolverLayout->addWidget(solverPDAMin, numRow, 1);
    hydraulicSolverLayout->addWidget(solverPDAMinUnitLabel, numRow++, 2);
    hydraulicSolverLayout->addWidget(solverPDARequiredLabel, numRow, 0);
    hydraulicSolverLayout->addWidget(solverPDARequired, numRow, 1);
    hydraulicSolverLayout->addWidget(solverPDARequiredUnitLabel, numRow++, 2);

    // (2.2.1) Pipe Damage Modeling (i.e., Material)
    QLabel *pipeDamageModelLabel = new QLabel("Pipe Damage Modeling");

    // (2.2.1.1) CI as defualt values
    QStringList pipeDamageModelDataValues = {"CI", "-0.0038", "0.1096", "0.0196", "2", "1"};
    
    // (2.2.1.2) creating the table
    QStringList pipeDamageModelColumns = {"name", "alpha", "beta", "gamma", "a", "b"};
    pipeDamageModelingTable = new SC_TableEdit("pipe_damage_model", pipeDamageModelColumns, 1, pipeDamageModelDataValues, NULL, true, 1);

    // (2.2.2) Pipe Damage Modeling (i.e., Material)
    QLabel *nodeDamageModelLabel = new QLabel("Node Damage Modeling");
    QStringList nodeDamageModelDataValues = {"0.0036", "1", "0", "0", "-0.877", "1", "0", "0", "0.0248", "1", "1", "0", "0", "0"};
    QStringList nodeDamageModelColumns = {"a", "aa", "b", "bb", "c", "cc", "d", "dd", "e", "ee1", "ee2", "f", "ff1", "ff2"};
    nodeDamageModelingTable = new SC_TableEdit("node_damage_model", nodeDamageModelColumns, 1, nodeDamageModelDataValues, NULL, true, 0, false);



    // (2.2) add Damage Modleing widgets to the Damage Modleing layout
    numRow = 0;
    hydraulicDamageModleingLayout->addWidget(pipeDamageModelLabel, numRow++, 0);
    hydraulicDamageModleingLayout->addWidget(pipeDamageModelingTable, numRow++, 0, 1, 3);
    hydraulicDamageModleingLayout->addWidget(nodeDamageModelLabel, numRow++, 0);
    hydraulicDamageModleingLayout->addWidget(nodeDamageModelingTable, numRow++, 0, 1, 3);

    //
    // restoration widget
    //

    // (3) setting up restoration widget, layouts and group boxes
    QWidget *restorationWidget = new QWidget();
    QBoxLayout *restorationLayout = new QBoxLayout(QBoxLayout::TopToBottom);
    restorationWidget->setLayout(restorationLayout);

    QGroupBox *restorationGeneralGroupBox = new QGroupBox("General");
    QGridLayout *restorationGeneralLayout = new QGridLayout();
    restorationGeneralGroupBox->setLayout(restorationGeneralLayout);

    QGroupBox *restorationDiscoveryGroupBox = new QGroupBox("Discovery");
    QGridLayout *restorationDiscoveryLayout = new QGridLayout();
    restorationDiscoveryGroupBox->setLayout(restorationDiscoveryLayout);

    restorationLayout->addWidget(restorationGeneralGroupBox);
    restorationLayout->addWidget(restorationDiscoveryGroupBox);

    restorationLayout->addStretch();

    // (3.1.1) Restoration On and Off
    QLabel *restorationOnAndOffLabel = new QLabel("Restoration On");
    restorationOnCheckBox = new SC_CheckBox("Restoration_on", true);
;
    // (3.1.2) Policy definition
    QLabel *policyDefinitionLabel = new QLabel("Policy Definition File");
    policyDefinitionFile = new SC_FileEdit("Policy Definition");

    // (3.1.3) Minimum job time
    QLabel *minimumJobTimeLabel = new QLabel("Minimum job time");
    minimumJobTimeLineEdit = new SC_IntLineEdit("minimum_job_time", 3600, 1, 3600*24*10000);
    QLabel *minimumJobTimeUnitLabel = new QLabel("seconds"); 

    // (3.1) Adding to the group box "General" widgets to its layout
    
    // To take the file edits widgets out of its QWidget and Layout Container and add them directly to
    // the tab's layout. In this way, we solve the problem of unintended indention because of Qwisget
    // at the begining of the SC_FileEdit
    QLayoutItem * const item0 = policyDefinitionFile->layout()->itemAt(0);
    QLayoutItem * const item1 = policyDefinitionFile->layout()->itemAt(1);
    dynamic_cast<QWidgetItem *>(item0) ;
    dynamic_cast<QWidgetItem *>(item1) ;

    numRow = 0;
    restorationGeneralLayout->addWidget(restorationOnAndOffLabel, numRow, 0);
    restorationGeneralLayout->addWidget(restorationOnCheckBox, numRow++, 1);
    restorationGeneralLayout->addWidget(policyDefinitionLabel, numRow, 0);
    restorationGeneralLayout->addWidget(item0->widget(), numRow, 1, 1, 2);
    restorationGeneralLayout->addWidget(item1->widget(), numRow++, 3);
    restorationGeneralLayout->addWidget(minimumJobTimeLabel, numRow, 0);
    restorationGeneralLayout->addWidget(minimumJobTimeLineEdit, numRow, 1);
    restorationGeneralLayout->addWidget(minimumJobTimeUnitLabel, numRow++, 2);


    // Restoration Discovery Groupbox
    QWidget *pipeDiscoveryWidget = new QWidget();
    QWidget *nodeDiscoveryWidget = new QWidget();
    QWidget *tankDiscoveryWidget = new QWidget();
    QWidget *pumpDiscoveryWidget = new QWidget();

    // making the restoration disovery tabs
    QTabWidget *theRestoprationDiscoveryTab = new QTabWidget();
    theRestoprationDiscoveryTab->addTab(pipeDiscoveryWidget, "Pipe");
    theRestoprationDiscoveryTab->addTab(nodeDiscoveryWidget, "Node");
    theRestoprationDiscoveryTab->addTab(tankDiscoveryWidget, "Tank");
    theRestoprationDiscoveryTab->addTab(pumpDiscoveryWidget, "Pump");

    restorationDiscoveryLayout->addWidget(theRestoprationDiscoveryTab);

    // creating layout for discovery widgets and adding to tehri reespective widget
    QGridLayout *pipeDiscoveryLayout = new QGridLayout();
    QGridLayout *nodeDiscoveryLayout = new QGridLayout();
    QGridLayout *tankDiscoveryLayout = new QGridLayout();
    QGridLayout *pumpDiscoveryLayout = new QGridLayout();

    pipeDiscoveryWidget->setLayout(pipeDiscoveryLayout);
    nodeDiscoveryWidget->setLayout(nodeDiscoveryLayout);
    tankDiscoveryWidget->setLayout(tankDiscoveryLayout);
    pumpDiscoveryWidget->setLayout(pumpDiscoveryLayout);

    // adding pipe discovery widgets
    pipeLeakBasedRadioButton = new SC_QRadioButton("Pipe_Leak_Based", pipeDiscoveryWidget);
    pipeTimeBasedRadioButton = new SC_QRadioButton("Pipe_Time_Based", pipeDiscoveryWidget);

    pipeDiscoveryLeakAmountLineEdit = new SC_DoubleLineEdit("pipe_leak_amount", 0.025, 0, 1000000, 0.0001);
    pipeDiscoveryTimeWindowLineEdit = new SC_IntLineEdit("pipe_leak_time", 43200, 1, 3600*24*10000);
    pipeTimeBasedDiscoveryTable = new SC_TableEdit("pipe_time_discovery_ratio", {"Time", "Ratio"}, 1, {"0", "1"}, NULL, true, 0);

    QLabel *pipeDiscoveryLeakAmountLabel= new QLabel("Leak Amount");
    pipeDiscoveryLayout->addWidget(pipeLeakBasedRadioButton, 0, 0, 1, 1);
    pipeDiscoveryLayout->addWidget(new QWidget(), 1, 0, 1, 1);
    pipeDiscoveryLayout->addWidget(pipeDiscoveryLeakAmountLabel, 1, 1, 1, 1);
    pipeDiscoveryLayout->addWidget(pipeDiscoveryLeakAmountLineEdit, 1, 2, 1, 1);

    QLabel *pipeDiscoveryTimeWindowLabel = new QLabel("Time Window");
    pipeDiscoveryLayout->addWidget(pipeDiscoveryTimeWindowLabel, 1, 3, 1, 1);
    pipeDiscoveryLayout->addWidget(pipeDiscoveryTimeWindowLineEdit, 1, 4, 1, 1);
    pipeDiscoveryLayout->addWidget(new QWidget(), 1, 5, 1, 1);
    pipeDiscoveryLayout->setColumnStretch(5,1);

    pipeDiscoveryLayout->addWidget(pipeTimeBasedRadioButton, 2, 0, 1, 1);
    pipeDiscoveryLayout->addWidget(new QWidget(), 3, 0, 1, 1);
    pipeDiscoveryLayout->addWidget(pipeTimeBasedDiscoveryTable, 3, 1, 1, 4);

    connect( pipeLeakBasedRadioButton, &SC_QRadioButton::toggled, pipeDiscoveryLeakAmountLineEdit, &SC_DoubleLineEdit::setEnabled);
    connect( pipeLeakBasedRadioButton, &SC_QRadioButton::toggled, pipeDiscoveryTimeWindowLineEdit, &SC_DoubleLineEdit::setEnabled);
    connect( pipeLeakBasedRadioButton, &SC_QRadioButton::toggled, pipeDiscoveryLeakAmountLabel, &QLabel::setEnabled);
    connect( pipeLeakBasedRadioButton, &SC_QRadioButton::toggled, pipeDiscoveryTimeWindowLabel, &QLabel::setEnabled);

    connect( pipeTimeBasedRadioButton, &SC_QRadioButton::toggled, pipeTimeBasedDiscoveryTable, &SC_TableEdit::setEnabled);
    pipeLeakBasedRadioButton->setChecked(true);
    pipeTimeBasedDiscoveryTable->setEnabled(false);

    // adding node discovery widgets
    nodeLeakBasedRadioButton = new SC_QRadioButton("Node_Leak_Based", nodeDiscoveryWidget);
    nodeTimeBasedRadioButton = new SC_QRadioButton("Node_Time_Based", nodeDiscoveryWidget);

    nodeDiscoveryLeakAmountLineEdit = new SC_DoubleLineEdit("node_leak_amount", 0.001, 0, 1000000, 0.0001);
    nodeDiscoveryTimeWindowLineEdit = new SC_IntLineEdit("node_leak_time", 43200, 1, 3600*24*10000);
    nodeTimeBasedDiscoveryTable = new SC_TableEdit("node_time_discovery_ratio", {"Time", "Ratio"}, 1, {"0", "1"}, NULL, true, 0);

    QLabel *nodeDiscoveryLeakAmountLabel= new QLabel("Leak Amount");
    nodeDiscoveryLayout->addWidget(nodeLeakBasedRadioButton, 0, 0, 1, 1);
    nodeDiscoveryLayout->addWidget(new QWidget(), 1, 0, 1, 1);
    nodeDiscoveryLayout->addWidget(nodeDiscoveryLeakAmountLabel, 1, 1, 1, 1);
    nodeDiscoveryLayout->addWidget(nodeDiscoveryLeakAmountLineEdit, 1, 2, 1, 1);


    QLabel *nodeDiscoveryTimeWindowLabel = new QLabel("Time Window");
    nodeDiscoveryLayout->addWidget(nodeDiscoveryTimeWindowLabel, 1, 3, 1, 1);
    nodeDiscoveryLayout->addWidget(nodeDiscoveryTimeWindowLineEdit, 1, 4, 1, 1);
    nodeDiscoveryLayout->addWidget(new QWidget(), 1, 5, 1, 1);
    nodeDiscoveryLayout->setColumnStretch(5,1);

    nodeDiscoveryLayout->addWidget(nodeTimeBasedRadioButton, 2, 0, 1, 1);
    nodeDiscoveryLayout->addWidget(new QWidget(), 3, 0, 1, 1);
    nodeDiscoveryLayout->addWidget(nodeTimeBasedDiscoveryTable, 3, 1, 1, 4);

    connect( nodeLeakBasedRadioButton, &SC_QRadioButton::toggled, nodeDiscoveryLeakAmountLineEdit, &SC_DoubleLineEdit::setEnabled);
    connect( nodeLeakBasedRadioButton, &SC_QRadioButton::toggled, nodeDiscoveryTimeWindowLineEdit, &SC_DoubleLineEdit::setEnabled);
    connect( nodeLeakBasedRadioButton, &SC_QRadioButton::toggled, nodeDiscoveryLeakAmountLabel, &QLabel::setEnabled);
    connect( nodeLeakBasedRadioButton, &SC_QRadioButton::toggled, nodeDiscoveryTimeWindowLabel, &QLabel::setEnabled);

    connect( nodeTimeBasedRadioButton, &SC_QRadioButton::toggled, nodeTimeBasedDiscoveryTable, &SC_TableEdit::setEnabled);
    nodeLeakBasedRadioButton->setChecked(true);
    nodeTimeBasedDiscoveryTable->setEnabled(false);

    // adding tank discovery widgets
    tankTimeBasedDiscoveryTable = new SC_TableEdit("tank_time_discovery_ratio", {"Time", "Ratio"}, 1, {"0", "1"}, NULL, true, 0);

    //tankDiscoveryLayout->addWidget(new QWidget(), 1, 0, 1, 1);
    tankDiscoveryLayout->addWidget(new QLabel("Time Based"), 0, 0, 1, 1);
    tankDiscoveryLayout->addWidget(tankTimeBasedDiscoveryTable, 1, 1, 1, 2);
    tankDiscoveryLayout->addWidget(new QWidget(), 1, 3, 1, 1);
    tankDiscoveryLayout->setColumnStretch(3,1);

    // adding tank discovery widgets
    pumpTimeBasedDiscoveryTable = new SC_TableEdit("pump_time_discovery_ratio", {"Time", "Ratio"}, 1, {"0", "1"}, NULL, true, 0);

    //pumpDiscoveryLayout->addWidget(new QWidget(), 1, 0, 1, 1);
    pumpDiscoveryLayout->addWidget(new QLabel("Time Based"), 0, 0, 1, 1);
    pumpDiscoveryLayout->addWidget(pumpTimeBasedDiscoveryTable, 1, 1, 1, 2);
    pumpDiscoveryLayout->addWidget(new QWidget(), 1, 3, 1, 1);
    pumpDiscoveryLayout->setColumnStretch(3,1);

    // stuff below for scrolling .. if no scrolling this->setLayout(mainLayout);

    // making the main tabs
    theTabWidget->addTab(simulationWidget, "Simulation");
    theTabWidget->addTab(hydraulicsWidget, "Hydraulics");
    theTabWidget->addTab(restorationWidget, "Restoration");
    
    QWidget     *mainGroup = new QWidget();
    mainGroup->setLayout(mainLayout);
    mainGroup->setMaximumWidth(windowWidth);
    
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setLineWidth(1);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setWidget(mainGroup);
    scrollArea->setMaximumWidth(windowWidth + 25);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(scrollArea);
    this->setLayout(layout);
}


RewetRecovery::~RewetRecovery()
{

}


void RewetRecovery::clear(void)
{

}


bool RewetRecovery::inputFromJSON(QJsonObject &jsonObject)
{
  this->clear();
  
  // inputSettings->inputFromJSON(jsonObject);
  // simulationTime->inputFromJSON(jsonObject);
  // solver->inputFromJSON(jsonObject);
  // policyDefinitionFile->inputFromJSON(jsonObject);

  eventTime->inputFromJSON(jsonObject);
  simulationTime->inputFromJSON(jsonObject);
  simulationTimeStep->inputFromJSON(jsonObject);
  lastTerminationCheckBox->inputFromJSON(jsonObject);
  demandMetTerminationCheckBox->inputFromJSON(jsonObject);
  demandMetTerminationTimeWindow->inputFromJSON(jsonObject);
  demandMetCriteriaRatio->inputFromJSON(jsonObject);
  solverPDARequired->inputFromJSON(jsonObject);
  solverSelection->inputFromJSON(jsonObject);
  solverPDAMin->inputFromJSON(jsonObject);
  // damage Modeling GroupBox
  pipeDamageModelingTable->inputFromJSON(jsonObject);
  nodeDamageModelingTable->inputFromJSON(jsonObject);

  // restoration
  restorationOnCheckBox->inputFromJSON(jsonObject);
  policyDefinitionFile->inputFromJSON(jsonObject);
  minimumJobTimeLineEdit->inputFromJSON(jsonObject);
  pipeLeakBasedRadioButton->inputFromJSON(jsonObject);
  pipeTimeBasedRadioButton->inputFromJSON(jsonObject);
  pipeDiscoveryLeakAmountLineEdit->inputFromJSON(jsonObject);
  pipeDiscoveryTimeWindowLineEdit->inputFromJSON(jsonObject);
  pipeTimeBasedDiscoveryTable->inputFromJSON(jsonObject);
  nodeLeakBasedRadioButton->inputFromJSON(jsonObject);
  nodeTimeBasedRadioButton->inputFromJSON(jsonObject);
  nodeDiscoveryLeakAmountLineEdit->inputFromJSON(jsonObject);
  nodeDiscoveryTimeWindowLineEdit->inputFromJSON(jsonObject);
  nodeTimeBasedDiscoveryTable->inputFromJSON(jsonObject);
  tankTimeBasedDiscoveryTable->inputFromJSON(jsonObject);
  pumpTimeBasedDiscoveryTable->inputFromJSON(jsonObject);  
  
  return true;
}

bool RewetRecovery::outputToJSON(QJsonObject &jsonObject)
{
  jsonObject["Application"] = "REWETRecovery";
  eventTime->outputToJSON(jsonObject);
  simulationTime->outputToJSON(jsonObject);
  simulationTimeStep->outputToJSON(jsonObject);
  lastTerminationCheckBox->outputToJSON(jsonObject);
  demandMetTerminationCheckBox->outputToJSON(jsonObject);
  demandMetTerminationTimeWindow->outputToJSON(jsonObject);
  demandMetCriteriaRatio->outputToJSON(jsonObject);
  solverPDARequired->outputToJSON(jsonObject);
  solverSelection->outputToJSON(jsonObject);
  solverPDAMin->outputToJSON(jsonObject);
  // damage Modeling GroupBox
  pipeDamageModelingTable->outputToJSON(jsonObject);
  nodeDamageModelingTable->outputToJSON(jsonObject);

  // restoration
  restorationOnCheckBox->outputToJSON(jsonObject);
  policyDefinitionFile->outputToJSON(jsonObject);
  minimumJobTimeLineEdit->outputToJSON(jsonObject);
  pipeLeakBasedRadioButton->outputToJSON(jsonObject);
  pipeTimeBasedRadioButton->outputToJSON(jsonObject);
  pipeDiscoveryLeakAmountLineEdit->outputToJSON(jsonObject);
  pipeDiscoveryTimeWindowLineEdit->outputToJSON(jsonObject);
  pipeTimeBasedDiscoveryTable->outputToJSON(jsonObject);
  nodeLeakBasedRadioButton->outputToJSON(jsonObject);
  nodeTimeBasedRadioButton->outputToJSON(jsonObject);
  nodeDiscoveryLeakAmountLineEdit->outputToJSON(jsonObject);
  nodeDiscoveryTimeWindowLineEdit->outputToJSON(jsonObject);
  nodeTimeBasedDiscoveryTable->outputToJSON(jsonObject);
  tankTimeBasedDiscoveryTable->outputToJSON(jsonObject);
  pumpTimeBasedDiscoveryTable->outputToJSON(jsonObject);
  
  return true;
}

bool RewetRecovery::outputAppDataToJSON(QJsonObject &jsonObject) {

    //
    // per API, need to add name of application to be called in AppLication
    // and all data to be used in ApplicationDate
    //

    jsonObject["Application"] = "REWETRecovery";
    QJsonObject dataObj;
    jsonObject["ApplicationData"] = dataObj;

    return true;
}

bool RewetRecovery::inputAppDataFromJSON(QJsonObject &jsonObject) {
    Q_UNUSED(jsonObject);
    return true;
}


bool RewetRecovery::copyFiles(QString &destDir) {
  return policyDefinitionFile->copyFile(destDir);
}

bool RewetRecovery::outputCitation(QJsonObject &citation){
  QString REWETCitationKey = "REWET";
  QJsonValue REWETCitationValue( "\"citations\": [{\"citation\": \"Naeimi, Sina, and Rachel A. Davidson. “REWET: A Tool to Model System Functioning and Restoration of Damaged Water Supply Systems.” Journal of Infrastructure Systems 30.4 (2024): 04024024."},{\"citation\": \"Klise, K.A., Hart, D.B., Bynum, M., Hogge, J., Haxton, T., Murray, R., Burkhardt, J. (2020). Water Network Tool for Resilience (WNTR) User Manual: Version 0.2.3. U.S. EPA Office of Research and Development, Washington, DC, EPA/600/R-20/185, 82p.\"}, {\"Rossman, L., H. Woo, M. Tryby, F. Shang, R. Janke, AND T. Haxton. EPANET 2.2 User Manual. U.S. Environmental Protection Agency, Washington, DC, EPA/600/R-20/133, 2020.\"}]}");
  citation.insert(REWETCitationKey, REWETCitationValue);
  
  return true;
}

SC_ResultsWidget* RewetRecovery::getResultsWidget(QWidget *parent, QWidget *R2DresWidget, QMap<QString, QList<QString>> assetTypeToType)
{
    if (resultWidget==nullptr){
        resultWidget = new RewetResults(parent);
    }
    return resultWidget;
}
