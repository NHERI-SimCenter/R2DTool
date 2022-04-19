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
#include "SimCenterUnitsWidget.h"
#include "SiteWidget.h"
#include "SiteConfig.h"
#include "SiteConfigWidget.h"
#include "SiteGridWidget.h"
#include "SiteScatterWidget.h"
#include "MapViewSubWidget.h"
#include "Vs30Widget.h"
#include "BedrockDepthWidget.h"
#include "SoilModelWidget.h"
#include "GridNode.h"
#include "SimCenterPreferences.h"
#include "QGISSiteInputWidget.h"

#ifdef ARC_GIS
#include "ArcGISVisualizationWidget.h"
#include "FeatureCollectionLayer.h"
#include "GroupLayer.h"
#include "Layer.h"
#include "LayerListModel.h"
#include "SimpleMarkerSymbol.h"
#include "SimpleRenderer.h"
#include "SimCenterMapGraphicsView.h"
#include "MapGraphicsView.h"
#include "Map.h"
#include "Point.h"
#include "FeatureCollection.h"
#include "FeatureCollectionLayer.h"
#include "LayerTreeView.h"

using namespace Esri::ArcGISRuntime;
#endif

#ifdef Q_GIS
#include "QGISVisualizationWidget.h"

#include <qgsvectorlayer.h>
#include "SimCenterMapcanvasWidget.h"
#include "MapViewWindow.h"
#include <qgsmapcanvas.h>
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
#include <QGridLayout>
#include <QByteArray>
#include <QStringList>

RegionalSiteResponseWidget::RegionalSiteResponseWidget(VisualizationWidget* visWidget, QWidget *parent) : SimCenterAppWidget(parent), theVisualizationWidget(visWidget)
{
    progressBar = nullptr;
    inputWidget = nullptr;
    progressBarWidget = nullptr;
    theStackedWidget = nullptr;
    progressLabel = nullptr;
    eventFile = "";
    motionDir = "";
    unitsWidget = nullptr;
    //    theStackedWidget = nullptr;    
    mapViewSubWidget = nullptr;
    theSiteStackedWidget = nullptr;

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(this->getSiteWidget(visWidget));
    layout->addWidget(this->getRegionalSiteResponseWidget());
    layout->addStretch();
    this->setLayout(layout);

    // site data fetch process
    processSiteData = new QProcess(this);
    connect(processSiteData, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &RegionalSiteResponseWidget::handleProcessFinished);
    connect(processSiteData, &QProcess::readyReadStandardOutput, this, &RegionalSiteResponseWidget::handleProcessTextOutput);
    connect(processSiteData, &QProcess::started, this, &RegionalSiteResponseWidget::handleProcessStarted);

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
	QDir dirFile(theFile.path());
        // appData["inputEventFilePath"]=theFile.path();
	appData["inputEventFilePath"]=dirFile.dirName();
    } else {
        appData["inputEventFile"]=eventFile; // may be valid on others computer
        appData["inputEventFilePath"]=QString("");
    }
    
    QDir theDir(motionDir);
    if (theDir.exists()) {
      // appData["inputMotionDir"]=theDir.path();
      appData["inputMotionDir"]=theDir.dirName();
    } else {
      appData["inputMotionDir"]=QString(motionDir);
    }
    

    QFileInfo theScript(siteResponseScriptLineEdit->text());
    if (theScript.exists()) {
        appData["siteResponseScript"]=theScript.fileName();
        // appData["siteResponseScriptPath"]=theScript.path();
	QDir dirFile(theScript.path());
	appData["siteResponseScriptPath"]=dirFile.dirName();	
    } else {
      appData["siteResponseScript"]=siteResponseScriptLineEdit->text();
      appData["siteResponseScriptPath"]=QString("");
    }


    QFileInfo theSoil(soilFileLineEdit->text());
    if (theSoil.exists()) {
        appData["soilGridParametersFile"]=theSoil.fileName();
        //appData["soilGridParametersFilePath"]=theSoil.path();
    //QDir dirFile(theSoil.path());
    //appData["soilGridParametersFilePath"]=dirFile.dirName();
        // KZ: the site parameter file is copied to the same directory as the script path (see the copyFiles)
        appData["soilGridParametersFilePath"]=appData["siteResponseScriptPath"];
    } else {
      appData["soilGridParametersFile"]=soilFileLineEdit->text();
      appData["soilGridParametersFilePath"]=QString("");
    }

    // set Filter first
    this->setFilterString(filterLineEdit->text());
    appData["filter"]=filterLineEdit->text();
    
    jsonObject["ApplicationData"]=appData;

    return true;
}


bool RegionalSiteResponseWidget::outputToJSON(QJsonObject &jsonObj)
{
  bool res = unitsWidget->outputToJSON(jsonObj);
  jsonObj["eventFile"]="siteResponseOutputMotions/EventGrid.csv";
  
  return true;
}


