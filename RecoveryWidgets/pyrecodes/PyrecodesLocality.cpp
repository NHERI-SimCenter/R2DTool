/* *****************************************************************************
Copyright (c) 2016-2023, The Regents of the University of California (Regents).
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THE SOFTWARE IS PROVIDED "AS IS". REGENTS HAS NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*************************************************************************** */


#include <PyrecodesLocality.h>
#include <PyrecodesOptions.h>
#include <QGridLayout>
#include <QLabel>
#include <QGroupBox>
#include <PyrecodesSystemConfig.h>

#include <SC_FileEdit.h>
#include <SC_ComboBox.h>

#include <QLineEdit>
#include <QPlainTextEdit>
#include <QDebug>
#include <QPushButton>
#include <QWidget>
#include <QDialog>
#include <QStackedWidget>
#include <QComboBox>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>

// A starter template for the Components section. Uses R2D subsystem creators
// since the main use case is running pyrecodes from R2DTool (the creator reads
// the R2D exposure/damage JSON). The user edits this to match their region.
static const char *kComponentsTemplate =
"{\n"
"  \"Infrastructure\": [\n"
"    {\n"
"      \"TransportationSystem\": {\n"
"        \"CreatorClassName\": \"R2DSubsystemCreator\",\n"
"        \"CreatorFileName\": \"r2d_subsystem_creator\",\n"
"        \"Parameters\": {\n"
"          \"Resource\": [\"TransportationService\"],\n"
"          \"R2DJSONFile_Info\": \"{folder}/Exposure.json\",\n"
"          \"SubsystemNameInR2DJSON\": \"TransportationNetwork\",\n"
"          \"AssetTypes\": [\"Bridge\", \"Roadway\", \"Tunnel\"]\n"
"        }\n"
"      }\n"
"    }\n"
"  ],\n"
"  \"BuildingStock\": [\n"
"    {\n"
"      \"Buildings\": {\n"
"        \"CreatorClassName\": \"R2DSubsystemCreator\",\n"
"        \"CreatorFileName\": \"r2d_subsystem_creator\",\n"
"        \"Parameters\": {\n"
"          \"Resource\": [\"Shelter\", \"FunctionalHousing\"],\n"
"          \"R2DJSONFile_Info\": \"{folder}/Exposure.json\",\n"
"          \"SubsystemNameInR2DJSON\": \"Buildings\",\n"
"          \"AssetTypes\": [\"Buildings\"]\n"
"        }\n"
"      }\n"
"    }\n"
"  ],\n"
"  \"RecoveryResourceSuppliers\": []\n"
"}\n";

