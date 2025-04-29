/* *****************************************************************************
Copyright (c) 2016-2017, The Regents of the University of California (Regents).
All rights reserved.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the FreeBSD Project.

REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS 
PROVIDED "AS IS". REGENTS HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, 
UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

*************************************************************************** */

// Written by: fmk

#include "MultiHazardInputWidget.h"
#include "RasterHazardInputWidget.h"

// Qt headers
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QScrollArea>
#include <QPushButton>

MultiHazardInputWidget::MultiHazardInputWidget(QGISVisualizationWidget *vizWidget,
					       SimCenterAppWidget *theProto,
					       QString appT,
					       QWidget *parent)
  :SimCenterAppWidget(parent), thePrototype(theProto), appType(appT), theVisualizationWidget(vizWidget)
{
    theLayout = new QVBoxLayout(this);
    QHBoxLayout *horizontalControl = new QHBoxLayout();
    QPushButton *addButton = new QPushButton(" Add Hazard ");
    QPushButton *removeButton = new QPushButton("Remove Unchecked");
    horizontalControl->addWidget(addButton);
    horizontalControl->addWidget(removeButton);
    horizontalControl->addStretch();
    
    theLayout->addLayout(horizontalControl);
    theLayout->addStretch();
    this->addTab();

    connect(addButton, &QPushButton::clicked, this, [=](){
      this->addTab();
    });

    connect(removeButton, &QPushButton::clicked, this, [=](){
      this->removeCurrentTab();
    });
}



MultiHazardInputWidget::~MultiHazardInputWidget()
{

}

bool MultiHazardInputWidget::outputToJSON(QJsonObject &jsonObject)
{
   int numApps = theApps.size();
   if (numApps == 0) {
     this->errorMessage("MultiHazardInputWidget::inputFromJSON App needs at least one Entry");
     return false;
   }
   // output first as normal
   theApps.at(0)->outputToJSON(jsonObject);

   // place the rest in an appArray .. this leads to minimal changes in backend
   QJsonArray appArray;
   for (int i=1; i<numApps; i++) {
     QJsonObject modelData;
     SimCenterAppWidget *theWidget = theApps.at(i);
     theWidget->outputToJSON(modelData);
     appArray.append(modelData);     
   }
   jsonObject.insert(QString("multiple"),appArray);
   return true;
}


bool MultiHazardInputWidget::inputFromJSON(QJsonObject &jsonObject)
{
   int numApps = theApps.size();
   if (numApps == 0) {
     this->errorMessage("MultiHazardInputWidget::inputFromJSON App needs at least one Entry");
     return false;
   }
   // output first as normal
   theApps.at(0)->inputFromJSON(jsonObject);
   
   // if multiple keys exists .. get others
   if (jsonObject.contains("multiple")) {
     QJsonArray otherObjects = jsonObject["multiple"].toArray();
     int length = otherObjects.count();
     if (length != numApps-1) {
       return false;
     }
     
     for (int i=0; i<length; i++) {
       SimCenterAppWidget *theWidget = theApps.at(i+1);
       QJsonObject otherData = otherObjects.at(i).toObject();
       theWidget->inputFromJSON(otherData);	  
     } 
   }
   return true;
}


bool MultiHazardInputWidget::outputAppDataToJSON(QJsonObject &jsonObject)
{
   int numApps = theApps.size();
   if (numApps == 0) {
     this->errorMessage("App needs at least one Entry");
     return false;
   }
   
   // output first as normal
   theApps.at(0)->outputAppDataToJSON(jsonObject);

   // place the rest in an array keyed by "multiple"  .. this leads to minimal changes in backend
   QJsonArray appArray;
   for (int i=1; i<numApps; i++) {
     QJsonObject modelData;
     SimCenterAppWidget *theWidget = theApps.at(i);
     theWidget->outputAppDataToJSON(modelData);
     appArray.append(modelData);
   }
   jsonObject.insert(QString("multiple"),appArray);
   return true;
}


bool MultiHazardInputWidget::inputAppDataFromJSON(QJsonObject &jsonObject)
{

  // clear, add a tab and input the first
  this->clear();
  this->addTab();
  theApps.at(0)->inputAppDataFromJSON(jsonObject);
   
  // if "multiple" key exists .. create and input the others for each element in array multiple
  if (jsonObject.contains("multiple")) {
    QJsonArray otherObjects = jsonObject["multiple"].toArray();
    int length = otherObjects.count();
    for (int i=0; i<length; i++) {
      this->addTab();
      SimCenterAppWidget *theWidget = theApps.at(i+1);
      QJsonObject otherData = otherObjects.at(i).toObject();
      theWidget->inputAppDataFromJSON(otherData);	  
    } 
   }
   return true;  
}


bool MultiHazardInputWidget::copyFiles(QString &destDir)
{
    bool result = true;
    int numApp = theApps.size();
    for (int i=0; i<numApp; i++) {
        SimCenterAppWidget *theWidget = theApps.at(i);
        bool res = theWidget->copyFiles(destDir);
        if (res != true) result  = false;
    }
    return result;
}


bool MultiHazardInputWidget::outputCitation(QJsonObject &jsonObj)
{
    int numApps = theApps.size();

    for (int i=0; i<numApps; i++) {
        QJsonObject jsonObj_tmp;
        SimCenterAppWidget *theWidget = theApps.at(i);
        theWidget->outputCitation(jsonObj_tmp);

        if (!jsonObj_tmp.isEmpty()) {
          jsonObj.insert("model " + QString::number(i+1) , jsonObj_tmp);
        }

    }

    return true;
}


void MultiHazardInputWidget::clear(void)
{

  while (theLayout->count() > 2) {
    QLayoutItem *item = theLayout->itemAt(1);
    QWidget *widget = item->widget();
    if (widget != 0)
      widget->setParent(0);
    theLayout->removeItem(item);
  }
  
  theGroupBoxes.clear();
  theApps.clear();
}

int
MultiHazardInputWidget::addTab() {

  SimCenterAppWidget *theNewSelection = thePrototype->getClone();
    
    if (theNewSelection == 0) {
      errorMessage(QString("MultiHazardInputWidget::failed to return a valid widget when trying to add tab"));
      return -1;
    }

    QGroupBox *theNewBox = new QGroupBox();
    QHBoxLayout *theBoxLayout = new QHBoxLayout();
    theBoxLayout->addWidget(theNewSelection);
    theNewBox->setLayout(theBoxLayout);
    theNewBox->setContentsMargins(0,5,0,0);
    theNewBox->setCheckable(true);

    theLayout->insertWidget(theLayout->count() - 1, theNewBox);

    theApps.append(theNewSelection);
    theGroupBoxes.append(theNewBox);

    return 0;
}


int
MultiHazardInputWidget::removeCurrentTab() {
    int result = 0;
    int numApp = theApps.size();
    for (int i=numApp-1; i>=0; i--) {
        QGroupBox *theBox = theGroupBoxes.at(i);
	if (!theBox->isChecked()) {
	  QWidget *theApp = theApps.at(i);
	  QWidget *theGroupBox = theGroupBoxes.at(i);
	  theApps.removeAt(i);
	  theGroupBoxes.removeAt(i);
	  delete theApp;
	  delete theGroupBox;
	}
    }
    return 0;
}


