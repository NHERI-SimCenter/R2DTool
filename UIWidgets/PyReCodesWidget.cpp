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

// Written by: Sina Naeimi

#include "BrailsTranspInventoryGenerator.h"
#include "QGISVisualizationWidget.h"
#include "SimCenterMapcanvasWidget.h"
#include "GIS_Selection.h"
#include <qgsmapcanvas.h>
#include <PlainRectangle.h>
#include <BrailsGoogleDialog.h>
#include <SimCenterPreferences.h>
#include "ModularPython.h"
#include "PyReCodesWidget.h"
#include "AssetInputDelegate.h"

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

PyReCoDesWidget::PyReCoDesWidget(VisualizationWidget* visWidget, QWidget *parent) : SimCenterAppWidget(parent)
{
    QString workDir = SimCenterPreferences::getInstance()->getLocalWorkDir();
    
    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);

    QString inputDir = workDir + QDir::separator() + QString("tmp.SimCenter") + QDir::separator() + QString("input_data");
    QDir dir(inputDir);
    if (!dir.exists())
      dir.mkpath(inputDir);

    int numRow = 0;    
    QGridLayout *mainLayout = new QGridLayout(this);

    // 1. Damage State Data Source DIR
    damageStateDataSrource = new SC_DirEdit("damage_state_data_srource", true);
    mainLayout->addWidget(new QLabel("Damage State Data Source Directory:"), numRow, 0, 1, 1);
    mainLayout->addWidget(damageStateDataSrource, numRow, 1, 1, 3);
    damageStateDataSrource->setDirName(inputDir);

    // 2. Realization Selection
    realizationInputWidget = new AssetInputDelegate();
    mainLayout->addWidget(new QLabel("Damage Realization Selection:"), numRow, 4, 1, 1);
    mainLayout->addWidget(realizationInputWidget, numRow, 5, 1, 2);
    numRow++;

    // 3. Results Directory
    resultsDir = new SC_DirEdit("Results_dir", true);
    mainLayout->addWidget(new QLabel("Results Directory:"), numRow, 0, 1, 1);
    mainLayout->addWidget(resultsDir, numRow, 1, 1, 3);
    numRow++;

    // 4. Path To System Config File
    pathConfigFile = new SC_FileEdit("pyrecodes_config");
    mainLayout->addWidget(new QLabel("Config File:"), numRow, 0, 1, 1);
    mainLayout->addWidget(pathConfigFile, numRow, 1, 1, 3);
    numRow++;

    // 5. Path to the Component library
    pathComponentLibrary = new SC_FileEdit("pyrecodes_component_library");
    mainLayout->addWidget(new QLabel("Component library File:"), numRow, 0, 1, 1);
    mainLayout->addWidget(pathComponentLibrary, numRow, 1, 1, 3);
    numRow++;
    
    // 6. Path to the Component library
    pathLocalityDefinition = new SC_FileEdit("pyrecodes_locality_definitions");
    mainLayout->addWidget(new QLabel("Locality Definition File:"), numRow, 0, 1, 1);
    mainLayout->addWidget(pathLocalityDefinition, numRow, 1, 1, 3);
    numRow++;

    // 7. Path to the Component library
    pathWaterNetwork = new SC_FileEdit("water_network_definition_file");
    mainLayout->addWidget(new QLabel("Water Network Definiton File:"), numRow, 0, 1, 1);
    mainLayout->addWidget(pathWaterNetwork, numRow, 1, 1, 3);
    numRow++;

    // 8. Run Simulation Button
    runButton = new QPushButton("Run Simulation");
    mainLayout->addWidget(runButton, numRow, 1, 1, 2);
    connect(runButton, &QPushButton::clicked, this, &PyReCoDesWidget::runSimulation);
    numRow++;

    mainLayout->setRowStretch(numRow, 1);
    mainLayout->setColumnStretch(5, 1);

    QString dirPath = QString("C:/Users/naeim/OneDrive/Desktop/input_data_PY/tmp.SimCenter_rwhale/input_data");

    QString dirPath_damageStateDataSrource = QString("C:/Users/naeim/OneDrive/Documents/R2D/LocalWorkDir/tmp.SimCenter/Results");
    QString dirPath_resultsDir = QString("C:/Users/naeim/OneDrive/Documents/R2D/LocalWorkDir/tmp.SimCenter/Results");
    QString dirPath_pathConfigFile = dirPath + QDir::separator() + "Alameda_SystemConfiguration.json";\
    QString dirPath_pathComponentLibrary = dirPath + QDir::separator() + "Alameda_ComponentLibrary.json";
    QString dirPath_pathLocalityDefinition = dirPath + QDir::separator() + "Alameda_TravelAnalysisZones.geojson";
    
    damageStateDataSrource->setDirName(dirPath_damageStateDataSrource);
    resultsDir->setDirName(dirPath_resultsDir);
    QString filePath = dirPath + QDir::separator() + "Alameda_Case_Study" + QDir::separator() + "Alameda_Main.json";
    pathConfigFile->setFilename(dirPath_pathConfigFile);
    pathComponentLibrary->setFilename(dirPath_pathComponentLibrary);
    pathLocalityDefinition->setFilename(dirPath_pathLocalityDefinition);
    filePath = dirPath + QDir::separator() + "waterNetwork.inp";
    pathWaterNetwork->setFilename(filePath);

    realizationInputWidget->setText("1,");



}

