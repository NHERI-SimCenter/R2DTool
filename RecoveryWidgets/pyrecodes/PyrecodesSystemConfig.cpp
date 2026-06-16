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

THIS SSensorsTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES Sensors MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPBodiesE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT Sensors SUBSTITUTE GOODS OR SERVICES;
LBodiesS Sensors USE, DATA, OR PRSensorsITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY Sensors LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT Sensors THE USE Sensors THIS
SSensorsTWARE, EVEN IF ADVISED Sensors THE PBodiesSIBILITY Sensors SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the FreeBSD Project.

REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
THE IMPLIED WARRANTIES Sensors MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPBodiesE.
THE SSensorsTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS 
PROVIDED "AS IS". REGENTS HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, 
UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

*************************************************************************** */


#include <PyrecodesSystemConfig.h>
#include <PyrecodesOptions.h>
#include <QGridLayout>
#include <QPushButton>
#include <QTableWidget>
#include <QStringList>
#include <QHeaderView>
#include <QTabWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QComboBox>
#include <QFileInfo>
#include <QFile>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QFileDialog>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QMessageBox>
#include <QGroupBox>

#include <SC_ComboBox.h>

#include "Utils/RelativePathResolver.h"
#include "Utils/FileOperations.h"

#include <PyrecodesLocality.h>
#include <PyrecodesTemplates.h>

// helpers for combo cell widgets in tables
static QComboBox *makeCombo(const QStringList &items, bool allowEmpty = false) {
  QComboBox *cb = new QComboBox();
  if (allowEmpty)
    cb->addItem("");
  cb->addItems(items);
  return cb;
}
static QString comboTextAt(QTableWidget *t, int row, int col) {
  QComboBox *cb = qobject_cast<QComboBox *>(t->cellWidget(row, col));
  return cb ? cb->currentText() : QString();
}
static void setComboAt(QTableWidget *t, int row, int col, const QString &value) {
  QComboBox *cb = qobject_cast<QComboBox *>(t->cellWidget(row, col));
  if (cb) {
    int idx = cb->findText(value);
    if (idx < 0 && !value.isEmpty()) { cb->addItem(value); idx = cb->findText(value); }
    cb->setCurrentIndex(idx >= 0 ? idx : 0);
  }
}
static void setHeaderTipSC(QTableWidget *t, int col, const QString &tip) {
  if (QTableWidgetItem *h = t->horizontalHeaderItem(col))
    h->setToolTip(tip);
}
// make a table's columns user-resizable and sized to fit their header labels
static void setupColumns(QTableWidget *t) {
  QHeaderView *h = t->horizontalHeader();
  h->setSectionResizeMode(QHeaderView::Interactive);
  h->setMinimumSectionSize(40);
  t->resizeColumnsToContents();
}

