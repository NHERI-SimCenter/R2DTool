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

// Written by: Stevan Gavrilovic

#include "HurricanePreprocessor.h"
#include "LayerTreeView.h"
#include "HurricaneSelectionWidget.h"
#include "VisualizationWidget.h"
#include "WorkflowAppR2D.h"

#include "Feature.h"

#include <QApplication>
#include <QDialog>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QSpacerItem>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QDir>

using namespace Esri::ArcGISRuntime;

HurricaneSelectionWidget::HurricaneSelectionWidget(VisualizationWidget* visWidget, QWidget *parent) : SimCenterAppWidget(parent), theVisualizationWidget(visWidget)
{
    progressBar = nullptr;
    fileInputWidget = nullptr;
    progressBarWidget = nullptr;
    theStackedWidget = nullptr;
    progressLabel = nullptr;
    selectedHurricaneName = nullptr;
    selectedHurricaneSID = nullptr;

    eventFile = "";

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(this->getHurricaneSelectionWidget());
    layout->addStretch();
    this->setLayout(layout);

    //    eventFile = "/Users/steve/Desktop/ibtracs.last3years.list.v04r00.csv";
    //    this->loadUserHurricaneData();

}


HurricaneSelectionWidget::~HurricaneSelectionWidget()
{

}


bool HurricaneSelectionWidget::outputAppDataToJSON(QJsonObject &jsonObject) {

    jsonObject["Application"] = "UserInputHurricane";

    QJsonObject appData;
    //    QFileInfo theFile(eventFile);
    //    if (theFile.exists()) {
    //        appData["eventFile"]=theFile.fileName();
    //        appData["eventFileDir"]=theFile.path();
    //    } else {
    //        appData["eventFile"]=eventFile; // may be valid on others computer
    //        appData["eventFileDir"]=QString("");
    //    }
    //    QFileInfo theDir(motionDir);
    //    if (theDir.exists()) {
    //        appData["motionDir"]=theDir.absoluteFilePath();
    //    } else {
    //        appData["motionDir"]=QString("None");
    //    }

    jsonObject["ApplicationData"]=appData;

    return true;
}


bool HurricaneSelectionWidget::outputToJSON(QJsonObject &jsonObj)
{
    // qDebug() << "USER GM outputPLAIN";

    return true;
}


bool HurricaneSelectionWidget::inputAppDataFromJSON(QJsonObject &jsonObj)
{
    if (jsonObj.contains("ApplicationData")) {
        //        QJsonObject appData = jsonObj["ApplicationData"].toObject();

        //        QString fileName;
        //        QString pathToFile;

        //        if (appData.contains("eventFile"))
        //            fileName = appData["eventFile"].toString();
        //        if (appData.contains("eventFileDir"))
        //            pathToFile = appData["eventFileDir"].toString();
        //        else
        //            pathToFile=QDir::currentPath();

        //        QString fullFilePath= pathToFile + QDir::separator() + fileName;

        //        // adam .. adam .. adam
        //        if (!QFileInfo::exists(fullFilePath)){
        //            fullFilePath = pathToFile + QDir::separator()
        //                    + "input_data" + QDir::separator() + fileName;

        //            if (!QFile::exists(fullFilePath)) {
        //                qDebug() << "UserInputGM - could not find event file";
        //                return false;
        //            }
        //        }

        //        eventFileLineEdit->setText(fullFilePath);
        //        eventFile = fullFilePath;

        //        if (appData.contains("motionDir"))
        //            motionDir = appData["motionDir"].toString();

        //        QDir motionD(motionDir);

        //        if (!motionD.exists()){
        //            QString trialDir = QDir::currentPath() +
        //                    QDir::separator() + "input_data" + motionDir;
        //            if (motionD.exists(trialDir)) {
        //                motionDir = trialDir;
        //                motionDirLineEdit->setText(trialDir);
        //            } else {
        //                qDebug() << "UserInputGM - could not find motion dir" << motionDir << " " << trialDir;
        //                return false;
        //            }
        //        } else {
        //            motionDirLineEdit->setText(motionDir);
        //        }

        //        this->loadUserGMData();
        //        return true;
    }

    return false;
}


void HurricaneSelectionWidget::showUserGMLayers(bool state)
{
    auto layersTreeView = theVisualizationWidget->getLayersTree();


    if(state == false)
    {
        layersTreeView->removeItemFromTree("User Ground Motions");

        return;
    }


    // Check if there is a 'User Ground Motions' root item in the tree
    auto shakeMapTreeItem = layersTreeView->getTreeItem("User Ground Motions",nullptr);

    // If there is no item, create one
    if(shakeMapTreeItem == nullptr)
        shakeMapTreeItem = layersTreeView->addItemToTree("User Ground Motions",QString());

}


