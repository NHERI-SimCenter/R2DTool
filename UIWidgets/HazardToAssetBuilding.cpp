/* *****************************************************************************
Copyright (c) 2016-2021, The Regents of the University of California (Regents).
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
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
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

// Written by: Stevan Gavrilovic

#include "HazardToAssetBuilding.h"
#include "NearestNeighbourMapping.h"
#include "SiteSpecifiedMapping.h"
#include "NoArgSimCenterApp.h"
#include "SimCenterAppEventSelection.h"
#include "SimCenterAppSelection.h"
#include "SimCenterEventRegional.h"
#include "SimCenterPreferences.h"
#include "sectiontitle.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDebug>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QMetaEnum>
#include <QPushButton>

HazardToAssetBuilding::HazardToAssetBuilding(QString key, QWidget *parent)
  : SimCenterAppWidget(parent), jsonKey(key)
{
  
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(0);

    QHBoxLayout *theHeaderLayout = new QHBoxLayout();
    SectionTitle *label = new SectionTitle();
    label->setText(QString("Hazard to Local Asset Event"));
    label->setMinimumWidth(150);
    theHeaderLayout->addWidget(label);
    QSpacerItem *spacer = new QSpacerItem(50,0);
    theHeaderLayout->addItem(spacer);
    theHeaderLayout->addStretch(1);
    mainLayout->addLayout(theHeaderLayout);

    //
    // regional mapping to building box
    //

    theRegionalMapping = new SimCenterAppSelection(QString("Mapping Application"), jsonKey, this);
    QGroupBox* regionalMappingGroupBox = new QGroupBox("Regional Mapping", this);
    regionalMappingGroupBox->setContentsMargins(0,5,0,0);
    regionalMappingGroupBox->setLayout(theRegionalMapping->layout());
    mainLayout->addWidget(regionalMappingGroupBox);

    NearestNeighbourMapping *theNNMap = new NearestNeighbourMapping();
    theRegionalMapping->addComponent(QString("Nearest Neighbour"), QString("NearestNeighborEvents"), theNNMap);

    SiteSpecifiedMapping *theSSMap = new SiteSpecifiedMapping();
    theRegionalMapping->addComponent(QString("Site Specified"), QString("SiteSpecifiedEvents"), theSSMap);

    // NOTE: if adding something new, need to redo as only want to call this on currently selected item in appSelection
    connect(this,SIGNAL(hazardGridFileChangedSIGNAL(QString, QString)), theNNMap, SLOT(handleFileNameChanged(QString, QString)));


    /* **************************************************
    //
    // local mapping hazard to building
    //

    theLocalMapping = new SimCenterAppEventSelection(QString("Local Event Type"), QString("Events"), this);
    SimCenterAppWidget *simcenterEvent = new SimCenterEventRegional();
    connect(this,SIGNAL(eventTypeChangedSignal(QString&)), theLocalMapping, SLOT(currentEventTypeChanged(QString&)));

    theLocalMapping->addComponent(QString("SimCenterEvent"), QString("SimCenterEvent"), simcenterEvent);
    
    //SimCenterAppWidget *siteResponse = new NoArgSimCenterApp(QString("SiteResponse"));
    //theLocalMapping->addComponent(QString("Site Response"), QString("SiteResponse"), siteResponse);

    QGroupBox* localMappingGroupBox = new QGroupBox("Local Mapping", this);
    localMappingGroupBox->setContentsMargins(0,5,0,0);
    localMappingGroupBox->setLayout(theLocalMapping->layout());
    mainLayout->addWidget(localMappingGroupBox);
    ****************************************************** */
      
    mainLayout->addStretch();
    this->setLayout(mainLayout);
    this->setMaximumWidth(750);
}


HazardToAssetBuilding::~HazardToAssetBuilding()
{

}


