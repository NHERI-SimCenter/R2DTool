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

// Written by: Jinyan Zhao


#include "GroundFailureWidget.h"
#include "SimCenterAppSelection.h"
#include "LiqTriggerZhuEtAl2017.h"
#include "LiqLateralHazus2020.h"
#include "LiqVerticalHazus2020.h"
#include "LiqTriggerHazus2020.h"
#include "NoneWidget.h"
#include "SimCenterUnitsCombo.h"
#include "LiquefactionWidget.h"
#include "LandslideWidget.h"
#include "SimCenterPreferences.h"
#include "Utils/ProgramOutputDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QComboBox>
#include <QSet>
#include <QMessageBox>
#include <QGroupBox>
#include <QCheckBox>
#include <QTabWidget>
#include <QMessageBox>
#include <QApplication>
#include <QDir>


GroundFailureWidget::GroundFailureWidget(QWidget *parent) : SimCenterAppWidget(parent)
{
    auto mainLayout = new QVBoxLayout(this);

    gfGroupBox = new QGroupBox(this);
    gfGroupBox->setTitle("Ground Failure Source(s)");
    gfGroupBox->setContentsMargins(0,0,0,0);

    QGridLayout* groupBoxLayout = new QGridLayout(gfGroupBox);
    gfGroupBox->setLayout(groupBoxLayout);

    liquefactionCheckBox = new QCheckBox("Liquefaction");

    landslideCheckBox = new QCheckBox("Landslide");

    faultDispCheckBox = new QCheckBox("Fault Displacement");
    faultDispCheckBox->setEnabled(false);

//    unitsCombo = new SimCenterUnitsCombo(SimCenter::Unit::LENGTH, "PGD", this);
//    unitsCombo->setCurrentUnit(SimCenter::Unit::m);
//    QLabel* unitLabel = new QLabel("PGD Output Unit:");

    groupBoxLayout->addWidget(liquefactionCheckBox, 0, 0);
    groupBoxLayout->addWidget(landslideCheckBox, 0, 1);
    groupBoxLayout->addWidget(faultDispCheckBox, 0, 2);
//    groupBoxLayout->addWidget(unitLabel, 1, 0);
//    groupBoxLayout->addWidget(unitsCombo, 1, 1,1,2);
//    groupBoxLayout->setColumnStretch(3,1);

    theTabWidget = new QTabWidget();
    liquefactionWidget = new LiquefactionWidget(this);
    landslideWidget = new LandslideWidget(this);
    theTabWidget->addTab(liquefactionWidget, tr("Liquefaction"));
    theTabWidget->addTab(landslideWidget, tr("Landslide"));

    this->setConnections();
    liquefactionCheckBox->setChecked(false);
    landslideCheckBox->setChecked(false);
    this->handleSourceSelectionChanged();

    mainLayout->addWidget(gfGroupBox);
    mainLayout->addWidget(theTabWidget);
    mainLayout->addStretch(0);
    this->setLayout(mainLayout);

    downloadManager = std::make_unique<NetworkDownloadManager>(this);

}

void GroundFailureWidget::reset(void)
{

}

void GroundFailureWidget::setConnections()
{
    connect(this->liquefactionCheckBox, &QCheckBox::stateChanged, this, &GroundFailureWidget::handleSourceSelectionChanged);
    connect(this->landslideCheckBox, &QCheckBox::stateChanged, this, &GroundFailureWidget::handleSourceSelectionChanged);
}


void GroundFailureWidget::handleSourceSelectionChanged()
{
    while (theTabWidget->count() > 0) {
        theTabWidget->removeTab(0);
    }
    if (liquefactionCheckBox->isChecked()){
        this->checkAndDownloadDataBase();
        theTabWidget->addTab(liquefactionWidget, tr("Liquefaction"));
    }
    if (landslideCheckBox->isChecked()){
        this->checkAndDownloadDataBase();
        theTabWidget->addTab(landslideWidget, tr("Landslide"));
    }
}

void GroundFailureWidget::checkAndDownloadDataBase(){
    QString dataFolderPath = QCoreApplication::applicationDirPath() + QDir::separator() + "Databases"+ QDir::separator() + "groundFailure";
    QDir dataFolder(dataFolderPath);
    if(dataFolder.exists()){
        return;
    }else{
        QMessageBox msgBox;
        msgBox.setText("Warning:\n"
                       "SimCenter Liquefaction Database needs to be downloaded to use SimCenter default California geospatial data.\n"
                       "If don't download, geospatial data needs to be provided in GIS or Site File (.csv) format.\n"
                       "If download, please continue analysis after downloading completes.\n"
                       "Download the database?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);

        auto res = msgBox.exec();
        if (res != QMessageBox::Yes){
            return;
        } else {
            QString msg = "Downloading... May take a while... Please wait.";
            ProgramOutputDialog* statusDialog = ProgramOutputDialog::getInstance();
            statusDialog->appendText(msg);
            QApplication::processEvents();

            QStringList urls = {"https://zenodo.org/api/records/10892461"};
            QStringList names = {"groundFailure"};
            QString pathTodatabaseFolder = QCoreApplication::applicationDirPath() +
                                           QDir::separator() + "Databases";
            downloadManager->downloadData(urls, names, "ground failure model data", pathTodatabaseFolder);

//            downloadManager->downloadExamples();
            // DownloadSingleFileInfo
            //            QString infoUrl = "https://zenodo.org/api/records/10892461";
            //            QNetworkRequest request(url);
            //            QNetwork
        }
    }
}

bool GroundFailureWidget::inputFromJSON(QJsonObject& /*obj*/)
{
    return true;
}


bool GroundFailureWidget::outputToJSON(QJsonObject& obj)
{
    QJsonObject groundFailureObj;
    if (liquefactionCheckBox->isChecked()){
        QJsonObject liquefactionObj;
        liquefactionWidget->outputToJSON(liquefactionObj);
        groundFailureObj["Liquefaction"] = liquefactionObj;
    }
    if (landslideCheckBox->isChecked()){
        QJsonObject landslideObj;
        landslideWidget->outputToJSON(landslideObj);
        groundFailureObj["Landslide"] = landslideObj;
    }
    obj["GroundFailure"] = groundFailureObj;
//    obj["PGDunit"] = unitsCombo->currentText();
    return true;
}

//void GroundMotionModelsWidget::addGMMforSA(bool SAenabled){

//}