PyrecodesLocality::PyrecodesLocality(PyrecodesSystemConfig *theSC, QString name, QWidget *parent)
  :SimCenterWidget(parent), theSystemConfig(theSC)
{
  theName = new QLineEdit();
  theName->setText(name);
  theName->setToolTip("Locality name, e.g. \"Locality 1\". Referenced by distribution priorities\n"
		      "and links between localities in the System Configuration.");

  //
  // Coordinates: edited in their own pop-up window. The main window shows a
  // read-only summary and a button that opens the coordinates window. The
  // window uses a stacked widget so only the fields for the chosen type show.
  //

  QStringList coordTypes; coordTypes << "Centroid" << "GeoJSON" << "BoundingBox";
  coordType = new SC_ComboBox("coordType", coordTypes);
  coordType->setToolTip("How the locality's location is given:\n"
			"  Centroid    - a single point (X, Y)\n"
			"  GeoJSON     - a polygon read from a .geojson file (typical for R2D regions)\n"
			"  BoundingBox - a rectangle [xmin, ymin, xmax, ymax]");

  centroidX = new QLineEdit();   centroidX->setPlaceholderText("X");
  centroidX->setToolTip("Centroid X coordinate (longitude or projected X).");
  centroidY = new QLineEdit();   centroidY->setPlaceholderText("Y");
  centroidY->setToolTip("Centroid Y coordinate (latitude or projected Y).");
  geoJSONFile = new SC_FileEdit("geoJSONFile");
  geoJSONFile->setToolTip("Path to a .geojson file with the locality polygon.");
  boundingBox = new QLineEdit(); boundingBox->setPlaceholderText("[xmin, ymin, xmax, ymax]");
  boundingBox->setToolTip("Rectangle as a JSON array: [xmin, ymin, xmax, ymax].");

  // one page of fields per coordinate type
  coordStack = new QStackedWidget();

  QWidget *centroidPage = new QWidget();
  QGridLayout *centroidLayout = new QGridLayout(centroidPage);
  centroidLayout->addWidget(new QLabel("X:"), 0, 0);
  centroidLayout->addWidget(centroidX, 0, 1);
  centroidLayout->addWidget(new QLabel("Y:"), 1, 0);
  centroidLayout->addWidget(centroidY, 1, 1);
  centroidLayout->setRowStretch(2, 1);

  QWidget *geoPage = new QWidget();
  QGridLayout *geoLayout = new QGridLayout(geoPage);
  geoLayout->addWidget(new QLabel("GeoJSON file:"), 0, 0);
  geoLayout->addWidget(geoJSONFile, 0, 1);
  geoLayout->setRowStretch(1, 1);

  QWidget *bboxPage = new QWidget();
  QGridLayout *bboxLayout = new QGridLayout(bboxPage);
  bboxLayout->addWidget(new QLabel("Bounding Box:"), 0, 0);
  bboxLayout->addWidget(boundingBox, 0, 1);
  bboxLayout->setRowStretch(1, 1);

  coordStack->addWidget(centroidPage);   // index 0 -> Centroid
  coordStack->addWidget(geoPage);        // index 1 -> GeoJSON
  coordStack->addWidget(bboxPage);       // index 2 -> BoundingBox
  connect(coordType, QOverload<int>::of(&QComboBox::currentIndexChanged),
	  coordStack, &QStackedWidget::setCurrentIndex);

  // the dedicated coordinates window
  coordDialog = new QDialog(this);
  coordDialog->setWindowTitle("Define Coordinates");
  coordDialog->setMinimumWidth(420);
  QGridLayout *dlgLayout = new QGridLayout(coordDialog);
  QLabel *dlgIntro = new QLabel("Choose how this locality's location is specified, then fill in the fields.");
  dlgIntro->setWordWrap(true);
  dlgIntro->setStyleSheet("color: #555; font-style: italic;");
  dlgLayout->addWidget(dlgIntro, 0, 0, 1, 2);
  dlgLayout->addWidget(new QLabel("Type:"), 1, 0);
  dlgLayout->addWidget(coordType, 1, 1);
  dlgLayout->addWidget(coordStack, 2, 0, 1, 2);
  QPushButton *coordDoneButton = new QPushButton("Done");
  connect(coordDoneButton, &QPushButton::clicked, coordDialog, &QDialog::accept);
  dlgLayout->addWidget(coordDoneButton, 3, 1);

  // main-window summary + button to open the coordinates window
  QGroupBox *coordinates = new QGroupBox("Coordinates");
  QGridLayout *coordinatesLayout = new QGridLayout(coordinates);
  coordSummary = new QLineEdit();
  coordSummary->setReadOnly(true);
  coordSummary->setPlaceholderText("No coordinates set - click \"Define Coordinates…\"");
  QPushButton *editCoordButton = new QPushButton("Define Coordinates…");
  editCoordButton->setToolTip("Open the coordinates window to set this locality's location.");
  connect(editCoordButton, &QPushButton::clicked, this, [=]() {
    coordStack->setCurrentIndex(coordType->currentIndex());
    coordDialog->exec();
    coordSummary->setText(this->coordinateSummary());
  });
  coordinatesLayout->addWidget(coordSummary, 0, 0);
  coordinatesLayout->addWidget(editCoordButton, 0, 1);
  coordinatesLayout->setColumnStretch(0, 1);

  //
  // Components group: JSON editor (Infrastructure / BuildingStock /
  // RecoveryResourceSuppliers, each a list of subsystem creators)
  //

  QGroupBox *components = new QGroupBox("Components");
  QGridLayout *componentsLayout = new QGridLayout(components);
  componentsEdit = new QPlainTextEdit();
  componentsEdit->setPlainText(kComponentsTemplate);
  componentsEdit->setToolTip(
      "Components in this locality, grouped under Infrastructure / BuildingStock /\n"
      "RecoveryResourceSuppliers. Each entry names a subsystem and a creator:\n"
      "  JSONSubsystemCreator - list components inline via \"ComponentsInLocality\"\n"
      "  R2DSubsystemCreator  - build them from the R2D exposure file (R2DJSONFile_Info,\n"
      "                         SubsystemNameInR2DJSON, AssetTypes).\n"
      "Use \"Insert R2D Template\" for a starting point. Component names must exist in the Component Library.");

  QPushButton *insertTemplateButton = new QPushButton("Insert R2D Template");
  insertTemplateButton->setToolTip("Replace the editor contents with a ready-to-edit R2D components block.");
  connect(insertTemplateButton, &QPushButton::clicked, this, [=]() {
    componentsEdit->setPlainText(kComponentsTemplate);
  });
  QLabel *help = new QLabel(
      "Each entry under Infrastructure / BuildingStock / RecoveryResourceSuppliers "
      "is a subsystem created by one of: " + PyrecodesOptions::subsystemCreatorClasses().join(", ") + ".");
  help->setWordWrap(true);

  componentsLayout->addWidget(insertTemplateButton, 0, 0);
  componentsLayout->addWidget(help, 1, 0);
  componentsLayout->addWidget(componentsEdit, 2, 0);

  //
  // done button
  //

  QPushButton *doneButton = new QPushButton("Done");
  connect(doneButton, &QPushButton::clicked, this, [=]() {
    QStringList componentsSelected;
    QJsonParseError pe;
    QJsonDocument pd = QJsonDocument::fromJson(componentsEdit->toPlainText().toUtf8(), &pe);
    if (pe.error == QJsonParseError::NoError && pd.isObject()) {
      QJsonObject obj = pd.object();
      for (const QString &group : obj.keys()) {
	QJsonValue v = obj.value(group);
	if (v.isArray() && !v.toArray().isEmpty())
	  componentsSelected << group;
      }
    } else {
      this->errorMessage("Components JSON is not valid - please fix before saving.");
    }

    theSystemConfig->addOrUpdateLocalityTableEntry(theName->text(),
						   this->coordinateSummary(),
						   componentsSelected);
    this->close();
  });


  //
  // main layout
  //

  QGridLayout *layout = new QGridLayout();

  QLabel *intro = new QLabel(
      "Define one locality: give it a name, set its coordinates, and list the components it "
      "contains. Hover any field for help. Click Done when finished.");
  intro->setWordWrap(true);
  intro->setStyleSheet("color: #555; font-style: italic;");
  layout->addWidget(intro, 0, 0, 1, 4);

  layout->addWidget(new QLabel("Name:"),1,0);
  layout->addWidget(theName, 1,1);

  layout->addWidget(coordinates, 2, 0, 1, 4);
  layout->addWidget(components, 3, 0,  1, 4);
  layout->addWidget(doneButton, 4, 1, 1, 2);

  layout->setRowStretch(3,1);

  this->setLayout(layout);
}


