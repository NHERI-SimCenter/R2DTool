/* *****************************************************************************
Copyright (c) 2023, The Regents of the University of California (Regents).
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

#include <stdio.h>
#include "BrailsInventoryGenerator.h"
#include "QGISVisualizationWidget.h"
#include "SimCenterMapcanvasWidget.h"
#include "GIS_Selection.h"
#include "qgsvectorlayer.h"
#include "qgsrectangle.h"
#include "qstackedwidget.h"
#include <qgsmapcanvas.h>
#include <PlainRectangle.h>
#include <BrailsGoogleDialog.h>
//#include <PythonProcessHandler.h>
#include <SimCenterPreferences.h>
#include "ModularPython.h"

#include <QLabel>
#include <QPushButton>
#include <QStandardPaths>
#include <QGridLayout>

#include <SC_DoubleLineEdit.h>
#include <SC_FileEdit.h>
#include <SC_ComboBox.h>
#include <SC_IntLineEdit.h>
#include <SC_DoubleLineEdit.h>

BrailsInventoryGenerator::BrailsInventoryGenerator(VisualizationWidget* visWidget, QWidget* parent) : SimCenterAppWidget(parent)
{
  //    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	// Initialize QGIS visualization with map extent defined in destination (EPSG 3857) coordinates for a rectangle
	// with EPSG 4326 corner coordinates of (37.8227, -122.432) and (37.9739, -122.156):
	theVisualizationWidget = dynamic_cast<QGISVisualizationWidget*>(visWidget);
	QgsRectangle zoomRectangle(QgsPointXY(-13629067.89, 4554409.45), QgsPointXY(-13598343.72, 4575739.41));
	theVisualizationWidget->zoomToExtent(zoomRectangle);
	assert(visWidget);

	minLat = new SC_DoubleLineEdit("minLat", 37.8227);
	maxLat = new SC_DoubleLineEdit("maxLat", 37.9739);
	minLong = new SC_DoubleLineEdit("minLong", -122.432);
	maxLong = new SC_DoubleLineEdit("maxLong", -122.156);
	locationStr = new QLineEdit("");
	theOutputFile = new SC_FileEdit("outputFile");

	QString workDir = SimCenterPreferences::getInstance()->getLocalWorkDir();
	brailsDir = workDir + "/brails";
	QDir dir(brailsDir);
	if (!dir.exists())
		dir.mkpath(brailsDir);

	QString brailsOutput = brailsDir + "/inventory.geojson";
	theOutputFile->setFilename(brailsOutput);

	// Get BRAILS Attributes:
	QStringList attributes = getBRAILSAttributes();

	QString emptystr = "";
	fpGeojsonFile = new SC_FileEdit("fpGeojsonFile");
	fpGeojsonFile->setFilename(emptystr);
	fpAttrGeojsonFile = new SC_FileEdit("fpGeojsonFile");
	fpAttrGeojsonFile->setFilename(emptystr);

	invGeojsonFile = new SC_FileEdit("invGeojsonFile");
	invGeojsonFile->setFilename(emptystr);
	invAttrGeojsonFile = new SC_FileEdit("invGeojsonFile");
	invAttrGeojsonFile->setFilename(emptystr);

	// Put together the stacked widget for location input:
        QWidget* bboxWidget = new QWidget;
	QVBoxLayout* bboxWidgetLayout = new QVBoxLayout(bboxWidget);
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

    QWidget* locationNameWidget = new QWidget;
	QHBoxLayout* locationNameWidgetLayout = new QHBoxLayout(locationNameWidget);
	locationNameWidgetLayout->addWidget(new QLabel("Region name"));
	locationNameWidgetLayout->addWidget(locationStr);

    StackedWidget* stackedWidgetLocation = new StackedWidget;
	stackedWidgetLocation->addWidget(bboxWidget);
	stackedWidgetLocation->addWidget(locationNameWidget);

	// Put together the stacked widget for footprint input:
	QPushButton* showFootprintsButton1 = new QPushButton(tr("Show footprints"));
    QWidget* dummyFpSourceWidget1 = new QWidget;
	QHBoxLayout* dummyFpWidget1Layout = new QHBoxLayout(dummyFpSourceWidget1);
	dummyFpWidget1Layout->addWidget(showFootprintsButton1);
	dummyFpWidget1Layout->addStretch();
	dummyFpWidget1Layout->addStretch();

	QPushButton* showFootprintsButton2 = new QPushButton(tr("Show footprints"));
    QWidget* dummyFpSourceWidget2 = new QWidget;
	QHBoxLayout* dummyFpWidget2Layout = new QHBoxLayout(dummyFpSourceWidget2);
	dummyFpWidget2Layout->addWidget(showFootprintsButton2);
	dummyFpWidget2Layout->addStretch();
	dummyFpWidget2Layout->addStretch();

	QPushButton* showFootprintsButton3 = new QPushButton(tr("Show footprints"));
    QWidget* dummyFpSourceWidget3 = new QWidget;
	QHBoxLayout* dummyFpWidget3Layout = new QHBoxLayout(dummyFpSourceWidget3);
	dummyFpWidget3Layout->addWidget(showFootprintsButton3);
	dummyFpWidget3Layout->addStretch();
	dummyFpWidget3Layout->addStretch();

	QPushButton* showFootprintsButton4 = new QPushButton(tr("Show footprints"));
    QWidget* geojsonFootprintWidget = new QWidget;
	QHBoxLayout* geojsonFootprintWidget1Layout = new QHBoxLayout(geojsonFootprintWidget);
	QVBoxLayout* geojsonFpLeftLayout = new QVBoxLayout();
	geojsonFpLeftLayout->addWidget(new QLabel("GeoJSON file directory"));
	geojsonFpLeftLayout->addWidget(new QLabel("Attribute mapping file"));
	QVBoxLayout* geojsonFpMidLayout = new QVBoxLayout();
	geojsonFpMidLayout->addWidget(fpGeojsonFile);
	geojsonFpMidLayout->addWidget(fpAttrGeojsonFile);
	QHBoxLayout* geojsonFpRightLayout = new QHBoxLayout();
	geojsonFpRightLayout->addWidget(showFootprintsButton4);
	geojsonFootprintWidget1Layout->addLayout(geojsonFpLeftLayout);
	geojsonFootprintWidget1Layout->addLayout(geojsonFpMidLayout);
	geojsonFootprintWidget1Layout->addLayout(geojsonFpRightLayout);

    StackedWidget* stackedWidgetFootprintSource = new StackedWidget;
	stackedWidgetFootprintSource->addWidget(dummyFpSourceWidget1);
	stackedWidgetFootprintSource->addWidget(dummyFpSourceWidget2);
	stackedWidgetFootprintSource->addWidget(dummyFpSourceWidget3);
	stackedWidgetFootprintSource->addWidget(geojsonFootprintWidget);

	// Put together the stacked widget for baseline inventory input:
    QWidget* baselineNoneWidget = new QWidget;
	QHBoxLayout* baselineNoneWidgetLayout = new QHBoxLayout(baselineNoneWidget);
	baselineNoneWidgetLayout->addStretch();
	baselineNoneWidgetLayout->addStretch();
	baselineNoneWidgetLayout->addStretch();

	QPushButton* RawNSIInventoryButton = new QPushButton(tr("Show raw data"));
	QPushButton* ProcessedNSIInventoryButton = new QPushButton(tr("Show processed data"));
    QWidget* baselineInvNSIWidget = new QWidget;
	QHBoxLayout* baselineInvNSILayout = new QHBoxLayout(baselineInvNSIWidget);
	QVBoxLayout* NSIBaselineLeftLayout = new QVBoxLayout();
	NSIBaselineLeftLayout->addWidget(RawNSIInventoryButton);
	NSIBaselineLeftLayout->addWidget(ProcessedNSIInventoryButton);
	baselineInvNSILayout->addLayout(NSIBaselineLeftLayout);
	baselineInvNSILayout->addStretch();
	baselineInvNSILayout->addStretch();

	QPushButton* RawUserInventoryButton = new QPushButton(tr("Show raw data"));
	QPushButton* ProcessedUserInventoryButton = new QPushButton(tr("Show processed data"));
    QWidget* baselineInvUserDefinedWidget = new QWidget;
	QHBoxLayout* geojsonInvWidgetLayout = new QHBoxLayout(baselineInvUserDefinedWidget);
	QVBoxLayout* geojsonInvLeftLayout = new QVBoxLayout();
	geojsonInvLeftLayout->addWidget(new QLabel("GeoJSON file directory"));
	geojsonInvLeftLayout->addWidget(new QLabel("Attribute mapping file"));
	QVBoxLayout* geojsonInvCenterLayout = new QVBoxLayout();
	geojsonInvCenterLayout->addWidget(invGeojsonFile);
	geojsonInvCenterLayout->addWidget(invAttrGeojsonFile);
	QVBoxLayout* geojsonInvRightLayout = new QVBoxLayout();
	geojsonInvRightLayout->addWidget(RawUserInventoryButton);
	geojsonInvRightLayout->addWidget(ProcessedUserInventoryButton);
	geojsonInvWidgetLayout->addLayout(geojsonInvLeftLayout);
	geojsonInvWidgetLayout->addLayout(geojsonInvCenterLayout);
	geojsonInvWidgetLayout->addLayout(geojsonInvRightLayout);

    StackedWidget* stackedWidgetInventorySource = new StackedWidget;
	stackedWidgetInventorySource->addWidget(baselineNoneWidget);
	stackedWidgetInventorySource->addWidget(baselineInvNSIWidget);
	stackedWidgetInventorySource->addWidget(baselineInvUserDefinedWidget);

	// Put together the stacked widget for baseline inventory input:
    QWidget* allAttrWidget = new QWidget;
	QHBoxLayout* allAttrWidgetLayout = new QHBoxLayout(allAttrWidget);
	allAttrWidgetLayout->addStretch();
	allAttrWidgetLayout->addStretch();
	allAttrWidgetLayout->addStretch();
	allAttrWidgetLayout->addStretch();

    QWidget* allHazusSeismicWidget = new QWidget;
	QHBoxLayout* allHazusSeismicWidgetLayout = new QHBoxLayout(allHazusSeismicWidget);
	allHazusSeismicWidgetLayout->addStretch();
	allHazusSeismicWidgetLayout->addStretch();
	allHazusSeismicWidgetLayout->addStretch();
	allHazusSeismicWidgetLayout->addStretch();

    QWidget* enabledAttributesWidget = new QWidget;
	enabledAttributesWidgetLayout = new QGridLayout(enabledAttributesWidget);

	int counter = 0;
	foreach(QString attr, attributes) {
		int nrow = counter / 6;
		int ncol = counter % 6;
		++counter;

		QCheckBox* checkbox = new QCheckBox(attr, enabledAttributesWidget);
		checkbox->setChecked(false);
		enabledAttributesWidgetLayout->addWidget(checkbox, nrow, ncol);
	}
	enabledAttributesWidget->setLayout(enabledAttributesWidgetLayout);
	enabledAttributesWidget->show();

    StackedWidget* stackedAttributesWidget = new StackedWidget;
	stackedAttributesWidget->addWidget(allAttrWidget);
	stackedAttributesWidget->addWidget(allHazusSeismicWidget);
	stackedAttributesWidget->addWidget(enabledAttributesWidget);

	// Define the main grid layout:
	QGridLayout* mainLayout = new QGridLayout(this);

	// Define the combo box that prompts for outputs units:
	int numRow = 0;
	QStringList unitList; unitList << "m" << "ft";
    units = new SC_ComboBox("units", unitList);
	mainLayout->addWidget(new QLabel("Output units"), numRow, 0);
	mainLayout->addWidget(units, numRow, 1);

	// Define the combo box that prompts for query area definition:
	numRow++;
	QStringList locationList; locationList << "Bounding box" << "Region name";
    location = new SC_ComboBox("location", locationList);
    mainLayout->addWidget(new QLabel("Area definition"), numRow, 0);
    mainLayout->addWidget(location, numRow, 1);

	// Connect the query area definition combo box to the stacked widget for location input:
    QObject::connect(location, QOverload<int>::of(&QComboBox::currentIndexChanged), stackedWidgetLocation, &QStackedWidget::setCurrentIndex);
    mainLayout->addWidget(stackedWidgetLocation, numRow, 3);
    QPushButton* showRegionButton = new QPushButton(tr("Show region"));
    mainLayout->addWidget(showRegionButton, numRow, 4);
    connect(showRegionButton, SIGNAL(clicked()), this, SLOT(getLocationBoundary()));

    // Define the combo box that prompts for footprints source:
	numRow++;
	QStringList footprintSources; footprintSources << "Microsoft Global Building Footprints" << "OpenStreetMap" << "FEMA USA Structures" << "User-defined";
	footprintSource = new SC_ComboBox("footprint", footprintSources);
    mainLayout->addWidget(new QLabel("Footprint source"), numRow, 0);
    mainLayout->addWidget(footprintSource, numRow, 1);
    mainLayout->setColumnStretch(1,1);

	// Connect the footprint input combo box to the stacked widget for footprint source:
    QObject::connect(footprintSource, QOverload<int>::of(&QComboBox::currentIndexChanged), stackedWidgetFootprintSource, &QStackedWidget::setCurrentIndex);

    mainLayout->addWidget(stackedWidgetFootprintSource, numRow, 3);
    connect(showFootprintsButton1, SIGNAL(clicked()), this, SLOT(getFootprints()));
	connect(showFootprintsButton2, SIGNAL(clicked()), this, SLOT(getFootprints()));
	connect(showFootprintsButton3, SIGNAL(clicked()), this, SLOT(getFootprints()));
    connect(showFootprintsButton4, SIGNAL(clicked()), this, SLOT(getFootprints()));

	// Define the combo box that prompts for a baseline inventory selection:
	numRow++;
	QStringList baselineInventories; baselineInventories << "None" << "National Structure Inventory" << "User-defined";
	baselineInvSelection = new SC_ComboBox("baseline", baselineInventories);
	mainLayout->addWidget(new QLabel("Baseline inventory"), numRow, 0);
    mainLayout->addWidget(baselineInvSelection, numRow, 1);
    connect(RawNSIInventoryButton, &QPushButton::clicked, this, [this]() {getBaselineInv("raw"); });
	connect(ProcessedNSIInventoryButton, &QPushButton::clicked, this, [this]() {getBaselineInv("processed"); });
	connect(RawUserInventoryButton, &QPushButton::clicked, this, [this]() {getBaselineInv("raw"); });
	connect(ProcessedUserInventoryButton, &QPushButton::clicked, this, [this]() {getBaselineInv("processed"); });

	// Connect the baseline inventory selection combo box to the stacked widget for baseline inventories:
    QObject::connect(baselineInvSelection, QOverload<int>::of(&QComboBox::currentIndexChanged), stackedWidgetInventorySource, &QStackedWidget::setCurrentIndex);
    mainLayout->addWidget(stackedWidgetInventorySource, numRow, 3);

	// Define the combo box for selecting the building attributes that will be generated by BRAILS:
	numRow++;
	QStringList requestedAttributes; requestedAttributes << "All" << "HAZUS seismic attributes" << "Select from enabled attributes";
    attributeSelection = new SC_ComboBox("attributes", requestedAttributes);
	mainLayout->addWidget(new QLabel("Requested attributes"), numRow, 0);
    mainLayout->addWidget(attributeSelection, numRow, 1);

	// Connect the attribute selection combo box to the stacked widget for attributes:
    QObject::connect(attributeSelection, QOverload<int>::of(&QComboBox::currentIndexChanged), stackedAttributesWidget, &QStackedWidget::setCurrentIndex);
    mainLayout->addWidget(stackedAttributesWidget, numRow, 3);

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
    QStringList imputationAlgos; imputationAlgos << "None";
    imputationAlgoCombo = new SC_ComboBox("imputation", imputationAlgos);
    mainLayout->addWidget(new QLabel("Imputation algorithm"), numRow, 0);
    mainLayout->addWidget(imputationAlgoCombo, numRow, 1, 1, 3);

    /*
	connect(imputationAlgoCombo, &QComboBox::currentTextChanged, this, [=](QString text) {
		imputationAlgo = text;
		});
    */

	// Create the directory selector for BRAILS inventory output:
	numRow++;
	mainLayout->addWidget(new QLabel("Output directory"), numRow, 0);
    mainLayout->addWidget(theOutputFile, numRow, 1, 1, 7);

	// Create the Run BRAILS button:
	numRow++;
	QPushButton* runButton = new QPushButton(tr("Run BRAILS Inventory Generator"));
    mainLayout->addWidget(runButton, numRow, 0, 1, 8);
	connect(runButton, SIGNAL(clicked()), this, SLOT(runBRAILS()));

	// Show the GIS selection window for obtaining bounding box coordinates:
	numRow++;
    theSelectionWidget = new GIS_Selection(theVisualizationWidget);
    mainLayout->addWidget(theSelectionWidget, numRow, 0, 1, 8);
	connect(theSelectionWidget, SIGNAL(selectionGeometryChanged()), this, SLOT(coordsChanged()));

	// set current selections
	//imageSource=imageSourceCombo->currentText();
	imageSource = "Google";
}

