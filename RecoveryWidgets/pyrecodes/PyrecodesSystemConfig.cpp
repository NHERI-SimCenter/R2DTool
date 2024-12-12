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
#include <QLabel>
#include <QLineEdit>
#include <QFileInfo>
#include <QFile>
#include <QJsonDocument>
#include <QFileDialog>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QGroupBox>


#include "Utils/RelativePathResolver.h"
#include "Utils/FileOperations.h"

#include <PyrecodesLocality.h>

PyrecodesSystemConfig::PyrecodesSystemConfig(QWidget *parent)
  :SimCenterWidget(parent)
{
  //
  // first a QLineEdit to Load and Save the info to a file
  //
  
  QLineEdit   *theSystemFile = new QLineEdit();
  QPushButton *loadFileButton = new QPushButton("Load");
  QPushButton *saveFileButton = new QPushButton("Save");  

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
  layout->addWidget(new QLabel("System Config File:"),0,0);
  layout->addWidget(theSystemFile,0,1);
  layout->addWidget(loadFileButton,0,2);
  layout->addWidget(saveFileButton,0,3);
  layout->setColumnStretch(1,1);

  QGroupBox *theGroupBox = new QGroupBox("System Configuration");
  QGridLayout *groupLayout = new QGridLayout(theGroupBox);
  groupLayout->addWidget(theTabbedWidget,0,0);
  layout->addWidget(theGroupBox,1,0,1,4);
}


PyrecodesSystemConfig::~PyrecodesSystemConfig()
{

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

  for (int row = 0; row < theConstantsTable->rowCount(); ++row) {
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
  theResourcesTable->setRowCount(resourcesObject.size());
  
  for (const QString &key : resourcesObject.keys()) {

    QJsonValue value = resourcesObject.value(key);
    if (value.isObject()) {

      QString groupStr;
      QString distributionStr;
      
      QJsonObject valueObj = value.toObject();
      QJsonValue groupValue = valueObj["Group"];
      if (!groupValue.isNull() && groupValue.isString())
	groupStr = groupValue.toString();

      QJsonValue distributionValue = valueObj["DistributionModel"];
      if (!distributionValue.isNull() && distributionValue.isObject()) {
	QJsonObject distributionObject = distributionValue.toObject();
	if (!distributionObject["ClassName"].isNull())
	  distributionStr = distributionObject["ClassName"].toString();
      }
	
      // set table items
      theResourcesTable->setItem(numRow, 0, new QTableWidgetItem(key));
      theResourcesTable->setItem(numRow, 1, new QTableWidgetItem(groupStr));
      theResourcesTable->setItem(numRow, 2, new QTableWidgetItem(distributionStr));
      qDebug() << key << " " << groupStr << " " << distributionStr;
      numRow++;
      
    } else {
      
      // error Message
      QString msg(QString("In Resourses section, item with key") + key + QString(" is not valid JSON"));
      this->errorMessage(msg);
    }
  }



  //
  // read locality data & fill the table
  //

  numRow = 0;
  
  QJsonObject localityObject = jsonObject["Content"].toObject();  
  // theLocalityTable->setRowCount(localityObject.size());
  theLocalityTable->clearContents();
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
