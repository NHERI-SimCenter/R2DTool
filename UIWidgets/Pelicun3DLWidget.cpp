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

// Written by: 
//   Stevan Gavrilovic
//   Adam Zsarnoczay

#include "Pelicun3DLWidget.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QPushButton>
#include <QGridLayout>
#include <QGroupBox>
#include <QIntValidator>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>

Pelicun3DLWidget::Pelicun3DLWidget(QString assType, QWidget *parent):
  SimCenterAppWidget(parent), assetType(assType)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    QGroupBox* groupBox = new QGroupBox(this);
    groupBox->setTitle("Pelicun Damage and Loss Simulation Engine");

    QGridLayout* gridLayout = new QGridLayout(groupBox);

    QLabel* typeLabel = new QLabel(tr("Damage and Loss Method:"),this);
    DLTypeComboBox = new QComboBox(this);
    
    if (assetType == "Buildings") {
        DLTypeComboBox->addItem("Hazus Earthquake - Buildings");
        DLTypeComboBox->addItem("Hazus Earthquake - Stories");
        DLTypeComboBox->addItem("Hazus Hurricane Wind - Buildings");
        DLTypeComboBox->addItem("Hazus Hurricane Wind & Storm Surge - Buildings");
    } 
    else if (assetType == "Transportation"){
        DLTypeComboBox->addItem("Hazus Earthquake - Transportation");    
    }
    else if (assetType == "Power"){
        DLTypeComboBox->addItem("Hazus Earthquake - Electric Power");    
    }
    else if (assetType == "Water"){
        DLTypeComboBox->addItem("Hazus Earthquake - Potable Water");    
    }

    DLTypeComboBox->addItem("User-provided Models");
    DLTypeComboBox->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);

    connect(DLTypeComboBox,&QComboBox::currentTextChanged, this, &Pelicun3DLWidget::handleComboBoxChanged);

    QLabel* realizationsLabel = new QLabel(tr("Sample size:"));
    realizationsLineEdit = new QLineEdit();

    QIntValidator* validator = new QIntValidator(this);
    validator->setBottom(0);
    realizationsLineEdit->setValidator(validator);

    /*
    QLabel* eventTimeLabel = new QLabel(tr("Event time:"));
    eventTimeComboBox = new QComboBox();
    eventTimeComboBox->addItem("on");
    eventTimeComboBox->addItem("off");
    eventTimeComboBox->setCurrentIndex(1);
    */

    detailedResultsCheckBox = new QCheckBox("Output detailed results");
    logFileCheckBox = new QCheckBox("Log file",this);
    coupledEDPCheckBox = new QCheckBox("Coupled EDP");
    groundFailureCheckBox= new QCheckBox("Include ground failure");

    lifelineFacilityCheckBox = new QCheckBox("Use Lifeline Facility models");
    lifelineFacilityCheckBox->setToolTip("Use PGA-based damage models developed for lifeline facilities.");

    /*
    autoPopulateScriptWidget = new QWidget();
    auto autoPopScriptLabel = new QLabel("Auto-population script:");
    autoPopulationScriptLineEdit = new QLineEdit();
    auto browseButton = new QPushButton("Browse");
    connect(browseButton,&QPushButton::pressed,this,&Pelicun3DLWidget::handleBrowseButton1Pressed);
    */
    
    auto autoPopulateScriptLayout = new QHBoxLayout(autoPopulateScriptWidget);
    // autoPopulateScriptLayout->addWidget(autoPopScriptLabel);
    // autoPopulateScriptLayout->addWidget(autoPopulationScriptLineEdit);
    // autoPopulateScriptLayout->addWidget(browseButton);

    customModelDirWidget = new QWidget();
    auto customModelDirLabel = new QLabel("Folder with user-provided model data:");
    customModelDirLineEdit = new QLineEdit();
    auto browseButton2 = new QPushButton("Browse");
    connect(browseButton2,&QPushButton::pressed,this,&Pelicun3DLWidget::handleBrowseButton2Pressed);

    auto customModelDirLayout = new QHBoxLayout(customModelDirWidget);
    customModelDirLayout->addWidget(customModelDirLabel);
    customModelDirLayout->addWidget(customModelDirLineEdit);
    customModelDirLayout->addWidget(browseButton2);

    auto Vspacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);

    gridLayout->addWidget(typeLabel,0,0);
    gridLayout->addWidget(DLTypeComboBox,0,1);
    //gridLayout->addWidget(eventTimeLabel,1,0);
    //gridLayout->addWidget(eventTimeComboBox,1,1);
    gridLayout->addWidget(realizationsLabel,2,0);
    gridLayout->addWidget(realizationsLineEdit,2,1);

    gridLayout->addWidget(detailedResultsCheckBox,3,0);
    gridLayout->addWidget(logFileCheckBox,4,0);
    gridLayout->addWidget(coupledEDPCheckBox,5,0);
    gridLayout->addWidget(groundFailureCheckBox,6,0);
    gridLayout->addWidget(lifelineFacilityCheckBox,7,0);
    //gridLayout->addWidget(autoPopulateScriptWidget,7,0,1,2);
    gridLayout->addWidget(customModelDirWidget,8,0,1,2);

    gridLayout->addItem(Vspacer,9,0,1,2);

    layout->addWidget(groupBox);

    this->clear();


}


