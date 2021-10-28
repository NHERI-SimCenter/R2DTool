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

// Written by: Frank McKenna

#include "CSVReaderWriter.h"
#include "LayerTreeView.h"
#include "RegionalSiteResponseWidget.h"
#include "VisualizationWidget.h"
#include "WorkflowAppR2D.h"



#ifdef ARC_GIS
#include "ArcGISVisualizationWidget.h"
#include "FeatureCollectionLayer.h"
#include "GroupLayer.h"
#include "Layer.h"
#include "LayerListModel.h"
#include "SimpleMarkerSymbol.h"
#include "SimpleRenderer.h"

using namespace Esri::ArcGISRuntime;
#endif

#ifdef Q_GIS
#include "QGISVisualizationWidget.h"

#include <qgsvectorlayer.h>
#endif

#include <QApplication>
#include <QDialog>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QProgressBar>
#include <QComboBox>
#include <QPushButton>
#include <QSpacerItem>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QDir>
#include <QGroupBox>


RegionalSiteResponseWidget::RegionalSiteResponseWidget(VisualizationWidget* visWidget, QWidget *parent) : SimCenterAppWidget(parent), theVisualizationWidget(visWidget)
{
    progressBar = nullptr;
    inputWidget = nullptr;
    progressBarWidget = nullptr;
    userGMStackedWidget = nullptr;
    progressLabel = nullptr;
    eventFile = "";
    motionDir = "";

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(this->getRegionalSiteResponseWidget());
    layout->addStretch();
    this->setLayout(layout);

}


RegionalSiteResponseWidget::~RegionalSiteResponseWidget()
{

}


bool RegionalSiteResponseWidget::outputAppDataToJSON(QJsonObject &jsonObject) {

    jsonObject["Application"] = "RegionalSiteResponse";

    QJsonObject appData;
    QFileInfo theFile(eventFile);
    if (theFile.exists()) {
        appData["inputEventFile"]=theFile.fileName();
        appData["inputEventFileDir"]=theFile.path();
    } else {
        appData["inputEventFile"]=eventFile; // may be valid on others computer
        appData["inputEventFileDir"]=QString("");
    }
    
    QFileInfo theDir(motionDir);
    if (theDir.exists()) {
        appData["inputMotionDir"]=theDir.absoluteFilePath();
    } else {
        appData["inputMotionDir"]=QString(motionDir);
    }
    
    QFileInfo theScript(siteResponseScriptLineEdit->text());
    if (theScript.exists()) {
        appData["siteResponseScript"]=theScript.fileName();
        appData["scriptPath"]=theScript.path();
    } else {
      appData["siteResponseScript"]=siteResponseScriptLineEdit->text();
        appData["scriptPath"]=QString("");
    }


    QFileInfo theSoil(soilFileLineEdit->text());
    if (theSoil.exists()) {
        appData["soilGridParametersFile"]=theSoil.fileName();
        appData["soilParametersPath"]=theSoil.path();
    } else {
      appData["soilGridParametersFile"]=soilFileLineEdit->text();
      appData["soilParametersPath"]=QString("");
    }

    // just for when running the app (copyFiles places stuff in the following dir
    appData["motionDir"]=QString("simcInputMotion/");
    appData["modelDir"]=QString("simcModelDir/");
    
    jsonObject["ApplicationData"]=appData;

    return true;
}


bool RegionalSiteResponseWidget::outputToJSON(QJsonObject &jsonObj)
{
    return true;
}