QStackedWidget* HurricaneSelectionWidget::getHurricaneSelectionWidget(void)
{
    if (theStackedWidget)
        return theStackedWidget.get();

    theStackedWidget = std::make_unique<QStackedWidget>();
    theStackedWidget->setContentsMargins(0,0,0,0);

    //
    // file and dir input
    //

    fileInputWidget = new QWidget(this);
    QGridLayout *fileLayout = new QGridLayout(fileInputWidget);
    fileLayout->setContentsMargins(0,0,0,0);

    QLabel* selectComponentsText = new QLabel("Event File Listing Hurricanes",this);
    eventFileLineEdit = new QLineEdit();
    QPushButton *browseFileButton = new QPushButton("Browse",this);

    connect(browseFileButton,SIGNAL(clicked()),this,SLOT(chooseEventFileDialog()));

    mapViewSubWidget = std::make_unique<ResultsMapViewWidget>(nullptr);

    QHBoxLayout *selectedHurricaneLayout = new QHBoxLayout();

    QPushButton *selectHurricaneButton = new QPushButton("Select Hurricane",this);

    connect(selectHurricaneButton,&QPushButton::clicked,this,&HurricaneSelectionWidget::handleHurricaneSelect);

    QLabel* selectedHurricaneLabel = new QLabel("Hurricane Selected For Analysis: ",this);
    QLabel* SIDLabel = new QLabel(" - SID: ",this);

    selectedHurricaneName = new QLabel("None",this);
    selectedHurricaneSID = new QLabel("None",this);

    selectedHurricaneLayout->addWidget(selectHurricaneButton);
    selectedHurricaneLayout->addWidget(selectedHurricaneLabel);
    selectedHurricaneLayout->addWidget(selectedHurricaneName);
    selectedHurricaneLayout->addWidget(SIDLabel);
    selectedHurricaneLayout->addWidget(selectedHurricaneSID);
    selectedHurricaneLayout->addStretch(1);

    fileLayout->addWidget(selectComponentsText,   0,0);
    fileLayout->addWidget(eventFileLineEdit,      0,1);
    fileLayout->addWidget(browseFileButton,       0,2);
    fileLayout->addLayout(selectedHurricaneLayout, 1,0,1,3);
    fileLayout->addWidget(mapViewSubWidget.get(), 2,0,1,3);

    //
    // progress bar
    //

    progressBarWidget = new QWidget(this);
    auto progressBarLayout = new QVBoxLayout(progressBarWidget);
    progressBarWidget->setLayout(progressBarLayout);

    auto progressText = new QLabel("Loading user hurricane data. This may take a while.",progressBarWidget);
    progressLabel =  new QLabel("",this);
    progressBar = new QProgressBar(progressBarWidget);

    auto vspacer = new QSpacerItem(0,0,QSizePolicy::Minimum, QSizePolicy::Expanding);
    progressBarLayout->addItem(vspacer);
    progressBarLayout->addWidget(progressText,1, Qt::AlignCenter);
    progressBarLayout->addWidget(progressLabel,1, Qt::AlignCenter);
    progressBarLayout->addWidget(progressBar);
    progressBarLayout->addItem(vspacer);
    progressBarLayout->addStretch(1);

    //
    // add file and progress widgets to stacked widgets, then set defaults
    //

    theStackedWidget->addWidget(fileInputWidget);
    theStackedWidget->addWidget(progressBarWidget);

    theStackedWidget->setCurrentWidget(fileInputWidget);

    theStackedWidget->setWindowTitle("Select folder containing hurricanes");

    return theStackedWidget.get();
}


void HurricaneSelectionWidget::showHurricaneSelectDialog(void)
{

    if (!theStackedWidget)
    {
        this->getHurricaneSelectionWidget();
    }

    theStackedWidget->show();
    theStackedWidget->raise();
    theStackedWidget->activateWindow();
}


void HurricaneSelectionWidget::loadHurricaneTrackData(void)
{

    HurricanePreprocessor hurricaneImportTool(progressBar, theVisualizationWidget, this);

    theStackedWidget->setCurrentWidget(progressBarWidget);
    progressBarWidget->setVisible(true);

    QString errMsg;
    auto res = hurricaneImportTool.loadHurricaneTrackData(eventFile,errMsg);

    if(res != 0)
        this->userMessageDialog(errMsg);

    progressLabel->setVisible(false);

    // Reset the widget back to the input pane and close
    theStackedWidget->setCurrentWidget(fileInputWidget);
    fileInputWidget->setVisible(true);

    if(theStackedWidget->isModal())
        theStackedWidget->close();

    emit loadingComplete(true);

    return;
}


void HurricaneSelectionWidget::setCurrentlyViewable(bool status){

    if (status == true)
        mapViewSubWidget->setCurrentlyViewable(status);
}


void HurricaneSelectionWidget::chooseEventFileDialog(void)
{

    QFileDialog dialog(this);
    QString newEventFile = QFileDialog::getOpenFileName(this,tr("List of Hurricanes File"));
    dialog.close();

    // Return if the user cancels or enters same file
    if(newEventFile.isEmpty() || newEventFile == eventFile)
    {
        return;
    }

    // Set file name & entry in qLine edit
    eventFile = newEventFile;
    eventFileLineEdit->setText(eventFile);

    this->loadHurricaneTrackData();

    return;
}


void HurricaneSelectionWidget::clear(void)
{
    eventFile.clear();
    eventFileLineEdit->clear();
}


void HurricaneSelectionWidget::handleHurricaneSelect(void)
{
    auto selectedFeatures = theVisualizationWidget->getSelectedFeaturesList();

    if(selectedFeatures.empty())
        return;

    // Only select the first hurricane
    for(auto&& it : selectedFeatures)
    {
        auto attrbList = it->attributes();

        auto featType = attrbList->attributeValue("AssetType");

        if(featType.toString() != "HURRICANE")
            continue;

        auto hurricaneName = attrbList->attributeValue("NAME").toString();
        auto hurricaneSID = attrbList->attributeValue("SID").toString();

        selectedHurricaneName->setText(hurricaneName);
        selectedHurricaneSID->setText(hurricaneSID);

    }

}
