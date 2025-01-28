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


#include <PyrecodesComponent.h>
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
#include <QWidget>
#include <QJsonDocument>
#include <QJsonParseError>

PyrecodesComponent::PyrecodesComponent(QString initName, PyrecodesComponentLibrary *theCL, QWidget *parent)
  :SimCenterWidget(parent), theComponentLibrary(theCL)
{
  theName = new QLineEdit("name");
  theName->setText(initName);

  QStringList classTypes;
  classTypes << "StandardiReCoDeSComponent"
	     << "BuildingWithEmergencyCalls"
	     << "InfrastructureInterface"
	     << "R2DBridge"
	     << "R2DTunnel"
	     << "R2DRoadway"
	     << "R2DPipe";
  
  theClass = new SC_ComboBox("componentClass", classTypes);

  //
  // Recovery Relation & Table
  //

  QStringList recoveryTypes;
  recoveryTypes << "NoRecoveryActivityModel"
		<< "ComponentLevelRecoveryActivitiesModel"
		<< "InfrastructureInterfaceRecoveryModel";
  
  theRecoveryType = new SC_ComboBox("recoveryType", recoveryTypes);
  
  QStringList functionTypes;
  functionTypes << ""
		<< "Constant"
		<< "ReverseBinary"
		<< "MultipleSteps";
  
  theDamageFunctionalRelation = new SC_ComboBox("damageRelation", functionTypes);

  
  QStringList headingsRecovery; headingsRecovery << "Recovery Activity" << "Pure Execution Time" << "Resources Needed" << "PreceedingActivities";
  theRecoveryTable = new QTableWidget();
  theRecoveryTable->setColumnCount(4);
  theRecoveryTable->setHorizontalHeaderLabels(headingsRecovery);
  theRecoveryTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  theRecoveryTable->verticalHeader()->setVisible(false);
  theRecoveryTable->setShowGrid(true);
  theRecoveryTable->setStyleSheet("QTableWidget::item { border: 1px solid black; }");

  QPushButton *addRecoveryButton = new QPushButton("Add Demand");
  connect(addRecoveryButton, &QPushButton::clicked, this, [=]() {
    // add a row
    int row = theRecoveryTable->rowCount();
    theRecoveryTable->insertRow(row);
    for (int col = 0; col < 2; ++col) {
      QTableWidgetItem *item = new QTableWidgetItem(""); // Empty cell
      theRecoveryTable->setItem(row, col, item);
    }
  });      

  //
  // Supply Table
  //
  
  QStringList headingsSupply; headingsSupply << "Resource Name" << "Resource Amount" << "Functionality to Amount Relation" << "Unmet Demand To Amount Relation";
  theSupplyTable = new QTableWidget();
  theSupplyTable->setColumnCount(4);
  theSupplyTable->setHorizontalHeaderLabels(headingsSupply);
  theSupplyTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  theSupplyTable->verticalHeader()->setVisible(false);
  theSupplyTable->setShowGrid(true);
  theSupplyTable->setStyleSheet("QTableWidget::item { border: 1px solid black; }");        

  QPushButton *addSupplyButton = new QPushButton("Add Demand");
  connect(addSupplyButton, &QPushButton::clicked, this, [=]() {

    int row = theSupplyTable->rowCount();
    theSupplyTable->insertRow(row);
    QTableWidgetItem *name = new QTableWidgetItem(""); // Empty cell
    theSupplyTable->setItem(row, 0, name);
    QTableWidgetItem *amount = new QTableWidgetItem(""); // Empty cell
    theSupplyTable->setItem(row, 1, amount);            
    QComboBox *functionalityAmount = new QComboBox();
    functionalityAmount->addItems({"FA 1", "FA 2", "FA 3"});
    theSupplyTable->setCellWidget(row, 2, functionalityAmount);
    QComboBox *unmetDemandAmount = new QComboBox();
    unmetDemandAmount->addItems({"UDA 1", "UDA 2", "UDA 3"});
    theSupplyTable->setCellWidget(row, 3, unmetDemandAmount);            
  });      

  //
  // OpDemandTable
  //

  QStringList headingsOpDemand; headingsOpDemand << "Resource Name" << "Resource Amount" << "Functionality to Amount Relation";
  theOpDemandTable = new QTableWidget();
  theOpDemandTable->setColumnCount(3);
  theOpDemandTable->setHorizontalHeaderLabels(headingsOpDemand);
  theOpDemandTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  theOpDemandTable->verticalHeader()->setVisible(false);
  theOpDemandTable->setShowGrid(true);
  theOpDemandTable->setStyleSheet("QTableWidget::item { border: 1px solid black; }");

  // button and connection to add an OpDemand
  QPushButton *addOpDemandButton = new QPushButton("Add Demand");
  connect(addOpDemandButton, &QPushButton::clicked, this, [=]() {
    int row = theOpDemandTable->rowCount();
    theOpDemandTable->insertRow(row);
    QTableWidgetItem *name = new QTableWidgetItem(""); // Empty cell
    theOpDemandTable->setItem(row, 0, name);
    QTableWidgetItem *amount = new QTableWidgetItem(""); // Empty cell
    theOpDemandTable->setItem(row, 1, amount);            
    QComboBox *functionality = new QComboBox();
    functionality->addItems({"Demand Option 1", "Demand Option 2", "Demand Option 3"});
    theOpDemandTable->setCellWidget(row, 2, functionality);
  });    
  
  
  QPushButton *doneButton = new QPushButton("Done");
  connect(doneButton, &QPushButton::clicked, this, [=]() {
    theComponentLibrary->addOrUpdateComponentTableEntry(theName->text(),
							theClass->currentText(),
							theDamageFunctionalRelation->currentText(),
							QString(""),QString(""));
    // close the widget
    this->close();
  });  


  //
  // main layout
  //

  int numRow = 0;
  
  QGridLayout *layout = new QGridLayout();
  layout->addWidget(new QLabel("Name:"), numRow, 0);
  layout->addWidget(theName, numRow++, 1);

  layout->addWidget(new QLabel("Component Class:"), numRow, 0);
  layout->addWidget(theClass, numRow++, 1);  

  // try group box
  bool groupBox = true;
  if (groupBox == true) {

    QGroupBox *groupRecovery = new QGroupBox("Recovery Model");
    QGridLayout *groupRecoveryLayout = new QGridLayout(groupRecovery);
    groupRecoveryLayout->addWidget(new QLabel("RecoveryType:"), 0, 0);
    groupRecoveryLayout->addWidget(theRecoveryType, 0, 1);        
    groupRecoveryLayout->addWidget(new QLabel("Damage Functional Recovery:"), 1, 0);
    groupRecoveryLayout->addWidget(theDamageFunctionalRelation, 1, 1);    
    groupRecoveryLayout->addWidget(addRecoveryButton, 2, 0);
    groupRecoveryLayout->addWidget(theRecoveryTable, 3, 0, 1, 4);
    
    QGroupBox *groupSupply = new QGroupBox("Supply");
    QGridLayout *groupSupplyLayout = new QGridLayout(groupSupply);  
    groupSupplyLayout->addWidget(addSupplyButton, 0, 0);
    groupSupplyLayout->addWidget(theSupplyTable, 1, 0, 1, 4);

    QGroupBox *groupOpDemand = new QGroupBox("Operational Demand");
    QGridLayout *groupOpDemandLayout = new QGridLayout(groupOpDemand);  
    groupOpDemandLayout->addWidget(addOpDemandButton, 0, 0);
    groupOpDemandLayout->addWidget(theOpDemandTable, 1, 0, 1, 4);

    layout->addWidget(groupSupply, numRow++, 0, 1, 4);
    layout->addWidget(groupOpDemand, numRow++, 0, 1, 4);
    layout->addWidget(groupRecovery, numRow++, 0, 1, 4);
    
  }

  layout->addWidget(doneButton, numRow++, 1, 1, 2);
  layout->setRowStretch(4,1);
  
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

  QJsonObject componentClass;
  componentClass["ClassName"] = theClass->currentText();
  componentObject["ComponentClass"] = componentClass;  

  //
  // Fill in supplyObject
  //
  
  QJsonObject supplyObject;
  int numRows = theSupplyTable->rowCount();
  
  for (int i=0; i<numRows; i++) {

    QJsonObject obj;
    QTableWidgetItem *nameItem = theSupplyTable->item(i, 0);    
    QTableWidgetItem *amountItem = theSupplyTable->item(i, 1);    
    QTableWidgetItem *ftarItem = theSupplyTable->item(i, 2);
    QTableWidgetItem *udtarItem = theSupplyTable->item(i, 3);
    
    QString name = nameItem->text();
    QString udtar = udtarItem->text();
    bool ok;
    obj["Amount"] = amountItem->text().toDouble(&ok);
    obj["FunctionalityToAmountRelation"] = ftarItem->text();
    if (!udtar.isEmpty() && !udtar.isNull())
      obj["UnmetDemandToAmountRelation"] = udtar;
    supplyObject[name] = obj;
    
  }

  componentObject["Supply"] = supplyObject;
  
  //
  // parse op demand
  // 

  QJsonObject opDemandObject;
  
  numRows = theOpDemandTable->rowCount();
  for (int i=0; i<numRows; i++) {
    
    QJsonObject obj;

    QTableWidgetItem *nameItem = theOpDemandTable->item(i, 0);
    QTableWidgetItem *amountItem = theOpDemandTable->item(i, 1);
    QTableWidgetItem *ftarItem = theOpDemandTable->item(i, 2);
    
    QString name = nameItem->text();
    bool ok;
    obj["Amount"] = amountItem->text().toDouble(&ok);    
    obj["FunctionalityToAmountRelation"] = ftarItem->text();
    opDemandObject[name] = obj;
    
  }

  // only ouput if nothing there
  if (numRows != 0)
    componentObject["OperationDemand"] = opDemandObject;
  
  //
  // parse recovery model
  // 

  QJsonObject recObject;
  
  recObject["ClassName"] = theRecoveryType->currentText();


  QJsonObject damageFunctionalityRelation;
  damageFunctionalityRelation["Type"] = theDamageFunctionalRelation->currentText();
  recObject["DamageFunctionalityRelation"] = damageFunctionalityRelation;  

  QJsonObject parametersObject;

  numRows = theRecoveryTable->rowCount();
  for (int i=0; i<numRows; i++) {
    QJsonObject obj;

    QTableWidgetItem *nameItem = theRecoveryTable->item(i, 0);
    QTableWidgetItem *durationItem = theRecoveryTable->item(i, 1);
    QTableWidgetItem *demandItem = theRecoveryTable->item(i, 2);
    QTableWidgetItem *precedingItem = theRecoveryTable->item(i, 3);            
    
    QString name = nameItem->text();
    QJsonParseError parseError;

    QJsonArray demandArray;
    QJsonObject durationObject;
    QJsonArray precedingArray;
    
    QJsonDocument jsonDocDemand = QJsonDocument::fromJson(demandItem->text().toUtf8(), &parseError);
    if (jsonDocDemand.isArray()) 
      demandArray = jsonDocDemand.array();
    QJsonDocument jsonDocDuration = QJsonDocument::fromJson(durationItem->text().toUtf8(), &parseError);
    if (jsonDocDuration.isObject()) 
      durationObject = jsonDocDemand.object();
    QJsonDocument jsonDocPreceding = QJsonDocument::fromJson(precedingItem->text().toUtf8(), &parseError);
    if (jsonDocPreceding.isArray()) 
      precedingArray = jsonDocPreceding.array();		
    
    obj["Duration"] = durationObject;
    obj["Demand"] = demandArray;	
    obj["PrecedingActivities"]  = precedingArray;
    
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

  theSupplyTable->clearContents();
  QJsonValue supplyValue = jsonObject["Supply"];
  
  if (!supplyValue.isNull() && supplyValue.isObject()) {
    
    QJsonObject supplyObject = supplyValue.toObject();
    theSupplyTable->setRowCount(supplyObject.size());  

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
	
	QTableWidgetItem *newName = new QTableWidgetItem(key);      
	QTableWidgetItem *newAmount = new QTableWidgetItem(amount);
	QTableWidgetItem *newFTAR = new QTableWidgetItem(ftar);
	QTableWidgetItem *newUDTAR = new QTableWidgetItem(udtar);
	theSupplyTable->setItem(row, 0, newName);
	theSupplyTable->setItem(row, 1, newAmount);
	theSupplyTable->setItem(row, 2, newFTAR);
	theSupplyTable->setItem(row, 3, newUDTAR);
	
	row++;
      }
    }
  }

  //
  // parse op demand
  // 

  theOpDemandTable->clearContents();
  QJsonValue odValue = jsonObject["OperationDemand"];
  
  if (!odValue.isNull() && odValue.isObject()) {
    
    QJsonObject odObject = odValue.toObject();
    theOpDemandTable->setRowCount(odObject.size());  

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
	
	QTableWidgetItem *newName = new QTableWidgetItem(key);      
	QTableWidgetItem *newAmount = new QTableWidgetItem(amount);
	QTableWidgetItem *newFTAR = new QTableWidgetItem(ftar);
	theOpDemandTable->setItem(row, 0, newName);
	theOpDemandTable->setItem(row, 1, newAmount);
	theOpDemandTable->setItem(row, 2, newFTAR);
	
	row++;
      }
    }
  }


  //
  // parse recovery model
  // 

  theRecoveryTable->clearContents();
  QJsonValue recoveryValue = jsonObject["RecoveryModel"];
  
  if (!recoveryValue.isNull() && recoveryValue.isObject()) {
    
    QJsonObject recoveryObject = recoveryValue.toObject();

    // type
    QString className;
    QJsonValue classValue = recoveryObject["ClassName"];
    if (!classValue.isNull() && classValue.isString()) {
      className = classValue.toString();
      int index = theRecoveryType->findText(className);
      if (index != -1) {
	theRecoveryType->setCurrentIndex(index);
      }
    }


    // damage functional relation
    QString dfrName;
    QJsonValue dfrValue = recoveryObject["DamageFunctionalityRelation"];
    if (!dfrValue.isNull() && dfrValue.isObject()) {
      QJsonValue typeValue = dfrValue.toObject()["Type"];
      if (!typeValue.isNull() && typeValue.isString()) {
	dfrName = typeValue.toString();
	int index = theDamageFunctionalRelation->findText(dfrName);
	if (index != -1) {
	  theDamageFunctionalRelation->setCurrentIndex(index);
	}
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
    theRecoveryTable->setRowCount(parametersObject.size());  

    for (const QString &key : parametersObject.keys()) {

      QJsonValue value = parametersObject.value(key);
      if (!value.isNull() && value.isObject()) {
	QJsonObject obj = value.toObject();
	
	QString duration, demand, proceedingActivities;
	QJsonValue durationValue = obj["Duration"];
	QJsonValue demandValue = obj["Demand"];	
	QJsonValue preceedingValue = obj["PrecedingActivities"];
	
	if (!durationValue.isNull() && durationValue.isObject()) 
	  duration = QString(QJsonDocument(durationValue.toObject()).toJson(QJsonDocument::Compact));
	if (!demandValue.isNull() && demandValue.isArray()) 
	  demand = QString(QJsonDocument(demandValue.toArray()).toJson(QJsonDocument::Compact));
	if (!preceedingValue.isNull() && preceedingValue.isArray()) 
	  proceedingActivities = QString(QJsonDocument(preceedingValue.toArray()).toJson(QJsonDocument::Compact));

	QTableWidgetItem *newName = new QTableWidgetItem(key);      	
	QTableWidgetItem *newDuration = new QTableWidgetItem(duration);      
	QTableWidgetItem *newDemand = new QTableWidgetItem(demand);
	QTableWidgetItem *newProceeding = new QTableWidgetItem(proceedingActivities);
	theRecoveryTable->setItem(row, 0, newName);	
	theRecoveryTable->setItem(row, 1, newDuration);
	theRecoveryTable->setItem(row, 2, newDemand);
	theRecoveryTable->setItem(row, 3, newProceeding);
	
	row++;
      }
    }
  }


  
  theComponentLibrary->addOrUpdateComponentTableEntry(theName->text(),
						      theClass->currentText(),
						      theDamageFunctionalRelation->currentText(),
						      QString(""),QString(""));  
  return true;
}

bool
PyrecodesComponent::copyFiles(QString &dirName) {
  return true;
}