bool RegionalSiteResponseWidget::inputAppDataFromJSON(QJsonObject &jsonObj)
{
    // first pass jsonObj to QGISSiteWidget object if it's used
    // for loading an existing run we use the UserCSV
    this->m_siteConfigWidget->setSiteType(SiteConfig::SiteType::UserCSV);
    qDebug() << this->m_siteConfig->getType();
    if (this->m_siteConfig->getType()==SiteConfig::SiteType::UserCSV)
    {
        this->m_siteConfigWidget->getCsvSiteWidget()->inputAppDataFromJSON(jsonObj);
    }
  if (jsonObj.contains("ApplicationData")) {
        QJsonObject appData = jsonObj["ApplicationData"].toObject();

        QString fileName;
        QString pathToFile;

        if (appData.contains("inputEventFile"))
            fileName = appData["inputEventFile"].toString();
	
        if (appData.contains("inputEventFilePath")) {
	    QString path = appData["inputEventFilePath"].toString();
            pathToFile = path;
	    QDir pathToFileDir(pathToFile);
	    if (!pathToFileDir.exists()) {
	      pathToFile = QDir::currentPath() + QDir::separator() + path;
	      pathToFileDir.setPath(pathToFile);
	    } 
	    if (!pathToFileDir.exists()) {
	      pathToFile = QDir::currentPath() + QDir::separator() + "input_data" + QDir::separator() + path;
	    }
	} else if (appData.contains("inputMotionDir")) {
            QString path = appData["inputMotionDir"].toString();
	    pathToFile = path;
	    QDir pathToFileDir(pathToFile);
	    if (!pathToFileDir.exists()) {
	      pathToFile = QDir::currentPath() + QDir::separator() + path;
	      pathToFileDir.setPath(pathToFile);
	    } 
	    if (!pathToFileDir.exists()) {
	      pathToFile = QDir::currentPath() + QDir::separator() + "input_data" + QDir::separator() + path;
	    }
	} else
	  pathToFile=QDir::currentPath();

        QString fullFilePath= pathToFile + QDir::separator() + fileName;

	QString msg = QString("No Event File found in following locations: " ) + fullFilePath;

        //
        // files downloaded possibly from a remote run will be in input_data/relevantDIR
        //

        if (!QFileInfo::exists(fullFilePath)){
	  this->errorMessage(msg);
	  return false;
	} 

        eventFileLineEdit->setText(fullFilePath);
        eventFile = fullFilePath;

        if (appData.contains("inputMotionDir")) {
	  QString path = appData["inputMotionDir"].toString();
	  motionDir = path;
	  QDir pathToMotionDir(motionDir);
	  if (!pathToMotionDir.exists()) {
	    motionDir = QDir::currentPath() + QDir::separator() + path;
	    pathToMotionDir.setPath(motionDir);
	  } 
	  if (!pathToMotionDir.exists()) {
	    motionDir = QDir::currentPath() + QDir::separator() + "input_data" + QDir::separator() + path;
	  }	  
	} else {
	  motionDir = QFileInfo(eventFile).absolutePath();
	}
	  
        QDir motionD(motionDir);

	msg = QString("No Motion Dir found in following locations: ") + motionDir;	
	
        if (!motionD.exists()){
	  this->errorMessage(msg);
	  return false;
	}
	
	motionDirLineEdit->setText(motionDir);
	
        this->loadUserGMData();
	
        if (appData.contains("siteResponseScript") && appData.contains("siteResponseScriptPath")) {

            QString fileName = appData["siteResponseScript"].toString();
            QString pathToFile = appData["siteResponseScriptPath"].toString();
            QString fullFilePath= pathToFile + QDir::separator() + fileName;

            //
            // files downloaded possibly from a remote run will be in input_data/relevantDIR
            //

	    msg = QString("No site response script found in following locations: " ) + fullFilePath;
            if (!QFileInfo::exists(fullFilePath)){

	      fullFilePath = QDir::currentPath() + QDir::separator()
		+ "input_data" + QDir::separator() + pathToFile + QDir::separator() + fileName;

	      if (!QFile::exists(fullFilePath)) {
		msg += QString("; ") + fullFilePath; 		  
	
		fullFilePath = QDir::currentPath() + QDir::separator() + pathToFile +
		  QDir::separator() + fileName;
		
                if (!QFile::exists(fullFilePath)) {
		  msg += QString("; ") + fullFilePath;
		  this->errorMessage(msg);		  
		  return false;
                }
	      }
	    }
            siteResponseScriptLineEdit->setText(fullFilePath);
        } else {
	  this->errorMessage("RegionalSiteResponse - no siteResponseScript or siteResponseScriptPath key provided");
	  return false;
	}

        if (appData.contains("soilGridParametersFile") && appData.contains("soilGridParametersFilePath")) {

            QString fileName = appData["soilGridParametersFile"].toString();
            QString pathToFile = appData["soilGridParametersFilePath"].toString();
            QString fullFilePath= pathToFile + QDir::separator() + fileName;

            //
            // files downloaded possibly from a remote run will be in input_data/relevantDIR
            //

	    msg = QString("No soil grid parameters file found in following locations: " ) + fullFilePath;

            if (!QFileInfo::exists(fullFilePath)){

	      fullFilePath = QDir::currentPath() + QDir::separator()
		+ "input_data" + QDir::separator() + pathToFile + QDir::separator() + fileName;	    

	      if (!QFile::exists(fullFilePath)) {
		msg += QString("; ") + fullFilePath; 		  
	
		fullFilePath = QDir::currentPath() + QDir::separator() + pathToFile
		  + QDir::separator() + fileName;
		
                if (!QFile::exists(fullFilePath)) {
		  msg += QString("; ") + fullFilePath;
		  this->errorMessage(msg);		  
		  return false;
                }
	      }
            }
	    
            soilFileLineEdit->setText(fullFilePath);
	    
        } else {
	  this->errorMessage("RegionalSiteResponse - no soilGridParametersFile key provided");
	  return false;
	}

        if (appData.contains("filter"))
            // first send this filter to the site configure widget
            emit siteFilterSignal(appData["filter"].toString());
            this->setFilterString(appData["filter"].toString());

	return true;
  }

  
  // load the motions
  //  this->loadUserGMData();

  return false;
}

bool RegionalSiteResponseWidget::inputFromJSON(QJsonObject &jsonObj) {

  // read in the units
  
  bool res = unitsWidget->inputFromJSON(jsonObj);

  // If setting of units failed, provide default units and issue a warning
  if(!res)
    {
      auto paramNames = unitsWidget->getParameterNames();
      
      this->infoMessage("Warning: Failed to find units in SiteResponseWidget widget. Setting default units for the following parameters:");
      
        for(auto&& it : paramNames)
        {
            auto res = unitsWidget->setUnit(it,"g");

            if(res == 0)
                this->infoMessage("For parameter "+it+" setting default unit as: g");
            else
                this->errorMessage("Failed to set default units for parameter "+it);
        }

        this->infoMessage("Warning \\!/: Check if the units are correct!");
    }

    return res;
}

void RegionalSiteResponseWidget::showUserGMLayers(bool state)
{

}


