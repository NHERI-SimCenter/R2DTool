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


#include <PyrecodesComponentLibrary.h>
#include <QGridLayout>
#include <QPushButton>
#include <QTableWidget>
#include <QStringList>
#include <QHeaderView>

#include <PyrecodesComponent.h>

PyrecodesComponentLibrary::PyrecodesComponentLibrary(QWidget *parent)
  :SimCenterWidget(parent)
{

  QGridLayout *layout = new QGridLayout(this);

  // addConstant PushButton
  QPushButton *addComponentButton = new QPushButton("Add ComponentTemplate");  

  QStringList headingsComponent; headingsComponent << "Name" << "Class" << "Supply" << "Demand" << "Recovery Model";
  theComponentsTable = new QTableWidget();
  theComponentsTable->setColumnCount(5);
  theComponentsTable->setHorizontalHeaderLabels(headingsComponent);
  theComponentsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  theComponentsTable->verticalHeader()->setVisible(false);
  theComponentsTable->setShowGrid(true);
  theComponentsTable->setStyleSheet("QTableWidget::item { border: 1px solid black; }");  
  
  connect(addComponentButton, &QPushButton::clicked, this, [=]() {
    this->addComponent();
  });  

  layout->addWidget(addComponentButton, 0,0, 1, 2);
  layout->addWidget(theComponentsTable, 1, 0, 1, 5);  
}


PyrecodesComponentLibrary::~PyrecodesComponentLibrary()
{

}


void
PyrecodesComponentLibrary::addComponent() {
  PyrecodesComponent *theComponent = new PyrecodesComponent(this);
  theComponents.append(theComponent);
  theComponent->show();
}


void PyrecodesComponentLibrary::clear(void)
{

}

bool
PyrecodesComponentLibrary::outputToJSON(QJsonObject &jsonObject)
{
  return true;
}

bool
PyrecodesComponentLibrary::inputFromJSON(QJsonObject &jsonObject)
{
  return true;
}

bool
PyrecodesComponentLibrary::copyFiles(QString &dirName) {
  return true;
}

void
PyrecodesComponentLibrary::addOrUpdateComponentTableEntry(QString name, QString classT, QString supply, QString demand, QString recoveryModel)
{

  bool found = false;
  int row = 0;
  
  for (row = 0; row < theComponentsTable->rowCount(); ++row) {
    QTableWidgetItem *item = theComponentsTable->item(row, 0);
    if (item && item->text() == name) {
      found = true;
      break;
    }
  }
  if (!found) {
    row = theComponentsTable->rowCount();
    theComponentsTable->insertRow(row);
  }
  //
  // now add the items
  //
  
  QTableWidgetItem *newName = new QTableWidgetItem(name);
  QTableWidgetItem *newClass = new QTableWidgetItem(classT);
  QTableWidgetItem *newSupply = new QTableWidgetItem(supply);
  QTableWidgetItem *newDemand = new QTableWidgetItem(demand);
  QTableWidgetItem *newRecovery = new QTableWidgetItem(recoveryModel);      

  theComponentsTable->setItem(row, 0, newName);
  theComponentsTable->setItem(row, 1, newClass);
  theComponentsTable->setItem(row, 2, newSupply);
  theComponentsTable->setItem(row, 2, newDemand);
  theComponentsTable->setItem(row, 2, newRecovery);        
}
