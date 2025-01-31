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
#include "ResidualDemandWidget.h"
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
#include "ResidualDemandResults.h"

ResidualDemandWidget::ResidualDemandWidget(QWidget *parent) : SimCenterAppWidget(parent)
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

    // 2. Realization Selection
    damageInputMethodComboBox = new SC_ComboBox("DamageInputType",
                                                QStringList({
                                                    "Specific realization(s)"
                                                }));
    mainLayout->addWidget(new QLabel("Damage State Input:"), numRow, 0, 1, 1);
    mainLayout->addWidget(damageInputMethodComboBox, numRow, 1, 1, 1);

    realizationToAnalyzeLabel = new QLabel("Realizations to Analyze:");
    mainLayout->addWidget(realizationToAnalyzeLabel, numRow, 2, 1, 1);
    realizationInputWidget = new AssetInputDelegate();
    connect(realizationInputWidget,&QLineEdit::editingFinished,this,&ResidualDemandWidget::selectComponents);
    mainLayout->addWidget(realizationInputWidget, numRow, 3, 1, 3);
//    connect(this->damageInputMethodComboBox, &QComboBox::currentTextChanged,
//            this, &ResidualDemandWidget::handleInputTypeChanged);
    damageInputMethodComboBox->setCurrentIndex(0);
    this->handleInputTypeChanged();
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

    // 4. Path To post-event OD File
    pathODFilePost = new SC_FileEdit("od_file_post");
    postEventFileEditLable = new QLabel("Post-event traffic Demand File:");
    mainLayout->addWidget(postEventFileEditLable, numRow, 0, 1, 1);
    mainLayout->addWidget(pathODFilePost, numRow, 1, 1, 4);
    numRow++;

    //5. Post event OD checkbox
    postEventODCheckBox = new SC_CheckBox("od_file_post", false);
    mainLayout->addWidget(new QLabel("Use the same pre and post event traffic demand:"), numRow, 0, 1, 1);
    mainLayout->addWidget(postEventODCheckBox, numRow, 1, 1, 1);
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

    mainLayout->setRowStretch(numRow, 1);
    mainLayout->setColumnStretch(5, 1);

    connect(postEventODCheckBox, &SC_CheckBox::stateChanged, this, &ResidualDemandWidget::togglePostEventODFileEdit);
//    theR2DResultsWidget = WorkflowAppR2D::getInstance()->getTheResultsWidget();
//    theResidualDemandResultsWidget = new ResidualDemandResults(theR2DResultsWidget);

}

ResidualDemandWidget::~ResidualDemandWidget()
{

}

void ResidualDemandWidget::togglePostEventODFileEdit(int state){
    if (state == Qt::Checked){
        postEventFileEditLable->hide();
        pathODFilePost->hide();
    } else {
        postEventFileEditLable->show();
        pathODFilePost->show();
    }
}

void ResidualDemandWidget::clear(void)
{
    QString empty;
    pathCapacityMap = "";
    pathEdges = "";
    pathNodes= "";
    pathPreOD = "";
    pathPostOD = "";
    realizationInputWidget->clear();

    pathEdgesFile->setFilename(pathEdges);
    pathNodesFile->setFilename(pathNodes);
    pathCapacityMapFile->setFilename(pathCapacityMap);
    pathODFilePre->setFilename(pathPreOD);
    pathODFilePost->setFilename(pathPostOD);
    simulationHourList->setText(empty);
    postEventODCheckBox->setChecked(false);
    twoWayEdgeCheckbox->setChecked(false);
    createAnimationCheckbox->setChecked(true);
    damageInputMethodComboBox->setCurrentIndex(0);
    if (resultWidget != nullptr) {
        resultWidget->clear();
    }
    if (theR2DResultsWidget != nullptr) {
        theR2DResultsWidget->clear();
    }

}

void ResidualDemandWidget::selectComponents(void)
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

void ResidualDemandWidget::handleInputTypeChanged(){
    if (damageInputMethodComboBox->currentIndex()==1){
        realizationToAnalyzeLabel->hide();
        realizationInputWidget->hide();
    } else if (damageInputMethodComboBox->currentIndex()==0){
        realizationInputWidget->show();
        realizationToAnalyzeLabel->show();
    }
}