QStackedWidget* RegionalSiteResponseWidget::getRegionalSiteResponseWidget(void)
{

  if (theStackedWidget)
    return theStackedWidget;
  
  theStackedWidget = new QStackedWidget();
  

  //
    // soil stuff: properties and site response script
    //

    inputWidget = new QWidget();
    QVBoxLayout *inputLayout = new QVBoxLayout(this);
    inputWidget->setLayout(inputLayout);
    
    QGroupBox *soilGroupBox = new QGroupBox("Soil Model");
    soilGroupBox->setContentsMargins(0,5,0,0);
    QGridLayout *soilLayout = new QGridLayout();
    soilGroupBox->setLayout(soilLayout);

    // KZ: I commented the soil properties line edit as we use the site widget to generate the csv now.
    //soilLayout->addWidget(new QLabel("Soil Properties"), 0, 0);
    soilFileLineEdit = new QLineEdit();
    //soilLayout->addWidget(soilFileLineEdit, 0, 1);
    //QPushButton *browseSoilFileButton = new QPushButton("Browse");
    //soilLayout->addWidget(browseSoilFileButton, 0, 2);
    /***
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
    ***/

    //connect(browseSoilFileButton, &QPushButton::clicked, this, &RegionalSiteResponseWidget::soilParamaterFileDialog);

    soilLayout->addWidget(new QLabel("Modeling Script"), 0, 0);
    siteResponseScriptLineEdit = new QLineEdit();
    soilLayout->addWidget(siteResponseScriptLineEdit,0, 1);
    QPushButton *browseScriptButton = new QPushButton("Browse");
    soilLayout->addWidget(browseScriptButton, 0, 2);

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


    //soilLayout->addWidget(new QLabel("Filter"), 1, 0);
    filterLineEdit = new QLineEdit();
    //soilLayout->addWidget(filterLineEdit, 1, 1);
    
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

    unitsWidget = new SimCenterUnitsWidget();
    
    inputLayout->addWidget(unitsWidget);
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

    theStackedWidget->addWidget(inputWidget);
    theStackedWidget->addWidget(progressBarWidget);

    theStackedWidget->setCurrentWidget(inputWidget);

    theStackedWidget->setWindowTitle("Select folder containing earthquake ground motions");

    return theStackedWidget;
}


QStackedWidget* RegionalSiteResponseWidget::getSiteWidget(VisualizationWidget* visWidget)
{
    if (theSiteStackedWidget)
      return theSiteStackedWidget;

    theSiteStackedWidget = new QStackedWidget();

    inputSiteWidget = new QWidget();
    QGridLayout *inputSiteLayout = new QGridLayout(this);
    inputSiteLayout->setContentsMargins(0,0,0,0);
    inputSiteWidget->setLayout(inputSiteLayout);

    // adding Site Config Widget
    m_siteConfig = new SiteConfig(this);
    bool soilResponse = true;
    m_siteConfigWidget = new SiteConfigWidget(*m_siteConfig, visWidget, soilResponse);
    inputSiteLayout->addWidget(m_siteConfigWidget,0,0,4,1);
    // connection: QGIS soil data completeness
    connect(m_siteConfigWidget, SIGNAL(soilDataCompleteSignal(bool)), this, SLOT(setSiteDataFile(bool)));
    connect(this, SIGNAL(writeSiteDataCsv(bool)), this, SLOT(setSiteDataFile(bool)));
    connect(this, SIGNAL(siteFilterSignal(QString)), m_siteConfigWidget, SLOT(setSiteFilterSlot(QString)));

    // adding vs30 widget
    m_vs30 = new Vs30(this);
    m_vs30Widget = new Vs30Widget(*m_vs30, *m_siteConfig);
    inputSiteLayout->addWidget(m_vs30Widget,0,1);

    // adding bedrock widget
    m_bedrockDepth = new BedrockDepth(this);
    m_bedrockDepthWidget = new BedrockDepthWidget(*m_bedrockDepth, *m_siteConfig);
    inputSiteLayout->addWidget(m_bedrockDepthWidget,1,1);

    // adding soil model widget
    m_soilModel = new SoilModel(this);
    m_soilModelWidget = new SoilModelWidget(*m_soilModel, *m_siteConfig);
    inputSiteLayout->addWidget(m_soilModelWidget,2,1);
    connect(m_siteConfigWidget, SIGNAL(siteTypeChangedSignal(SiteConfig::SiteType)), this, SLOT(setSoilModelWidget(SiteConfig::SiteType)));
    connect(m_siteConfigWidget, SIGNAL(activateSoilModelWidgetSignal(bool)), this, SLOT(activateSoilModelWidget(bool)));

    // set up directories
    this->setDir();

    // get site data button
    m_runButton = new QPushButton(tr("&Fetch Site Data"));
    inputSiteLayout->addWidget(m_runButton,3,1);
    connect(m_runButton, &QPushButton::clicked, this, [this]()
    {
        // Get the type of site definition, i.e., single or grid
        auto type = m_siteConfig->getType();
        if(type == SiteConfig::SiteType::Single)
        {
            //QString msg = "Single site selection not supported yet";
            //this->infoMessage(msg);
            //return;
        }
        else if(type == SiteConfig::SiteType::Grid)
        {
            if(!m_siteConfigWidget->getSiteGridWidget()->getGridCreated())
            {
                QString msg = "Please select a grid before continuing";
                this->statusMessage(msg);
                return;
            }
        }
        else if(type == SiteConfig::SiteType::Scatter)
        {
            if(!m_siteConfigWidget->getSiteScatterWidget()->siteFileExists())
            {
                QString msg = "Please choose a site file before continuing";
                this->statusMessage(msg);
                return;
            }
        }
        getSiteData();
    });

#ifdef ARC_GIS
    auto mapView = theVisualizationWidget->getMapViewWidget();

    // Create a map view that will be used for selecting the grid points
    mapViewSubWidget = std::make_unique<MapViewSubWidget>(mapView);

    auto userGrid = mapViewSubWidget->getGrid();
    userGrid->createGrid();
    userGrid->setSiteGridConfig(m_siteConfig);
    userGrid->setVisualizationWidget(theVisualizationWidget);
#endif

    connect(m_siteConfigWidget->getSiteGridWidget(), &SiteGridWidget::selectGridOnMap, this, &RegionalSiteResponseWidget::showGISWindow);

    theSiteStackedWidget->addWidget(inputSiteWidget);
    theSiteStackedWidget->setCurrentWidget(inputSiteWidget);

    // site data completeness
    siteDataFlag = false;
    soilModelFlag = true;

    return theSiteStackedWidget;
}


void RegionalSiteResponseWidget::showUserGMSelectDialog(void)
{

    if (!theStackedWidget)
    {
        this->getRegionalSiteResponseWidget();
    }

    theStackedWidget->show();
    theStackedWidget->raise();
    theStackedWidget->activateWindow();
}

