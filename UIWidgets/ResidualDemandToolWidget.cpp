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

#include "BrailsTranspInventoryGenerator.h"
#include "QGISVisualizationWidget.h"
#include "SimCenterMapcanvasWidget.h"
#include "GIS_Selection.h"
#include <qgsmapcanvas.h>
#include <PlainRectangle.h>
#include <BrailsGoogleDialog.h>
#include <SimCenterPreferences.h>
#include "ModularPython.h"
#include "ResidualDemandToolWidget.h"
#include "AssetInputDelegate.h"
#include <WorkflowAppR2D.h>

#include <QLabel>
#include <QPushButton>
#include <QStandardPaths>
#include <QGroupBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <SC_DoubleLineEdit.h>
#include <QComboBox>
#include <SC_DirEdit.h>
#include <SC_FileEdit.h>
#include <SC_CheckBox.h>
#include <SC_ComboBox.h>
#include <SC_IntLineEdit.h>
#include <SC_DoubleLineEdit.h>
#include <QSettings>
#include <QDir>
#include <QLineEdit>
#include <QCheckBox>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonArray>
#include <QString>
#include <QStringList>
#include <QGroupBox>
#include <ResultsWidget.h>

ResidualDemandToolWidget::ResidualDemandToolWidget(VisualizationWidget* visWidget, QWidget *parent) : SimCenterAppWidget(parent)
{
    
    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
//    QString workDir = SimCenterPreferences::getInstance()->getLocalWorkDir();
//    QString r2drunDir = workDir + QDir::separator() + QString("tmp.SimCenter") + QDir::separator() + QString("Results");
//    QDir dir(r2drunDir);
//    if (!dir.exists())
//        r2drunDir = QString("");
//        dir.mkpath(inputDir);

    int numRow = 0;
    QHBoxLayout *windowLayout = new QHBoxLayout(this);

    theGroupBox = new QGroupBox(this);
    theGroupBox->setTitle("Residual Demand Traffic Simulator");
    theGroupBox->setContentsMargins(0,0,0,0);
    QGridLayout *mainLayout = new QGridLayout();
    theGroupBox->setLayout(mainLayout);

    windowLayout->addWidget(theGroupBox);

    // 1. Results Directory
    resultsDir = new SC_DirEdit("Results_dir", true);
    mainLayout->addWidget(new QLabel("Results Directory:"), numRow, 0, 1, 1);
    mainLayout->addWidget(resultsDir, numRow, 1, 1, 5);
    numRow++;

    // 2. Realization Selection
    damageInputMethodComboBox = new SC_ComboBox("DamageInputType",
                                                QStringList({
                                                    "Most likely damage state",
                                                    "Specific realization(s)"
                                                }));
    mainLayout->addWidget(new QLabel("Damage State Input:"), numRow, 0, 1, 1);
    mainLayout->addWidget(damageInputMethodComboBox, numRow, 1, 1, 1);

    realizationToAnalyzeLabel = new QLabel("Realizations to Analyze:");
    mainLayout->addWidget(realizationToAnalyzeLabel, numRow, 2, 1, 1);
    realizationInputWidget = new AssetInputDelegate();
//    connect(realizationInputWidget,&AssetInputDelegate::componentSelectionComplete,this,
//            &ResidualDemandToolWidget::selectComponents);
    connect(realizationInputWidget,&QLineEdit::editingFinished,this,&ResidualDemandToolWidget::selectComponents);
    mainLayout->addWidget(realizationInputWidget, numRow, 3, 1, 3);
    connect(this->damageInputMethodComboBox, &QComboBox::currentTextChanged,
            this, &ResidualDemandToolWidget::handleInputTypeChanged);
    damageInputMethodComboBox->setCurrentIndex(1);
    numRow++;

    // 3. Path To Edges File
    pathEdgesFile = new SC_FileEdit("edges_geojson");
    mainLayout->addWidget(new QLabel("Road Network Edges Geojson File:"), numRow, 0, 1, 1);
    mainLayout->addWidget(pathEdgesFile, numRow, 1, 1, 4);
    numRow++;

    // 4. Path To Nodes File
    pathNodesFile = new SC_FileEdit("nodes_geojson");
    mainLayout->addWidget(new QLabel("Road Network Nodes Geojson File:"), numRow, 0, 1, 1);
    mainLayout->addWidget(pathNodesFile, numRow, 1, 1, 4);
    numRow++;

    // 4. Path To pre-event OD File
    pathODFilePre = new SC_FileEdit("od_file_pre");
    mainLayout->addWidget(new QLabel("Pre-event traffic Demand File:"), numRow, 0, 1, 1);
    mainLayout->addWidget(pathODFilePre, numRow, 1, 1, 4);
    numRow++;

    //5. Post event OD checkbox
    postEventODCheckBox = new SC_CheckBox("od_file_post", false);
    mainLayout->addWidget(new QLabel("Use the same pre and post event traffic demand:"), numRow, 0, 1, 1);
    mainLayout->addWidget(postEventODCheckBox, numRow, 1, 1, 1);
    numRow++;

    // 4. Path To post-event OD File
    pathODFilePost = new SC_FileEdit("od_file_post");
    postEventFileEditLable = new QLabel("Post-event traffic Demand File:");
    mainLayout->addWidget(postEventFileEditLable, numRow, 0, 1, 1);
    mainLayout->addWidget(pathODFilePost, numRow, 1, 1, 4);
    numRow++;


    // 5. Path To Capacity Map File
    pathCapacityMapFile = new SC_FileEdit("capacity_map");
    mainLayout->addWidget(new QLabel("Road Capacity Map JSON File:"), numRow, 0, 1, 1);
    mainLayout->addWidget(pathCapacityMapFile, numRow, 1, 1, 4);
    numRow++;

    // 6. Simulation Hour
    simulationHourList = new QLineEdit();
    mainLayout->addWidget(new QLabel("Simulation Hour List:"), numRow, 0, 1, 1);
    mainLayout->addWidget(simulationHourList, numRow, 1, 1, 3);
    numRow++;

    //7. Two way check box
    twoWayEdgeCheckbox = new SC_CheckBox("TwoWayEdges", false);
    mainLayout->addWidget(new QLabel("Assume All Edges as Two-way:"), numRow, 0, 1, 1);
    mainLayout->addWidget(twoWayEdgeCheckbox, numRow, 1, 1, 1);

    //7. CreateAnimation check box
    createAnimationCheckbox = new SC_CheckBox("CreateAnimation", true);
    mainLayout->addWidget(new QLabel("Create Congestion Animation:"), numRow, 2, 1, 1);
    mainLayout->addWidget(createAnimationCheckbox, numRow, 3, 1, 1);
    numRow++;

    // 8. Run Simulation Button
    runButton = new QPushButton("Run Simulation");
    mainLayout->addWidget(runButton, numRow, 1, 1, 2);
    connect(runButton, &QPushButton::clicked, this, &ResidualDemandToolWidget::runSimulation);
    numRow++;

    connect(postEventODCheckBox, &SC_CheckBox::stateChanged, this, &ResidualDemandToolWidget::togglePostEventODFileEdit);

    mainLayout->setRowStretch(numRow, 1);
    mainLayout->setColumnStretch(5, 1);

//    theR2DResultsWidget = WorkflowAppR2D::getInstance()->getTheResultsWidget();
//    theResidualDemandResultsWidget = new ResidualDemandResults(theR2DResultsWidget);

}

