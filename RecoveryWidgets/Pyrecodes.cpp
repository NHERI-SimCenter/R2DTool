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
// Written: fmk, Sina Naeimi

#include "Pyrecodes.h"
#include <QScrollArea>
#include <QLineEdit>
#include <QTabWidget>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QJsonObject>
#include <QDoubleValidator>
#include "LineEditSelectTool.h"

#include <SC_QRadioButton.h>
#include <SC_DoubleLineEdit.h>
#include <SC_FileEdit.h>
#include <SC_IntLineEdit.h>
#include <SC_ComboBox.h>
#include <SC_CheckBox.h>
#include <SC_TableEdit.h>
#include <RewetResults.h>


Pyrecodes::Pyrecodes(QWidget *parent)
  : SimCenterAppWidget(parent), resultWidget(0)
{
    //int windowWidth = 800;

    QGridLayout *mainLayout = new QGridLayout(this); // Set the parent to 'this'

    QWidget *mainWidget = new QWidget();
    // mainLayout->addWidget(mainWidget);

    QVBoxLayout *layout = new QVBoxLayout();
    mainWidget->setLayout(layout);


    int numRow = 0;

    // 2. Realization Selection
    realizationInputWidget = new LineEditSelectTool("Realization");
    mainLayout->addWidget(new QLabel("Damage Realization Selection:"), numRow, 0, 1, 1);
    mainLayout->addWidget(realizationInputWidget, numRow, 1, 1, 2);
    numRow++;

    // 4. Path To System Config File
    pathConfigFile = new SC_FileEdit("systemConfigFile");
    mainLayout->addWidget(new QLabel("Configuration File:"), numRow, 0, 1, 1);
    mainLayout->addWidget(pathConfigFile, numRow, 1, 1, 4);
    numRow++;

    // 5. Path to the Component library
    pathComponentLibrary = new SC_FileEdit("componentLibraryFile");
    mainLayout->addWidget(new QLabel("Component library File:"), numRow, 0, 1, 1);
    mainLayout->addWidget(pathComponentLibrary, numRow, 1, 1, 4);
    numRow++;
    
    // 6. Path to the Component library
    pathLocalityDefinition = new SC_FileEdit("localityGeojsonFile");
    mainLayout->addWidget(new QLabel("Locality Definition File:"), numRow, 0, 1, 1);
    mainLayout->addWidget(pathLocalityDefinition, numRow, 1, 1, 4);
    numRow++;

    mainLayout->setRowStretch(numRow, 1);
    //mainLayout->setColumnStretch(numRow, 1);

}


Pyrecodes::~Pyrecodes()
{

}


void Pyrecodes::clear(void)
{
    if (resultWidget != nullptr) {
        resultWidget->clear();
    }
}


bool Pyrecodes::inputFromJSON(QJsonObject &jsonObject)
{
  this->clear();
  
  realizationInputWidget->inputFromJSON(jsonObject);
  pathConfigFile->inputFromJSON(jsonObject);
  pathComponentLibrary->inputFromJSON(jsonObject);
  pathLocalityDefinition->inputFromJSON(jsonObject);
  
  return true;
}

bool Pyrecodes::outputToJSON(QJsonObject &jsonObject)
{
  jsonObject["Application"] = "Pyrecodes";

  realizationInputWidget->outputToJSON(jsonObject);
  pathConfigFile->outputToJSON(jsonObject);
  pathComponentLibrary->outputToJSON(jsonObject);
  pathLocalityDefinition->outputToJSON(jsonObject);
  
  return true;
}

bool Pyrecodes::outputAppDataToJSON(QJsonObject &jsonObject) {

    //
    // per API, need to add name of application to be called in AppLication
    // and all data to be used in ApplicationDate
    //

    jsonObject["Application"] = "Pyrecodes";
    QJsonObject dataObj;
    jsonObject["ApplicationData"] = dataObj;

    return true;
}

bool Pyrecodes::inputAppDataFromJSON(QJsonObject &jsonObject) {
    Q_UNUSED(jsonObject);
    return true;
}


bool Pyrecodes::copyFiles(QString &destDir) {

  realizationInputWidget->copyFile(destDir);
  pathConfigFile->copyFile(destDir);
  pathComponentLibrary->copyFile(destDir);
  pathLocalityDefinition->copyFile(destDir);

  return true;
}

bool Pyrecodes::outputCitation(QJsonObject &citation){
  QString pyrecodesCitationKey = "PyReCoDes";
  QJsonValue pyrecodesCitationValue( "\"citations\": [{\"citation\": \"Add\"}]}");
  citation.insert(pyrecodesCitationKey, pyrecodesCitationValue);
  
  return true;
}

SC_ResultsWidget* Pyrecodes::getResultsWidget(QWidget *parent, QWidget *R2DresWidget, QMap<QString, QList<QString>> assetTypeToType)
{
    if (resultWidget==nullptr){
        resultWidget = new RewetResults(parent);
    }
    return resultWidget;
}