bool Pelicun3DLWidget::outputAppDataToJSON(QJsonObject &jsonObject)
{

    jsonObject.insert("Application","Pelicun3");

    QJsonObject appDataObj;

    appDataObj.insert("DL_Method",DLTypeComboBox->currentText());

    if (appDataObj["DL_Method"] == "Hazus Hurricane Wind & Storm Surge - Buildings") {
        appDataObj.insert(
            "DL_Method",
            "Hazus Hurricane Wind - Buildings, Hazus Hurricane Storm Surge - Buildings"
        );
    }

    appDataObj.insert("Realizations",realizationsLineEdit->text().toInt());
    appDataObj.insert("detailed_results",detailedResultsCheckBox->isChecked());
    appDataObj.insert("log_file",logFileCheckBox->isChecked());
    appDataObj.insert("coupled_EDP",coupledEDPCheckBox->isChecked());
    //appDataObj.insert("event_time",eventTimeComboBox->currentText());
    appDataObj.insert("ground_failure",groundFailureCheckBox->isChecked());
    appDataObj.insert("lifeline_facility",lifelineFacilityCheckBox->isChecked());
    appDataObj.insert("regional", "true");

    /*
    QString autoScript = autoPopulationScriptLineEdit->text();

    if (autoScript.contains("PelicunDefault")){
      appDataObj.insert("auto_script", autoScript);
    } else {
        QFileInfo test_auto(autoScript);        
        if (test_auto.exists()) {
            appDataObj.insert("auto_script", test_auto.fileName());
            appDataObj.insert("path_to_auto_script", test_auto.path());
        } else {
            this->errorMessage(
                "Missing auto-population script file or the specified "
                "auto-population script does not exist: " + autoScript);
            return false;
        }
    }
    */

    // only deal with the custom model dir if needed
    if(!customModelDirWidget->isHidden())
    {
        auto customModelDir = customModelDirLineEdit->text();

        QFileInfo test_auto(customModelDir);        
        if (test_auto.exists()) {
            // custom_model_dir holds the relative location of these files in the input_dir
            appDataObj.insert("custom_model_dir", "CustomDLModels");
            // the path_to_custom_model dir is the path to the source 
            appDataObj.insert("path_to_custom_model_dir", customModelDir);
        } else {
            this->errorMessage(
                "Missing user-provided model data or the specified data "
                "location does not exist: " + customModelDir);
            return false;
        }
    }

    jsonObject.insert("ApplicationData",appDataObj);

    return true;
}


