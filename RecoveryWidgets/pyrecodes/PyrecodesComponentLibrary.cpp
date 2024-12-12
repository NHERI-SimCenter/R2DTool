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
#include <QLineEdit>
#include <QFileInfo>
#include <QFile>
#include <QJsonDocument>
#include <QFileDialog>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QGroupBox>
#include <QLabel>
#include <QMenu>

#include "Utils/RelativePathResolver.h"
#include "Utils/FileOperations.h"
#include <PyrecodesComponent.h>

PyrecodesComponentLibrary::PyrecodesComponentLibrary(QWidget *parent)
  :SimCenterWidget(parent)
{

  //
  // first a QLineEdit to Load and Save the info to a file
  //
  
  QLineEdit   *theComponentLibraryFile = new QLineEdit();
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
    theComponentLibraryFile->setText(fileName);
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
    theComponentLibraryFile->setText(fileName);    
    
  });    
  
  
  // addConstant PushButton
  QPushButton *addComponentButton = new QPushButton("Add Component");  

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

  connect(theComponentsTable, SIGNAL(cellClicked(int,int)),this,SLOT(bringUpJobActionMenu(int,int)));
  connect(theComponentsTable, SIGNAL(cellPressed(int,int)),this,SLOT(bringUpJobActionMenu(int,int)));  
  
  QGridLayout *layout = new QGridLayout(this);
  layout->addWidget(new QLabel("Component Library File:"),0,0);
  layout->addWidget(theComponentLibraryFile,0,1);
  layout->addWidget(loadFileButton,0,2);
  layout->addWidget(saveFileButton,0,3);
  layout->setColumnStretch(1,1);

  QGroupBox *theGroupBox = new QGroupBox("Component Library");
  QGridLayout *groupLayout = new QGridLayout(theGroupBox);
  groupLayout->addWidget(addComponentButton, 0,0, 1, 2);
  groupLayout->addWidget(theComponentsTable, 1, 0, 1, 5);
  
  layout->addWidget(theGroupBox,1,0,1,4);  
}


PyrecodesComponentLibrary::~PyrecodesComponentLibrary()
{

}


void
PyrecodesComponentLibrary::addComponent() {
  QString name;
  PyrecodesComponent *theComponent = new PyrecodesComponent(name, this);
  theComponents.append(theComponent);
  theComponent->resize(this->size());
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
  theComponents.clear();
  theComponentsTable->clearContents();

  for (const QString &key : jsonObject.keys()) {

    QJsonValue value = jsonObject.value(key);
    if (!value.isNull() && value.isObject()) {
      QJsonObject componentObj = value.toObject();
    
      PyrecodesComponent *theComponent = new PyrecodesComponent(key, this);
      theComponent->inputFromJSON(componentObj);
      theComponents.append(theComponent);
    } else {
      QString msg(QString("Reading component ") + key + QString(". Not valid JSON"));
      this->errorMessage(msg);
    }
  }
  
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

void
PyrecodesComponentLibrary::bringUpJobActionMenu(int row, int col){
    Q_UNUSED(col);









    triggeredRow = row;
    QMenu jobMenu;

    jobMenu.addAction("Edit Component", this, SLOT(editComponent()));
    jobMenu.addAction("Delete Component", this, SLOT(deleteComponent()));


    jobMenu.exec(QCursor::pos());
}


void
PyrecodesComponentLibrary::deleteComponent() {
  QTableWidgetItem *itemName = theComponentsTable->item(triggeredRow,0);
  QString name = itemName->text();
  qDebug() << "NAME: " << name;  
  
  // remove from QList
  for (QList<PyrecodesComponent *>::iterator it = theComponents.begin(); it != theComponents.end(); ++it) {
    PyrecodesComponent *theComponent = *it;    
    if (theComponent->theName->text() == name) {
      theComponents.erase(it);

      // remove table row
      theComponentsTable->removeRow(triggeredRow);
  
      
      break;          
    }
  }
}

void
PyrecodesComponentLibrary::editComponent() {
  QTableWidgetItem *itemName = theComponentsTable->item(triggeredRow,0);
  QString name = itemName->text();
  qDebug() << "NAME: " << name;
  for (QList<PyrecodesComponent *>::iterator it = theComponents.begin(); it != theComponents.end(); ++it) {
    PyrecodesComponent *theComponent = *it;
    qDebug() << "name: " << theComponent->theName->text();    
    if (theComponent->theName->text() == name) {
      theComponent->resize(this->size());
      theComponent->show();
      break;          // Exit the loop after removing
    }
  }
}
    


