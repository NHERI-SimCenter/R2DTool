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

#include "Pyrecodes3.h"
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QJsonObject>
#include <QFileDialog>
#include <QTabWidget>

#include <SC_FileEdit.h>
#include <SimCenterWidget.h>
#include <PyrecodesSystemConfig.h>
#include <PyrecodesComponentLibrary.h>

Pyrecodes3::Pyrecodes3(QWidget *parent)
   : SimCenterAppWidget(parent)
{
  QGridLayout *mainLayout = new QGridLayout(this); // Set the parent to 'this'
  this->setLayout(mainLayout);

  theComponentLibrary = new PyrecodesComponentLibrary();
  theSystemConfiguration = new PyrecodesSystemConfig();

  QTabWidget *theTabWidget = new QTabWidget();  
  theTabWidget->addTab(theComponentLibrary, "Componet Library");
  theTabWidget->addTab(theSystemConfiguration, "System Configuration");
  
  // label for the citation
  QLabel *citation = new QLabel("Users should cite this as follows: "
				"Blagojević, Nikola, and Stojadinović, Božidar. (2023). "
				"pyrecodes: an open-source library for regional recovery simulation and disaster resilience assessment of the built environment (v0.1.0). Chair of Structural Dynamics and Earthquake Engineering, ETH Zurich. https://doi.org/10.5905/ethz-1007-700");
  
  citation->setWordWrap(true);

  mainLayout->addWidget(theTabWidget, 0, 0, 1, 4);
  mainLayout->addWidget(citation,     1, 0, 1, 2);
  mainLayout->setRowStretch(0, 1);


  

  
}


Pyrecodes3::~Pyrecodes3()
{

}


void Pyrecodes3::clear(void)
{

}


bool Pyrecodes3::inputFromJSON(QJsonObject &jsonObject)
{
  Q_UNUSED(jsonObject);
  return true;  
}


bool Pyrecodes3::outputToJSON(QJsonObject &jsonObject)
{
  jsonObject["Application"] = "Pyrecodes3";
    
  return true;
}

bool Pyrecodes3::outputAppDataToJSON(QJsonObject &jsonObject) {

  jsonObject["Application"] = "Pyrecodes3";
  QJsonObject dataObj;
  jsonObject["ApplicationData"] = dataObj;
  
  return true;
}

bool Pyrecodes3::inputAppDataFromJSON(QJsonObject &jsonObject) {
  Q_UNUSED(jsonObject);    
  return true;
}


bool Pyrecodes3::copyFiles(QString &destDir) {

  return true;
}

bool Pyrecodes3::outputCitation(QJsonObject &citation){
  citation.insert("PyReCoDes",QString("Blagojević, Nikola, and Stojadinović, Božidar. (2023). pyrecodes: an open-source library for regional recovery simulation and disaster resilience assessment of the built environment (v0.1.0). Chair of Structural Dynamics and Earthquake Engineering, ETH Zurich. https://doi.org/10.5905/ethz-1007-700"));
  
  return true;
}

SC_ResultsWidget* Pyrecodes3::getResultsWidget(QWidget *parent, QWidget *R2DresWidget, QMap<QString, QList<QString>> assetTypeToType)
{

  qDebug() << "ERROR: SC_ResultsWidget* Pyrecodes3::getResultsWidget()";
  return 0;
  
}