PyrecodesLocality::~PyrecodesLocality()
{

}

void PyrecodesLocality::clear(void)
{

}

QString
PyrecodesLocality::coordinateSummary()
{
  QString type = coordType->currentText();
  if (type == "Centroid")
    return QString("Centroid (%1, %2)").arg(centroidX->text(), centroidY->text());
  else if (type == "GeoJSON")
    return QString("GeoJSON: %1").arg(geoJSONFile->getFilename());
  else
    return QString("BoundingBox %1").arg(boundingBox->text());
}

bool
PyrecodesLocality::outputToJSON(QJsonObject &jsonObject)
{

  QJsonObject obj;

  //
  // Coordinates
  //

  QJsonObject coordObj;
  QString type = coordType->currentText();
  if (type == "Centroid") {
    QJsonObject centroid;
    bool okX, okY;
    centroid["X"] = centroidX->text().toDouble(&okX);
    centroid["Y"] = centroidY->text().toDouble(&okY);
    coordObj["Centroid"] = centroid;
  } else if (type == "GeoJSON") {
    QJsonObject geo;
    geo["Filename"] = geoJSONFile->getFilename();
    coordObj["GeoJSON"] = geo;
  } else if (type == "BoundingBox") {
    QJsonParseError pe;
    QJsonDocument pd = QJsonDocument::fromJson(boundingBox->text().toUtf8(), &pe);
    if (pe.error == QJsonParseError::NoError && pd.isArray())
      coordObj["BoundingBox"] = pd.array();
    else
      coordObj["BoundingBox"] = boundingBox->text();
  }
  obj["Coordinates"] = coordObj;

  //
  // Components (parse the JSON editor)
  //

  QJsonParseError pe;
  QJsonDocument pd = QJsonDocument::fromJson(componentsEdit->toPlainText().toUtf8(), &pe);
  if (pe.error == QJsonParseError::NoError && pd.isObject())
    obj["Components"] = pd.object();
  else
    obj["Components"] = QJsonObject();

  jsonObject[theName->text()] = obj;

  return true;
}