bool
RegionalSiteResponseWidget::copyFiles(QString &destDir)
{
    // create dir and copy motion files
    QDir destDIR(destDir);

    QString motion = "simcMotion";
    QString model = "simcModel";
    
    QDir theMotionDir(motionDir);    
    if (theMotionDir.exists()) 
      motion=theMotionDir.dirName();

    QFileInfo theScript(siteResponseScriptLineEdit->text());
    if (theScript.exists()) {
	QDir theScriptDir(theScript.path());
	model=theScriptDir.dirName();	
    }
    
    QString newModelDir = destDir + QDir::separator() + model;
    QString newMotionDir = destDir + QDir::separator() + motion;   

    destDIR.mkpath(newModelDir);
    destDIR.mkpath(newMotionDir);

    // copy soil model file if any
    if (!m_soilModelWidget->getModelPathFile().isEmpty())
    {
        this->copyFile(m_soilModelWidget->getModelPathFile(), destDir);
    }

    QString soilParameteres = soilFileLineEdit->text();
    QFileInfo soilParametersFile(soilParameteres);
    if (soilParametersFile.exists())
      this->copyFile(soilParameteres, newModelDir);
    else {
        // here we need to see if the site data is complete
        // if so we will send a signal to let QGIS site to write the csv for soilParameters
        // this happens when user directly loaded a complete site data csv (so no data fetching)
        // if not, report the error
        if (siteDataFlag)
            emit writeSiteDataCsv(true);
        else
        {
            this->errorMessage("RegionalSiteResponse:copyFiles soil parameters files does not exist: "+soilParameteres);
            return false;
        }
    }

    QString script = siteResponseScriptLineEdit->text();
    QFileInfo scriptFile(script);
    if (scriptFile.exists()) {
      this->copyPath(scriptFile.path(), newModelDir, false);
    } else {
      this->errorMessage("RegionalSiteResponse:copyFiles script file does not exist: "+scriptFile.path());
      return false;
    }   

    QFileInfo eventFileInfo(eventFile);
    if (eventFileInfo.exists()) {
      this->copyFile(eventFile, newMotionDir);
    } else {
      this->errorMessage("RegionalSiteResponse:copyFiles event grid file does not exist: "+eventFile);
      return false;
    }

    if (theMotionDir.exists()) {
      return this->copyPath(motionDir, newMotionDir, false);
    } else {
      this->errorMessage("RegionalSiteResponse:copyFiles motion Dir file does not exist: "+motionDir);
      return false;
    }

    // rename the motion dir
    QDir dir_motions(newMotionDir);
    dir_motions.rename(newMotionDir, dir_motions.dirName()+QDir::separator()+"inputMotions");

    // should never get here
    return false;
}

#ifdef Q_GIS
void RegionalSiteResponseWidget::loadUserGMData(void)
{
    auto qgisVizWidget = static_cast<QGISVisualizationWidget*>(theVisualizationWidget);

    if(qgisVizWidget == nullptr)
    {
        qDebug()<<"Failed to cast to ArcGISVisualizationWidget";
        return;
    }

    CSVReaderWriter csvTool;

    QString err;
    QVector<QStringList> data = csvTool.parseCSVFile(eventFile, err);

    if(!err.isEmpty())
    {
        this->errorMessage(err);
        return;
    }

    if(data.empty())
        return;

    this->showProgressBar();

    QApplication::processEvents();

    //progressBar->setRange(0,inputFiles.size());
    progressBar->setRange(0, data.count());
    progressBar->setValue(0);

    // Get the headers in the first station file - assume that the rest will be the same
    auto rowStr = data.at(1);
    auto stationName = rowStr[0];

    // Path to station files, e.g., site0.csv
    auto stationFilePath = motionDir + QDir::separator() + stationName;

    QString err2;
    QVector<QStringList> sampleStationData = csvTool.parseCSVFile(stationFilePath,err);

    // Return if there is an error or the station data is empty
    if(!err2.isEmpty())
    {
        this->errorMessage("Could not parse the first station with the following error: "+err2);
        return;
    }

    if(sampleStationData.size() < 2)
    {
        this->errorMessage("The file " + stationFilePath + " is empty");
        return;
    }

    // Get the header file
    auto stationDataHeadings = sampleStationData.first();

    // Create the fields
    QList<QgsField> attribFields;
    attribFields.push_back(QgsField("AssetType", QVariant::String));
    attribFields.push_back(QgsField("TabName", QVariant::String));
    attribFields.push_back(QgsField("Station Name", QVariant::String));
    attribFields.push_back(QgsField("Latitude", QVariant::Double));
    attribFields.push_back(QgsField("Longitude", QVariant::Double));

    for(auto&& it : stationDataHeadings)
    {
        attribFields.push_back(QgsField(it, QVariant::String));
        unitsWidget->addNewUnitItem(it);
    }

    // Set the scale at which the layer will become visible - if scale is too high, then the entire view will be filled with symbols
    // gridLayer->setMinScale(80000);

    // Pop off the row that contains the header information
    data.pop_front();

    auto numRows = data.size();

    int count = 0;

    auto maxToDisp = 20;

    QgsFeatureList featureList;
    // Get the data
    for(int i = 0; i<numRows; ++i)
    {
        auto rowStr = data.at(i);

        auto stationName = rowStr[0];

        // Path to station files, e.g., site0.csv
        auto stationPath = motionDir + QDir::separator() + stationName;

        bool ok;
        auto lon = rowStr[1].toDouble(&ok);

        if(!ok)
        {
            QString errMsg = "Error longitude to a double, check the value";
            this->errorMessage(errMsg);

            this->hideProgressBar();

            return;
        }

        auto lat = rowStr[2].toDouble(&ok);

        if(!ok)
        {
            QString errMsg = "Error latitude to a double, check the value";
            this->errorMessage(errMsg);

            this->hideProgressBar();

            return;
        }

        GroundMotionStation GMStation(stationPath,lat,lon);

        try
        {
            GMStation.importGroundMotions();
        }
        catch(QString msg)
        {
            auto errorMessage = "Error importing ground motion file: " + stationName+"\n"+msg;
            this->errorMessage(errorMessage);

            this->hideProgressBar();

            return;
        }

        auto stationData = GMStation.getStationData();

        // create the feature attributes
        QgsAttributes featAttributes(attribFields.size());

        auto latitude = GMStation.getLatitude();
        auto longitude = GMStation.getLongitude();

        featAttributes[0] = "GroundMotionGridPoint";     // "AssetType"
        featAttributes[1] = "Ground Motion Grid Point";  // "TabName"
        featAttributes[2] = stationName;                 // "Station Name"
        featAttributes[3] = latitude;                    // "Latitude"
        featAttributes[4] = longitude;                   // "Longitude"

        // The number of headings in the file
        auto numParams = stationData.front().size();

        maxToDisp = (maxToDisp<stationData.size() ? maxToDisp : stationData.size());

        QVector<QString> dataStrs(numParams);

        for(int i = 0; i<maxToDisp-1; ++i)
        {
            auto stationParams = stationData[i];

            for(int j = 0; j<numParams; ++j)
            {
                dataStrs[j] += stationParams[j] + ", ";
            }
        }

        for(int j = 0; j<numParams; ++j)
        {
            auto str = dataStrs[j] ;
            str += stationData[maxToDisp-1][j];

            if(maxToDisp<stationData.size())
                str += "...";

            featAttributes[5+j] = str;
        }

        // Create the feature
        QgsFeature feature;
        feature.setGeometry(QgsGeometry::fromPointXY(QgsPointXY(longitude,latitude)));
        feature.setAttributes(featAttributes);
        featureList.append(feature);

        ++count;
        progressLabel->clear();
        progressBar->setValue(count);

        QApplication::processEvents();
    }


    auto vectorLayer = qgisVizWidget->addVectorLayer("Point", "Ground Motion Input Grid");

    if(vectorLayer == nullptr)
    {
        this->errorMessage("Error creating a layer");
        this->hideProgressBar();
        return;
    }

    auto dProvider = vectorLayer->dataProvider();
    auto res = dProvider->addAttributes(attribFields);

    if(!res)
    {
        this->errorMessage("Error adding attribute fields to layer");
        qgisVizWidget->removeLayer(vectorLayer);
        this->hideProgressBar();
        return;
    }

    vectorLayer->updateFields(); // tell the vector layer to fetch changes from the provider

    dProvider->addFeatures(featureList);
    vectorLayer->updateExtents();

    qgisVizWidget->createSymbolRenderer(QgsSimpleMarkerSymbolLayerBase::Cross,Qt::black,2.0,vectorLayer);

    progressLabel->setVisible(false);

    // Reset the widget back to the input pane and close
    this->hideProgressBar();

    /*
    if(theStackedWidget->isModal())
        theStackedWidget->close();
    */
    
   emit loadingComplete(true);

    emit outputDirectoryPathChanged(motionDir, eventFile);

    return;
}
#endif