BrailsInventoryGenerator::~BrailsInventoryGenerator()
{

}

void BrailsInventoryGenerator::clear(void)
{

}

void
BrailsInventoryGenerator::loadVectorLayer(QString outputFile, QString layerName){
    theVisualizationWidget->addVectorLayer(outputFile, layerName, "ogr");
    theVisualizationWidget->zoomToActiveLayer();
}

QStringList
BrailsInventoryGenerator::getBRAILSAttributes(void) {
	// Get today's date:
	std::time_t now;
	std::time(&now);
	std::tm ptm;
	
    #ifdef _WIN32
            localtime_s(&ptm, &now);
    #else
            localtime_r(&now, &ptm);
    #endif

	std::string datestr = std::to_string(ptm.tm_year + 1900) + "-" + std::to_string(ptm.tm_mon + 1) + "-" + std::to_string(ptm.tm_mday);
	QString date = QString::fromStdString(datestr);

	// Generate the file path for an attribute file name including today's date:
	QString attrFile = brailsDir + "/brailsBuildingAttributes(" + date + ").txt";

	// Check if the file exists in brailsdir. If not create it by calling BRAILS:
	QFile file(attrFile);

	if (!file.exists()) {
		QString appDir = SimCenterPreferences::getInstance()->getAppDir();
		QDir scriptDir(appDir + QDir::separator());
		scriptDir.cd("applications");
		scriptDir.cd("tools");
		scriptDir.cd("BRAILS");
		QString fpDownloadScript = scriptDir.absoluteFilePath("getBRAILSAttributes.py");

		QStringList scriptArgs;
		scriptArgs << QString("--outputFile") << attrFile;

		qDebug() << "BRAILS script: " << fpDownloadScript;
		qDebug() << "BRAILS args: " << scriptArgs;
		ModularPython* thePy = new ModularPython(brailsDir);
		thePy->run(fpDownloadScript, scriptArgs);
	}

	// Read the outputFile and parse the contained attributes into a QStringList:
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		qDebug() << "Failed to open the file:" << file.errorString();
	}

	QStringList attributes;
	QTextStream in(&file);
	while (!in.atEnd()) {
		QString attr = in.readLine();
		attributes.append(attr);
	}
	file.close();

	return attributes;
}

