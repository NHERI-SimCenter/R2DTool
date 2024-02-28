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

// Written by: Barbaros Cetiner, Frank McKenna, Stevan Gavrilovic

#include "BrailsInventoryGenerator.h"
#include "QGISVisualizationWidget.h"
#include "SimCenterMapcanvasWidget.h"
#include "GIS_Selection.h"
#include "qstackedwidget.h"
#include <qgsmapcanvas.h>
#include <PlainRectangle.h>
#include <BrailsGoogleDialog.h>
//#include <PythonProcessHandler.h>
#include <SimCenterPreferences.h>

#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QStandardPaths>
#include <QGroupBox>
#include <QGridLayout>
#include <SC_DoubleLineEdit.h>
#include <QComboBox>

#include <SC_FileEdit.h>
#include <SC_ComboBox.h>
#include <SC_IntLineEdit.h>
#include <SC_DoubleLineEdit.h>
#include <QSettings>

BrailsInventoryGenerator::BrailsInventoryGenerator(VisualizationWidget* visWidget, QWidget *parent) : SimCenterAppWidget(parent)
{
    theVisualizationWidget = dynamic_cast<QGISVisualizationWidget*>(visWidget);
    assert(visWidget);
    
    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);

    minLat  = new SC_DoubleLineEdit("minLat",37.8227);
    maxLat  = new SC_DoubleLineEdit("maxLat",37.9739);
    minLong = new SC_DoubleLineEdit("minLong",-122.432);
    maxLong = new SC_DoubleLineEdit("maxLong",-122.156);
    locationStr = new QLineEdit("");
    theOutputFile = new SC_FileEdit("outputFile");

    QString appDir = SimCenterPreferences::getInstance()->getLocalWorkDir();
    QString brailsDir = appDir + "/brails";
    QDir dir(brailsDir);
    if (!dir.exists())
      dir.mkpath(brailsDir);

    QString brailsOutput = brailsDir + "/inventory.geojson";
    theOutputFile->setFilename(brailsOutput);

    fpGeojsonFile = new SC_FileEdit("fpGeojsonFile");
    fpGeojsonFile->setFilename(brailsDir);
    fpAttrGeojsonFile = new SC_FileEdit("fpGeojsonFile");
    fpAttrGeojsonFile->setFilename(brailsDir);

    invGeojsonFile = new SC_FileEdit("invGeojsonFile");
    invGeojsonFile->setFilename(brailsDir);
    invAttrGeojsonFile = new SC_FileEdit("invGeojsonFile");
    invAttrGeojsonFile->setFilename(brailsDir);

    QWidget *bboxWidget = new QWidget;
    QVBoxLayout *bboxWidgetLayout = new QVBoxLayout(bboxWidget);
    QHBoxLayout* bboxTopLayout = new QHBoxLayout();
    bboxTopLayout->addWidget(new QLabel("Latitude"));
    bboxTopLayout->addWidget(new QLabel("Longitude"));
    QHBoxLayout* bboxBottomLayout = new QHBoxLayout();
    bboxBottomLayout->addWidget(new QLabel("min:"));
    bboxBottomLayout->addWidget(minLat);
    bboxBottomLayout->addWidget(new QLabel("max:"));
    bboxBottomLayout->addWidget(maxLat);
    bboxBottomLayout->addWidget(new QLabel("min:"));
    bboxBottomLayout->addWidget(minLong);
    bboxBottomLayout->addWidget(new QLabel("max:"));
    bboxBottomLayout->addWidget(maxLong);
    bboxWidgetLayout->addLayout(bboxTopLayout);
    bboxWidgetLayout->addLayout(bboxBottomLayout);

    QWidget *locationNameWidget = new QWidget;
    QHBoxLayout *locationNameWidgetLayout = new QHBoxLayout(locationNameWidget);
    locationNameWidgetLayout->addWidget(new QLabel("Region name"));
    locationNameWidgetLayout->addWidget(locationStr);

    // Put together the stacked widget for footprint input:
    QStackedWidget *stackedWidgetLocation = new QStackedWidget;
    stackedWidgetLocation->addWidget(bboxWidget);
    stackedWidgetLocation->addWidget(locationNameWidget);
    stackedWidgetLocation->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    QWidget *dummyFpSourceWidget1 = new QWidget;
    QWidget *dummyFpSourceWidget2 = new QWidget;
    QWidget *dummyFpSourceWidget3 = new QWidget;
    QWidget *geojsonFootprintWidget1 = new QWidget;
    QVBoxLayout *geojsonFootprintWidget1Layout = new QVBoxLayout(geojsonFootprintWidget1);
    QHBoxLayout* geojsonFpTopLayout = new QHBoxLayout();
    geojsonFpTopLayout->addWidget(new QLabel("GeoJSON file directory"));
    geojsonFpTopLayout->addWidget(fpGeojsonFile);
    QHBoxLayout* geojsonFpBottomLayout = new QHBoxLayout();
    geojsonFpBottomLayout->addWidget(new QLabel("Attribute mapping file"));
    geojsonFpBottomLayout->addWidget(fpAttrGeojsonFile);

    geojsonFootprintWidget1Layout->addLayout(geojsonFpTopLayout);
    geojsonFootprintWidget1Layout->addLayout(geojsonFpBottomLayout);

    QStackedWidget *stackedWidgetFootprintSource = new QStackedWidget;
    stackedWidgetFootprintSource->addWidget(dummyFpSourceWidget1);
    stackedWidgetFootprintSource->addWidget(dummyFpSourceWidget2);
    stackedWidgetFootprintSource->addWidget(dummyFpSourceWidget3);
    stackedWidgetFootprintSource->addWidget(geojsonFootprintWidget1);


    QWidget *dummyInvSourceWidget1 = new QWidget;
    QWidget *dummyInvSourceWidget2 = new QWidget;
    QWidget *baselineInvWidget = new QWidget;
    QVBoxLayout *geojsonInvWidgetLayout = new QVBoxLayout(baselineInvWidget);
    QHBoxLayout* geojsonInvTopLayout = new QHBoxLayout();
    geojsonInvTopLayout->addWidget(new QLabel("GeoJSON file directory"));
    geojsonInvTopLayout->addWidget(invGeojsonFile);
    QHBoxLayout* geojsonInvBottomLayout = new QHBoxLayout();
    geojsonInvBottomLayout->addWidget(new QLabel("Attribute mapping file"));
    geojsonInvBottomLayout->addWidget(invAttrGeojsonFile);
    geojsonInvWidgetLayout->addLayout(geojsonInvTopLayout);
    geojsonInvWidgetLayout->addLayout(geojsonInvBottomLayout);

    QStackedWidget *stackedWidgetInventorySource = new QStackedWidget;
    stackedWidgetInventorySource->addWidget(dummyInvSourceWidget1);
    stackedWidgetInventorySource->addWidget(dummyInvSourceWidget2);
    stackedWidgetInventorySource->addWidget(baselineInvWidget);

    QGridLayout *mainLayout = new QGridLayout(this);

    // Define the combo box that prompts for outputs units:
    int numRow = 0;
    QStringList unitList; unitList << "m" << "ft";    
    units = new SC_ComboBox("units", unitList);
    mainLayout->addWidget(new QLabel("Output units"),numRow,0);
    mainLayout->addWidget(units,numRow,1);

    // Define the combo box that prompts for query area definition:
    numRow++;
    QStringList locationList; locationList << "Bounding box" << "Region name";
    location = new SC_ComboBox("location", locationList);
    mainLayout->addWidget(new QLabel("Query area definition"),numRow,0);
    mainLayout->addWidget(location,numRow,1);

    // Connect the query area definition combo box to the stacked widget for location input:
    QObject::connect(location, QOverload<int>::of(&QComboBox::currentIndexChanged), stackedWidgetLocation, &QStackedWidget::setCurrentIndex);
    mainLayout->addWidget(stackedWidgetLocation,numRow,3);

    // Define the combo box that prompts for footprints source:
    numRow++;
    QStringList footprintSources; footprintSources << "Microsoft Global Building Footprints" << "OpenStreetMap"  << "FEMA USA Structures" << "User-defined";
    footprintSource = new SC_ComboBox("footprint", footprintSources);
    mainLayout->addWidget(new QLabel("Footprint source"),numRow,0);
    mainLayout->addWidget(footprintSource,numRow,1);

    // Connect the footprint input combo box to the stacked widget for footprint source:
    QObject::connect(footprintSource, QOverload<int>::of(&QComboBox::currentIndexChanged), stackedWidgetFootprintSource, &QStackedWidget::setCurrentIndex);
    mainLayout->addWidget(stackedWidgetFootprintSource,numRow,3);

    // Define the combo box that prompts for a baseline inventory selection:
    numRow++;
    QStringList baselineInventories; baselineInventories << "None" << "National Structure Inventory" << "User-defined";
    baselineInvSelection = new SC_ComboBox("baseline", baselineInventories);
    mainLayout->addWidget(new QLabel("Baseline inventory selection"),numRow,0);
    mainLayout->addWidget(baselineInvSelection,numRow,1);

    // Connect the baseline inventory selection combo box to the stacked widget for baseline inventories:
    QObject::connect(baselineInvSelection, QOverload<int>::of(&QComboBox::currentIndexChanged), stackedWidgetInventorySource, &QStackedWidget::setCurrentIndex);
    mainLayout->addWidget(stackedWidgetInventorySource,numRow,3);

    // Define the combo box for selecting the building attributes that will be generated by BRAILS:
    numRow++;
    QStringList requestedAttributes; requestedAttributes << "All" << "HAZUS seismic attributes" << "Select from enabled attributes" << "Footprints only";
    attributeSelected = new SC_ComboBox("baseline", requestedAttributes);
    mainLayout->addWidget(new QLabel("Requested attributes"),numRow,0);
    mainLayout->addWidget(attributeSelected,numRow,1);

    /*
    numRow++;
    mainLayout->addWidget(new QLabel("Image Source"),numRow,0);
    QComboBox *imageSourceCombo = new QComboBox();
    imageSourceCombo->addItem("Google");
    imageSourceCombo->addItem("Local");
    imageSourceCombo->addItem("NHERI DesignSafe");
    imageSourceCombo->addItem("NHERI Rapid");
    mainLayout->addWidget(imageSourceCombo,numRow,1,1,3);
    
    connect(imageSourceCombo, &QComboBox::currentTextChanged, this, [=](QString text) {
      imageSource = text;
    });
    */

    // Define the combo box for selecting the imputation algorithm:
    numRow++;
    mainLayout->addWidget(new QLabel("Imputation algorithm"),numRow,0);
    QComboBox *imputationAlgoCombo = new QComboBox();
    imputationAlgoCombo->addItem("None");    
    mainLayout->addWidget(imputationAlgoCombo,numRow,1,1,3);

    connect(imputationAlgoCombo, &QComboBox::currentTextChanged, this, [=](QString text) {
      imputationAlgo = text;
    });

    // Create the directory selector for BRAILS inventory output:
    numRow++;
    mainLayout->addWidget(new QLabel("BRAILS output file directory"),numRow,0);
    mainLayout->addWidget(theOutputFile,numRow,1,1,7);

    // Create the Run BRAILS button:
    numRow++;
    QPushButton *runButton = new QPushButton(tr("Run BRAILS"));
    mainLayout->addWidget(runButton,numRow,0,1,8);
    connect(runButton,SIGNAL(clicked()),this,SLOT(runBRAILS()));

    // Show the GIS selection window for obtaining bounding box coordinates:
    numRow++;
    theSelectionWidget = new GIS_Selection(theVisualizationWidget);
    mainLayout->addWidget(theSelectionWidget,numRow,0,1,8);
    connect(theSelectionWidget,SIGNAL(selectionGeometryChanged()), this, SLOT(coordsChanged()));

    // set current selections
    //imageSource=imageSourceCombo->currentText();
    imageSource="Google";
    imputationAlgo=imputationAlgoCombo->currentText();
}