PyrecodesSystemConfig::PyrecodesSystemConfig(QWidget *parent)
  :SimCenterWidget(parent)
{
  //
  // first a QLineEdit to Load and Save the info to a file
  //
  
  theSystemFile = new QLineEdit();
  theSystemFile->setToolTip("Path of the system configuration file (set when you Load or Save).");
  QPushButton *loadFileButton = new QPushButton("Load");
  loadFileButton->setToolTip("Load an existing system configuration JSON file.");
  QPushButton *loadTemplateButton = new QPushButton("Load Template");
  loadTemplateButton->setToolTip("Start from a built-in example covering every section (Constants, Localities,\n"
				 "Resources, Damage Input, Resilience Calculators) that you can edit.");
  QPushButton *saveFileButton = new QPushButton("Save");
  saveFileButton->setToolTip("Save the system configuration to a JSON file (used by pyrecodes / R2DTool).");

  //
  // code for when user pushes the loadTemplateButton
  //   - load the embedded starter system-configuration template
  //
  connect(loadTemplateButton, &QPushButton::clicked, this, [=]() {
    QJsonDocument doc = QJsonDocument::fromJson(PyrecodesTemplates::systemConfiguration().toUtf8());
    QJsonObject jsonObj = doc.object();
    this->clear();
    if (this->inputFromJSON(jsonObj))
      theSystemFile->setText("<template>");
    else
      this->errorMessage("Could not load system configuration template");
  });

  //
  // code for when user pushes the loadFileButton
  //   - basically open file, load it into json, and then invoke inputFromJSON
  //
  
  connect(loadFileButton, &QPushButton::clicked, this, [=]() {

    // put up a dialog to allow user to get a file
    QString fileTypeStr = QString("All files (*.*)");    
    QString fileName=QFileDialog::getOpenFileName(this,tr("Open File"),"", fileTypeStr); 

    // open file
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
      this->errorMessage(QString("Could Not Open File: ") + fileName);
      return;
    }

    // read the file into a json object
    QString val;
    val=file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(val.toUtf8());
    QJsonObject jsonObj = doc.object();

    //Resolve absolute paths from relative ones in json objects
    QFileInfo fileInfo(fileName);
    SCUtils::ResolveAbsolutePaths(jsonObj, fileInfo.dir());    

    // close file
    file.close();

    // clear current contents & invoke inputFromJSON
    this->clear();
    bool result = this->inputFromJSON(jsonObj);
    
    if (result == true)
    theSystemFile->setText(fileName);
    else {
        this->errorMessage(QString("Error Parsing JSON file: ") + fileName);
    }
  });

  //
  // code for when user pushes the saveFileButton
  //   - basically open file, create a JSON object and invoke outputToJSON, write that object to file
  //
  
  connect(saveFileButton, &QPushButton::clicked, this, [=]() {

    // use a dialog to get a file to open
    
    QFileDialog dialog(this, "Save System Config");
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    QStringList filters;
    filters << "Json files (*.json)"
            << "All files (*)";

    dialog.setNameFilters(filters);

    if (dialog.exec() == QDialog::Rejected) {
      this->errorMessage("Save Dile dialog Rejected");
      return;
    }

    QString fileName = dialog.selectedFiles().first();

    // open file, make sure file not read only
    
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(fileName),
                                  file.errorString()));
        return;
    }

    //
    // create a json object, fill it in & then use a QJsonDocument
    // to write the contents of the object to the file in JSON format
    //

    QJsonObject json;
    this->outputToJSON(json);

    //Resolve relative paths before saving
    QFileInfo fileInfo(fileName);
    SCUtils::ResolveRelativePaths(json, fileInfo.dir());

    QJsonDocument doc(json);
    file.write(doc.toJson());

    // close file
    file.close();

    // set current file
    theSystemFile->setText(fileName);    
    
  });    
  
  //
  // next we will break up using a QTabbedWidget
  //

  QTabWidget *theTabbedWidget = new QTabWidget();

  //
  // constants tab
  //
  
  QWidget *constantsWidget = new QWidget();
  QGridLayout *constantsLayout = new QGridLayout(constantsWidget);

  QLabel *constantsIntro = new QLabel(
      "Global simulation settings. Required: START_TIME_STEP (usually 0), MAX_TIME_STEP "
      "(how long to simulate) and DISASTER_TIME_STEP (when the hazard hits, e.g. 1). "
      "Value may be a number, a JSON list or a JSON object.");
  constantsIntro->setWordWrap(true);
  constantsIntro->setStyleSheet("color: #555; font-style: italic;");

  // addConstant PushButton
  QPushButton *addConstantButton = new QPushButton("Add Constant");
  addConstantButton->setToolTip("Add a global setting (name/value pair).");

  QStringList headingsConstant; headingsConstant << "Name" << "Value";
  theConstantsTable = new QTableWidget();
  theConstantsTable->setColumnCount(2);
  theConstantsTable->setHorizontalHeaderLabels(headingsConstant);
  setupColumns(theConstantsTable);
  theConstantsTable->verticalHeader()->setVisible(false);
  theConstantsTable->setShowGrid(true);
  theConstantsTable->setAlternatingRowColors(true);
  if (theConstantsTable->horizontalHeaderItem(0)) theConstantsTable->horizontalHeaderItem(0)->setToolTip("Constant name, e.g. MAX_TIME_STEP");
  if (theConstantsTable->horizontalHeaderItem(1)) theConstantsTable->horizontalHeaderItem(1)->setToolTip("Number, or JSON list/object, e.g. 1000  or  [\"Shelter\",\"FunctionalHousing\"]");
      
  connect(addConstantButton, &QPushButton::clicked, this, [=]() {
    
    // add a row and set values to empty strings
    int row = theConstantsTable->rowCount();
    theConstantsTable->insertRow(row);
    for (int col = 0; col < 2; ++col) {
      QTableWidgetItem *item = new QTableWidgetItem(""); // Empty cell
      theConstantsTable->setItem(row, col, item);
    }
    
  });  

  constantsLayout->addWidget(constantsIntro, 0, 0, 1, 4);
  constantsLayout->addWidget(addConstantButton, 1,0);
  constantsLayout->addWidget(theConstantsTable, 2, 0, 1, 4);

  
  //
  //   localities tab
  //
  
  QWidget *localitiesWidget = new QWidget();
  QGridLayout *layoutLocalities = new QGridLayout(localitiesWidget);

  QLabel *localitiesIntro = new QLabel(
      "Localities are the geographic units of the region (a community, neighbourhood, or TAZ). "
      "Each locality has coordinates and a set of components. Click \"Add Locality\" to define one.");
  localitiesIntro->setWordWrap(true);
  localitiesIntro->setStyleSheet("color: #555; font-style: italic;");

  // addLocality PushButton
  QPushButton *addLocalityButton = new QPushButton("Add Locality");
  addLocalityButton->setToolTip("Open a dialog to define a locality: its coordinates and components.");


  connect(addLocalityButton, &QPushButton::clicked, this, [=]() {
    this->addLocality();
  });

  // locality table

  QStringList headingsLocality; headingsLocality << "Locality" << "Coordinates" << "Components";
  theLocalityTable = new QTableWidget();
  theLocalityTable->setColumnCount(3);
  theLocalityTable->setHorizontalHeaderLabels(headingsLocality);
  setupColumns(theLocalityTable);
  theLocalityTable->verticalHeader()->setVisible(false);
  theLocalityTable->setShowGrid(true);
  theLocalityTable->setAlternatingRowColors(true);
  theLocalityTable->setToolTip("Localities defined so far (name, coordinates summary, component groups present).");

  layoutLocalities->addWidget(localitiesIntro, 0, 0, 1, 4);
  layoutLocalities->addWidget(addLocalityButton, 1,0);
  layoutLocalities->addWidget(theLocalityTable, 2, 0, 1, 4);
  
  //
  // resources tab
  //

  QWidget *resourcesWidget = new QWidget();
  QGridLayout *layoutResources = new QGridLayout(resourcesWidget);
  
  QLabel *resourcesIntro = new QLabel(
      "Resources are what components exchange (e.g. ElectricPower, PotableWater, RepairCrew). "
      "For each, choose a Group and a Distribution Model that decides how the resource is shared "
      "across the region after the disaster. Names must match those used in the Component Library.");
  resourcesIntro->setWordWrap(true);
  resourcesIntro->setStyleSheet("color: #555; font-style: italic;");

  QPushButton *addResourceButton = new QPushButton("Add Resource");
  addResourceButton->setToolTip("Add a resource and choose how it is distributed.");

  QStringList headingsResources;
  headingsResources << "Name" << "Group" << "Distribution Model" << "Distribution Model Parameters (JSON)";
  theResourcesTable = new QTableWidget();
  theResourcesTable->setColumnCount(4);
  theResourcesTable->setHorizontalHeaderLabels(headingsResources);
  setupColumns(theResourcesTable);
  theResourcesTable->verticalHeader()->setVisible(false);
  theResourcesTable->setShowGrid(true);
  theResourcesTable->setAlternatingRowColors(true);
  setHeaderTipSC(theResourcesTable, 0, "Resource name, e.g. ElectricPower. Must match Supply/Demand in the Component Library.");
  setHeaderTipSC(theResourcesTable, 1, "Resource group: Utilities (most), TransferService (transport links),\n"
		"RecoveryResources (crews/money), BridgeService.");
  setHeaderTipSC(theResourcesTable, 2, "How the resource is allocated when scarce:\n"
		"  UtilityDistributionModel - network utilities (power, water, comms)\n"
		"  HousingDistributionModel - shelter / functional housing\n"
		"  TransferServiceDistributionModelPotentialPaths - transport over a path network.");
  setHeaderTipSC(theResourcesTable, 3, "Model-specific settings as JSON, e.g. a DistributionPriority block.\n"
		"Use {} if none. See an example via \"Load Template\".");

  connect(addResourceButton, &QPushButton::clicked, this, [=]() {
    int row = theResourcesTable->rowCount();
    theResourcesTable->insertRow(row);
    theResourcesTable->setItem(row, 0, new QTableWidgetItem(""));
    theResourcesTable->setCellWidget(row, 1, makeCombo(PyrecodesOptions::resourceGroups()));
    theResourcesTable->setCellWidget(row, 2, makeCombo(PyrecodesOptions::distributionModelClasses()));
    theResourcesTable->setItem(row, 3, new QTableWidgetItem("{}"));
  });

  layoutResources->addWidget(resourcesIntro, 0, 0, 1, 4);
  layoutResources->addWidget(addResourceButton, 1, 0);
  layoutResources->addWidget(theResourcesTable, 2, 0, 1, 4);

  //
  // damage input tab
  //

  QWidget *damageWidget = new QWidget();
  QGridLayout *layoutDamage = new QGridLayout(damageWidget);

  QLabel *damageIntro = new QLabel(
      "Defines the initial damage applied to components at the disaster time step.\n"
      "  R2DDamageInput  - reads an R2D damage results file (use this when running from R2DTool)\n"
      "  ListDamageInput - damage given inline as a list of values, one per damaged component\n"
      "  FileDamageInput - reads damage from a separate file.");
  damageIntro->setWordWrap(true);
  damageIntro->setStyleSheet("color: #555; font-style: italic;");

  theDamageInputClass = new SC_ComboBox("damageInputClass", PyrecodesOptions::damageInputClasses());
  theDamageInputClass->setToolTip("Source of the initial damage. The FileName field is filled in automatically.");
  theDamageInputParams = new QPlainTextEdit();
  theDamageInputParams->setToolTip(
      "Parameters for the chosen class, as JSON.\n"
      "  R2DDamageInput : {\"DamageFile\": \"{folder}/Damage.json\", \"DistributionModelDamage\": [\"PotableWater\"]}\n"
      "  ListDamageInput: [0.4, 0.0, 0.4, ...]   (one damage value per component, in build order)");
  theDamageInputParams->setPlaceholderText(
      "Parameters as JSON, e.g.\n"
      "{\n  \"DamageFile\": \"{folder}/Damage.json\",\n  \"DistributionModelDamage\": [\"PotableWater\"]\n}");
  layoutDamage->addWidget(damageIntro, 0, 0, 1, 2);
  layoutDamage->addWidget(new QLabel("Damage Input Class:"), 1, 0);
  layoutDamage->addWidget(theDamageInputClass, 1, 1);
  layoutDamage->addWidget(new QLabel("Parameters (JSON):"), 2, 0, Qt::AlignTop);
  layoutDamage->addWidget(theDamageInputParams, 2, 1);
  layoutDamage->setColumnStretch(1, 1);
  layoutDamage->setRowStretch(2, 1);

  //
  // resilience calculator tab
  //

  QWidget *resilienceWidget = new QWidget();
  QGridLayout *layoutResilience = new QGridLayout(resilienceWidget);

  QLabel *resilienceIntro = new QLabel(
      "Resilience calculators are the metrics computed during the simulation. You can add several. "
      "  ReCoDeSCalculator - lack-of-resilience (unmet demand) for the listed resources\n"
      "  NISTGoalsCalculator - time to reach target functionality levels\n"
      "  R2DComponentRecoveryTimeCalculator - per-component recovery times (for R2DTool).");
  resilienceIntro->setWordWrap(true);
  resilienceIntro->setStyleSheet("color: #555; font-style: italic;");

  QPushButton *addResilienceButton = new QPushButton("Add Resilience Calculator");
  addResilienceButton->setToolTip("Add a resilience metric to compute during the simulation.");

  QStringList headingsResilience;
  headingsResilience << "Calculator Class" << "Parameters (JSON)";
  theResilienceTable = new QTableWidget();
  theResilienceTable->setColumnCount(2);
  theResilienceTable->setHorizontalHeaderLabels(headingsResilience);
  setupColumns(theResilienceTable);
  theResilienceTable->verticalHeader()->setVisible(false);
  theResilienceTable->setShowGrid(true);
  theResilienceTable->setAlternatingRowColors(true);
  setHeaderTipSC(theResilienceTable, 0, "Metric to compute. FileName is filled in automatically.");
  setHeaderTipSC(theResilienceTable, 1, "Parameters as JSON, e.g.\n"
		"  ReCoDeSCalculator : {\"Scope\": \"All\", \"Resources\": [\"ElectricPower\"]}\n"
		"  NISTGoalsCalculator: [{\"Resource\": \"ElectricPower\", \"DesiredFunctionalityLevel\": 0.95, \"Scope\": \"All\"}]");

  connect(addResilienceButton, &QPushButton::clicked, this, [=]() {
    int row = theResilienceTable->rowCount();
    theResilienceTable->insertRow(row);
    theResilienceTable->setCellWidget(row, 0, makeCombo(PyrecodesOptions::resilienceCalculatorClasses()));
    theResilienceTable->setItem(row, 1, new QTableWidgetItem("{\"Scope\": \"All\", \"Resources\": []}"));
  });

  layoutResilience->addWidget(resilienceIntro, 0, 0, 1, 4);
  layoutResilience->addWidget(addResilienceButton, 1, 0);
  layoutResilience->addWidget(theResilienceTable, 2, 0, 1, 4);

  //
  // add widgets to theTabbedWidget
  //

  theTabbedWidget->addTab(constantsWidget,"Constants");
  theTabbedWidget->addTab(localitiesWidget,"Localities");
  theTabbedWidget->addTab(resourcesWidget,"Resources");
  theTabbedWidget->addTab(damageWidget,"Damage Input");
  theTabbedWidget->addTab(resilienceWidget,"Resilience Calculators");

  //
  // finally add tabbedWidget to a layout for this
  //
  
  QGridLayout *layout = new QGridLayout(this);

  QLabel *intro = new QLabel(
      "The System Configuration describes the region: its localities, the resources exchanged, "
      "the initial damage, and the resilience metrics to compute. Fill in each tab below "
      "(start with \"Load Template\"), then \"Save\". Hover any field or column header for help.");
  intro->setWordWrap(true);
  intro->setStyleSheet("color: #555; font-style: italic;");
  layout->addWidget(intro, 0, 0, 1, 5);

  layout->addWidget(new QLabel("System Config File:"),1,0);
  layout->addWidget(theSystemFile,1,1);
  layout->addWidget(loadFileButton,1,2);
  layout->addWidget(loadTemplateButton,1,3);
  layout->addWidget(saveFileButton,1,4);
  layout->setColumnStretch(1,1);

  QGroupBox *theGroupBox = new QGroupBox("System Configuration");
  QGridLayout *groupLayout = new QGridLayout(theGroupBox);
  groupLayout->addWidget(theTabbedWidget,0,0);
  layout->addWidget(theGroupBox,2,0,1,5);
}