void
BrailsInventoryGenerator::getLocationBoundary(void)
{
	regionData regionInp;
	if (location->currentText() == "Bounding box") {
		regionInp.minLat = minLat->getDouble();
		regionInp.maxLat = maxLat->getDouble();
		regionInp.minLong = minLong->getDouble();
		regionInp.maxLong = maxLong->getDouble();
		regionInp.location = "";
	}
	else if (location->currentText() == "Region name") {
		regionInp.minLat = 0.0;
		regionInp.maxLat = 0.0;
		regionInp.minLong = 0.0;
		regionInp.maxLong = 0.0;
		regionInp.location = locationStr->text();
	}

	// Generate the file path for the boundary output file:
	regionInp.outputFile = brailsDir + "/regionBoundary.geojson";

	QString appDir = SimCenterPreferences::getInstance()->getAppDir();
	QDir scriptDir(appDir + QDir::separator());
	scriptDir.cd("applications");
	scriptDir.cd("tools");
	scriptDir.cd("BRAILS");
	QString locationBoundaryScript = scriptDir.absoluteFilePath("getBRAILSLocationBoundary.py");

	QStringList scriptArgs;
    scriptArgs << QString("--latMin") << QString::number(regionInp.minLat)
               << QString("--latMax") << QString::number(regionInp.maxLat)
               << QString("--longMin") << QString::number(regionInp.minLong)
               << QString("--longMax") << QString::number(regionInp.maxLong)
               << QString("--location") << regionInp.location
               << QString("--outputFile") << regionInp.outputFile;

	qDebug() << "BRAILS script: " << locationBoundaryScript;
	qDebug() << "BRAILS args: " << scriptArgs;
	ModularPython* thePy = new ModularPython(brailsDir);
	thePy->run(locationBoundaryScript, scriptArgs);

    // Load the vector layer:
    loadVectorLayer(regionInp.outputFile, "RegionBoundary");
	theVisualizationWidget->setActiveLayerFillNull();
}