#ifdef ARC_GIS
void RegionalSiteResponseWidget::loadUserGMData(void)
{

    auto arcVizWidget = static_cast<ArcGISVisualizationWidget*>(theVisualizationWidget);

    if(arcVizWidget == nullptr)
    {
        qDebug()<<"Failed to cast to ArcGISVisualizationWidget";
        return;
    }

    CSVReaderWriter csvTool;

    QString err;
    QVector<QStringList> data = csvTool.parseCSVFile(eventFile, err);

    if(!err.isEmpty())
    {
        this->errorMessage(err);
        return;
    }

    if(data.empty())
        return;

    theStackedWidget->setCurrentWidget(progressBarWidget);
    progressBarWidget->setVisible(true);

    QApplication::processEvents();

    //progressBar->setRange(0,inputFiles.size());
    progressBar->setRange(0, data.count());

    progressBar->setValue(0);

    // Create the table to store the fields
    QList<Field> tableFields;
    tableFields.append(Field::createText("AssetType", "NULL",4));
    tableFields.append(Field::createText("TabName", "NULL",4));
    tableFields.append(Field::createText("Station Name", "NULL",4));
    tableFields.append(Field::createText("Latitude", "NULL",8));
    tableFields.append(Field::createText("Longitude", "NULL",9));
    tableFields.append(Field::createText("Number of Ground Motions","NULL",4));
    tableFields.append(Field::createText("Ground Motions","",1));

    auto gridFeatureCollection = new FeatureCollection(this);

    // Create the feature collection table/layers
    auto gridFeatureCollectionTable = new FeatureCollectionTable(tableFields, GeometryType::Point, SpatialReference::wgs84(), this);
    gridFeatureCollection->tables()->append(gridFeatureCollectionTable);

    auto gridLayer = new FeatureCollectionLayer(gridFeatureCollection,this);

    gridLayer->setName("Ground Motion Grid Points");
    gridLayer->setAutoFetchLegendInfos(true);

    // Create red cross SimpleMarkerSymbol
    SimpleMarkerSymbol* crossSymbol = new SimpleMarkerSymbol(SimpleMarkerSymbolStyle::Cross, QColor("black"), 6, this);

    // Create renderer and set symbol to crossSymbol
    SimpleRenderer* renderer = new SimpleRenderer(crossSymbol, this);
    renderer->setLabel("Ground motion grid points");

    // Set the renderer for the feature layer
    gridFeatureCollectionTable->setRenderer(renderer);

    // Set the scale at which the layer will become visible - if scale is too high, then the entire view will be filled with symbols
    // gridLayer->setMinScale(80000);

    // Pop off the row that contains the header information
    data.pop_front();

    auto numRows = data.size();

    int count = 0;

    // Get the data
    for(int i = 0; i<numRows; ++i)
    {
        auto rowStr = data.at(i);

        auto stationName = rowStr[0];

        // Path to station files, e.g., site0.csv
        auto stationPath = motionDir + QDir::separator() + stationName;

        bool ok;
        auto lon = rowStr[1].toDouble(&ok);

        if(!ok)
        {
            QString errMsg = "Error longitude to a double, check the value";
            this->errorMessage(errMsg);

            theStackedWidget->setCurrentWidget(inputWidget);
            progressBarWidget->setVisible(false);

            return;
        }

        auto lat = rowStr[2].toDouble(&ok);

        if(!ok)
        {
            QString errMsg = "Error latitude to a double, check the value";
            this->errorMessage(errMsg);

            theStackedWidget->setCurrentWidget(inputWidget);
            progressBarWidget->setVisible(false);

            return;
        }

        GroundMotionStation GMStation(stationPath,lat,lon);

        try
        {
            GMStation.importGroundMotions();
        }
        catch(QString msg)
        {

            auto errorMessage = "Error importing ground motion file: " + stationName+"\n"+msg;

            this->errorMessage(errorMessage);

            theStackedWidget->setCurrentWidget(inputWidget);
            progressBarWidget->setVisible(false);

            return;
        }

        // create the feature attributes
        QMap<QString, QVariant> featureAttributes;

        //  auto attrbText = GMStation.
        //  auto attrbVal = pointData[i];
        //  featureAttributes.insert(attrbText,attrbVal);

        auto vecGMs = GMStation.getStationGroundMotions();
        featureAttributes.insert("Number of Ground Motions", vecGMs.size());


        QString GMNames;
        for(int i = 0; i<vecGMs.size(); ++i)
        {
            auto GMName = vecGMs.at(i).getName();

            GMNames.append(GMName);

            if(i != vecGMs.size()-1)
                GMNames.append(", ");

        }

        featureAttributes.insert("Station Name", stationName);
        featureAttributes.insert("Ground Motions", GMNames);
        featureAttributes.insert("AssetType", "GroundMotionGridPoint");
        featureAttributes.insert("TabName", "Ground Motion Grid Point");

        auto latitude = GMStation.getLatitude();
        auto longitude = GMStation.getLongitude();

        featureAttributes.insert("Latitude", latitude);
        featureAttributes.insert("Longitude", longitude);

        // Create the point and add it to the feature table
        Point point(longitude,latitude);
        Feature* feature = gridFeatureCollectionTable->createFeature(featureAttributes, point, this);

        gridFeatureCollectionTable->addFeature(feature);


        ++count;
        progressLabel->clear();
        progressBar->setValue(count);

        QApplication::processEvents();
    }

    // Create a new layer
    auto layersTreeView = arcVizWidget->getLayersTree();

    // Check if there is a 'User Ground Motions' root item in the tree
    auto userInputTreeItem = layersTreeView->getTreeItem("User Ground Motions", nullptr);

    // If there is no item, create one
    if(userInputTreeItem == nullptr)
    {
        auto itemUID = theVisualizationWidget->createUniqueID();
        userInputTreeItem = layersTreeView->addItemToTree("User Ground Motions", itemUID);
    }


    // Add the event layer to the layer tree
    //    auto eventItem = layersTreeView->addItemToTree(eventFile, QString(), userInputTreeItem);

    progressLabel->setVisible(false);

    // Add the event layer to the map
    arcVizWidget->addLayerToMap(gridLayer,userInputTreeItem);

    // Reset the widget back to the input pane and close
    theStackedWidget->setCurrentWidget(inputWidget);
    inputWidget->setVisible(true);

    if(theStackedWidget->isModal())
        theStackedWidget->close();

    emit loadingComplete(true);

    emit outputDirectoryPathChanged(motionDir, eventFile);

    return;
}
#endif


