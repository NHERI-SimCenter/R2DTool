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


#include <PyrecodesComponent.h>
#include <SC_FileEdit.h>
#include <SC_StringLineEdit.h>
#include <SC_ComboBox.h>
#include <PyrecodesComponentLibrary.h>


#include <QGridLayout>
#include <QLabel>
#include <QGroupBox>
#include <QDebug>
#include <QTableWidget>
#include <QHeaderView>
#include <QListWidget>
#include <QPushButton>
#include <QWidget>

PyrecodesComponent::PyrecodesComponent(PyrecodesComponentLibrary *theCL, QWidget *parent)
  :SimCenterWidget(parent), theComponentLibrary(theCL)
{
  theName = new SC_StringLineEdit("name");

  QStringList classTypes; classTypes << "CompClass-1" << "CompClass-2";
  theClass = new SC_ComboBox("componentClass", classTypes);

  //
  // Recovery Relation & Table
  //

  QStringList functionTypes; functionTypes << "RecRel-1" << "RecRel-2";  
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
    groupRecoveryLayout->addWidget(new QLabel("Damage Functional Recovery:"), 0, 0);
    groupRecoveryLayout->addWidget(theDamageFunctionalRelation, 0, 1);    
    groupRecoveryLayout->addWidget(addRecoveryButton, 1, 0);
    groupRecoveryLayout->addWidget(theRecoveryTable, 2, 0, 1, 4);
    
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
  return true;
}

bool
PyrecodesComponent::inputFromJSON(QJsonObject &jsonObject)
{
  return true;
}

bool
PyrecodesComponent::copyFiles(QString &dirName) {
  return true;
}
