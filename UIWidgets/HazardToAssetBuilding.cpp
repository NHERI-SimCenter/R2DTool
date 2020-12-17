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
// Latest revision: 09.30.2020

#include "HazardToAssetBuilding.h"
#include "SimCenterPreferences.h"
#include "sectiontitle.h"
#include "RegionalMappingWidget.h"
#include <SimCenterAppEventSelection.h>
#include <NoArgSimCenterApp.h>

#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QJsonArray>
#include <QPushButton>
#include <QCheckBox>
#include <QJsonObject>
#include <QComboBox>
#include <QDebug>
#include <QList>
#include <QLabel>
#include <QLineEdit>
#include <QMetaEnum>


HazardToAssetBuilding::HazardToAssetBuilding(QWidget *parent)
    : SimCenterAppWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QHBoxLayout *theHeaderLayout = new QHBoxLayout();
    SectionTitle *label = new SectionTitle();
    label->setText(QString("Hazard to Local Asset Event"));
    label->setMinimumWidth(150);
    theHeaderLayout->addWidget(label);
    QSpacerItem *spacer = new QSpacerItem(50,10);
    theHeaderLayout->addItem(spacer);
    theHeaderLayout->addStretch(1);

    mainLayout->addLayout(theHeaderLayout);


    theRegionalMapping = new RegionalMappingWidget(this);
    QGroupBox* regionalMappingGroupBox = new QGroupBox("Regional Mapping", this);
    regionalMappingGroupBox->setContentsMargins(0,5,0,0);
    regionalMappingGroupBox->setLayout(theRegionalMapping->layout());
    mainLayout->addWidget(regionalMappingGroupBox);

    theLocalMapping = new SimCenterAppEventSelection(QString("Local Event Type"), QString("Events"), this);
    SimCenterAppWidget *simcenterEvent = new NoArgSimCenterApp(QString("SimCenterEvent"));
    SimCenterAppWidget *siteResponse = new NoArgSimCenterApp(QString("SiteResponse"));

    theLocalMapping->addComponent(QString("SimCenterEvent"), QString("SimCenterEvent"), simcenterEvent);
    theLocalMapping->addComponent(QString("Site Response"), QString("SiteResponse"), siteResponse);

    QGroupBox* localMappingGroupBox = new QGroupBox("Local Mapping", this);
    localMappingGroupBox->setContentsMargins(0,5,0,0);
    localMappingGroupBox->setLayout(theLocalMapping->layout());
    mainLayout->addWidget(localMappingGroupBox);

    mainLayout->addStretch();
    this->setLayout(mainLayout);
    this->setMinimumWidth(640);
    this->setMaximumWidth(750);
}


HazardToAssetBuilding::~HazardToAssetBuilding()
{

}


bool HazardToAssetBuilding::outputToJSON(QJsonObject &jsonObj)
{
    theRegionalMapping->outputToJSON(jsonObj);
    theLocalMapping->outputToJSON(jsonObj);

    return true;
}


bool HazardToAssetBuilding::inputFromJSON(QJsonObject &jsonObject){

    return true;
}

bool HazardToAssetBuilding::outputAppDataToJSON(QJsonObject &jsonObj)
{
    qDebug() << "HazardToAss ;; ouputAppData";

    theRegionalMapping->outputAppDataToJSON(jsonObj);
    theLocalMapping->outputAppDataToJSON(jsonObj);

    return true;
}


bool HazardToAssetBuilding::inputAppDataFromJSON(QJsonObject &jsonObject){

    return true;
}