void
BrailsInventoryGenerator::getFootprints(void)
{
	fpData fpInp;
	if (location->currentText() == "Bounding box") {
		fpInp.minLat = minLat->getDouble();
		fpInp.maxLat = maxLat->getDouble();
		fpInp.minLong = minLong->getDouble();
		fpInp.maxLong = maxLong->getDouble();
		fpInp.location = "";
	}
	else if (location->currentText() == "Region name") {
		fpInp.minLat = 0.0;
		fpInp.maxLat = 0.0;
		fpInp.minLong = 0.0;
		fpInp.maxLong = 0.0;
		fpInp.location = locationStr->text();
	}

	fpInp.fpSourceAttrMap = "";
	if (footprintSource->currentText() == "Microsoft Global Building Footprints") {
		fpInp.fpSource = "ms";
	}
	else if (footprintSource->currentText() == "OpenStreetMap") {
		fpInp.fpSource = "osm";
	}
	else if (footprintSource->currentText() == "FEMA USA Structures") {
		fpInp.fpSource = "usastr";
	}
	else if (footprintSource->currentText() == "User-defined") {
		fpInp.fpSource = fpGeojsonFile->getFilename();
		fpInp.fpSourceAttrMap = fpAttrGeojsonFile->getFilename();
	}

	QString printSuffix;
	if (fpInp.fpSource.contains("geojson") == NULL && fpInp.fpSource.contains("csv") == NULL) {
		printSuffix = fpInp.fpSource;
	}
	else {
		printSuffix = "userdefined";
	}

	fpInp.outputFile = brailsDir + "/footprints_" + printSuffix + ".geojson";
	fpInp.units = units->currentText();

	QString appDir = SimCenterPreferences::getInstance()->getAppDir();
	QDir scriptDir(appDir + QDir::separator());
	scriptDir.cd("applications");
	scriptDir.cd("tools");
	scriptDir.cd("BRAILS");
	QString fpDownloadScript = scriptDir.absoluteFilePath("getBRAILSFootprints.py");

	QStringList scriptArgs;
	scriptArgs << QString("--latMin") << QString::number(fpInp.minLat)
               << QString("--latMax") << QString::number(fpInp.maxLat)
               << QString("--longMin") << QString::number(fpInp.minLong)
               << QString("--longMax") << QString::number(fpInp.maxLong)
               << QString("--location") << fpInp.location
               << QString("--fpSource") << fpInp.fpSource
               << QString("--fpSourceAttrMap") << fpInp.fpSourceAttrMap
               << QString("--outputFile") << fpInp.outputFile
               << QString("--lengthUnit") << fpInp.units;

	qDebug() << "BRAILS script: " << fpDownloadScript;
	qDebug() << "BRAILS args: " << scriptArgs;
	ModularPython* thePy = new ModularPython(brailsDir);
	thePy->run(fpDownloadScript, scriptArgs);

	// Load the vector layer:
	QString layerName = "BuildingFootprints_" + printSuffix;
    loadVectorLayer(fpInp.outputFile, layerName);
}

