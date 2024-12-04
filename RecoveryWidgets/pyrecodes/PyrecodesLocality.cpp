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


#include <PyrecodesLocality.h>
#include <QGridLayout>
#include <QLabel>
#include <QGroupBox>
#include <PyrecodesSystemConfig.h>

#include <SC_CheckBox.h>
#include <SC_FileEdit.h>
#include <SC_StringLineEdit.h>


#include <QDebug>
#include <QComboBox>
#include <QListWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QWidget>

PyrecodesLocality::PyrecodesLocality(PyrecodesSystemConfig *theSC, QWidget *parent)
  :SimCenterWidget(parent), theSystemConfig(theSC)
{
  theName = new SC_StringLineEdit("name");
  coordinateFile = new SC_FileEdit("coordinateFile");
  boundingBox = new SC_StringLineEdit("boundingBox");
  buildingsCB = new SC_CheckBox("buildings");
  infrastructureCB = new SC_CheckBox("infrastructure");
  resourceSupplierCB = new SC_CheckBox("resourcseSupplier");
  QPushButton *doneButton = new QPushButton("Done");

  //
  // create a group box for the coordinates and another for the components
  //
    
  QGroupBox *coordinates = new QGroupBox("Coordinates");
  QGridLayout *coordinatesLayout = new QGridLayout(coordinates);
  coordinatesLayout->addWidget(new QLabel("GeoJSON file:"),0,0);  
  coordinatesLayout->addWidget(coordinateFile,0,1);  
  coordinatesLayout->addWidget(new QLabel("Bounding Box:"),1,0);
  coordinatesLayout->addWidget(boundingBox,1,1);    

  QGroupBox *components = new QGroupBox("Components");
  QGridLayout *componentsLayout = new QGridLayout(components);
  componentsLayout->addWidget(buildingsCB, 0, 0);
  componentsLayout->addWidget(new QLabel("Buildings"), 0,1);
  componentsLayout->addWidget(infrastructureCB, 0, 2);  
  componentsLayout->addWidget(new QLabel("Infrastructure"), 0,3);

  /* do with a list widget instead .. keeping around just in case
  waterCB = new SC_CheckBox("water");
  transportationCB = new SC_CheckBox("transportation");
  componentsLayout->addWidget(waterCB, 1, 2);  
  componentsLayout->addWidget(new QLabel("Water"), 1,3);
  componentsLayout->addWidget(transportationCB, 2, 2);  
  componentsLayout->addWidget(new QLabel("Transportation"), 2,3);  
  */
  
  infrastructureList = new QListWidget();  
  infrastructureList->addItem("Water");
  infrastructureList->addItem("Transportation");
  
  for (int i = 0; i < infrastructureList->count(); ++i) {
    QListWidgetItem *item = infrastructureList->item(i);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(Qt::Unchecked);
  }

  componentsLayout->addWidget(infrastructureList, 1,3);  
  componentsLayout->addWidget(resourceSupplierCB, 0, 4);    
  componentsLayout->addWidget(new QLabel("Recovery Resource Supplier"),0,5);
  componentsLayout->setColumnStretch(1,1);
  componentsLayout->setColumnStretch(3,1);
  componentsLayout->setColumnStretch(5,1);    
    
  //
  // connections
  //
  
  connect(infrastructureList, &QListWidget::itemChanged, this, [this]() {
    QStringList selected;
    for (int i = 0; i < infrastructureList->count(); ++i) {
      QListWidgetItem *item = infrastructureList->item(i);
      if (item->checkState() == Qt::Checked) {
	selected << item->text();
      }
    }
  });

  connect(doneButton, &QPushButton::clicked, this, [=]() {

    // TO_DO: if file specified, open file and add geometry info 
    QString coordinates = boundingBox->text();
    QStringList componentsSelected;
    if (buildingsCB->checkState() == Qt::Checked)
      componentsSelected << "Buildings";
    if (infrastructureCB->checkState() == Qt::Checked)	{
      componentsSelected << "Infrastructure";
      for (int i = 0; i < infrastructureList->count(); ++i) {
	QListWidgetItem *item = infrastructureList->item(i);
	if (item->checkState() == Qt::Checked) {
	  componentsSelected << item->text();
	}
      }
    }
    
    // set table entries
    theSystemConfig->addOrUpdateLocalityTableEntry(theName->text(),
					   coordinates,
					   componentsSelected);

    // close the widget
    this->close();
  });  


  //
  // main layout
  //

  QGridLayout *layout = new QGridLayout();
  layout->addWidget(new QLabel("Name:"),0,0);
  layout->addWidget(theName, 0,1);

  layout->addWidget(coordinates, 1, 0, 1, 4);
  layout->addWidget(components, 2, 0,  1, 4);
  layout->addWidget(doneButton, 3, 1, 1, 2);

  layout->setRowStretch(4,1);
  
  
  this->setLayout(layout);
}


PyrecodesLocality::~PyrecodesLocality()
{

}

void PyrecodesLocality::clear(void)
{

}

bool
PyrecodesLocality::outputToJSON(QJsonObject &jsonObject)
{
  return true;
}

bool
PyrecodesLocality::inputFromJSON(QJsonObject &jsonObject)
{
  return true;
}

bool
PyrecodesLocality::copyFiles(QString &dirName) {
  return true;
}