bool
PyrecodesLocality::inputFromJSON(QJsonObject &jsonObject)
{

  QStringList componentsSelected;

  //
  // Coordinates
  //

  QJsonValue coordsValue = jsonObject["Coordinates"];
  if (coordsValue.isObject()) {
    QJsonObject coordObj = coordsValue.toObject();
    if (coordObj.contains("Centroid") && coordObj["Centroid"].isObject()) {
      coordType->setCurrentText("Centroid");
      QJsonObject c = coordObj["Centroid"].toObject();
      centroidX->setText(QString::number(c["X"].toDouble()));
      centroidY->setText(QString::number(c["Y"].toDouble()));
    } else if (coordObj.contains("GeoJSON") && coordObj["GeoJSON"].isObject()) {
      coordType->setCurrentText("GeoJSON");
      QJsonObject g = coordObj["GeoJSON"].toObject();
      QString fn = g.contains("Filename") ? g["Filename"].toString() : g["FileName"].toString();
      geoJSONFile->setFilename(fn);
    } else if (coordObj.contains("BoundingBox")) {
      coordType->setCurrentText("BoundingBox");
      QJsonValue bb = coordObj["BoundingBox"];
      if (bb.isArray())
	boundingBox->setText(QString(QJsonDocument(bb.toArray()).toJson(QJsonDocument::Compact)));
      else
	boundingBox->setText(bb.toString());
    }
  }
  coordStack->setCurrentIndex(coordType->currentIndex());
  coordSummary->setText(coordinateSummary());

  //
  // Components
  //

  QJsonValue componentsValue = jsonObject["Components"];
  if (componentsValue.isObject()) {
    QJsonObject componentsObj = componentsValue.toObject();
    componentsEdit->setPlainText(QString(QJsonDocument(componentsObj).toJson(QJsonDocument::Indented)));
    for (const QString &group : componentsObj.keys()) {
      QJsonValue v = componentsObj.value(group);
      if (v.isArray() && !v.toArray().isEmpty())
	componentsSelected << group;
    }
  }

  theSystemConfig->addOrUpdateLocalityTableEntry(theName->text(),
						 coordinateSummary(),
						 componentsSelected);
  return true;
}

bool
PyrecodesLocality::copyFiles(QString &dirName) {
  Q_UNUSED(dirName);
  return true;
}