void
BrailsInventoryGenerator::getBaselineInv(QString outputDataType)
{
	binvData binvInp;

	QString printSuffix1;
	if (baselineInvSelection->currentText() == "National Structure Inventory") {
		binvInp.invInput = "NSI";
		binvInp.invAttributeMap = "";
		printSuffix1 = "NSI";
	}
	else {
		binvInp.invInput = invGeojsonFile->getFilename();
		binvInp.invAttributeMap = invAttrGeojsonFile->getFilename();
		printSuffix1 = "UserDefined";
	}

	QString printSuffix2;
	if (outputDataType == "raw") {
		printSuffix2 = "Raw";
	}
	else {
		printSuffix2 = "Processed";
	}

	if (printSuffix1 == "UserDefined" && printSuffix2 == "Raw") {
		binvInp.outputFile = binvInp.invInput;
	}
	else {
		if (location->currentText() == "Bounding box") {
			binvInp.minLat = minLat->getDouble();
			binvInp.maxLat = maxLat->getDouble();
			binvInp.minLong = minLong->getDouble();
			binvInp.maxLong = maxLong->getDouble();
			binvInp.location = "";
		}
		else if (location->currentText() == "Region name") {
			binvInp.minLat = 0.0;
			binvInp.maxLat = 0.0;
			binvInp.minLong = 0.0;
			binvInp.maxLong = 0.0;
			binvInp.location = locationStr->text();
		}

		if (footprintSource->currentText() == "Microsoft Global Building Footprints") {
			binvInp.fpSource = "ms";
		}
		else if (footprintSource->currentText() == "OpenStreetMap") {
			binvInp.fpSource = "osm";
		}
		else if (footprintSource->currentText() == "FEMA USA Structures") {
			binvInp.fpSource = "usastr";
		}
		else if (footprintSource->currentText() == "User-defined") {
			binvInp.fpSource = fpGeojsonFile->getFilename();
		}

		binvInp.outputDataType = outputDataType;

		binvInp.outputFile = brailsDir + "/BaselineInventory_" + printSuffix1 + printSuffix2 + ".geojson";
		binvInp.units = units->currentText();

		QString appDir = SimCenterPreferences::getInstance()->getAppDir();
		QDir scriptDir(appDir + QDir::separator());
		scriptDir.cd("applications");
		scriptDir.cd("tools");
		scriptDir.cd("BRAILS");
		QString fpDownloadScript = scriptDir.absoluteFilePath("getBRAILSBaselineInv.py");

		QStringList scriptArgs;
		scriptArgs << QString("--latMin") << QString::number(binvInp.minLat)
                   << QString("--latMax") << QString::number(binvInp.maxLat)
                   << QString("--longMin") << QString::number(binvInp.minLong)
                   << QString("--longMax") << QString::number(binvInp.maxLong)
                   << QString("--location") << binvInp.location
                   << QString("--fpSource") << binvInp.fpSource
                   << QString("--invInput") << binvInp.invInput
                   << QString("--invAttributeMap") << binvInp.invAttributeMap
                   << QString("--outputDataType") << binvInp.outputDataType
                   << QString("--outputFile") << binvInp.outputFile
                   << QString("--lengthUnit") << binvInp.units;

		qDebug() << "BRAILS script: " << fpDownloadScript;
		qDebug() << "BRAILS args: " << scriptArgs;
		ModularPython* thePy = new ModularPython(brailsDir);
		thePy->run(fpDownloadScript, scriptArgs);
	}

	// Load the vector layer:
    QString layerName = "BaselineInventory_" + printSuffix1 + printSuffix2;
    loadVectorLayer(binvInp.outputFile, layerName);
}

