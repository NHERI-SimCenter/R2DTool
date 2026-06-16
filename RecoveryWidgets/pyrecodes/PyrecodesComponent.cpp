/* *****************************************************************************
Copyright 2016-2023, The Regents of the University of California (Regents).
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


#include <PyrecodesComponent.h>
#include <PyrecodesOptions.h>
#include <SC_FileEdit.h>
#include <SC_StringLineEdit.h>
#include <SC_ComboBox.h>
#include <PyrecodesComponentLibrary.h>

#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <QLineEdit>
#include <QGridLayout>
#include <QLabel>
#include <QGroupBox>
#include <QDebug>
#include <QTableWidget>
#include <QHeaderView>
#include <QListWidget>
#include <QPushButton>
#include <QComboBox>
#include <QWidget>
#include <QJsonDocument>
#include <QJsonParseError>

// helper: make a QComboBox to drop into a table cell, optionally with a
// leading blank entry (used where a value is optional in the schema)
static QComboBox *makeRelationCombo(bool allowEmpty = false) {
  QComboBox *cb = new QComboBox();
  if (allowEmpty)
    cb->addItem("");
  cb->addItems(PyrecodesOptions::relationTypes());
  cb->setToolTip("Relation shape: Linear (proportional), Constant (always full),\n"
		 "Binary (all-or-nothing), Reverse* variants. Leave blank if optional.");
  return cb;
}

// helper: read the text of a QComboBox sitting in a table cell (empty if none)
static QString comboTextAt(QTableWidget *table, int row, int col) {
  QComboBox *cb = qobject_cast<QComboBox *>(table->cellWidget(row, col));
  return cb ? cb->currentText() : QString();
}

static void setComboAt(QTableWidget *table, int row, int col, const QString &value) {
  QComboBox *cb = qobject_cast<QComboBox *>(table->cellWidget(row, col));
  if (cb) {
    int idx = cb->findText(value);
    cb->setCurrentIndex(idx >= 0 ? idx : 0);
  }
}

// helper: attach a tooltip to a table's column header
static void setHeaderTip(QTableWidget *table, int col, const QString &tip) {
  if (QTableWidgetItem *h = table->horizontalHeaderItem(col))
    h->setToolTip(tip);
}

// helper: make a table's columns user-resizable (drag the header borders) and
// sized to fit their header labels initially.
static void setupColumns(QTableWidget *t) {
  QHeaderView *h = t->horizontalHeader();
  h->setSectionResizeMode(QHeaderView::Interactive);
  h->setMinimumSectionSize(40);
  t->resizeColumnsToContents();
}

// helper: merge the keys of a JSON-object held in a table cell into obj. Lets
// the UI round-trip any schema fields it has no dedicated column for (e.g.
// PostDisasterIncreaseDueToEmergencyCalls on a demand entry).
static void mergeExtraJson(QTableWidget *t, int row, int col, QJsonObject &obj) {
  QTableWidgetItem *it = t->item(row, col);
  if (!it) return;
  QString s = it->text().trimmed();
  if (s.isEmpty()) return;
  QJsonParseError pe;
  QJsonDocument d = QJsonDocument::fromJson(s.toUtf8(), &pe);
  if (pe.error == QJsonParseError::NoError && d.isObject()) {
    QJsonObject extra = d.object();
    for (const QString &k : extra.keys())
      obj[k] = extra.value(k);
  }
}

// helper: comma-joined non-empty names in column `col` of a table, for the
// summary shown in the Component Library table's Supply / Demand columns.
static QString joinedColumnNames(QTableWidget *t, int col) {
  QStringList names;
  for (int r = 0; r < t->rowCount(); ++r) {
    QTableWidgetItem *it = t->item(r, col);
    if (it && !it->text().trimmed().isEmpty())
      names << it->text().trimmed();
  }
  return names.join(", ");
}

// helper: collect keys of obj not in `known` into a compact JSON-object string
// (empty if there are none), for display in an "Other (JSON)" cell.
static QString extraJsonString(const QJsonObject &obj, const QStringList &known) {
  QJsonObject extra;
  for (const QString &k : obj.keys())
    if (!known.contains(k))
      extra[k] = obj.value(k);
  if (extra.isEmpty())
    return QString();
  return QString(QJsonDocument(extra).toJson(QJsonDocument::Compact));
}

PyrecodesComponent::PyrecodesComponent(QString initName, PyrecodesComponentLibrary *theCL, QWidget *parent)
  :SimCenterWidget(parent), theComponentLibrary(theCL)
{
  theName = new QLineEdit("name");
  theName->setText(initName);
  theName->setToolTip("Unique component name. This is the name you reference from the\n"
		      "System Configuration (Localities and distribution priorities),\n"
		      "e.g. ElectricPowerPlant, BuildingStockUnit.");

  // Component class - full pyrecodes 0.3.0 list
  theClass = new SC_ComboBox("componentClass", PyrecodesOptions::componentClasses());
  theClass->setToolTip("pyrecodes component class.\n"
		       "  StandardiReCoDeSComponent - generic supplier/consumer component\n"
		       "  InfrastructureInterface   - boundary between subsystems\n"
		       "  R2DBuilding/R2DBridge/...  - use these when running from R2DTool\n"
		       "                               (built from the R2D exposure file).");

  //
  // Recovery Relation & Table
  //

  theRecoveryType = new SC_ComboBox("recoveryType", PyrecodesOptions::recoveryModelClasses());
  theRecoveryType->setToolTip("How the component recovers after damage.\n"
			      "  NoRecoveryActivityModel                - never changes (no repair)\n"
			      "  ComponentLevelRecoveryActivitiesModel  - repaired via the recovery activities below\n"
			      "  InfrastructureInterfaceRecoveryModel   - recovery follows the connected subsystem.");

  // Damage->functionality relation type (real relation classes)
  theDamageFunctionalRelation = new SC_ComboBox("damageRelation", PyrecodesOptions::relationTypes());
  theDamageFunctionalRelation->setToolTip("Maps damage/repair progress to functionality (0..1).\n"
			      "  ReverseBinary - functional only once fully repaired (0 until done, then 1)\n"
			      "  Linear        - functionality recovers proportionally to repair progress\n"
			      "  Constant      - functionality is independent of damage.");

  QStringList headingsRecovery;
  headingsRecovery << "Recovery Activity" << "Duration" << "Demand" << "Preceding Activities";
  theRecoveryTable = new QTableWidget();
  theRecoveryTable->setColumnCount(4);
  theRecoveryTable->setHorizontalHeaderLabels(headingsRecovery);
  setupColumns(theRecoveryTable);
  theRecoveryTable->verticalHeader()->setVisible(false);
  theRecoveryTable->setShowGrid(true);
  theRecoveryTable->setAlternatingRowColors(true);
  setHeaderTip(theRecoveryTable, 0, "Name of the repair step, e.g. RapidInspection, Repair, CleanUp.");
  setHeaderTip(theRecoveryTable, 1, "Duration as a probability distribution (JSON), e.g.\n"
		"  {\"Deterministic\": {\"Value\": 10}}  or\n"
		"  {\"Lognormal\": {\"Median\": 30, \"Dispersion\": 0.4}}   (units = time steps)");
  setHeaderTip(theRecoveryTable, 2, "Resources this step consumes (JSON array), e.g.\n"
		"  [{\"Resource\": \"RepairCrew\", \"Amount\": 1}]   Use [] if none.");
  setHeaderTip(theRecoveryTable, 3, "Steps that must finish first (JSON array of activity names),\n"
		"  e.g. [\"RapidInspection\"].  Use [] for none.");

  QPushButton *addRecoveryButton = new QPushButton("Add Recovery Activity");
  addRecoveryButton->setToolTip("Add a repair step. A row is pre-filled with a valid example you can edit.");
  connect(addRecoveryButton, &QPushButton::clicked, this, [=]() {
    int row = theRecoveryTable->rowCount();
    theRecoveryTable->insertRow(row);
    // pre-fill with valid 0.3.0 templates so the user sees the expected shape
    theRecoveryTable->setItem(row, 0, new QTableWidgetItem("Repair"));
    theRecoveryTable->setItem(row, 1, new QTableWidgetItem("{\"Lognormal\": {\"Median\": 10, \"Dispersion\": 0.3}}"));
    theRecoveryTable->setItem(row, 2, new QTableWidgetItem("[{\"Resource\": \"RepairCrew\", \"Amount\": 1}]"));
    theRecoveryTable->setItem(row, 3, new QTableWidgetItem("[]"));
  });

  //
  // Supply Table
  //

  QStringList headingsSupply;
  headingsSupply << "Resource Name" << "Amount" << "Functionality To Amount Relation"
		 << "Unmet Demand To Amount Relation" << "Other (JSON)";
  theSupplyTable = new QTableWidget();
  theSupplyTable->setColumnCount(5);
  theSupplyTable->setHorizontalHeaderLabels(headingsSupply);
  setupColumns(theSupplyTable);
  theSupplyTable->verticalHeader()->setVisible(false);
  theSupplyTable->setShowGrid(true);
  theSupplyTable->setAlternatingRowColors(true);
  setHeaderTip(theSupplyTable, 0, "Resource this component provides, e.g. ElectricPower, Communication.\n"
		"Must match a resource name defined in the System Configuration.");
  setHeaderTip(theSupplyTable, 1, "Maximum amount supplied when fully functional (a number).");
  setHeaderTip(theSupplyTable, 2, "How supply scales with the component's functionality:\n"
		"  Linear = proportional, Constant = always full, Binary = all-or-nothing.");
  setHeaderTip(theSupplyTable, 3, "Optional: how supply scales when the component's own demand is unmet.\n"
		"Leave blank if not applicable.");
  setHeaderTip(theSupplyTable, 4, "Optional: any extra keys for this entry as a JSON object, e.g.\n"
		"  {\"PostDisasterIncreaseDueToEmergencyCalls\": \"True\"}. Use {} or leave blank if none.");

  QPushButton *addSupplyButton = new QPushButton("Add Supply");
  addSupplyButton->setToolTip("Add a resource this component supplies to the community.");
  connect(addSupplyButton, &QPushButton::clicked, this, [=]() {
    int row = theSupplyTable->rowCount();
    theSupplyTable->insertRow(row);
    theSupplyTable->setItem(row, 0, new QTableWidgetItem(""));
    theSupplyTable->setItem(row, 1, new QTableWidgetItem("1"));
    theSupplyTable->setCellWidget(row, 2, makeRelationCombo(false));        // FunctionalityToAmountRelation
    theSupplyTable->setCellWidget(row, 3, makeRelationCombo(true));         // UnmetDemandToAmountRelation (optional)
    theSupplyTable->setItem(row, 4, new QTableWidgetItem(""));              // Other keys (JSON)
  });

  //
  // OpDemandTable
  //

  QStringList headingsOpDemand;
  headingsOpDemand << "Resource Name" << "Amount" << "Functionality To Amount Relation" << "Other (JSON)";
  theOpDemandTable = new QTableWidget();
  theOpDemandTable->setColumnCount(4);
  theOpDemandTable->setHorizontalHeaderLabels(headingsOpDemand);
  setupColumns(theOpDemandTable);
  theOpDemandTable->verticalHeader()->setVisible(false);
  theOpDemandTable->setShowGrid(true);
  theOpDemandTable->setAlternatingRowColors(true);
  setHeaderTip(theOpDemandTable, 0, "Resource this component needs to operate, e.g. ElectricPower.\n"
		"Must match a resource name defined in the System Configuration.");
  setHeaderTip(theOpDemandTable, 1, "Amount required for full operation (a number).");
  setHeaderTip(theOpDemandTable, 2, "How the demand scales with functionality (usually Linear).");
  setHeaderTip(theOpDemandTable, 3, "Optional: any extra keys for this entry as a JSON object, e.g.\n"
		"  {\"PostDisasterIncreaseDueToEmergencyCalls\": \"True\"}. Use {} or leave blank if none.");

  QPushButton *addOpDemandButton = new QPushButton("Add Operation Demand");
  addOpDemandButton->setToolTip("Add a resource this component consumes to stay operational.");
  connect(addOpDemandButton, &QPushButton::clicked, this, [=]() {
    int row = theOpDemandTable->rowCount();
    theOpDemandTable->insertRow(row);
    theOpDemandTable->setItem(row, 0, new QTableWidgetItem(""));
    theOpDemandTable->setItem(row, 1, new QTableWidgetItem("1"));
    theOpDemandTable->setCellWidget(row, 2, makeRelationCombo(false));
    theOpDemandTable->setItem(row, 3, new QTableWidgetItem(""));
  });


  QPushButton *doneButton = new QPushButton("Done");
  connect(doneButton, &QPushButton::clicked, this, [=]() {
    theComponentLibrary->addOrUpdateComponentTableEntry(theName->text(),
							theClass->currentText(),
							joinedColumnNames(theSupplyTable, 0),
							joinedColumnNames(theOpDemandTable, 0),
							theRecoveryType->currentText());
    // close the widget
    this->close();
  });


  //
  // main layout
  //

  int numRow = 0;

  QGridLayout *layout = new QGridLayout();

  QLabel *intro = new QLabel(
      "Define one component (asset type). Set its class, what it supplies and needs, "
      "and how it recovers. Hover any field or column header for help. Click Done when finished.");
  intro->setWordWrap(true);
  intro->setStyleSheet("color: #555; font-style: italic;");
  layout->addWidget(intro, numRow++, 0, 1, 2);

  layout->addWidget(new QLabel("Name:"), numRow, 0);
  layout->addWidget(theName, numRow++, 1);

  layout->addWidget(new QLabel("Component Class:"), numRow, 0);
  layout->addWidget(theClass, numRow++, 1);

  QGroupBox *groupRecovery = new QGroupBox("Recovery Model");
  QGridLayout *groupRecoveryLayout = new QGridLayout(groupRecovery);
  groupRecoveryLayout->addWidget(new QLabel("Recovery Type:"), 0, 0);
  groupRecoveryLayout->addWidget(theRecoveryType, 0, 1);
  groupRecoveryLayout->addWidget(new QLabel("Damage Functionality Relation:"), 1, 0);
  groupRecoveryLayout->addWidget(theDamageFunctionalRelation, 1, 1);
  groupRecoveryLayout->addWidget(addRecoveryButton, 2, 0);
  groupRecoveryLayout->addWidget(theRecoveryTable, 3, 0, 1, 4);

  QGroupBox *groupSupply = new QGroupBox("Supply");
  QGridLayout *groupSupplyLayout = new QGridLayout(groupSupply);
  groupSupplyLayout->addWidget(addSupplyButton, 0, 0);
  groupSupplyLayout->addWidget(theSupplyTable, 1, 0, 1, 4);

  QGroupBox *groupOpDemand = new QGroupBox("Operation Demand");
  QGridLayout *groupOpDemandLayout = new QGridLayout(groupOpDemand);
  groupOpDemandLayout->addWidget(addOpDemandButton, 0, 0);
  groupOpDemandLayout->addWidget(theOpDemandTable, 1, 0, 1, 4);

  layout->addWidget(groupSupply, numRow++, 0, 1, 4);
  layout->addWidget(groupOpDemand, numRow++, 0, 1, 4);
  layout->addWidget(groupRecovery, numRow++, 0, 1, 4);

  layout->addWidget(doneButton, numRow++, 1, 1, 2);
  layout->setRowStretch(numRow, 1);

  this->setLayout(layout);
}


PyrecodesComponent::~PyrecodesComponent()
{

}

void PyrecodesComponent::clear(void)
{

}

bool
PyrecodesComponent::outputToJSON(QJsonObject &jsonObject)
{

  QJsonObject componentObject;

  // ComponentClass: ClassName + FileName (pyrecodes 0.3.0)
  QJsonObject componentClass;
  QString className = theClass->currentText();
  componentClass["ClassName"] = className;
  QString classFile = PyrecodesOptions::fileNameForClass(className);
  if (!classFile.isEmpty())
    componentClass["FileName"] = classFile;
  componentObject["ComponentClass"] = componentClass;

  //
  // Fill in supplyObject
  //

  QJsonObject supplyObject;
  int numRows = theSupplyTable->rowCount();

  for (int i = 0; i < numRows; i++) {

    QJsonObject obj;
    QTableWidgetItem *nameItem = theSupplyTable->item(i, 0);
    QTableWidgetItem *amountItem = theSupplyTable->item(i, 1);
    if (nameItem == nullptr || nameItem->text().isEmpty())
      continue;

    QString name = nameItem->text();
    bool ok;
    obj["Amount"] = amountItem ? amountItem->text().toDouble(&ok) : 0.0;
    obj["FunctionalityToAmountRelation"] = comboTextAt(theSupplyTable, i, 2);
    QString udtar = comboTextAt(theSupplyTable, i, 3);
    if (!udtar.isEmpty())
      obj["UnmetDemandToAmountRelation"] = udtar;
    mergeExtraJson(theSupplyTable, i, 4, obj);   // any extra schema keys
    supplyObject[name] = obj;
  }

  componentObject["Supply"] = supplyObject;

  //
  // parse op demand
  //

  QJsonObject opDemandObject;

  numRows = theOpDemandTable->rowCount();
  for (int i = 0; i < numRows; i++) {

    QJsonObject obj;
    QTableWidgetItem *nameItem = theOpDemandTable->item(i, 0);
    QTableWidgetItem *amountItem = theOpDemandTable->item(i, 1);
    if (nameItem == nullptr || nameItem->text().isEmpty())
      continue;

    QString name = nameItem->text();
    bool ok;
    obj["Amount"] = amountItem ? amountItem->text().toDouble(&ok) : 0.0;
    obj["FunctionalityToAmountRelation"] = comboTextAt(theOpDemandTable, i, 2);
    mergeExtraJson(theOpDemandTable, i, 3, obj);   // any extra schema keys
    opDemandObject[name] = obj;
  }

  if (!opDemandObject.isEmpty())
    componentObject["OperationDemand"] = opDemandObject;

  //
  // parse recovery model
  //

  QJsonObject recObject;

  QString recoveryClass = theRecoveryType->currentText();
  recObject["ClassName"] = recoveryClass;
  QString recoveryFile = PyrecodesOptions::fileNameForClass(recoveryClass);
  if (!recoveryFile.isEmpty())
    recObject["FileName"] = recoveryFile;

  QJsonObject damageFunctionalityRelation;
  damageFunctionalityRelation["Type"] = theDamageFunctionalRelation->currentText();
  recObject["DamageFunctionalityRelation"] = damageFunctionalityRelation;

  // Parameters: one entry per recovery activity
  //   <ActivityName>: { Duration:{<dist>:{...}}, Demand:[...], PrecedingActivities:[...] }
  QJsonObject parametersObject;

  numRows = theRecoveryTable->rowCount();
  for (int i = 0; i < numRows; i++) {
    QTableWidgetItem *nameItem = theRecoveryTable->item(i, 0);
    QTableWidgetItem *durationItem = theRecoveryTable->item(i, 1);
    QTableWidgetItem *demandItem = theRecoveryTable->item(i, 2);
    QTableWidgetItem *precedingItem = theRecoveryTable->item(i, 3);

    if (nameItem == nullptr || nameItem->text().isEmpty())
      continue;
    QString name = nameItem->text();

    QJsonObject obj;
    QJsonParseError parseError;

    QJsonObject durationObject;
    if (durationItem) {
      QJsonDocument jsonDocDuration = QJsonDocument::fromJson(durationItem->text().toUtf8(), &parseError);
      if (jsonDocDuration.isObject())
	durationObject = jsonDocDuration.object();
    }

    QJsonArray demandArray;
    if (demandItem) {
      QJsonDocument jsonDocDemand = QJsonDocument::fromJson(demandItem->text().toUtf8(), &parseError);
      if (jsonDocDemand.isArray())
	demandArray = jsonDocDemand.array();
    }

    QJsonArray precedingArray;
    if (precedingItem) {
      QJsonDocument jsonDocPreceding = QJsonDocument::fromJson(precedingItem->text().toUtf8(), &parseError);
      if (jsonDocPreceding.isArray())
	precedingArray = jsonDocPreceding.array();
    }

    obj["Duration"] = durationObject;
    obj["Demand"] = demandArray;
    obj["PrecedingActivities"] = precedingArray;

    parametersObject[name] = obj;
  }

  recObject["Parameters"] = parametersObject;

  componentObject["RecoveryModel"] = recObject;

  QString name = theName->text();
  jsonObject[name] = componentObject;

  return true;
}

bool
PyrecodesComponent::inputFromJSON(QJsonObject &jsonObject)
{

  //
  // parse component class
  //

  QJsonValue componentValue = jsonObject["ComponentClass"];
  if (!componentValue.isNull() && componentValue.isObject()) {

    QJsonObject componentObject = componentValue.toObject();
    QJsonValue compObjectClassValue = componentObject["ClassName"];
    if (!compObjectClassValue.isNull() && compObjectClassValue.isString()) {
      QString classString = compObjectClassValue.toString();
      int index = theClass->findText(classString);
      if (index != -1) {
	theClass->setCurrentIndex(index);
      }
    }
  }

  //
  // parse supply
  //

  theSupplyTable->setRowCount(0);
  QJsonValue supplyValue = jsonObject["Supply"];

  if (!supplyValue.isNull() && supplyValue.isObject()) {

    QJsonObject supplyObject = supplyValue.toObject();

    int row = 0;
    for (const QString &key : supplyObject.keys()) {

      QJsonValue value = supplyObject.value(key);
      if (!value.isNull() && value.isObject()) {
	QJsonObject obj = value.toObject();

	QString amount, ftar, udtar;
	QJsonValue amountValue = obj["Amount"];
	QJsonValue ftarValue = obj["FunctionalityToAmountRelation"];
	QJsonValue udtarValue = obj["UnmetDemandToAmountRelation"];

	if (!amountValue.isNull() && amountValue.isDouble())
	  amount = QString::number(amountValue.toDouble());
	if (!ftarValue.isNull() && ftarValue.isString())
	  ftar = ftarValue.toString();
	if (!udtarValue.isNull() && udtarValue.isString())
	  udtar = udtarValue.toString();

	QString extra = extraJsonString(obj, QStringList()
					<< "Amount" << "FunctionalityToAmountRelation"
					<< "UnmetDemandToAmountRelation");

	theSupplyTable->insertRow(row);
	theSupplyTable->setItem(row, 0, new QTableWidgetItem(key));
	theSupplyTable->setItem(row, 1, new QTableWidgetItem(amount));
	theSupplyTable->setCellWidget(row, 2, makeRelationCombo(false));
	theSupplyTable->setCellWidget(row, 3, makeRelationCombo(true));
	setComboAt(theSupplyTable, row, 2, ftar);
	setComboAt(theSupplyTable, row, 3, udtar);
	theSupplyTable->setItem(row, 4, new QTableWidgetItem(extra));

	row++;
      }
    }
  }

  //
  // parse op demand
  //

  theOpDemandTable->setRowCount(0);
  QJsonValue odValue = jsonObject["OperationDemand"];

  if (!odValue.isNull() && odValue.isObject()) {

    QJsonObject odObject = odValue.toObject();

    int row = 0;
    for (const QString &key : odObject.keys()) {

      QJsonValue value = odObject.value(key);
      if (!value.isNull() && value.isObject()) {
	QJsonObject obj = value.toObject();

	QString amount, ftar;
	QJsonValue amountValue = obj["Amount"];
	QJsonValue ftarValue = obj["FunctionalityToAmountRelation"];

	if (!amountValue.isNull() && amountValue.isDouble())
	  amount = QString::number(amountValue.toDouble());
	if (!ftarValue.isNull() && ftarValue.isString())
	  ftar = ftarValue.toString();

	QString extra = extraJsonString(obj, QStringList()
					<< "Amount" << "FunctionalityToAmountRelation");

	theOpDemandTable->insertRow(row);
	theOpDemandTable->setItem(row, 0, new QTableWidgetItem(key));
	theOpDemandTable->setItem(row, 1, new QTableWidgetItem(amount));
	theOpDemandTable->setCellWidget(row, 2, makeRelationCombo(false));
	setComboAt(theOpDemandTable, row, 2, ftar);
	theOpDemandTable->setItem(row, 3, new QTableWidgetItem(extra));

	row++;
      }
    }
  }


  //
  // parse recovery model
  //

  theRecoveryTable->setRowCount(0);
  QJsonValue recoveryValue = jsonObject["RecoveryModel"];

  if (!recoveryValue.isNull() && recoveryValue.isObject()) {

    QJsonObject recoveryObject = recoveryValue.toObject();

    // type
    QJsonValue classValue = recoveryObject["ClassName"];
    if (!classValue.isNull() && classValue.isString()) {
      int index = theRecoveryType->findText(classValue.toString());
      if (index != -1)
	theRecoveryType->setCurrentIndex(index);
    }

    // damage functional relation
    QJsonValue dfrValue = recoveryObject["DamageFunctionalityRelation"];
    if (!dfrValue.isNull() && dfrValue.isObject()) {
      QJsonValue typeValue = dfrValue.toObject()["Type"];
      if (!typeValue.isNull() && typeValue.isString()) {
	int index = theDamageFunctionalRelation->findText(typeValue.toString());
	if (index != -1)
	  theDamageFunctionalRelation->setCurrentIndex(index);
      }
    }

    QJsonValue parametersValue = recoveryObject["Parameters"];
    QJsonObject parametersObject;
    if (!parametersValue.isNull() && parametersValue.isObject())
      parametersObject = parametersValue.toObject();

    //
    // fill in the recovery table
    //

    int row = 0;
    for (const QString &key : parametersObject.keys()) {

      QJsonValue value = parametersObject.value(key);
      if (!value.isNull() && value.isObject()) {
	QJsonObject obj = value.toObject();

	QString duration, demand, precedingActivities;
	QJsonValue durationValue = obj["Duration"];
	QJsonValue demandValue = obj["Demand"];
	QJsonValue precedingValue = obj["PrecedingActivities"];

	if (!durationValue.isNull() && durationValue.isObject())
	  duration = QString(QJsonDocument(durationValue.toObject()).toJson(QJsonDocument::Compact));
	if (!demandValue.isNull() && demandValue.isArray())
	  demand = QString(QJsonDocument(demandValue.toArray()).toJson(QJsonDocument::Compact));
	if (!precedingValue.isNull() && precedingValue.isArray())
	  precedingActivities = QString(QJsonDocument(precedingValue.toArray()).toJson(QJsonDocument::Compact));

	theRecoveryTable->insertRow(row);
	theRecoveryTable->setItem(row, 0, new QTableWidgetItem(key));
	theRecoveryTable->setItem(row, 1, new QTableWidgetItem(duration));
	theRecoveryTable->setItem(row, 2, new QTableWidgetItem(demand));
	theRecoveryTable->setItem(row, 3, new QTableWidgetItem(precedingActivities));

	row++;
      }
    }
  }


  theComponentLibrary->addOrUpdateComponentTableEntry(theName->text(),
						      theClass->currentText(),
						      joinedColumnNames(theSupplyTable, 0),
						      joinedColumnNames(theOpDemandTable, 0),
						      theRecoveryType->currentText());
  return true;
}

bool
PyrecodesComponent::copyFiles(QString &dirName) {
  Q_UNUSED(dirName);
  return true;
}
