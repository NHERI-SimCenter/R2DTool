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

#include "PelicunDLWidget.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QPushButton>
#include <QGridLayout>
#include <QGroupBox>
#include <QIntValidator>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>

PelicunDLWidget::PelicunDLWidget(QWidget *parent): SimCenterAppWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    QGroupBox* groupBox = new QGroupBox(this);
    groupBox->setTitle("Pelicun Damage and Loss Prediction Methodology");

    QGridLayout* gridLayout = new QGridLayout(groupBox);

    QLabel* typeLabel = new QLabel(tr("Damage and Loss Method:"),this);
    DLTypeComboBox = new QComboBox(this);
    DLTypeComboBox->addItem("HAZUS MH EQ");
    DLTypeComboBox->addItem("HAZUS MH EQ IM");
    DLTypeComboBox->addItem("HAZUS MH HU");
    DLTypeComboBox->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);

    connect(DLTypeComboBox,&QComboBox::currentTextChanged, this, &PelicunDLWidget::handleComboBoxChanged);

    QLabel* realizationsLabel = new QLabel(tr("Number of realizations:"),this);
    realizationsLineEdit = new QLineEdit(this);

    QIntValidator* validator = new QIntValidator(this);
    validator->setBottom(0);
    realizationsLineEdit->setValidator(validator);
    realizationsLineEdit->setText("5");

    QLabel* eventTimeLabel = new QLabel(tr("Event time:"),this);
    eventTimeComboBox = new QComboBox(this);
    eventTimeComboBox->addItem("on");
    eventTimeComboBox->addItem("off");
    eventTimeComboBox->setCurrentIndex(1);

    detailedResultsCheckBox = new QCheckBox("Output detailed results",this);
    logFileCheckBox = new QCheckBox("Log file",this);
    logFileCheckBox->setChecked(true);
    coupledEDPCheckBox = new QCheckBox("Coupled EDP",this);
    groundFailureCheckBox= new QCheckBox("Include ground failure",this);

    autoPopulateScriptWidget = new QWidget(this);
    auto autoPopulateScriptLayout = new QHBoxLayout(autoPopulateScriptWidget);
    autoPopulationScriptLineEdit = new QLineEdit(this);
    auto browseButton = new QPushButton("Browse",this);

    connect(browseButton,&QPushButton::pressed,this,&PelicunDLWidget::handleBrowseButtonPressed);

    auto autoPopScriptLabel = new QLabel("Auto populate script:");

    autoPopulateScriptLayout->addWidget(autoPopScriptLabel);
    autoPopulateScriptLayout->addWidget(autoPopulationScriptLineEdit);
    autoPopulateScriptLayout->addWidget(browseButton);

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

    gridLayout->addItem(Vspacer,8,0,1,2);

    layout->addWidget(groupBox);

    autoPopulateScriptWidget->hide();
}


bool PelicunDLWidget::outputAppDataToJSON(QJsonObject &jsonObject)
{

    jsonObject.insert("Application","pelicun");

    QJsonObject appDataObj;

    appDataObj.insert("DL_Method",DLTypeComboBox->currentText());
    appDataObj.insert("Realizations",realizationsLineEdit->text().toInt());
    appDataObj.insert("detailed_results",detailedResultsCheckBox->isChecked());
    appDataObj.insert("log_file",logFileCheckBox->isChecked());
    appDataObj.insert("coupled_EDP",coupledEDPCheckBox->isChecked());
    appDataObj.insert("event_time",eventTimeComboBox->currentText());
    appDataObj.insert("ground_failure",groundFailureCheckBox->isChecked());

    auto scriptPath = autoPopulationScriptLineEdit->text();
    if(!scriptPath.isEmpty())
        appDataObj.insert("auto_script",scriptPath);

    jsonObject.insert("ApplicationData",appDataObj);

    return true;
}


bool PelicunDLWidget::inputAppDataFromJSON(QJsonObject &jsonObject)
{
    if (jsonObject.contains("ApplicationData"))
    {
        QJsonObject appData = jsonObject["ApplicationData"].toObject();

        if (appData.contains("DL_Method"))
            DLTypeComboBox->setCurrentText(appData["DL_Method"].toString());

        if (appData.contains("Realizations"))
            if (appData["Realizations"].isString()){
                realizationsLineEdit->setText(appData["Realizations"].toString());
            } else {
                realizationsLineEdit->setText(QString::number(appData["Realizations"].toInt()));
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
                auto currPath = QDir::currentPath();

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
    }

    return true;
}


void PelicunDLWidget::clear(void)
{
    DLTypeComboBox->setCurrentIndex(0);
    realizationsLineEdit->clear();
    eventTimeComboBox->setCurrentIndex(1);
    detailedResultsCheckBox->setChecked(false);
    logFileCheckBox->setChecked(false);
    coupledEDPCheckBox->setChecked(false);
    groundFailureCheckBox->setChecked(false);
}



bool PelicunDLWidget::copyFiles(QString &destName)
{
    auto compLineEditText = autoPopulationScriptLineEdit->text();

    if(compLineEditText.isEmpty())
        return true;

    QFileInfo componentFile(compLineEditText);

    if (!componentFile.exists())
        return false;

    // Do not copy the file, output a new csv which will have the changes that the user makes in the table
    return this->copyFile(compLineEditText, destName);
}


void PelicunDLWidget::handleComboBoxChanged(const QString &text)
{

    if(text.compare("HAZUS MH HU") == 0)
    {
        groundFailureCheckBox->hide();
        autoPopulateScriptWidget->show();
    }
    else
    {
        groundFailureCheckBox->show();
        autoPopulateScriptWidget->hide();
    }
}


void PelicunDLWidget::handleBrowseButtonPressed(void)
{
    QFileDialog dialog(this);
    QString scriptFile = QFileDialog::getOpenFileName(this,tr("Auto-populate Script"));
    dialog.close();

    // Return if the user cancels or enters same file
    if(scriptFile.isEmpty())
        return;

    autoPopulationScriptLineEdit->setText(scriptFile);
}

