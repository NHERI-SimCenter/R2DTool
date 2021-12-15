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
    theStackedWidget = nullptr;
    progressLabel = nullptr;
    eventFile = "";
    motionDir = "";
    unitsWidget = nullptr;
    //    theStackedWidget = nullptr;    
    
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
	QDir dirFile(theSoil.path());
	appData["soilGridParametersFilePath"]=dirFile.dirName();		
    } else {
      appData["soilGridParametersFile"]=soilFileLineEdit->text();
      appData["soilGridParametersFilePath"]=QString("");
    }

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


    soilLayout->addWidget(new QLabel("Filter"), 2, 0);
    filterLineEdit = new QLineEdit();
    soilLayout->addWidget(filterLineEdit, 2, 1);    
    
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

    QString soilParameteres = soilFileLineEdit->text();
    QFileInfo soilParametersFile(soilParameteres);
    if (soilParametersFile.exists())
      this->copyFile(soilParameteres, newModelDir);
    else {
      qDebug() << "RegionalSiteResponse:copyFiles soil parameters files does not exist:" << soilParameteres;
      return false;
    }

    QString script = siteResponseScriptLineEdit->text();
    QFileInfo scriptFile(script);
    if (scriptFile.exists()) {
      this->copyPath(scriptFile.path(), newModelDir, false);
    } else {
      qDebug() << "RegionalSiteResponse:copyFiles script file does not exist:" << scriptFile;
      return false;
    }   

    QFileInfo eventFileInfo(eventFile);
    if (eventFileInfo.exists()) {
      this->copyFile(eventFile, newMotionDir);
    } else {
      qDebug() << "RegionalSiteResponse:copyFiles event grid file does not exist:" << eventFile;
      return false;
    }

    if (theMotionDir.exists()) {
      return this->copyPath(motionDir, newMotionDir, false);
    } else {
      qDebug() << "RegionalSiteResponse:copyFiles motion Dir file does not exist:" << motionDir;
      return false;
    }

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
  filterLineEdit->setText(filter);
}

QString RegionalSiteResponseWidget::getFilterString(void)
{
    return filterLineEdit->text();
}