bool RegionalSiteResponseWidget::inputAppDataFromJSON(QJsonObject &jsonObj)
{
    if (jsonObj.contains("ApplicationData")) {
        QJsonObject appData = jsonObj["ApplicationData"].toObject();

        QString fileName;
        QString pathToFile;

        if (appData.contains("inputEventFile"))
            fileName = appData["inputEventFile"].toString();
        if (appData.contains("inputEventFileDir"))
            pathToFile = appData["inputEventFileDir"].toString();
        else
            pathToFile=QDir::currentPath();

        QString fullFilePath= pathToFile + QDir::separator() + fileName;

        //
        // files downloaded possibly from a remote run will be in input_data/relevantDIR
        //

        if (!QFileInfo::exists(fullFilePath)){
            pathToFile=QDir::currentPath();
            fullFilePath = pathToFile + QDir::separator()
                    + "input_data" + QDir::separator() + QString("simcMotionDir") + QDir::separator() + fileName;

            if (!QFile::exists(fullFilePath)) {
                this->errorMessage("RegionalSiteResponse - could not find event file");
                return false;
            }
        }

        eventFileLineEdit->setText(fullFilePath);
        eventFile = fullFilePath;

        if (appData.contains("inputMotionDir"))
            motionDir = appData["inputMotionDir"].toString();

        QDir motionD(motionDir);

        if (!motionD.exists()){
            QString trialDir = QDir::currentPath() +
                    QDir::separator() + "input_data" + QDir::separator() + "simcInputMotion";
            if (motionD.exists(trialDir)) {
                motionDir = trialDir;
                motionDirLineEdit->setText(trialDir);
            } else {
                this->errorMessage("RegionalSiteResponse - could not find motion dir" + motionDir + " " + trialDir);
                return false;
            }
        } else {
            motionDirLineEdit->setText(motionDir);
        }

        this->loadUserGMData();

        if (appData.contains("siteResponseScript")) {

            QString fileName = appData["siteResponseScript"].toString();
            QString pathToFile = appData["scriptPath"].toString();
            QString fullFilePath= pathToFile + QDir::separator() + fileName;

            //
            // files downloaded possibly from a remote run will be in input_data/relevantDIR
            //

	    qDebug() << "full SR path: " << fullFilePath;
	    
            if (!QFileInfo::exists(fullFilePath)){
                fullFilePath = QDir::currentPath() + QDir::separator()
                        + "input_data" + QDir::separator() + QString("simcModelDir") + QDir::separator() + fileName;

                if (!QFile::exists(fullFilePath)) {
                    this->errorMessage("RegionalSiteResponse - could not find site response script");
                    return false;
                }
            }
            siteResponseScriptLineEdit->setText(fullFilePath);
        }

        if (appData.contains("soilGridParametersFile")) {

            QString fileName = appData["soilGridParametersFile"].toString();
            QString pathToFile = appData["soilParametersPath"].toString();
            QString fullFilePath= pathToFile + QDir::separator() + fileName;

            //
            // files downloaded possibly from a remote run will be in input_data/relevantDIR
            //

	    qDebug() << "full SP file: " << fullFilePath;
	    
            if (!QFileInfo::exists(fullFilePath)){
                fullFilePath = QDir::currentPath() + QDir::separator()
                        + "input_data" + QDir::separator() + fileName;

                if (!QFile::exists(fullFilePath)) {
                    this->errorMessage("RegionalSiteResponse - could not find site response script");
                    return false;
                }
            }
            soilFileLineEdit->setText(fullFilePath);
        }

        return true;
    }

    return false;
}


void RegionalSiteResponseWidget::showUserGMLayers(bool state)
{

}


