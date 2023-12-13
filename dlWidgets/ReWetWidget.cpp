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

// Written by: Stevan Gavrilovic

#include "ReWetWidget.h"

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

ReWetWidget::ReWetWidget(QWidget *parent): SimCenterAppWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    QGroupBox* groupBox = new QGroupBox(this);
    groupBox->setTitle("Pelicun Damage and Loss Prediction Methodology");

    QGridLayout* gridLayout = new QGridLayout(groupBox);

    QLabel* typeLabel = new QLabel(tr("Damage and Loss Method:"),this);
    DLTypeComboBox = new QComboBox(this);
//    DLTypeComboBox->addItem("HAZUS MH EQ");

    DLTypeComboBox->addItem("HAZUS MH EQ IM");
//    DLTypeComboBox->addItem("HAZUS MH HU");
    DLTypeComboBox->addItem("User-provided Fragilities");
    DLTypeComboBox->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);

    connect(DLTypeComboBox,&QComboBox::currentTextChanged, this, &ReWetWidget::handleComboBoxChanged);

    QLabel* realizationsLabel = new QLabel(tr("Number of realizations:"));
    realizationsLineEdit = new QLineEdit();

    QIntValidator* validator = new QIntValidator(this);
    validator->setBottom(0);
    realizationsLineEdit->setValidator(validator);
    realizationsLineEdit->setText("5");

    QLabel* eventTimeLabel = new QLabel(tr("Event time:"));
    eventTimeComboBox = new QComboBox();
    eventTimeComboBox->addItem("on");
    eventTimeComboBox->addItem("off");
    eventTimeComboBox->setCurrentIndex(1);

    detailedResultsCheckBox = new QCheckBox("Output detailed results");
    logFileCheckBox = new QCheckBox("Log file",this);
    logFileCheckBox->setChecked(true);
    coupledEDPCheckBox = new QCheckBox("Coupled EDP");
    groundFailureCheckBox= new QCheckBox("Include ground failure");

    autoPopulateScriptWidget = new QWidget();
    auto autoPopScriptLabel = new QLabel("Auto-populate script:");
    autoPopulationScriptLineEdit = new QLineEdit();
    auto browseButton = new QPushButton("Browse");
    connect(browseButton,&QPushButton::pressed,this,&ReWetWidget::handleBrowseButton1Pressed);

    auto autoPopulateScriptLayout = new QHBoxLayout(autoPopulateScriptWidget);
    autoPopulateScriptLayout->addWidget(autoPopScriptLabel);
    autoPopulateScriptLayout->addWidget(autoPopulationScriptLineEdit);
    autoPopulateScriptLayout->addWidget(browseButton);

    fragDirWidget = new QWidget();
    auto fragFolderLabel = new QLabel("Folder containing user-defined fragility function:");
    fragilityDirLineEdit = new QLineEdit();
    auto browseButton2 = new QPushButton("Browse");
    connect(browseButton2,&QPushButton::pressed,this,&ReWetWidget::handleBrowseButton2Pressed);

    auto fragDirLayout = new QHBoxLayout(fragDirWidget);
    fragDirLayout->addWidget(fragFolderLabel);
    fragDirLayout->addWidget(fragilityDirLineEdit);
    fragDirLayout->addWidget(browseButton2);

    auto Vspacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);

    gridLayout->addWidget(typeLabel,0,0);
    gridLayout->addWidget(DLTypeComboBox,0,1);
    gridLayout->addWidget(eventTimeLabel,1,0);
    gridLayout->addWidget(eventTimeComboBox,1,1);
    gridLayout->addWidget(realizationsLabel,2,0);
    gridLayout->addWidget(realizationsLineEdit,2,1);

    gridLayout->addWidget(detailedResultsCheckBox,3,0);
    gridLayout->addWidget(logFileCheckBox,4,0);
    gridLayout->addWidget(coupledEDPCheckBox,5,0);
    gridLayout->addWidget(groundFailureCheckBox,6,0);
    gridLayout->addWidget(autoPopulateScriptWidget,7,0,1,2);
    gridLayout->addWidget(fragDirWidget,8,0,1,2);

    gridLayout->addItem(Vspacer,9,0,1,2);

    layout->addWidget(groupBox);

    autoPopulateScriptWidget->hide();
    fragDirWidget->hide();
}