bool Pelicun3DLWidget::inputAppDataFromJSON(QJsonObject &jsonObject)
{
    if (jsonObject.contains("ApplicationData"))
    {
        QJsonObject appData = jsonObject["ApplicationData"].toObject();

        QString dlMethod;
        if (appData.contains("DL_Method"))
        {

            // backwards compatibility
            QMap<QString, QString> dlMethodMap;

            if (assetType == "Buildings") {
                dlMethodMap["HAZUS MH EQ IM"] = "Hazus Earthquake - Buildings";
                dlMethodMap["HAZUS MH EQ CSM"] = "Hazus Earthquake - Buildings";
                dlMethodMap["HAZUS MH EQ Story"] = "Hazus Earthquake - Stories";
                dlMethodMap["HAZUS MH HU"] = "Hazus Hurricane Wind & Storm Surge - Buildings";
                dlMethodMap["HAZUS MH EQ"] = "Hazus Earthquake - Stories";
            } 
            else if (assetType == "Transportation"){
                dlMethodMap["HAZUS MH EQ IM"] = "Hazus Earthquake - Transportation";
            }
            else if (assetType == "Power"){
                dlMethodMap["HAZUS MH EQ IM"] = "Hazus Earthquake - Electric Power";
            }
            else if (assetType == "Water"){
                dlMethodMap["HAZUS MH EQ IM"] = "Hazus Earthquake - Potable Water";
            }

            dlMethodMap["User-provided Fragilities"] = "User-provided Models";

            if (dlMethodMap.contains(appData["DL_Method"].toString())) {

                if ((appData["DL_Method"].toString() == "HAZUS MH EQ IM") && 
                    (assetType == "Buildings")
                    ){
                    appData["lifeline_facility"] = true;
                }

                dlMethod = dlMethodMap[appData["DL_Method"].toString()];
            } else {
                dlMethod = appData["DL_Method"].toString();    
            }

            auto index = DLTypeComboBox->findText(dlMethod);

            if(index == -1)
            {
                this->errorMessage(
                    "The Damage and Loss method specified in the input file is "
                    "not available in this version of R2D:"+dlMethod);
                return false;
            }

            DLTypeComboBox->setCurrentIndex(index);
        }

        if (appData.contains("Realizations")) {
            if (appData["Realizations"].isString()){
                realizationsLineEdit->setText(appData["Realizations"].toString());
            } else {
                realizationsLineEdit->setText(QString::number(appData["Realizations"].toInt()));
            }
        }

        if (appData.contains("coupled_EDP"))
            coupledEDPCheckBox->setChecked(appData["coupled_EDP"].toBool());

        if (appData.contains("detailed_results"))
            detailedResultsCheckBox->setChecked(appData["detailed_results"].toBool());

        if (appData.contains("ground_failure"))
            groundFailureCheckBox->setChecked(appData["ground_failure"].toBool());

        if (appData.contains("lifeline_facility"))
            lifelineFacilityCheckBox->setChecked(appData["lifeline_facility"].toBool());

        if (appData.contains("log_file"))
            logFileCheckBox->setChecked(appData["log_file"].toBool());

        //if (appData.contains("event_time"))
        //    eventTimeComboBox->setCurrentText(appData["event_time"].toString());

        /*
        if (appData.contains("auto_script")){
            auto autoScript = appData["auto_script"].toString();

            if (autoScript.startsWith("PelicunDefault")){
                autoPopulationScriptLineEdit->setText(autoScript);
            } else {

                QString autoScriptPath;

                if (appData.contains("path_to_auto_script")) {
                    autoScriptPath = appData["path_to_auto_script"].toString();

                } else {
                    autoScriptPath = QDir::currentPath();
                }

                auto fullPathToAutoScript = autoScriptPath + QDir::separator() + autoScript;

                QFileInfo test_auto(fullPathToAutoScript);
                if (test_auto.exists()) {
                    autoPopulationScriptLineEdit->setText(fullPathToAutoScript);
                } else {
                    this->infoMessage(
                        "Warning: the auto-population script file does not "
                        "exist: "+fullPathToAutoScript);
                }
            }
        }
        */

        // only check the custom model dir when we have user-provided models
        if (dlMethod == "User-provided Models"){

            // Note that custom_model_dir is only used in the backend, we do not
            // need to load it here

            if (appData.contains("path_to_custom_model_dir")){

                QString customModelDir = appData["path_to_custom_model_dir"].toString();

                QDir fileInfo;
                if (fileInfo.exists(customModelDir)){
                    customModelDirLineEdit->setText(customModelDir);

                } else {
                    this->infoMessage(
                        "Warning: the directory for custom model files does not "
                        "exist: "+customModelDir);

                }

            }
        }
    }

    return true;
}