void BrailsInventoryGenerator::runBRAILS(void)
{

	BrailsData brailsData;
	// Parse unit input:
	brailsData.units = units->currentText();

	// Parse location input:
	if (location->currentText() == "Bounding box") {
		brailsData.minLat = minLat->getDouble();
		brailsData.maxLat = maxLat->getDouble();
		brailsData.minLong = minLong->getDouble();
		brailsData.maxLong = maxLong->getDouble();
		brailsData.location = "";
	}
	else if (location->currentText() == "Region name") {
		brailsData.minLat = 0.0;
		brailsData.maxLat = 0.0;
		brailsData.minLong = 0.0;
		brailsData.maxLong = 0.0;
		brailsData.location = locationStr->text();
	}

	// Parse footprint input:
	brailsData.fpSourceAttrMap = "";
	if (footprintSource->currentText() == "Microsoft Global Building Footprints") {
		brailsData.fpSource = "ms";
	}
	else if (footprintSource->currentText() == "OpenStreetMap") {
		brailsData.fpSource = "osm";
	}
	else if (footprintSource->currentText() == "FEMA USA Structures") {
		brailsData.fpSource = "usastr";
	}
	else if (footprintSource->currentText() == "User-defined") {
		brailsData.fpSource = fpGeojsonFile->getFilename();
		brailsData.fpSourceAttrMap = fpAttrGeojsonFile->getFilename();
	}

	// Parse baseline inventory selection:
	brailsData.invAttributeMap = "";
    if (baselineInvSelection->currentText() == "None") {
		brailsData.invInput = "None";
	}
	else if (baselineInvSelection->currentText() == "National Structure Inventory") {
		brailsData.invInput = "NSI";
	}
	else if (baselineInvSelection->currentText() == "User-defined") {
		brailsData.invInput = invGeojsonFile->getFilename();
		brailsData.invAttributeMap = invAttrGeojsonFile->getFilename();
	}

	// Parse attribute selection:
	if (attributeSelection->currentText() == "All") {
		brailsData.attrRequested = "all";
	}
	else if (attributeSelection->currentText() == "HAZUS seismic attributes") {
		brailsData.attrRequested = "hazuseq";
	}
	else if (attributeSelection->currentText() == "Select from enabled attributes") {
		// Write all the selected attributes in a string delimited with comma:
		QString attrSelection = "";
		int numRows = enabledAttributesWidgetLayout->rowCount();
		int numCols = enabledAttributesWidgetLayout->columnCount();
		for (int row = 0; row < numRows; ++row) {
			for (int col = 0; col < numCols; ++col) {
				QLayoutItem* item = enabledAttributesWidgetLayout->itemAtPosition(row, col);
				if (item && item->widget()) {
					QCheckBox* checkbox = qobject_cast<QCheckBox*>(item->widget());
					if (checkbox) {
						QString attributeName = checkbox->text();
						if (checkbox->isChecked()) {
							;
							attrSelection = attrSelection + "," + attributeName;
						}
					}
				}
			}
		}

		// Remove the first character if it is a comma:
		if (attrSelection.startsWith(",")) {
			attrSelection.remove(0, 1);
			brailsData.attrRequested = attrSelection;
		}
	}

	// Parse imputation algorithm selection:
	brailsData.imputationAlgo = imputationAlgoCombo->currentText();

    // Parse output file directory entered:
    brailsData.outputFile = theOutputFile->getFilename();

    // Parse the image source:
    brailsData.imageSource = imageSource;

	if (imageSource == "Google") {
		statusMessage("Starting BRAILS...");
		if (theGoogleDialog == 0) {
			theGoogleDialog = new BrailsGoogleDialog(this);
		}
		theGoogleDialog->setData(brailsData);
		theGoogleDialog->show();
		theGoogleDialog->raise();
		theGoogleDialog->activateWindow();
		statusMessage("BRAILS-Buildings widget window is activated");
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
