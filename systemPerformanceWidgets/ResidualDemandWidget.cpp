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
                                                    "Most likely damage state",
                                                    "Specific realization(s)"
                                                }));
    mainLayout->addWidget(new QLabel("Damage State Input:"), numRow, 0, 1, 1);
    mainLayout->addWidget(damageInputMethodComboBox, numRow, 1, 1, 1);

    realizationToAnalyzeLabel = new QLabel("Realizations to Analyze:");
    mainLayout->addWidget(realizationToAnalyzeLabel, numRow, 2, 1, 1);
    realizationInputWidget = new AssetInputDelegate();
    mainLayout->addWidget(realizationInputWidget, numRow, 3, 1, 3);
    connect(this->damageInputMethodComboBox, &QComboBox::currentTextChanged,
            this, &ResidualDemandWidget::handleInputTypeChanged);
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

    // 4. Path To Nodes File
    pathODFile = new SC_FileEdit("od_file");
    mainLayout->addWidget(new QLabel("Traffic Demand File:"), numRow, 0, 1, 1);
    mainLayout->addWidget(pathODFile, numRow, 1, 1, 4);
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

//    theR2DResultsWidget = WorkflowAppR2D::getInstance()->getTheResultsWidget();
//    theResidualDemandResultsWidget = new ResidualDemandResults(theR2DResultsWidget);

}

ResidualDemandWidget::~ResidualDemandWidget()
{

}

void ResidualDemandWidget::clear(void)
{

}
void ResidualDemandWidget::handleInputTypeChanged(){
    if (damageInputMethodComboBox->currentIndex()==0){
        realizationToAnalyzeLabel->hide();
        realizationInputWidget->hide();
    } else if (damageInputMethodComboBox->currentIndex()==1){
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
    // traffic demand
    fileText = pathODFile->getFilename();
    fileInfo.setFile(fileText);

    if (!fileInfo.exists())
    {
        this->errorMessage("The traffic demand file path does not exist.");
        return false;
    }
    this->copyFile(fileText, dirName);
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
        jsonArray.append(str.toInt());
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
}