void Pelicun3DLWidget::clear(void)
{
    DLTypeComboBox->setCurrentIndex(0);

    realizationsLineEdit->setText("500");

    //eventTimeComboBox->setCurrentIndex(1);
    detailedResultsCheckBox->setChecked(false);
    logFileCheckBox->setChecked(true);
    coupledEDPCheckBox->setChecked(true);

    this->handleComboBoxChanged(DLTypeComboBox->currentText());

}

void Pelicun3DLWidget::clearParams(void)
{   
    groundFailureCheckBox->hide();
    groundFailureCheckBox->setChecked(false);

    lifelineFacilityCheckBox->hide();
    lifelineFacilityCheckBox->setChecked(false);   
    
    //autoPopulationScriptLineEdit->clear();

    customModelDirWidget->hide();
    customModelDirLineEdit->clear();
}



bool Pelicun3DLWidget::copyFiles(QString &destName)
{
    /*
    auto autoScript = autoPopulationScriptLineEdit->text();

    if (autoScript.contains("PelicunDefault")){
        // pass

    } else if (autoScript.isEmpty()) {

        this->errorMessage(
            "The auto-population script for damage and loss assessment is "
            "missing");
            return false;

    } else {

        QFileInfo autoScriptFile(autoScript);

        if (!autoScriptFile.exists()) {
            this->errorMessage(
                "Could not find auto script file: "+ autoScript);
            return false;
        }

        QDir fileDir(autoScriptFile.absolutePath());
        QString lastDir = fileDir.dirName();
        if (lastDir != QString("input_data")) {

            bool res = this->copyPath(
                autoScriptFile.absolutePath(), destName, false);

            if (!res) {
                this->errorMessage(
                    "Could not copy the directory containing auto script file: "
                    + autoScript);
                return res;
            }

        } else {

            // Do not copy the file, output a new csv which will have the 
            // changes that the user makes in the table
            // WHAT DOES THE ABOVE COMMENT MEAN??? /azs
            bool res = this->copyFile(autoScript, destName);

            if (!res) {
                this->errorMessage(
                    "Could not copy auto script file: " + autoScript);
                return res;
            }
        }
    }
    */

    // only copy model data if needed

    if(!customModelDirWidget->isHidden())
    {
        auto customModelDirPath = customModelDirLineEdit->text();

        if (customModelDirPath.isEmpty()){
            this->errorMessage(
                "The user-provided model data for damage and loss assessment is "
                "missing");
            return false;

        } else {

            QFileInfo customModelDir(customModelDirPath);

            if (!customModelDir.exists()) {
                this->errorMessage(
                    "Could not find custom model directory: "
                    + customModelDirPath);
                return false;
            }

            // Destination folder path is hard-coded. 
            // Edit only if you know what you're doing.
            auto destinationDirectory = 
                destName + QDir::separator() + "CustomDLModels";

            auto res = recursiveCopy(customModelDirPath, destinationDirectory);

            if (!res) {
                this->errorMessage(
                    "Could not copy the directory containing custom model files: "
                    + customModelDirPath);
                return res;
            }
        }
    }

    return true;
}


void Pelicun3DLWidget::handleComboBoxChanged(const QString &text)
{
    this->clearParams();

    if (text.contains("Hazus Earthquake - Buildings")) {
        groundFailureCheckBox->show();
        lifelineFacilityCheckBox->show();

    } else if (text.contains("Hazus Hurricane")) {
        // placeholder

    } else if (text.contains("User")) {
        customModelDirWidget->show();
    }
}