void ResidualDemandToolWidget::togglePostEventODFileEdit(int state){
    if (state == Qt::Checked){
        postEventFileEditLable->hide();
        pathODFilePost->hide();
    } else {
        postEventFileEditLable->show();
        pathODFilePost->show();
    }
}

ResidualDemandToolWidget::~ResidualDemandToolWidget()
{

}

void ResidualDemandToolWidget::clear(void)
{

}

void ResidualDemandToolWidget::selectComponents(void)
{
    try
    {
        realizationInputWidget->selectComponents();
    }
    catch (const QString msg)
    {
        this->errorMessage(msg);
    }
}

void ResidualDemandToolWidget::handleInputTypeChanged(){
    if (damageInputMethodComboBox->currentIndex()==0){
        realizationToAnalyzeLabel->hide();
        realizationInputWidget->hide();
    } else if (damageInputMethodComboBox->currentIndex()==1){
        realizationInputWidget->show();
        realizationToAnalyzeLabel->show();
    }
}
void ResidualDemandToolWidget::runSimulation(void)
{

    if(!this->initiateWorkDir()){
        return;
    }
//    QString workDir = SimCenterPreferences::getInstance()->getLocalWorkDir();
//    QString appInputDir = workDir + QDir::separator() + QString("tmp.SimCenter") + QDir::separator() + QString("input_data");
    
//    // Check for any missing files and directories

//    QDir dir(appInputDir);
//    if (!dir.exists())
//        dir.mkpath(appInputDir);

    QDir dir;
    auto resultsDir_dir = resultsDir->getDirName();
    dir.setPath(resultsDir_dir);
    if (resultsDir_dir.isEmpty() || !dir.exists()){
        this->errorMessage("Missing the Result Directory. Please select a directory.");
        return;
    }

    auto pathEdgesFile_path = pathEdgesFile->getFilename();
    if (pathEdgesFile_path.isEmpty() || !QFile::exists(pathEdgesFile_path)){
        this->errorMessage("Missing the network edges file. Please select a network edges file.");
        return;
    }

    auto pathNodesFile_path = pathNodesFile->getFilename();
    if (pathNodesFile_path.isEmpty() || !QFile::exists(pathNodesFile_path)){
        this->errorMessage("Missing the network nodes file. Please select a network nodes file.");
        return;
    }

    auto pathCapacityMapFile_path = pathCapacityMapFile->getFilename();
    if (pathCapacityMapFile_path.isEmpty() || !QFile::exists(pathCapacityMapFile_path)){
        this->errorMessage("Missing the capacity map file. Please select the capacity map file.");
        return;
    }


    // Check for any file types
    if (!pathCapacityMapFile_path.toLower().endsWith(".json")){
        this->errorMessage("The capacity map file must be a JSON file.");
        return;
    }

    QString realizationsString = realizationInputWidget->getComponentAnalysisList();
    
    qDebug() << "Realizations: " << realizationsString;

    // Create the Residual Demand Config JSON file

    QJsonObject config;
    config.insert("TwoWayEdges", twoWayEdgeCheckbox->isChecked());
    config.insert("CreateAnimation", createAnimationCheckbox->isChecked());

    // Move the capacity map to config file
    QFile capacityMapfile(pathCapacityMapFile_path);
    if (!capacityMapfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Could not open file for reading:" << pathCapacityMapFile_path;
        return;
    }
    QByteArray jsonData = capacityMapfile.readAll();
    capacityMapfile.close();
    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &parseError);

    // Check for parsing errors
    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << "JSON parse error:" << parseError.errorString();
        return;
    }

    // Convert QJsonDocument to QJsonObject
    if (!jsonDoc.isObject()) {
        qDebug() << "The JSON data is not an object.";
        return;
    }

    QJsonObject jsonObj = jsonDoc.object();
    config.insert("CapacityMap", jsonObj);

    QJsonObject damageInputJsonObject;
    if (damageInputMethodComboBox->currentIndex()==0){
        damageInputJsonObject.insert("Type", "MostlikelyDamageState");
    } else if(damageInputMethodComboBox->currentIndex()==1) {
    QJsonObject damageinputParameterQObject;
    damageinputParameterQObject.insert("Filter", realizationInputWidget->getComponentAnalysisList()); // TODO: Add the filter
    damageInputJsonObject.insert("Parameters", damageinputParameterQObject);
    damageInputJsonObject.insert("Type", "SpecificRealization");
    }
    config.insert("DamageInput", damageInputJsonObject);
    QString hourlist = simulationHourList->text();
    QStringList stringList = hourlist.split(',');
    QJsonArray jsonArray;
    for (const QString &str : stringList) {
        if (str.isEmpty()){
            continue;
        }
        bool ok;
        int strInt = str.toInt(&ok);
        if (ok){
            jsonArray.append(strInt);
        } else {
            this->errorMessage(QString("Invalide Hour List input: ") + str);
            return;
        }
    }
    config.insert("HourList", jsonArray);

    // Save the Recovery Main Input JSON file
    QString pathToConfigFile = appInputPath + QDir::separator() + "residual_demand_config.json";
    QFile file(pathToConfigFile);

    QString strFromObj = QJsonDocument(config).toJson(QJsonDocument::Indented);

    if(!file.open(QIODevice::WriteOnly))
    {
        this->errorMessage("Error: Cannot write the Residual Demand Configuration JSON file.");
        file.close();
    }
    else
    {
        QTextStream out(&file); out << strFromObj;
        file.close();
    }



    // Create Script to run PyReCoDes Wrapper 

    QStringList scriptArgs;
    scriptArgs << QString("--edgeFile")  << pathEdgesFile_path;
    scriptArgs << QString("--nodeFile")  << pathNodesFile_path;
    scriptArgs << QString("--ODFilePre")  << pathODFilePre->getFilename();
    if (postEventODCheckBox->isChecked()){
        // Use the same post and pre event OD
        scriptArgs << QString("--ODFilePost")  << pathODFilePre->getFilename();
    } else {
        scriptArgs << QString("--ODFilePost")  << pathODFilePost->getFilename();
    }
    scriptArgs << QString("--configFile")  << pathToConfigFile;
    scriptArgs << QString("--r2dRunDir")  << resultsDir_dir;
    scriptArgs << QString("--residualDemandRunDir")  << appOutputPath;

    QString scriptPath = SimCenterPreferences::getInstance()->getAppDir() + QDir::separator()
                         + "applications" + QDir::separator() + "systemPerformance" + QDir::separator()
                         + "ResidualDemand" + QDir::separator() + "run_residual_demand.py";

    std::unique_ptr<ModularPython> thePy = std::make_unique<ModularPython>(resultsDir_dir);

    connect(thePy.get(),SIGNAL(runComplete()),this,SLOT(handleProcessFinished()));

    this->handleProcessStarted();
    qDebug() << "Running ResidualDemand\n" << scriptPath << scriptArgs;
    thePy->run(scriptPath, scriptArgs);

    disconnect(thePy.get(),SIGNAL(runComplete()),this,SLOT(handleProcessFinished()));
}