bool ReWetWidget::outputAppDataToJSON(QJsonObject &jsonObject)
{

    jsonObject.insert("Application","Pelicun3");

    QJsonObject appDataObj;

    appDataObj.insert("DL_Method",DLTypeComboBox->currentText());
    appDataObj.insert("Realizations",realizationsLineEdit->text().toInt());
    appDataObj.insert("detailed_results",detailedResultsCheckBox->isChecked());
    appDataObj.insert("log_file",logFileCheckBox->isChecked());
    appDataObj.insert("coupled_EDP",coupledEDPCheckBox->isChecked());
    appDataObj.insert("event_time",eventTimeComboBox->currentText());
    appDataObj.insert("ground_failure",groundFailureCheckBox->isChecked());
    appDataObj.insert("regional", "true");
    if (DLTypeComboBox->currentText().compare("HAZUS MH EQ IM")==0){
        appDataObj.insert("auto_script", "PelicunDefault/Hazus_Earthquake_IM.py");
    }
    // test separating the path and filename of auto-population codes (KZ)
    QFileInfo test_auto(autoPopulationScriptLineEdit->text());
    if(test_auto.exists())
    {
        appDataObj.insert("auto_script",test_auto.fileName());
        appDataObj.insert("path_to_auto_script",test_auto.path());
    }


    if(!fragDirWidget->isHidden())
    {
        auto sourcePath = fragilityDirLineEdit->text();
        if(sourcePath.isEmpty())
        {
            this->errorMessage("Specify a folder with custom fragility functions");
            return false;
        }

        // Fixed folder name
        QFileInfo test_fragDir(sourcePath);
        if(test_fragDir.exists())
        {
            appDataObj.insert("custom_fragility_dir","CustomFragilities");
        }
    }


    /*
    if(!scriptPath.isEmpty())
        appDataObj.insert("auto_script",scriptPath);
    */

    jsonObject.insert("ApplicationData",appDataObj);

    return true;
}