bool ResidualDemandWidget::copyFiles(QString &dirName){
    QString fileText;
    QFileInfo fileInfo;
    // edges
    fileText = pathEdgesFile->getFilename();
    fileInfo.setFile(fileText);

    if (!fileInfo.exists())
    {
        this->errorMessage("The edges file path does not exist.");
        return false;
    }
    this->copyFile(fileText, dirName);
    // nodes
    fileText = pathNodesFile->getFilename();
    fileInfo.setFile(fileText);

    if (!fileInfo.exists())
    {
        this->errorMessage("The edges file path does not exist.");
        return false;
    }
    this->copyFile(fileText, dirName);
    // traffic demand pre
    fileText = pathODFilePre->getFilename();
    fileInfo.setFile(fileText);

    if (!fileInfo.exists())
    {
        this->errorMessage("The traffic demand file path does not exist.");
        return false;
    }
    this->copyFile(fileText, dirName);
    // traffic demand post
    if (!postEventODCheckBox->isChecked()){
        fileText = pathODFilePost->getFilename();
        fileInfo.setFile(fileText);

        if (!fileInfo.exists())
        {
            this->errorMessage("The traffic demand file path does not exist.");
            return false;
        }
        this->copyFile(fileText, dirName);
    }
    // Capacity reduction mapp
    fileText = pathCapacityMapFile->getFilename();
    fileInfo.setFile(fileText);

    if (!fileInfo.exists())
    {
        this->errorMessage("The capacity reduction file path does not exist.");
        return false;
    }
    this->copyFile(fileText, dirName);

    // Create the config file
    QString realizationsString = realizationInputWidget->getComponentAnalysisList();
    QJsonObject config;
    config.insert("TwoWayEdges", twoWayEdgeCheckbox->isChecked());
    config.insert("CreateAnimation", createAnimationCheckbox->isChecked());

    // Move the capacity map to config file
    QFile capacityMapfile(pathCapacityMapFile->getFilename());
    if (!capacityMapfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString error =  QString("Could not open file for reading:") + pathCapacityMapFile->getFilename();
        this->errorMessage(error);
        return false;
    }
    QByteArray jsonData = capacityMapfile.readAll();
    capacityMapfile.close();
    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &parseError);

    // Check for parsing errors
    if (parseError.error != QJsonParseError::NoError) {
        QString error = QString("JSON parse error:") + parseError.errorString();
        this->errorMessage(error);
        return false;
    }

    // Convert QJsonDocument to QJsonObject
    if (!jsonDoc.isObject()) {
        QString error = QString("The JSON data is not an object.");
        this->errorMessage(error);
        return false;
    }

    QJsonObject jsonObj = jsonDoc.object();
    config.insert("CapacityMap", jsonObj);

    QJsonObject damageInputJsonObject;
    if (damageInputMethodComboBox->currentIndex()==1){
        damageInputJsonObject.insert("Type", "MostlikelyDamageState");
    } else if(damageInputMethodComboBox->currentIndex()==0) {
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
            return false;
        }
    }
    config.insert("HourList", jsonArray);

    // Save the Recovery Main Input JSON file
    QString pathToConfigFile = dirName + QDir::separator() + "residual_demand_config.json";
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
    return true;
}