BrailsInventoryGenerator::~BrailsInventoryGenerator()
{

}

void BrailsInventoryGenerator::clear(void)
{

}

void BrailsInventoryGenerator::runBRAILS(void)
{

  // make sure output dir exists
  QFileInfo fileInfo(theOutputFile->getFilename());
  QString outputPath = fileInfo.absolutePath();
  QString fileName = fileInfo.baseName();  
  QDir dir(outputPath);
    if (!dir.exists())
      dir.mkpath(outputPath);
    
  BrailsData brailsData;
  if (location->currentText()=="Bounding box") {
    brailsData.minLat = minLat->getDouble();
    brailsData.maxLat = maxLat->getDouble();
    brailsData.minLong = minLong->getDouble();
    brailsData.maxLong = maxLong->getDouble();
    brailsData.location = "";
  } else if (location->currentText()=="Region name") {
    brailsData.minLat = 0.0;
    brailsData.maxLat = 0.0;
    brailsData.minLong = 0.0;
    brailsData.maxLong = 0.0;
    brailsData.location = locationStr->text();
  }

  brailsData.fpSourceAttrMap = "";
  if (footprintSource->currentText()=="Microsoft Global Building Footprints") {
    brailsData.fpSource = "ms";
  } else if (footprintSource->currentText()=="OpenStreetMap") {
    brailsData.fpSource = "osm";
  } else if (footprintSource->currentText()=="FEMA USA Structures") {
    brailsData.fpSource = "usastr";
  } else if (footprintSource->currentText()=="User-defined") {
    brailsData.fpSource = fpGeojsonFile->getFilename();
    brailsData.fpSourceAttrMap = fpAttrGeojsonFile->getFilename();
  }
  brailsData.outputFile =theOutputFile->getFilename();
  brailsData.imageSource = imageSource;
  brailsData.imputationAlgo = imputationAlgo;
  brailsData.units = units->currentText();

  if (imageSource == "Google") {
    errorMessage("Starting BRAILS .. ");
    if (theGoogleDialog == 0) {
      theGoogleDialog = new BrailsGoogleDialog(this);
    }
    theGoogleDialog->setData(brailsData);
    theGoogleDialog->show();
    theGoogleDialog->raise();
    theGoogleDialog->activateWindow();
    errorMessage("Window shown and activated .. ");    
  }
}


void BrailsInventoryGenerator::coordsChanged(void)
{
  QVector<double> points = theSelectionWidget->getSelectedPoints();
  minLat->setText(QString::number(points.at(0)));
  minLong->setText(QString::number(points.at(1)));
  maxLat->setText(QString::number(points.at(6)));
  maxLong->setText(QString::number(points.at(7)));    
}