bool Pelicun3DLWidget::recursiveCopy(const QString &sourcePath, const QString &destPath)
{
    QFileInfo srcFileInfo(sourcePath);

    if (srcFileInfo.isDir())
    {
        QDir targetDir(destPath);

        if (!targetDir.mkpath(destPath))
            return false;

        QDir sourceDir(sourcePath);

        QStringList fileNames = sourceDir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);

        foreach (const QString &fileName, fileNames)
        {
            const QString newSrcFilePath = sourcePath + QDir::separator() + fileName;
            const QString newDestFilePath = destPath  + QDir::separator() + fileName;

            if (!recursiveCopy(newSrcFilePath, newDestFilePath))
                return false;
        }
    } else {

        // Rewrite the file in the event that it already exists
        if(QFile::exists(destPath))
            if(!QFile::remove(destPath))
                return false;

        if (!QFile::copy(sourcePath, destPath))
            return false;
    }

    return true;
}

/*
void Pelicun3DLWidget::handleBrowseButton1Pressed(void)
{

    QFileDialog dialog(this);
    QString scriptFile = QFileDialog::getOpenFileName(this,tr("Auto-population Script"));
    dialog.close();

    // Return if the user cancels or enters same file
    if(scriptFile.isEmpty())
        return;

    autoPopulationScriptLineEdit->setText(scriptFile);
} 
*/

void Pelicun3DLWidget::handleBrowseButton2Pressed(void)
{
    QFileDialog dialog(this);
    QString customModelDir = QFileDialog::getExistingDirectory(this,tr("Folder with user-provided model data"));
    dialog.close();

    // Return if the user cancels or enters same file
    if(customModelDir.isEmpty())
        return;

    customModelDirLineEdit->setText(customModelDir);
}

//QMainWindow* Pelicun3DLWidget::getPostProcessor(QWidget *parent, SimCenterAppWidget* visWidget){
//    VisualizationWidget* visWidgetDownCast = dynamic_cast<VisualizationWidget>
//    QMainWindow* returnPtr = dynamic_cast<QMainWindow*> (new Pelicun3PostProcessor());
//    return returnPtr;
//};
SC_ResultsWidget* Pelicun3DLWidget::getResultsWidget(QWidget *parent, QWidget *R2DresWidget, QMap<QString, QList<QString>> assetTypeToType)
{
    if (resultWidget==nullptr){
        resultWidget = new Pelicun3PostProcessor(parent, R2DresWidget, assetTypeToType);
    }
    return resultWidget;
}


bool
Pelicun3DLWidget::outputCitation(QJsonObject &jsonObject)
{
  QJsonObject citationPelicun;
  citationPelicun.insert("citation",
"Adam Zsarnoczay, John Vouvakis Manousakis, Jinyan Zhao, Kuanshi Zhong, \
Pouria Kourehpaz (2024). NHERI-SimCenter/pelicun: v3.3. \
Zenodo. https://doi.org/10.5281/zenodo.10896145");
  citationPelicun.insert("description",
"This reference indicates the version of the tool used for the simulation.");

  QJsonObject citationPelicunMarker;
  citationPelicunMarker.insert("citation",
"Adam Zsarnoczay, Gregory G. Deierlein, \
PELICUN - A Computational Framework for Estimating Damage, Loss, and Community \
Resilience, \
Proceedings of the 17th World Conference on Earthquake Engineering, Japan, 2020");
  citationPelicunMarker.insert("description",
"This paper describes the Pelicun damage and loss assessment framework. Please \
reference it if your work results from using the Pelicun engine in the SimCenter \
tools.");

  QJsonArray citationsArray;
  citationsArray.push_back(citationPelicun);
  citationsArray.push_back(citationPelicunMarker);

  jsonObject.insert("citations", citationsArray);
  
  return true;
}