bool ResidualDemandWidget::inputAppDataFromJSON(QJsonObject &jsonObject) {
    if (jsonObject.contains("Application")) {
        if ("ResidualDemand" != jsonObject["Application"].toString()) {
            this->errorMessage("ResidualDemandWidget::inputAppDataFromJSON app name conflict");
            return false;
        }
    }

    if (!jsonObject.contains("ApplicationData")) {
        this->errorMessage("Could not find the 'ApplicationData' key in 'ResidualDemandWidget' input");
        return false;
    }

    QJsonObject appData = jsonObject["ApplicationData"].toObject();

    // Damage Input
    if (appData.contains("DamageInput")) {
        QJsonObject damageInputObj = appData["DamageInput"].toObject();
        if (damageInputObj.contains("Type")) {
            QString damageInputType = damageInputObj["Type"].toString();
            if (damageInputType == "MostlikelyDamageState") {
                damageInputMethodComboBox->setCurrentIndex(1);
            } else if (damageInputType == "SpecificRealization") {
                damageInputMethodComboBox->setCurrentIndex(0);
                if (damageInputObj.contains("Parameters")) {
                    QJsonObject damageInputPara = damageInputObj["Parameters"].toObject();
                    if (damageInputPara.contains("Filter")) {
                        realizationInputWidget->setText(damageInputPara["Filter"].toString());
                        this->selectComponents();
                    }
                }
            } else {
                this->errorMessage(QString("The damage input method ") + damageInputType
                                   + QString(" is not valid for Residual Demand."));
                return false;
            }
        }
    }

    // TwoWay Assumption
    if (appData.contains("TwoWayEdges")) {
        twoWayEdgeCheckbox->setChecked(appData["TwoWayEdges"].toBool());
    }

    // Create Animation
    if (appData.contains("CreateAnimation")) {
        createAnimationCheckbox->setChecked(appData["CreateAnimation"].toBool());
    }

    // HourList
    if (appData.contains("HourList")) {
        QString hourListString = "";
        if (appData["HourList"].isString()) {
            simulationHourList->setText(appData["HourList"].toString());
        } else if (appData["HourList"].isArray()) {
            QJsonArray hourListArray = appData["HourList"].toArray();
            for (const QJsonValue &value : hourListArray) {
                if (value.isDouble()) {
                    double num = value.toDouble();
                    // Check if num is int
                    if (num == static_cast<int>(num)) {
                        int intNum = static_cast<int>(num);
                        hourListString += QString::number(intNum);
                        hourListString += ",";
                    } else {
                        this->errorMessage(QString("Only integers are allowed in hour list array of Residual Demand input."));
                        return false;
                    }
                } else {
                    this->errorMessage(QString("The hour list array is invalid in Residual Demand input"));
                    return false;
                }
            }
            // Remove the last ","
            if (hourListString.endsWith(",")) {
                hourListString.chop(1);
            }
            simulationHourList->setText(hourListString);
        } else {
            this->errorMessage(QString("The hour list array is invalid in Residual Demand input"));
            return false;
        }
    }

    // Capacity map
    if (appData.contains("CapacityMap")) {
        pathCapacityMap = appData["CapacityMap"].toString();
        pathCapacityMapFile->setFilename(pathCapacityMap);
    }

    // NetworkEdgesGeojson
    if (appData.contains("NetworkEdgesGeojson")) {
        pathEdges = appData["NetworkEdgesGeojson"].toString();
        pathEdgesFile->setFilename(pathEdges);
    }

    // NetworkNodesGeojson
    if (appData.contains("NetworkNodesGeojson")) {
        pathNodes = appData["NetworkNodesGeojson"].toString();
        pathNodesFile->setFilename(pathNodes);
    }

    // PreEventDemand
    if (appData.contains("PreEventDemand")) {
        pathPreOD = appData["PreEventDemand"].toString();
        pathODFilePre->setFilename(pathPreOD);
    }

    // PostEventDemand
    if (appData.contains("PostEventDemand")) {
        pathPostOD = appData["PostEventDemand"].toString();
        pathODFilePost->setFilename(pathPostOD);
    }

    // PostEventDemand check
    if (appData.contains("ConstantDemand")) {
        postEventODCheckBox->setChecked(appData["ConstantDemand"].toBool());
    }

    return true;
}

QString getFileBaseName(QString fullPath){
    QFileInfo fileInfo(fullPath);
    QString fileName = fileInfo.fileName();
    return fileName;
}

bool ResidualDemandWidget::outputAppDataToJSON(QJsonObject &jsonObject){
    jsonObject.insert("Application","ResidualDemand");
    QJsonObject appDataObj;
    appDataObj.insert("edgeFile", getFileBaseName(pathEdgesFile->getFilename()));
    appDataObj.insert("nodeFile", getFileBaseName(pathNodesFile->getFilename()));
    appDataObj.insert("ODFilePre", getFileBaseName(pathODFilePre->getFilename()));
    if (postEventODCheckBox->isChecked()){
        appDataObj.insert("ODFilePost", getFileBaseName(pathODFilePre->getFilename()));
    } else {
        appDataObj.insert("ODFilePost", getFileBaseName(pathODFilePost->getFilename()));
    }

    appDataObj.insert("configFile", "residual_demand_config.json");
    jsonObject.insert("ApplicationData",appDataObj);
    return true;
}

SC_ResultsWidget* ResidualDemandWidget::getResultsWidget(QWidget *parent, QWidget *R2DresWidget, QMap<QString, QList<QString>> assetTypeToType)
{
    if (resultWidget==nullptr){
        resultWidget = new ResidualDemandResults(parent);
    }
    return resultWidget;
}

bool
ResidualDemandWidget::outputCitation(QJsonObject &jsonObject)
{
  QJsonObject citation;
  citation.insert("citation",
"Zhao, Bingyu, Krishna Kumar, Gerry Casey, and Kenichi Soga. 2019. “Agent-Based Model (ABM) for City-Scale Traffic Simulation: A Case Study on San Francisco,” International Conference on Smart Infrastructure and Construction 2019 (ICSIC). January 2019, 203-212, https://doi.org/10.1680/icsic.64669.203");
  citation.insert("description",
"This reference described the development of the Residual Demand traffic flow simulator.");



  QJsonArray citationsArray;
  citationsArray.push_back(citation);

  jsonObject.insert("citations", citationsArray);

  return true;
}


