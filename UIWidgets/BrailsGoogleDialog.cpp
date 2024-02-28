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

// Written by: Frank McKenna, Barbaros Cetiner

#include "BrailsGoogleDialog.h"
#include <QGridLayout>
#include <QLabel>
#include <QDir>
#include <QFileInfo>
#include <QPushButton>
#include <QLineEdit>
#include <QSettings>
#include <QCheckBox>

#include "ModularPython.h"
#include "SimCenterPreferences.h"
#include "SC_IntLineEdit.h"

BrailsGoogleDialog::BrailsGoogleDialog(QWidget *parent)
  : QDialog(parent)
{
    this->setWindowTitle("Using Google API");
    auto layout = new QGridLayout(this);

    layout->addWidget(new QLabel("Google API Key"), 0, 0, 1, 2);
    auto apiKeyLabel =
      new QLabel("If you dont have a Google API key, you can <a href=\"https://developers.google.com/maps/documentation/embed/get-api-key#:~:text=Go%20to%20the%20Googlele%20Maps%20Platform%20%3E%20Credentials%20page.&text=On%20the%20Credentials%20page%2C%20click,your%20newly%20created%20API%20key\">follow the instructions</a>");
    apiKeyLabel->setTextFormat(Qt::RichText);
    apiKeyLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    apiKeyLabel->setOpenExternalLinks(true);
    layout->addWidget(apiKeyLabel, 1, 0, 1, 2);
    
    layout->addWidget(new QLabel("API Key"), 2, 0);

    // fill in key if already saved
    apiKey = new QLineEdit();
    apiKey->setEchoMode(QLineEdit::Password);
    layout->addWidget(apiKey, 2, 1);

    QSettings settings;
    auto googleAPISetting = settings.value("GoogleAPI");
    if(googleAPISetting.isValid() && !googleAPISetting.isNull())
        apiKey->setText(googleAPISetting.toString());
   
    //Save Password
    auto savePassword = new QPushButton("Save Key");
    layout->addWidget(savePassword,2,2);
    connect(savePassword,&QPushButton::clicked, this, [=]() {
      QSettings settings;
      settings.setValue("GoogleAPI", apiKey->text());
    });
    
    layout->addWidget(new QLabel("# Images"),3,0);
    numBuildings = new SC_IntLineEdit("numBuildings",10);
    layout->addWidget(numBuildings, 3,1);

    layout->addWidget(new QLabel("Seed"),4,0);
    seed = new SC_IntLineEdit("seed",10);
    layout->addWidget(seed, 4,1);

    QPushButton *runButton = new QPushButton(tr("Get Images and Process"));
    layout->addWidget(runButton, 5,0,1,3);
    connect(runButton,SIGNAL(clicked()),this,SLOT(startBrails()));
}

void
BrailsGoogleDialog::setData(BrailsData &theData){
  brailsData = theData;
}


void
BrailsGoogleDialog::startBrails(void){

  QString appDir = SimCenterPreferences::getInstance()->getAppDir();
  QDir scriptDir(appDir + QDir::separator());
  scriptDir.cd("applications");
  scriptDir.cd("tools");
  scriptDir.cd("BRAILS");
  QString brailsScript = scriptDir.absoluteFilePath("runBrails.py");  

  
  QStringList scriptArgs;
  scriptArgs << QString("--latMin")  << QString::number(brailsData.minLat)
	     << QString("--latMax")  << QString::number(brailsData.maxLat)
	     << QString("--longMin") << QString::number(brailsData.minLong)
	     << QString("--longMax") << QString::number(brailsData.maxLong)
         << QString("--location") << brailsData.location
         << QString("--fpSource") << brailsData.fpSource
         << QString("--fpAttrMap") << brailsData.fpSourceAttrMap
	     << QString("--lengthUnit") << brailsData.units 
	     << QString("--seed")    << QString::number(seed->getInt())
	     << "--numBuildings" << QString::number(numBuildings->getInt())
	     << "--googKey" << apiKey->text()
	     << "--outputFile" << brailsData.outputFile;
			      
  QFileInfo fileInfo(brailsData.outputFile);
  QString outputPath = fileInfo.absolutePath();
  QString fileName = fileInfo.baseName();
  qDebug() << "BRAILS script: " << brailsScript;
  qDebug() << "BRAILS args: " << scriptArgs;
  ModularPython *thePy = new ModularPython(outputPath);
  thePy->run(brailsScript,scriptArgs);

  this->hide();
}

void
BrailsGoogleDialog::handleBrailsFinished(){
  qDebug() << "Brails Finished!\n";
}