bool ReWetWidget::inputAppDataFromJSON(QJsonObject &jsonObject)
{
    if (jsonObject.contains("ApplicationData"))
    {
        QJsonObject appData = jsonObject["ApplicationData"].toObject();

        if (appData.contains("DL_Method"))
        {
            auto dlMethod = appData["DL_Method"].toString();
            auto index = DLTypeComboBox->findText(dlMethod);

            if(index == -1)
            {
                this->errorMessage("Could not find the damage and loss method "+dlMethod);
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

        if (appData.contains("log_file"))
            logFileCheckBox->setChecked(appData["log_file"].toBool());

        if (appData.contains("event_time"))
            eventTimeComboBox->setCurrentText(appData["event_time"].toString());

        if (appData.contains("auto_script"))
        {
            auto pathToScript = appData["auto_script"].toString();

            QFileInfo fileInfo;

            if (fileInfo.exists(pathToScript))
            {
                autoPopulationScriptLineEdit->setText(pathToScript);
            }
            else
            {
                // test separating the path and filename of auto-population codes (KZ)
                QString currPath;
                if (appData.contains("path_to_auto_script"))
                    currPath = appData["path_to_auto_script"].toString();
                else
                    currPath = QDir::currentPath();

                auto pathToComponentInfoFile = currPath + QDir::separator() + pathToScript;

                if (fileInfo.exists(pathToComponentInfoFile))
                {
                    autoPopulationScriptLineEdit->setText(pathToComponentInfoFile);
                }
                else
                {
                    // adam .. adam .. adam
                    pathToComponentInfoFile = currPath + QDir::separator()
                            + "input_data" + QDir::separator() + pathToScript;

                    if (fileInfo.exists(pathToComponentInfoFile))
                        autoPopulationScriptLineEdit->setText(pathToComponentInfoFile);
                    else
                        this->infoMessage("Warning: the script file "+pathToScript+ " does not exist");
                }
            }
        }

        if (appData.contains("custom_fragility_dir"))
        {
            QString fragilityDir = appData["custom_fragility_dir"].toString();

            QDir fileInfo;

            if (fileInfo.exists(fragilityDir))
            {
                fragilityDirLineEdit->setText(fragilityDir);
            }
            else
            {
                // Try the current path
                QString currPath = QDir::currentPath();

                QString fullPathToFragilityDir = currPath + QDir::separator() + fragilityDir;

                if (fileInfo.exists(fullPathToFragilityDir))
                {
                    fragilityDirLineEdit->setText(fullPathToFragilityDir);
                }
                else
                {
                    // adam .. adam .. adam
                    fullPathToFragilityDir = currPath + QDir::separator()
                            + "input_data" + QDir::separator() + fragilityDir;

                    if (fileInfo.exists(fullPathToFragilityDir))
                        fragilityDirLineEdit->setText(fullPathToFragilityDir);
                    else {
                        errorMessage("Warning: the fargility dir does not seem to exist: " + fragilityDir);
                        errorMessage("Warning: the fargility dir does not seem to exist: " + fragilityDir);
			errorMessage("tried: " + fullPathToFragilityDir);
			fullPathToFragilityDir = currPath + QDir::separator() + fragilityDir;
			errorMessage("and: " + fullPathToFragilityDir);			
		    }
                }
            }
        }


    }

    return true;
}


void ReWetWidget::clear(void)
{
    DLTypeComboBox->setCurrentIndex(0);
    realizationsLineEdit->clear();
    eventTimeComboBox->setCurrentIndex(1);
    detailedResultsCheckBox->setChecked(false);
    logFileCheckBox->setChecked(false);
    coupledEDPCheckBox->setChecked(false);
    groundFailureCheckBox->setChecked(false);
    autoPopulationScriptLineEdit->clear();
    fragilityDirLineEdit->clear();
}

void ReWetWidget::clearParams(void)
{
    realizationsLineEdit->clear();
    eventTimeComboBox->setCurrentIndex(1);
    detailedResultsCheckBox->setChecked(false);
    logFileCheckBox->setChecked(false);
    coupledEDPCheckBox->setChecked(false);
    groundFailureCheckBox->setChecked(false);
    autoPopulationScriptLineEdit->clear();
    fragilityDirLineEdit->clear();
}



bool ReWetWidget::copyFiles(QString &destName)
{
    auto compLineEditText = autoPopulationScriptLineEdit->text();

    if(compLineEditText.isEmpty())
        return true;

    QFileInfo componentFile(compLineEditText);

    if (!componentFile.exists()) {
        QString msg = QString("Could not find auto script file: ") + compLineEditText;
        this->errorMessage(msg);
        return false;
    }

    QDir fileDir(componentFile.absolutePath());
    QString lastDir = fileDir.dirName();
    if (lastDir != QString("input_data")) {

        bool res = this->copyPath(componentFile.absolutePath(), destName, false);
        if (!res) {
            QString msg = QString("Could not copy dir containing auto script file: ") + compLineEditText;
            this->errorMessage(msg);
            return res;
        }

    } else {

        // Do not copy the file, output a new csv which will have the changes that the user makes in the table
        bool res = this->copyFile(compLineEditText, destName);
        if (!res) {
            QString msg = QString("Could not copy auto script file: ") + compLineEditText;
            this->errorMessage(msg);
            return res;
        }
    }

    if(!fragDirWidget->isHidden())
    {
        auto sourcePath = fragilityDirLineEdit->text();

        if(sourcePath.isEmpty())
        {
            this->errorMessage("Specify a folder with custom fragility functions");
            return false;
        }

        // Folder path fixed Fixed do not change
        auto destinationDirectory = destName + QDir::separator() + "CustomFragilities";

        auto res = recursiveCopy(sourcePath,destinationDirectory);

        if(!res)
            return res;
    }

    return true;
}


void ReWetWidget::handleComboBoxChanged(const QString &text)
{
    this->clearParams();

    if(text.compare("HAZUS MH EQ") == 0 || text.compare("HAZUS MH EQ IM") == 0)
    {
        groundFailureCheckBox->show();
        autoPopulateScriptWidget->hide();
    }
    else
    {
        groundFailureCheckBox->hide();
        autoPopulateScriptWidget->show();
    }

    if(text.compare("User-provided Fragilities") == 0)
        fragDirWidget->show();
    else
        fragDirWidget->hide();
}


bool ReWetWidget::recursiveCopy(const QString &sourcePath, const QString &destPath)
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


void ReWetWidget::handleBrowseButton1Pressed(void)
{
    QFileDialog dialog(this);
    QString scriptFile = QFileDialog::getOpenFileName(this,tr("Auto-populate Script"));
    dialog.close();

    // Return if the user cancels or enters same file
    if(scriptFile.isEmpty())
        return;

    autoPopulationScriptLineEdit->setText(scriptFile);
}


void ReWetWidget::handleBrowseButton2Pressed(void)
{
    QFileDialog dialog(this);
    QString fragFolder = QFileDialog::getExistingDirectory(this,tr("Folder containing user-provided fragility data"));
    dialog.close();

    // Return if the user cancels or enters same file
    if(fragFolder.isEmpty())
        return;

    fragilityDirLineEdit->setText(fragFolder);
}