PyReCoDesWidget::~PyReCoDesWidget()
{

}

void PyReCoDesWidget::clear(void)
{

}

void PyReCoDesWidget::runSimulation(void)
{
  
    QString workDir = SimCenterPreferences::getInstance()->getLocalWorkDir();
    QString appInputDir = workDir + QDir::separator() + QString("tmp.SimCenter") + QDir::separator() + QString("input_data");
    
    // Check for any missing files and directories

    QString realizationString = realizationInputWidget->text();
    
    QDir dir(appInputDir);
    if (!dir.exists())
        dir.mkpath(appInputDir);

    auto damageStateDataSrource_dir = damageStateDataSrource->getDirName();
    dir.setPath(damageStateDataSrource_dir);

    if (damageStateDataSrource_dir.isEmpty() || !dir.exists()){
        this->errorMessage("Missing the Damage State Data Source Directory. Please select a directory.");
        return;
    }

    if (realizationString.isEmpty()){
        this->errorMessage("Please select at least one realization.");
        return;
    }

    auto resultsDir_dir = resultsDir->getDirName();
    dir.setPath(resultsDir_dir);
    if (resultsDir_dir.isEmpty() || !dir.exists()){
        this->errorMessage("Missing the Result Directory. Please select a directory.");
        return;
    }

    auto pathConfigFile_path = pathConfigFile->getFilename();
    if (pathConfigFile_path.isEmpty() || !QFile::exists(pathConfigFile_path)){
        this->errorMessage("Missing the PyReCoDes Config file. Please select the PyReCoDes Config file.");
        return;
    }

    auto pathComponentLibraryFile_path = pathComponentLibrary->getFilename();
    if (pathComponentLibraryFile_path.isEmpty() || !QFile::exists(pathComponentLibraryFile_path)){
        this->errorMessage("Missing the PyReCoDes Component Library file. Please select the PyReCoDes Component Library file.");
        return;
    }

    auto localityDefinitionFile_path = pathLocalityDefinition->getFilename();
    if (localityDefinitionFile_path.isEmpty() || !QFile::exists(localityDefinitionFile_path)){
        this->errorMessage("Missing the PyReCoDes Locality Definition file. Please select the PyReCoDes Locality Definition file.");
        return;
    }

    auto inpFile_path = pathWaterNetwork->getFilename();
    if (inpFile_path.isEmpty() || !QFile::exists(inpFile_path)){
        this->errorMessage("Missing the Water Distribution Network file (EPANET *.inp). Please select the Water Distribution Network file.");
        return;
    }


    // Check for any file types
    if (!pathConfigFile_path.toLower().endsWith(".json")){
        this->errorMessage("The PyReCoDes Config file must be a JSON file.");
        return;
    }

    if (!pathComponentLibraryFile_path.toLower().endsWith(".json")){
        this->errorMessage("The PyReCoDes Component Library file must be a JSON file.");
        return;
    }

    if (!localityDefinitionFile_path.toLower().endsWith(".geojson")){
        this->errorMessage("The PyReCoDes Locality Definition file must be a JSON file.");
        return;
    }

    if (!inpFile_path.toLower().endsWith(".inp")){
        this->errorMessage("The Water Distribution Network file must be an EPANET *.inp file.");
        return;
    }

    

    // Create the PyReCoDes Main Input JSON file

    QJsonObject mainFile;
    QJsonObject componentLibraryJsonObject;
    QJsonObject systemConfigJsonObject;
    QJsonObject damageInputJsonObject;

    componentLibraryJsonObject.insert("ComponentLibraryCreatorFileName", "json_component_library_creator");
    componentLibraryJsonObject.insert("ComponentLibraryCreatorClassName", "JSONComponentLibraryCreator");
    componentLibraryJsonObject.insert("ComponentLibraryFile", pathComponentLibraryFile_path);

    systemConfigJsonObject.insert("SystemCreatorClassName", "ConcreteSystemCreator");
    systemConfigJsonObject.insert("SystemCreatorFileName", "concrete_system_creator");
    systemConfigJsonObject.insert("SystemClassName", "BuiltEnvironment");
    systemConfigJsonObject.insert("SystemFileName", "built_environment");
    systemConfigJsonObject.insert("SystemConfigurationFile", pathConfigFile_path);

    QJsonObject damageinputParameterQObject;
    damageinputParameterQObject.insert("Filter", realizationString);
    damageInputJsonObject.insert("Parameters", damageinputParameterQObject);
    damageInputJsonObject.insert("Type", "SpecificRealization");

    mainFile.insert("ComponentLibrary", componentLibraryJsonObject);
    mainFile.insert("System", systemConfigJsonObject);
    mainFile.insert("DamageInput", damageInputJsonObject);

    // Save the PyReCoDes Main Input JSON file
    QString pathToMainFile = appInputDir + QDir::separator() + "pyrecodes_main_input.json";
    QFile file(pathToMainFile);

    QString strFromObj = QJsonDocument(mainFile).toJson(QJsonDocument::Indented);

    if(!file.open(QIODevice::WriteOnly))
    {
        this->errorMessage("Error: Cannot write the PyReCoDes Main Input JSON file.");
        file.close();
    }
    else
    {
        QTextStream out(&file); out << strFromObj;
        file.close();
    }

    // Create Script to run PyReCoDes Wrapper 

    QStringList scriptArgs;
    scriptArgs << QString("--mainFile")  << pathToMainFile;
    scriptArgs << QString("--systemConfigFile")  << pathConfigFile_path;
    scriptArgs << QString("--componentLibraryFile")  << pathComponentLibraryFile_path;
    scriptArgs << QString("--localityGeojsonFile")  << localityDefinitionFile_path;
    scriptArgs << QString("--r2dRunDir")  << damageStateDataSrource_dir;
    // scriptArgs << QString("--resultsDir")  << resultsDir_dir;
    scriptArgs << QString("--INPFile")  << inpFile_path;

    QString scriptPath = SimCenterPreferences::getInstance()->getAppDir() + QDir::separator()
                         + "applications" + QDir::separator() + "performREC" + QDir::separator()
                         + "pyrecodes" + QDir::separator() + "run_pyrecodes.py";

    std::unique_ptr<ModularPython> thePy = std::make_unique<ModularPython>(resultsDir_dir);

    connect(thePy.get(),SIGNAL(runComplete()),this,SLOT(handleProcessFinished()));

    this->handleProcessStarted();
    qDebug() << "Running PyReCoDes\n" << scriptPath << scriptArgs;
    thePy->run(scriptPath, scriptArgs);

    disconnect(thePy.get(),SIGNAL(runComplete()),this,SLOT(handleProcessFinished()));
}

void PyReCoDesWidget::handleProcessStarted()
{
    this->runButton->setEnabled(false);
}

void PyReCoDesWidget::handleProcessFinished()
{
    this->runButton->setEnabled(true);
}