bool HazardToAssetBuilding::outputToJSON(QJsonObject &jsonObj)
{
  /*
    bool result = true;
    QJsonObject data;
    QJsonObject writeObj;
    
    if (!jsonKey.isEmpty())
      writeObj = data;
    else
      writeObj = jsonObj;
      
    if (theRegionalMapping->outputToJSON(writeObj)  != true) {
      errorMessage(QString("Regional Mapping Widget returned failure in outputToJSON"));
      result = false;
    }
    
    //if (theLocalMapping->outputToJSON(writeObj)  != true) {
    //      errorMessage(QString("Local Mapping Widget returned failure in outputToJSON"));      
    //  result = false;
    // }

    
    if (!jsonKey.isEmpty())
      jsonObj[jsonKey] = data;
    */

    bool result = true;
      
    if (theRegionalMapping->outputToJSON(jsonObj)  != true) {
      errorMessage(QString("Regional Mapping Widget returned failure in outputToJSON"));
      result = false;
    }
      
    return result;
}


bool HazardToAssetBuilding::inputFromJSON(QJsonObject &jsonObj){

  /*
    QJsonObject &readObj = jsonObj;
  
    if (!jsonKey.isEmpty()) {
      if (!jsonObj.contains(jsonKey)) {      
	QString errorMsg(QString("HazardToAssetBuilding keyWord: ") + jsonKey +
			 QString(" not in json"));
	errorMessage(errorMsg);
      } else
	readObj = jsonObj[jsonKey].toObject();
    }
    
    bool result = true;
    if (theRegionalMapping->inputFromJSON(readObj) != true)
        result = false;
  */
  /*
    if (theLocalMapping->inputFromJSON(readObj) != true)
        result = false;
    */
    bool result = true;
    if (theRegionalMapping->inputFromJSON(jsonObj) != true)
        result = false;    
    return result;
}


bool HazardToAssetBuilding::outputAppDataToJSON(QJsonObject &jsonObj)
{
  /*
    QJsonObject data;
    QJsonObject *writeObj = &jsonObj;
    
    if (!jsonKey.isEmpty())
      writeObj = &data;
    
    bool result = true;
    if (theRegionalMapping->outputAppDataToJSON(*writeObj) != true)
        result = false;

	// if (theLocalMapping->outputAppDataToJSON(*writeObj) != true)
	//   result = false;

    
    if (!jsonKey.isEmpty())
      jsonObj[jsonKey] = *writeObj;
    */

    bool result = true;
    if (theRegionalMapping->outputAppDataToJSON(jsonObj) != true)
        result = false;

    return result;
}


bool HazardToAssetBuilding::inputAppDataFromJSON(QJsonObject &jsonObj){

  QJsonObject readObj;
	
    if (!jsonKey.isEmpty()) {
      if (!jsonObj.contains(jsonKey)) {      
	QString errorMsg(QString("HazardToAssetBuilding keyWord: ") + jsonKey +
			 QString(" not in json"));
	errorMessage(errorMsg);
	return false;
      } else
	readObj = jsonObj[jsonKey].toObject();
    } else
      readObj = jsonObj;
  
    bool result = true;
    if (theRegionalMapping->inputAppDataFromJSON(readObj) != true)
        result = false;
    /*
    if (theLocalMapping->inputAppDataFromJSON(readObj) != true)
        result = false;
    */
    
    return result;
}


void HazardToAssetBuilding::hazardGridFileChangedSlot(QString motionDir, QString eventFile)
{
    emit hazardGridFileChangedSIGNAL(motionDir, eventFile);
}


void HazardToAssetBuilding::eventTypeChangedSlot(QString eventType)
{
    emit eventTypeChangedSignal(eventType);
}


bool HazardToAssetBuilding::copyFiles(QString &destName)
{
    bool result = true;

    if (theRegionalMapping->copyFiles(destName) != true) {
        result = false;
	
	return result;
    }
    /*
    if (theLocalMapping->copyFiles(destName) != true) {
        result = false;
    }
    */

    return result;
}


void HazardToAssetBuilding::clear(void)
{
    theRegionalMapping->clear();
    // theLocalMapping->clear();
}