void ResidualDemandToolWidget::handleProcessStarted()
{
    // this->runButton->setEnabled(false);
}

void ResidualDemandToolWidget::handleProcessFinished()
{
    this->runButton->setEnabled(true);

//    QTabWidget* resTabWidget = theResultsWidget->getTabWidget();
//    QString assetType = "TransportationNetwork";
//        // check if assetType is in resTabWidget
//        // If yes, add new tab
//        // if not, add to the tab
//        bool tabExist = false;
//        for (int tab_i = 0; tab_i < resTabWidget->count(); tab_i++){
//            if (resTabWidget->tabText(tab_i) == assetType){
//                tabExist = true;
//                break;
//            }
//        }
//        // If not exsit
//        if (!tabExist){
//            activeSPResultsWidgets[assetType]->addResultTab(assetType, resultsDirectory);
//        } else {
//            // If exsits
//            activeSPResultsWidgets[assetType]->addResultSubtab(assetType, activeDLResultsWidgets[assetType], resultsDirectory);
//        }
//    }

}

bool ResidualDemandToolWidget::initiateWorkDir(){
    QString workDir = SimCenterPreferences::getInstance()->getLocalWorkDir();
    if(workDir.isEmpty())
    {
        QString errorMessage = QString("Set the Local Jobs Directory location in preferences.");

        this->errorMessage(errorMessage);

        return false;
    }
    QDir dirWork(workDir);
    if (!dirWork.exists())
        if (!dirWork.mkpath(workDir))
        {
            QString errorMessage = QString("Could not load the Working Directory: ") + workDir
                                   + QString(". Change the Local Jobs Directory location in preferences.");

            this->errorMessage(errorMessage);

            return false;
        }

    workDir += QDir::separator() + QString("SystemPerformance");
    dirWork.setPath(workDir);
    if (!dirWork.exists())
        if (!dirWork.mkpath(workDir))
        {
            QString errorMessage = QString("Could not create the Working Directory: ") + workDir
                                   + QString(". Change the Local Jobs Directory location in preferences.");

            this->errorMessage(errorMessage);

            return false;
        }
    workDir += QDir::separator() + QString("ResidualDemand");
    dirWork.setPath(workDir);
    if (!dirWork.exists())
        if (!dirWork.mkpath(workDir))
        {
            QString errorMessage = QString("Could not create the Working Directory: ") + workDir
                                   + QString(". Change the Local Jobs Directory location in preferences.");

            this->errorMessage(errorMessage);

            return false;
        }
    QString inputDir = workDir + QDir::separator() + QString("input");
    appInputPath = inputDir;
    dirWork.setPath(inputDir);
    if (dirWork.exists()){
        if (!dirWork.removeRecursively()) {
            QString errorMessage = QString("Cound not clear the working directory: ") + workDir
                                   + QString(". Try remove the directory manually.");
            this->errorMessage(errorMessage);
            return false;
        }
    }

    if (!dirWork.mkpath(inputDir))
    {
        QString errorMessage = QString("Could not create the Working Directory: ") + workDir
                               + QString(". Change the Local Jobs Directory location in preferences.");

        this->errorMessage(errorMessage);

        return false;
    }


    QString outputDir = workDir + QDir::separator() + QString("output");
    appOutputPath = outputDir;
    dirWork.setPath(outputDir);
    if (dirWork.exists()){
        if (!dirWork.removeRecursively()) {
            QString errorMessage = QString("Cound not clear the working directory: ") + workDir
                                   + QString(". Try remove the directory manually.");
            this->errorMessage(errorMessage);
            return false;
        }
    }
    if (!dirWork.mkpath(outputDir))
    {
        QString errorMessage = QString("Could not create the Working Directory: ") + workDir
                               + QString(". Change the Local Jobs Directory location in preferences.");

        this->errorMessage(errorMessage);

        return false;
    }

    return true;
}
