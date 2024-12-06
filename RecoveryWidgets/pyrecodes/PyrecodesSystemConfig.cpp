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
#include <QGridLayout>
#include <QPushButton>
#include <QTableWidget>
#include <QStringList>
#include <QHeaderView>
#include <QTabWidget>

#include <PyrecodesLocality.h>


PyrecodesSystemConfig::PyrecodesSystemConfig(QWidget *parent)
  :SimCenterWidget(parent)
{
  //
  // we will break up using a QTabbedWidget
  //
  
  QTabWidget *theTabbedWidget = new QTabWidget();

  //
  // constants tab
  //
  
  QWidget *constantsWidget = new QWidget();
  QGridLayout *constantsLayout = new QGridLayout(constantsWidget);

  // addConstant PushButton
  QPushButton *addConstantButton = new QPushButton("Add Constant");  

  QStringList headingsConstant; headingsConstant << "Name" << "Value";
  theConstantsTable = new QTableWidget();
  theConstantsTable->setColumnCount(2);
  theConstantsTable->setHorizontalHeaderLabels(headingsConstant);
  theConstantsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  theConstantsTable->verticalHeader()->setVisible(false);
  theConstantsTable->setShowGrid(true);
  theConstantsTable->setStyleSheet("QTableWidget::item { border: 1px solid black; }");  
    
  
  connect(addConstantButton, &QPushButton::clicked, this, [=]() {
    
    // add a row and set values to empty strings
    int row = theConstantsTable->rowCount();
    theConstantsTable->insertRow(row);
    for (int col = 0; col < 2; ++col) {
      QTableWidgetItem *item = new QTableWidgetItem(""); // Empty cell
      theConstantsTable->setItem(row, col, item);
    }
    
  });  

  constantsLayout->addWidget(addConstantButton, 0,0);
  constantsLayout->addWidget(theConstantsTable, 1, 0, 1, 4);  

  
  //
  //   localities tab
  //
  
  QWidget *localitiesWidget = new QWidget();
  QGridLayout *layoutLocalities = new QGridLayout(localitiesWidget);

  // addLocality PushButton
  QPushButton *addLocalityButton = new QPushButton("Add Locality");


  connect(addLocalityButton, &QPushButton::clicked, this, [=]() {
    this->addLocality();
  });

  // locality table
  
  QStringList headingsLocality; headingsLocality << "Locality" << "Coordinates" << "Components";
  theLocalityTable = new QTableWidget();
  theLocalityTable->setColumnCount(3);
  theLocalityTable->setHorizontalHeaderLabels(headingsLocality);
  theLocalityTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  theLocalityTable->verticalHeader()->setVisible(false);
  theLocalityTable->setShowGrid(true);
  theLocalityTable->setStyleSheet("QTableWidget::item { border: 1px solid black; }");
  
  layoutLocalities->addWidget(addLocalityButton, 0,0);
  layoutLocalities->addWidget(theLocalityTable, 1, 0, 1, 4);
  
  //
  // resources tab
  //

  QWidget *resourcesWidget = new QWidget();
  QGridLayout *layoutResources = new QGridLayout(resourcesWidget);
  
  QPushButton *addResourceButton = new QPushButton("Add Resourcse");

  QStringList headingsResources; headingsResources << "Name" << "Group" << "Distribution Model";
  theResourcesTable = new QTableWidget();
  theResourcesTable->setColumnCount(3);
  theResourcesTable->setHorizontalHeaderLabels(headingsResources);
  theResourcesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  theResourcesTable->verticalHeader()->setVisible(false);  
  theResourcesTable->setShowGrid(true);
  theResourcesTable->setStyleSheet("QTableWidget::item { border: 1px solid black; }");
  
  connect(addResourceButton, &QPushButton::clicked, this, [=]() {
    
    // add a row and set values to empty strings
    int row = theResourcesTable->rowCount();
    theResourcesTable->insertRow(row);
    for (int col = 0; col < 3; ++col) {
      QTableWidgetItem *item = new QTableWidgetItem(""); // Empty cell
      theResourcesTable->setItem(row, col, item);
    }
  });  

  
  layoutResources->addWidget(addResourceButton,0, 0);
  layoutResources->addWidget(theResourcesTable, 1, 0, 1, 4);

  //
  // add widgets to theTabbedWidget
  //

  theTabbedWidget->addTab(constantsWidget,"Constants");    
  theTabbedWidget->addTab(localitiesWidget,"Localities");  
  theTabbedWidget->addTab(resourcesWidget,"Resources");

  //
  // finally add tabbedWidget to a layout for this
  // 
  QGridLayout *layout = new QGridLayout(this);
  layout->addWidget(theTabbedWidget,0,0);
}


PyrecodesSystemConfig::~PyrecodesSystemConfig()
{

}


void
PyrecodesSystemConfig::addLocality() {
  PyrecodesLocality *theLocality = new PyrecodesLocality(this);
  theLocalities.append(theLocality);
  theLocality->show();
}


void PyrecodesSystemConfig::clear(void)
{

}

bool
PyrecodesSystemConfig::outputToJSON(QJsonObject &jsonObject)
{
  return true;
}

bool
PyrecodesSystemConfig::inputFromJSON(QJsonObject &jsonObject)
{
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
  
}