PyrecodesSystemConfig::~PyrecodesSystemConfig()
{

}


QString
PyrecodesSystemConfig::getFileName() {
  return theSystemFile->text();
}

void
PyrecodesSystemConfig::addLocality() {
  QString initName;
  PyrecodesLocality *theLocality = new PyrecodesLocality(this, initName);
  theLocalities.append(theLocality);
  theLocality->show();
}


void PyrecodesSystemConfig::clear(void)
{

}

bool
PyrecodesSystemConfig::outputToJSON(QJsonObject &jsonObject)
{
  
  QJsonObject constantsObject;
  qDebug() << "Constants";
  
  for (int row = 0; row < theConstantsTable->rowCount(); row++) {
    QJsonObject obj;
    QTableWidgetItem *keyItem = theConstantsTable->item(row, 0);
    QTableWidgetItem *valueItem = theConstantsTable->item(row, 1);
    
    if (keyItem && valueItem) {
      QString key = keyItem->text();
      QString value = valueItem->text();
      
      // Attempt to parse the second column as JSON
      QJsonParseError parseError;
      QJsonDocument jsonDoc = QJsonDocument::fromJson(value.toUtf8(), &parseError);

      if (parseError.error == QJsonParseError::NoError) {
	// Check if it is a JSON array or object
	if (jsonDoc.isArray()) {
	  constantsObject[key] = jsonDoc.array();
	} else if (jsonDoc.isObject()) {
	  constantsObject[key] = jsonDoc.object();
	}
      } else {
	// Fallback: Try to parse as a number
	bool isNumber = false;
	double numberValue = value.toDouble(&isNumber);
	
	if (isNumber) {
	  constantsObject[key] = numberValue; // Store as a number
	} else {
	  constantsObject[key] = value;      // Store as a string
	}
      }
    }
  }

  jsonObject["Constants"]=constantsObject;

  //
  // read resourcses data & fill the table
  //

  QJsonObject resourcesObject;

  for (int row = 0; row < theResourcesTable->rowCount(); row++) {

    QJsonObject obj;
    QTableWidgetItem *keyItem = theResourcesTable->item(row, 0);
    if (keyItem == nullptr || keyItem->text().isEmpty())
      continue;

    QString key = keyItem->text();
    QString group = comboTextAt(theResourcesTable, row, 1);
    QString distributionClass = comboTextAt(theResourcesTable, row, 2);
    QTableWidgetItem *paramsItem = theResourcesTable->item(row, 3);

    QJsonObject distrObj;
    distrObj["ClassName"] = distributionClass;
    QString distrFile = PyrecodesOptions::fileNameForClass(distributionClass);
    if (!distrFile.isEmpty())
      distrObj["FileName"] = distrFile;

    // optional Parameters JSON object
    if (paramsItem && !paramsItem->text().trimmed().isEmpty()) {
      QJsonParseError pe;
      QJsonDocument pd = QJsonDocument::fromJson(paramsItem->text().toUtf8(), &pe);
      if (pe.error == QJsonParseError::NoError && pd.isObject())
	distrObj["Parameters"] = pd.object();
    }

    obj["DistributionModel"] = distrObj;
    obj["Group"] = group;

    resourcesObject[key] = obj;
  }
  jsonObject["Resources"]=resourcesObject;

  //
  // DamageInput
  //

  QString damageClass = theDamageInputClass->currentText();
  if (!damageClass.isEmpty()) {
    QJsonObject damageObject;
    damageObject["ClassName"] = damageClass;
    QString damageFile = PyrecodesOptions::fileNameForClass(damageClass);
    if (!damageFile.isEmpty())
      damageObject["FileName"] = damageFile;

    QString paramsText = theDamageInputParams->toPlainText().trimmed();
    if (!paramsText.isEmpty()) {
      QJsonParseError pe;
      QJsonDocument pd = QJsonDocument::fromJson(paramsText.toUtf8(), &pe);
      if (pe.error == QJsonParseError::NoError && pd.isObject())
	damageObject["Parameters"] = pd.object();
      else if (pe.error == QJsonParseError::NoError && pd.isArray())
	damageObject["Parameters"] = pd.array();
      else
	// not a JSON object/array: keep as a plain string, e.g. a file path
	// like "{folder}/example_2_damage_input.txt" (used by FileDamageInput)
	damageObject["Parameters"] = paramsText;
    }
    jsonObject["DamageInput"] = damageObject;
  }

  //
  // ResilienceCalculator (array)
  //

  QJsonArray resilienceArray;
  for (int row = 0; row < theResilienceTable->rowCount(); row++) {
    QString calcClass = comboTextAt(theResilienceTable, row, 0);
    if (calcClass.isEmpty())
      continue;
    QJsonObject calcObj;
    calcObj["ClassName"] = calcClass;
    QString calcFile = PyrecodesOptions::fileNameForClass(calcClass);
    if (!calcFile.isEmpty())
      calcObj["FileName"] = calcFile;

    QTableWidgetItem *paramsItem = theResilienceTable->item(row, 1);
    if (paramsItem && !paramsItem->text().trimmed().isEmpty()) {
      QJsonParseError pe;
      QJsonDocument pd = QJsonDocument::fromJson(paramsItem->text().toUtf8(), &pe);
      if (pe.error == QJsonParseError::NoError) {
	if (pd.isObject())
	  calcObj["Parameters"] = pd.object();
	else if (pd.isArray())
	  calcObj["Parameters"] = pd.array();
      }
    }
    resilienceArray.append(calcObj);
  }
  if (!resilienceArray.isEmpty())
    jsonObject["ResilienceCalculator"] = resilienceArray;


  QJsonObject contentObject;

  for (int i = 0; i < theLocalities.size(); ++i) {
    PyrecodesLocality *theLocality = theLocalities[i];
    theLocality->outputToJSON(contentObject);
  }
  jsonObject["Content"] = contentObject;
  
  return true;
}