QStackedWidget* RegionalSiteResponseWidget::getRegionalSiteResponseWidget(void)
{
    if (userGMStackedWidget)
        return userGMStackedWidget.get();

    userGMStackedWidget = std::make_unique<QStackedWidget>();

    //
    // soil stuff: properties and site response script
    //

    inputWidget = new QWidget();
    QVBoxLayout *inputLayout = new QVBoxLayout(this);
    inputWidget->setLayout(inputLayout);
    
    QGroupBox *soilGroupBox = new QGroupBox("Soil Properties");
    soilGroupBox->setContentsMargins(0,5,0,0);
    QGridLayout *soilLayout = new QGridLayout();
    soilGroupBox->setLayout(soilLayout);

    soilLayout->addWidget(new QLabel("Soil Properties"), 0, 0);
    soilFileLineEdit = new QLineEdit();
    soilLayout->addWidget(soilFileLineEdit, 0, 1);
    QPushButton *browseSoilFileButton = new QPushButton("Browse");
    soilLayout->addWidget(browseSoilFileButton, 0, 2);


    connect(browseSoilFileButton, &QPushButton::clicked, this, [this]() {
        QString fileName = QFileDialog::getOpenFileName(this, "Specify Soil Properties", "",  "CSV files (*.csv) ;;All files (*)");
         qDebug() << fileName;
          if (!fileName.isEmpty())
          {
              qDebug() << fileName;
              soilFileLineEdit->setText(fileName);
              qDebug() << "soilFile: " << soilFileLineEdit->text();
          }
       }
    );

    //connect(browseSoilFileButton, &QPushButton::clicked, this, &RegionalSiteResponseWidget::soilParamaterFileDialog);

    soilLayout->addWidget(new QLabel("Site Response Script"), 1, 0);
    siteResponseScriptLineEdit = new QLineEdit();
    soilLayout->addWidget(siteResponseScriptLineEdit,1, 1);
    QPushButton *browseScriptButton = new QPushButton("Browse");
    soilLayout->addWidget(browseScriptButton, 1, 2);

    connect(browseScriptButton, &QPushButton::clicked, this, [this](){
         QString fileName = QFileDialog::getOpenFileName(this, "Specify Simulation Script", "",  "Tcl Files (*.tcl)\
       ;;All files (*)");
          qDebug() << fileName;
           if (!fileName.isEmpty())
           {
               qDebug() << fileName;
               siteResponseScriptLineEdit->setText(fileName);
               qDebug() << "script: " << siteResponseScriptLineEdit->text();
           }
        }
    );

    inputLayout->addWidget(soilGroupBox);

    //
    // motion stuff: Event File & directory
    //

    QGroupBox *motionGroupBox = new QGroupBox("Input Motions");
    motionGroupBox->setContentsMargins(0,5,0,0);
    QGridLayout *motionLayout = new QGridLayout();
    motionGroupBox->setLayout(motionLayout);
    
    QLabel* selectComponentsText = new QLabel("Event File Listing Motions");
    eventFileLineEdit = new QLineEdit();
    QPushButton *browseFileButton = new QPushButton("Browse");
    connect(browseFileButton,SIGNAL(clicked()),this,SLOT(chooseEventFileDialog()));

    motionLayout->addWidget(selectComponentsText);
    motionLayout->addWidget(eventFileLineEdit,    0,1);
    motionLayout->addWidget(browseFileButton,     0,2);

    QLabel* selectFolderText = new QLabel("Folder Containing Motions",this);
    motionDirLineEdit = new QLineEdit();
    QPushButton *browseFolderButton = new QPushButton("Browse",this);
    connect(browseFolderButton,SIGNAL(clicked()),this,SLOT(chooseMotionDirDialog()));

    motionLayout->addWidget(selectFolderText,   1,0);
    motionLayout->addWidget(motionDirLineEdit, 1,1);
    motionLayout->addWidget(browseFolderButton, 1,2);
    //motionLayout->setRowStretch(2,1);
    motionLayout->setColumnStretch(1,3);

    inputLayout->addWidget(motionGroupBox);
    inputLayout->addStretch();


    //
    // progress bar
    //

    progressBarWidget = new QWidget(this);
    auto progressBarLayout = new QVBoxLayout(progressBarWidget);
    progressBarWidget->setLayout(progressBarLayout);

    auto progressText = new QLabel("Loading user ground motion data. This may take a while.",progressBarWidget);
    progressLabel =  new QLabel(" ",this);
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

    userGMStackedWidget->addWidget(inputWidget);
    userGMStackedWidget->addWidget(progressBarWidget);

    userGMStackedWidget->setCurrentWidget(inputWidget);

    userGMStackedWidget->setWindowTitle("Select folder containing earthquake ground motions");

    return userGMStackedWidget.get();
}


void RegionalSiteResponseWidget::showUserGMSelectDialog(void)
{

    if (!userGMStackedWidget)
    {
        this->getRegionalSiteResponseWidget();
    }

    userGMStackedWidget->show();
    userGMStackedWidget->raise();
    userGMStackedWidget->activateWindow();
}

bool
RegionalSiteResponseWidget::copyFiles(QString &destDir)
{
    // create dir and copy motion files
    QDir destDIR(destDir);
    QString modelDir = destDir + QDir::separator() + QString("simcModelDir");
    QString motionDir = destDir + QDir::separator() + QString("simcMotionDir");
    destDIR.mkpath(modelDir);
    destDIR.mkpath(motionDir);


    QString soilParameteres = soilFileLineEdit->text();
    QFileInfo soilParametersFile(soilParameteres);
    if (soilParametersFile.exists())
      this->copyFile(soilParameteres, destDir);
    else {
      qDebug() << "RegionalSiteResponse:copyFiles soil parameters files does not exist:" << soilParameteres;
      return false;
    }

    QString script = siteResponseScriptLineEdit->text();
    QFileInfo scriptFile(script);
    if (scriptFile.exists()) {
      this->copyPath(scriptFile.path(), modelDir, false);
    } else {
      qDebug() << "RegionalSiteResponse:copyFiles script file does not exist:" << scriptFile;
      return false;
    }   

    QFileInfo eventFileInfo(eventFile);
    if (eventFileInfo.exists()) {
      this->copyFile(eventFile, motionDir);
    } else {
      qDebug() << "RegionalSiteResponse:copyFiles event grid file does not exist:" << eventFile;
      return false;
    }


    QDir motionDirInfo(motionDir);
    if (motionDirInfo.exists()) {
      return this->copyPath(motionDir, motionDir, false);
    } else {
      qDebug() << "RegionalSiteResponse:copyFiles motion Dir file does not exist:" << motionDir;
      return false;
    }

    // should never get here
    return false;
}

