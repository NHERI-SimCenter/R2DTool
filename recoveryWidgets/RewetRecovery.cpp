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

// Written: fmk

#include "RewetRecovery.h"
#include <QScrollArea>
#include <QLineEdit>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QJsonObject>

#include <SC_DoubleLineEdit.h>
#include <SC_FileEdit.h>
#include <SC_IntLineEdit.h>
#include <SC_ComboBox.h>

RewetRecovery::RewetRecovery(QWidget *parent)
    : SimCenterAppWidget(parent)
{
    int windowWidth = 800;

    QGridLayout *mainLayout = new QGridLayout();

    QTabWidget *theTabWidget = new QTabWidget();
    mainLayout->addWidget(theTabWidget);

    
    //
    // simulation widget
    //
    
    QWidget *simulationWidget = new QWidget();

    //
    // hydraulics widget
    //
    
    QWidget *hydraulicsWidget = new QWidget();

    //
    // restoration widget
    //
    
    QWidget *restorationWidget = new QWidget();    
    
    theTabWidget->addTab(simulationWidget, "Simulation");
    theTabWidget->addTab(hydraulicsWidget, "Hydraulics");
    theTabWidget->addTab(restorationWidget, "Restoration");


    // stuff below for scrolling .. if no scrolling this->setLayout(mainLayout);
    
    QWidget     *mainGroup = new QWidget();
    mainGroup->setLayout(mainLayout);
    mainGroup->setMaximumWidth(windowWidth);
    
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setLineWidth(1);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setWidget(mainGroup);
    scrollArea->setMaximumWidth(windowWidth + 25);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(scrollArea);
    this->setLayout(layout);
}


RewetRecovery::~RewetRecovery()
{

}


void RewetRecovery::clear(void)
{

}


bool RewetRecovery::inputFromJSON(QJsonObject &jsonObject)
{
  this->clear();
  
  //   inputSettings->inputFromJSON(jsonObject);
  
  return true;
}

bool RewetRecovery::outputToJSON(QJsonObject &jsonObject)
{
  jsonObject["Application"] = "RewetRecovery";
    
  
  // inputSettings->outputToJSON(jsonObject);
  
  return true;
}

bool RewetRecovery::outputAppDataToJSON(QJsonObject &jsonObject) {

    //
    // per API, need to add name of application to be called in AppLication
    // and all data to be used in ApplicationDate
    //

    jsonObject["Application"] = "RewetRecovery";
    QJsonObject dataObj;
    jsonObject["ApplicationData"] = dataObj;

    return true;
}
bool RewetRecovery::inputAppDataFromJSON(QJsonObject &jsonObject) {
    Q_UNUSED(jsonObject);
    return true;
}


bool RewetRecovery::copyFiles(QString &destDir) {

}