void RegionalSiteResponseWidget::soilParamaterFileDialog(void)
{
    theStackedWidget->show();
    theStackedWidget->raise();
    theStackedWidget->activateWindow();

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

void RegionalSiteResponseWidget::showProgressBar(void)
{
    theStackedWidget->setCurrentWidget(progressBarWidget);
    inputWidget->setVisible(false);
    progressBarWidget->setVisible(true);
}

void RegionalSiteResponseWidget::hideProgressBar(void)
{
    theStackedWidget->setCurrentWidget(inputWidget);
    progressBarWidget->setVisible(false);
    inputWidget->setVisible(true);
}



void RegionalSiteResponseWidget::setFilterString(const QString& filter)
{
    if (filter.isEmpty())
        filterLineEdit->setText(m_siteConfigWidget->getFilter());
    else
        filterLineEdit->setText(filter);
}

QString RegionalSiteResponseWidget::getFilterString(void)
{
    return filterLineEdit->text();
}


void RegionalSiteResponseWidget::showGISWindow(void)
{
    //    theVisualizationWidget->testNewMapCanvas();

#ifdef ARC_GIS
    mapViewSubWidget->addGridToScene();
#endif

#ifdef Q_GIS
    if(mapViewSubWidget == nullptr)
    {
        auto mapViewWidget = theVisualizationWidget->getMapViewWidget("Select grid on map");
        mapViewSubWidget = new MapViewWindow(mapViewWidget);

        auto mapCanvas = mapViewWidget->mapCanvas();

        userGrid = new RectangleGrid(mapCanvas);
        // Also important to get events from QGIS
        mapCanvas->setMapTool(userGrid);
        userGrid->createGrid();
        userGrid->setSiteGridConfig(m_siteConfig);
        userGrid->setVisualizationWidget(theVisualizationWidget);
    }
#endif

    mapViewSubWidget->show();
    userGrid->show();
}

#ifdef ARC_GIS
void RegionalSiteResponseWidget::setCurrentlyViewable(bool status){

    if (status == true)
        mapViewSubWidget->setCurrentlyViewable(status);
}
#endif

void RegionalSiteResponseWidget::setDir(void)
{
    QString localWorkDir = SimCenterPreferences::getInstance()->getLocalWorkDir();
    if(localWorkDir.isEmpty())
    {
        QString errorMessage = QString("Set the Local Jobs Directory location in preferences.");
        this->errorMessage(errorMessage);
        return;
    }
    // Store data in a ground motions folder under hazard simulation
    localWorkDir += QDir::separator() + QString("HazardSimulation") + QDir::separator() + QString("SiteData");
    QDir dirWork(localWorkDir);
    if (!dirWork.exists())
        if (!dirWork.mkpath(localWorkDir))
        {
            QString errorMessage = QString("Could not load the Directory: ") + localWorkDir;
            this->errorMessage(errorMessage);
            return;
        }
    // set directory
    inputSiteDataDir = localWorkDir + QDir::separator() + QString("Input");
    QDir dirInput(inputSiteDataDir);
    if (!dirInput.exists())
        if (!dirInput.mkpath(inputSiteDataDir))
        {
            QString errorMessage = QString("Could not load the Directory: ") + inputSiteDataDir;
            this->errorMessage(errorMessage);
            return;
        }
    outputSiteDataDir = localWorkDir + QDir::separator() + QString("Output");
    QDir dirOutput(outputSiteDataDir);
    if (!dirOutput.exists())
        if (!dirOutput.mkpath(outputSiteDataDir))
        {
            QString errorMessage = QString("Could not load the Directory: ") + outputSiteDataDir;
            this->errorMessage(errorMessage);
            return;
        }
}

void RegionalSiteResponseWidget::setSiteDataFile(bool flag)
{
    siteDataFlag = flag;
    if (flag)
    {
        if(!m_siteConfigWidget->getCsvSiteWidget()->copyFiles(outputSiteDataDir))
        {
            this->errorMessage("Error copying site file to output directory");
        }
        else
        {
            QFileInfo csv_origin(m_siteConfigWidget->getCsvSiteWidget()->getPathToComponentFile());
            QFile csv_file(outputSiteDataDir + QDir::separator() + csv_origin.fileName());
            if (!csv_file.fileName().contains("SiteModelData.csv", Qt::CaseSensitive))
            {
                qDebug() << "overriding SiteModelData.csv...";
                QFileInfo csv_t(outputSiteDataDir + QDir::separator() + "SiteModelData.csv");
                if (csv_t.exists() && csv_t.isFile())
                {
                    QFile csv_tf(outputSiteDataDir + QDir::separator() + "SiteModelData.csv");
                    csv_tf.remove();
                }
                csv_file.rename(outputSiteDataDir + QDir::separator() + "SiteModelData.csv");
            }
        }
        soilFileLineEdit->setText(outputSiteDataDir + QDir::separator() + "SiteModelData.csv");
        this->statusMessage("Soil file path is set up.");
    }
    else
    {
        this->statusMessage("Soil file path is reset.");
        // site data not complete - so clean up the soil file line edit
        soilFileLineEdit->setText("");
    }
}

void RegionalSiteResponseWidget::getSiteData(void)
{
    // site json
    int minID = 0;
    int maxID = 1;
    if(m_siteConfig->getType() == SiteConfig::SiteType::Single)
    {
        maxID = 0;
    }
    else if(m_siteConfig->getType() == SiteConfig::SiteType::Grid)
    {
        maxID = m_siteConfig->siteGrid().getNumSites() - 1;
    }
    else if(m_siteConfig->getType() == SiteConfig::SiteType::Scatter)
    {
        minID = m_siteConfigWidget->getSiteScatterWidget()->getMinID();
        maxID = m_siteConfigWidget->getSiteScatterWidget()->getMaxID();
    }
    else if(m_siteConfig->getType() == SiteConfig::SiteType::UserCSV)
    {
        QString filterIDs = m_siteConfigWidget->getCsvSiteWidget()->getFilterString();
        if (filterIDs.isEmpty())
        {
            this->statusMessage("Warning: no filters defined - will load all sites.");
            m_siteConfigWidget->getCsvSiteWidget()->selectAllComponents();
            filterIDs = m_siteConfigWidget->getCsvSiteWidget()->getFilterString();
        }
        QStringList IDs = filterIDs.split(QRegExp(",|-"), QString::SkipEmptyParts);
        int tmpMin = 10000000;
        int tmpMax = 0;
        for (int i = 0; i < IDs.size(); i++) {
            if (IDs[i].toInt() > tmpMax)
                tmpMax = IDs[i].toInt();
            if (IDs[i].toInt() < tmpMin)
                tmpMin = IDs[i].toInt();
        }
        minID = tmpMin;
        maxID = tmpMax;
    }
    QJsonObject siteObj;
    siteObj.insert("Type", "From_CSV");
    siteObj.insert("input_file", "SiteFile.csv");
    siteObj.insert("output_file", "SiteModelData.csv");
    siteObj.insert("min_ID", minID);
    siteObj.insert("max_ID", maxID);

    // vs30 json
    QJsonObject vs30Obj;
    m_vs30->outputToJSON(vs30Obj);
    siteObj.insert("Vs30", vs30Obj);

    // depth to rock json
    QJsonObject depthToRockObj;
    m_bedrockDepth->outputToJSON(depthToRockObj);
    siteObj.insert("BedrockDepth", depthToRockObj);

    if (soilModelFlag) {
        // soil model
        QJsonObject soilModelObj;
        m_soilModel->outputToJSON(soilModelObj);
        siteObj.insert("SoilModel", soilModelObj);
    }

    // direcotry
    QJsonObject dirObj;
    dirObj.insert("Input", inputSiteDataDir);
    dirObj.insert("Output", outputSiteDataDir);
    dirObj.insert("Work", outputSiteDataDir);

    // configuration json
    QJsonObject configFile;
    configFile.insert("Site",siteObj);
    configFile.insert("Directory", dirObj);

    // write site files
    // the type of site definition, i.e., single or grid
    auto type = m_siteConfig->getType();
    QVector<QStringList> gridData;
    QStringList headerRow = {"Station", "Latitude", "Longitude"};
    gridData.push_back(headerRow);
    bool writeSiteFile = true;
    if(type == SiteConfig::SiteType::Single)
    {
        // The latitude and longitude
        auto longitude = m_siteConfigWidget->getSiteWidget()->get_longitude();
        auto latitude = m_siteConfigWidget->getSiteWidget()->get_latitude();
        QStringList stationRow;
        stationRow.push_back(QString::number(0));
        stationRow.push_back(QString::number(latitude));
        stationRow.push_back(QString::number(longitude));
        gridData.push_back(stationRow);
    }
    else if(type == SiteConfig::SiteType::Grid)
    {
        if(!m_siteConfigWidget->getSiteGridWidget()->getGridCreated())
        {
            QString msg = "Select a grid before continuing";
            this->statusMessage(msg);
            return;
        }
#ifdef ARC_GIS
        // Create the objects needed to visualize the grid in the GIS
        auto siteGrid = mapViewSubWidget->getGrid();

        // Get the vector of grid nodes
        auto gridNodeVec = siteGrid->getGridNodeVec();
#endif
#ifdef Q_GIS
        // Get the vector of grid nodes
        auto gridNodeVec = userGrid->getGridNodeVec();
        auto mapCanvas = mapViewSubWidget->getMapCanvasWidget()->mapCanvas();
#endif
        for(int i = 0; i<gridNodeVec.size(); ++i)
        {
            auto gridNode = gridNodeVec.at(i);
            QStringList stationRow;
            // The station id
            stationRow.push_back(QString::number(i));
            auto screenPoint = gridNode->getPoint();
            // The latitude and longitude
            auto longitude = theVisualizationWidget->getLongFromScreenPoint(screenPoint,mapCanvas);
            auto latitude = theVisualizationWidget->getLatFromScreenPoint(screenPoint,mapCanvas);
            stationRow.push_back(QString::number(latitude));
            stationRow.push_back(QString::number(longitude));
            gridData.push_back(stationRow);
        }
    }
    else if(type == SiteConfig::SiteType::Scatter)
    {
        // Site file will be copied to the input directory
        writeSiteFile = false;
        if(!m_siteConfigWidget->getSiteScatterWidget()->copySiteFile(inputSiteDataDir))
        {
            this->errorMessage("Error copying site file to inputput directory");
        }
    }
    else if(type == SiteConfig::SiteType::UserCSV)
    {
        // Site file will be copied to the input directory
        writeSiteFile = false;
        if(!m_siteConfigWidget->getCsvSiteWidget()->copyFiles(inputSiteDataDir))
        {
            this->errorMessage("Error copying site file to inputput directory");
        }
        else
        {
            QFileInfo csv_origin(m_siteConfigWidget->getCsvSiteWidget()->getPathToComponentFile());
            QFile csv_file(inputSiteDataDir + QDir::separator() + csv_origin.fileName());
            if (!csv_file.fileName().contains("SiteFile.csv", Qt::CaseSensitive))
            {
                QFileInfo csv_t(inputSiteDataDir + QDir::separator() + "SiteFile.csv");
                if (csv_t.exists() && csv_t.isFile())
                {
                    QFile csv_tf(inputSiteDataDir + QDir::separator() + "SiteFile.csv");
                    csv_tf.remove();
                }
                csv_file.rename(inputSiteDataDir + QDir::separator() + "SiteFile.csv");
            }
        }
    }
    QString err;
    if(writeSiteFile)
    {
        QString pathToSiteLocationFile = inputSiteDataDir + QDir::separator() + "SiteFile.csv";
        CSVReaderWriter csvTool;
        auto res = csvTool.saveCSVFile(gridData, pathToSiteLocationFile, err);
        if(res != 0)
        {
            this->errorMessage(err);
            return;
        }
    }
    QString strFromObj = QJsonDocument(configFile).toJson(QJsonDocument::Indented);
    QString pathToConfigFile = inputSiteDataDir + QDir::separator() + "EQHazardConfiguration.json";
    QFile file(pathToConfigFile);
    if(!file.open(QIODevice::WriteOnly))
        file.close();
    else
    {
        QTextStream out(&file); out << strFromObj;
        file.close();
    }
    // run the backend to get site data
    auto pythonPath = SimCenterPreferences::getInstance()->getPython();
    QString pathToHazardSimScript = SimCenterPreferences::getInstance()->getAppDir() + QDir::separator()
            + "applications" + QDir::separator() + "performRegionalEventSimulation" + QDir::separator()
            + "regionalGroundMotion" + QDir::separator() + "HazardSimulation.py";
    QFileInfo hazardFileInfo(pathToHazardSimScript);
    if (!hazardFileInfo.exists()) {
        QString errorMessage = QString("ERROR - hazardApp does not exist") + pathToHazardSimScript;
        this->errorMessage(errorMessage);
        qDebug() << errorMessage;
        return;
    }
    QStringList args = {pathToHazardSimScript,"--hazard_config",pathToConfigFile,"--job_type","Site"};
    // run
    qDebug()<<"Hazard Simulation Command:"<<args[0]<<" "<<args[1]<<" "<<args[2]<<" "<<args[3]<<" "<<args[4];
    this->statusMessage("Fetching site data in the backgroud...");
    processSiteData->start(pythonPath, args);
    processSiteData->waitForFinished();
}

void RegionalSiteResponseWidget::handleProcessStarted(void)
{
    this->m_runButton->setEnabled(false);
}

void RegionalSiteResponseWidget::handleProcessTextOutput(void)
{
    QByteArray output = processSiteData->readAllStandardOutput();
    this->statusMessage(QString(output));
}

void RegionalSiteResponseWidget::handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    this->m_runButton->setEnabled(true);

    if(exitStatus == QProcess::ExitStatus::CrashExit)
    {
        QString errText("Error, the site data fetching failed.");
        this->errorMessage(errText);
        this->getProgressDialog()->hideProgressBar();

        return;
    }

    if(exitCode != 0)
    {
        QString errText("An error occurred in the site data fetching, the exit code is " + QString::number(exitCode));
        this->errorMessage(errText);
        this->getProgressDialog()->hideProgressBar();

        return;
    }

    // set site data file directory
    QString siteDataFilePath = outputSiteDataDir + QDir::separator() + "SiteModelData.csv";
    soilFileLineEdit->setText(siteDataFilePath);

    // filter
    this->setFilterString(filterLineEdit->text());

    // switch QGISSite Input widget
    this->m_siteConfigWidget->setSiteType(SiteConfig::SiteType::UserCSV);

    // set filter
    emit siteFilterSignal(filterLineEdit->text());

    // reload
    if(m_siteConfig->getType() == SiteConfig::SiteType::UserCSV)
    {
        m_siteConfigWidget->getCsvSiteWidget()->reloadComponentData(siteDataFilePath);
    }

}

void RegionalSiteResponseWidget::setSoilModelWidget(SiteConfig::SiteType siteType)
{
    if (siteType==SiteConfig::SiteType::UserCSV)
    {
        m_soilModelWidget->setVisible(false);
        soilModelFlag = false;
    }
    else
    {
        m_soilModelWidget->setVisible(true);
        soilModelFlag = true;
    }
}

void RegionalSiteResponseWidget::activateSoilModelWidget(bool flag)
{
    if (flag)
    {
        m_soilModelWidget->setVisible(true);
        soilModelFlag = true;
    }
    else
    {
        m_soilModelWidget->setVisible(false);
        soilModelFlag = false;
    }
}