void RegionalSiteResponseWidget::loadUserGMData(void) {
  

    emit loadingComplete(true);

    return;
}

void RegionalSiteResponseWidget::soilParamaterFileDialog(void)
{
    userGMStackedWidget->show();
    userGMStackedWidget->raise();
    userGMStackedWidget->activateWindow();

    QFileDialog dialog(this);
    QString newFile = QFileDialog::getOpenFileName(this,tr("Soil Parameters File"));
    dialog.close();
    soilFileLineEdit->setText(newFile);
}
void RegionalSiteResponseWidget::soilScriptFileDialog(void)
{
    QFileDialog dialog(this);
    QString newFile = QFileDialog::getOpenFileName(this,tr("Site Response Script"));
    dialog.close();
    siteResponseScriptLineEdit->setText(newFile);

}

void RegionalSiteResponseWidget::chooseEventFileDialog(void)
{

    QFileDialog dialog(this);
    QString newEventFile = QFileDialog::getOpenFileName(this,tr("Event Grid File"));
    dialog.close();

    // Return if the user cancels or enters same file
    if(newEventFile.isEmpty() || newEventFile == eventFile)
    {
        return;
    }

    // Set file name & entry in qLine edit

    // if file
    //    check valid
    //    set motionDir if file in dir that contains all the motions
    //    invoke loadUserGMData

    CSVReaderWriter csvTool;

    QString err;
    QVector<QStringList> data = csvTool.parseCSVFile(newEventFile, err);

    if(!err.isEmpty())
    {
        this->errorMessage(err);
        return;
    }

    if(data.empty())
        return;

    eventFile = newEventFile;
    eventFileLineEdit->setText(eventFile);

    // check if file in dir with all motions, if so set motionDir
    // Pop off the row that contains the header information
    data.pop_front();
    auto numRows = data.size();
    int count = 0;
    QFileInfo eventFileInfo(eventFile);
    QDir fileDir(eventFileInfo.absolutePath());
    QStringList filesInDir = fileDir.entryList(QStringList() << "*", QDir::Files);

    // check all files are there
    bool allThere = true;
    for(int i = 0; i<numRows; ++i) {
        auto rowStr = data.at(i);
        auto stationName = rowStr[0];
        if (!filesInDir.contains(stationName)) {
            allThere = false;
            i=numRows;
        }
    }

    if (allThere == true) {
        motionDir = fileDir.path();
        motionDirLineEdit->setText(fileDir.path());
        this->loadUserGMData();
    } else {
        QDir motionDirDir(motionDir);
        if (motionDirDir.exists()) {
            QStringList filesInDir = motionDirDir.entryList(QStringList() << "*", QDir::Files);
            bool allThere = true;
            for(int i = 0; i<numRows; ++i) {
                auto rowStr = data.at(i);
                auto stationName = rowStr[0];
                if (!filesInDir.contains(stationName)) {
                    allThere = false;
                    i=numRows;
                }
            }
            if (allThere == true)
                this->loadUserGMData();
        }
    }

    return;
}


void RegionalSiteResponseWidget::chooseMotionDirDialog(void)
{

    QFileDialog dialog(this);

    dialog.setFileMode(QFileDialog::Directory);
    QString newPath = dialog.getExistingDirectory(this, tr("Dir containing specified motions"));
    dialog.close();

    // Return if the user cancels or enters same dir
    if(newPath.isEmpty() || newPath == motionDir)
    {
        return;
    }

    motionDir = newPath;
    motionDirLineEdit->setText(motionDir);

    // check if dir contains EventGrid.csv file, if it does set the file
    QFileInfo eventFileInfo(newPath, "EventGrid.csv");
    if (eventFileInfo.exists()) {
        eventFile = newPath + "/EventGrid.csv";
        eventFileLineEdit->setText(eventFile);
    }

    // could check files exist if eventFile set, but need something to give an error if not all there
    this->loadUserGMData();

    return;
}


void RegionalSiteResponseWidget::clear(void)
{
    eventFile.clear();
    motionDir.clear();

    eventFileLineEdit->clear();
    motionDirLineEdit->clear();

    stationList.clear();
}