bool
PyrecodesSystemConfig::inputFromJSON(QJsonObject &jsonObject)
{

  //
  // read constants data & fill the table
  //
  
  QJsonObject constantsObject = jsonObject["Constants"].toObject();  
  int numRow = 0;
  theConstantsTable->clearContents();    
  theConstantsTable->setRowCount(constantsObject.size());
  
  for (const QString &key : constantsObject.keys()) {

    QJsonValue value = constantsObject.value(key);

    // set key
    QTableWidgetItem *keyItem = new QTableWidgetItem(key);
    theConstantsTable->setItem(numRow, 0, keyItem);
    
    QString valueStr;
    // now value
    if (value.isString()) {
      valueStr = value.toString();
    } else if (value.isDouble()) {
      valueStr = QString::number(value.toDouble());
    } else if (value.isBool()) {
      
      bool boolValue = value.toBool();
      if (boolValue == true)
	valueStr = "True";
      else
	valueStr = "False";
      
    } else if (value.isObject()) {
      valueStr = QString(QJsonDocument(value.toObject()).toJson(QJsonDocument::Compact));
    } else if (value.isArray()) {
      valueStr = QString(QJsonDocument(value.toArray()).toJson(QJsonDocument::Compact));	  
    } else if (value.isNull()) {
      
    }
    
    QTableWidgetItem *valueItem = new QTableWidgetItem(valueStr);
    theConstantsTable->setItem(numRow, 1, valueItem);
    
    numRow++;
  }


  //
  // read resourcses data & fill the table
  //

  numRow = 0;
  
  QJsonObject resourcesObject = jsonObject["Resources"].toObject();
  theResourcesTable->clearContents();
  theResourcesTable->setRowCount(0);

  for (const QString &key : resourcesObject.keys()) {

    QJsonValue value = resourcesObject.value(key);
    if (value.isObject()) {

      QString groupStr;
      QString distributionStr;
      QString paramsStr;

      QJsonObject valueObj = value.toObject();
      QJsonValue groupValue = valueObj["Group"];
      if (!groupValue.isNull() && groupValue.isString())
	groupStr = groupValue.toString();

      QJsonValue distributionValue = valueObj["DistributionModel"];
      if (!distributionValue.isNull() && distributionValue.isObject()) {
	QJsonObject distributionObject = distributionValue.toObject();
	if (!distributionObject["ClassName"].isNull())
	  distributionStr = distributionObject["ClassName"].toString();
	QJsonValue paramsValue = distributionObject["Parameters"];
	if (paramsValue.isObject())
	  paramsStr = QString(QJsonDocument(paramsValue.toObject()).toJson(QJsonDocument::Compact));
      }

      theResourcesTable->insertRow(numRow);
      theResourcesTable->setItem(numRow, 0, new QTableWidgetItem(key));
      theResourcesTable->setCellWidget(numRow, 1, makeCombo(PyrecodesOptions::resourceGroups()));
      theResourcesTable->setCellWidget(numRow, 2, makeCombo(PyrecodesOptions::distributionModelClasses()));
      setComboAt(theResourcesTable, numRow, 1, groupStr);
      setComboAt(theResourcesTable, numRow, 2, distributionStr);
      theResourcesTable->setItem(numRow, 3, new QTableWidgetItem(paramsStr));
      numRow++;

    } else {

      // error Message
      QString msg(QString("In Resources section, item with key") + key + QString(" is not valid JSON"));
      this->errorMessage(msg);
    }
  }

  //
  // read DamageInput
  //

  QJsonValue damageValue = jsonObject["DamageInput"];
  theDamageInputParams->clear();
  if (damageValue.isObject()) {
    QJsonObject damageObj = damageValue.toObject();
    QJsonValue classValue = damageObj["ClassName"];
    if (classValue.isString()) {
      int idx = theDamageInputClass->findText(classValue.toString());
      if (idx >= 0) theDamageInputClass->setCurrentIndex(idx);
    }
    QJsonValue paramsValue = damageObj["Parameters"];
    if (paramsValue.isObject())
      theDamageInputParams->setPlainText(QString(QJsonDocument(paramsValue.toObject()).toJson(QJsonDocument::Indented)));
    else if (paramsValue.isArray())
      theDamageInputParams->setPlainText(QString(QJsonDocument(paramsValue.toArray()).toJson(QJsonDocument::Indented)));
    else if (paramsValue.isString())
      theDamageInputParams->setPlainText(paramsValue.toString());
    else if (paramsValue.isDouble())
      theDamageInputParams->setPlainText(QString::number(paramsValue.toDouble()));
  }

  //
  // read ResilienceCalculator
  //

  theResilienceTable->clearContents();
  theResilienceTable->setRowCount(0);
  QJsonValue resilienceValue = jsonObject["ResilienceCalculator"];
  if (resilienceValue.isArray()) {
    QJsonArray arr = resilienceValue.toArray();
    int row = 0;
    for (const QJsonValue &cv : arr) {
      if (!cv.isObject()) continue;
      QJsonObject calcObj = cv.toObject();
      theResilienceTable->insertRow(row);
      theResilienceTable->setCellWidget(row, 0, makeCombo(PyrecodesOptions::resilienceCalculatorClasses()));
      setComboAt(theResilienceTable, row, 0, calcObj["ClassName"].toString());
      QJsonValue paramsValue = calcObj["Parameters"];
      QString paramsStr;
      if (paramsValue.isObject())
	paramsStr = QString(QJsonDocument(paramsValue.toObject()).toJson(QJsonDocument::Compact));
      else if (paramsValue.isArray())
	paramsStr = QString(QJsonDocument(paramsValue.toArray()).toJson(QJsonDocument::Compact));
      theResilienceTable->setItem(row, 1, new QTableWidgetItem(paramsStr));
      row++;
    }
  }



  //
  // read locality data & fill the table
  //

  numRow = 0;
  
  QJsonObject localityObject = jsonObject["Content"].toObject();
  theLocalityTable->clearContents();
  theLocalityTable->setRowCount(0);
  theLocalities.clear();
  
  for (const QString &key : localityObject.keys()) {

    QJsonValue value = localityObject.value(key);
    if (value.isObject()) {
      QJsonObject data = value.toObject();

      PyrecodesLocality *theLocality = new PyrecodesLocality(this, key);
      theLocality->inputFromJSON(data);
      theLocalities.append(theLocality);
      
    } else {
      
      // error Message
      QString msg(QString("In Resourses section, item with key") + key + QString(" is not valid JSON"));
      this->errorMessage(msg);
    }
  }  
  
  return true;
}

bool
PyrecodesSystemConfig::copyFiles(QString &dirName) {
  return true;
}

void
PyrecodesSystemConfig::addOrUpdateLocalityTableEntry(QString name, QString bbox, QStringList components)
{

  bool found = false;
  int row = 0;
  
  for (row = 0; row < theLocalityTable->rowCount(); ++row) {
    QTableWidgetItem *item = theLocalityTable->item(row, 0);
    if (item && item->text() == name) {
      found = true;
      break;
    }
  }
  if (!found) {
    row = theLocalityTable->rowCount();
    theLocalityTable->insertRow(row);
  }
  //
  // now add the items
  //
  
  QTableWidgetItem *newName = new QTableWidgetItem(name);
  QTableWidgetItem *newCoords = new QTableWidgetItem(bbox);
  QString commaSeparatedComponentsString = components.join(", ");
  QTableWidgetItem *newComponents = new QTableWidgetItem(commaSeparatedComponentsString);    
  theLocalityTable->setItem(row, 0, newName);
  theLocalityTable->setItem(row, 1, newCoords);
  theLocalityTable->setItem(row, 2, newComponents);  

  qDebug() << name;
}
