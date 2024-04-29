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
#include "BrailsInventoryGenerator.h"
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

BrailsGoogleDialog::BrailsGoogleDialog(BrailsInventoryGenerator* parent)
    : QDialog(parent), parent(parent)
{
	// Initiate a grid layout:
	this->setWindowTitle("Using Google Imagery");
	auto layout = new QGridLayout(this);

	// Add the text and help elements for Google API key:
	layout->addWidget(new QLabel("Google API Key"), 2, 0);
	HelpWidget* helpWidget = new HelpWidget();
	layout->addWidget(helpWidget, 2, 1);

	// Fill in API key if previously saved:
	apiKey = new QLineEdit();
	apiKey->setEchoMode(QLineEdit::Password);
	layout->addWidget(apiKey, 2, 2);
	QSettings settings;
	auto googleAPISetting = settings.value("GoogleAPI");
	if (googleAPISetting.isValid() && !googleAPISetting.isNull())
		apiKey->setText(googleAPISetting.toString());

	// Create a push button to save Google API key:
	auto savePassword = new QPushButton("Save Key");
	layout->addWidget(savePassword, 2, 3);
	connect(savePassword, &QPushButton::clicked, this, [=]() {
		QSettings settings;
		settings.setValue("GoogleAPI", apiKey->text());
		});

	// Define the row prompting number of buildings:
	layout->addWidget(new QLabel("Number of buildings"), 3, 0);
	numBuildings = new SC_IntLineEdit("numBuildings", 100);
	layout->addWidget(numBuildings, 3, 2);
	QCheckBox* numBldgCheckbox = new QCheckBox("All buildings", this);
	layout->addWidget(numBldgCheckbox, 3, 3);
    getAllBuildings = false;
    connect(numBldgCheckbox, SIGNAL(clicked(bool)), this, SLOT(disableNumBuildings(bool)));

	// Define the row prompting random seed:
	seedLabel = new QLabel("Random seed");
	layout->addWidget(seedLabel, 4, 0);
	seed = new SC_IntLineEdit("seed", 7);
	layout->addWidget(seed, 4, 2);

	// Create a push button to run BRAILS:
	QPushButton* runButton = new QPushButton(tr("Get building inventory"));
	layout->addWidget(runButton, 5, 0, 1, 4);
	connect(runButton, SIGNAL(clicked()), this, SLOT(startBrails()));
}

void
BrailsGoogleDialog::setData(BrailsData& theData) {
	brailsData = theData;
}

void
BrailsGoogleDialog::disableNumBuildings(bool checked) {
	if (checked) {
		numBuildings->setEnabled(false);
		seedLabel->setVisible(false);
		seed->setVisible(false);
		getAllBuildings = true;
	}
	else {
		numBuildings->setEnabled(true);
		seedLabel->setVisible(true);
		seed->setVisible(true);
		getAllBuildings = false;
	}
}

void
BrailsGoogleDialog::startBrails(void) {

	QString appDir = SimCenterPreferences::getInstance()->getAppDir();
	QDir scriptDir(appDir + QDir::separator());
	scriptDir.cd("applications");
	scriptDir.cd("tools");
	scriptDir.cd("BRAILS");
	QString brailsScript = scriptDir.absoluteFilePath("runBrails.py");

    if (brailsData.fpSourceAttrMap.isEmpty()){
        brailsData.fpSourceAttrMap = "\"\"";
        }
    if (brailsData.invAttributeMap.isEmpty()){
        brailsData.invAttributeMap = "\"\"";
    }
    if (brailsData.location.isEmpty()){
        brailsData.location = "\"\"";
    }
	QStringList scriptArgs;
    scriptArgs << QString("--latMin") << QString::number(brailsData.minLat)
               << QString("--latMax") << QString::number(brailsData.maxLat)
               << QString("--longMin") << QString::number(brailsData.minLong)
               << QString("--longMax") << QString::number(brailsData.maxLong)
               << QString("--lengthUnit") << brailsData.units
               << QString("--location") << brailsData.location
               << QString("--fpSource") << brailsData.fpSource
               << QString("--fpAttrMap") << brailsData.fpSourceAttrMap
               << QString("--invInput") << brailsData.invInput
               << QString("--invAttributeMap") << brailsData.invAttributeMap
               << QString("--attrRequested") << brailsData.attrRequested
               << "--outputFile" << brailsData.outputFile
               << QString("--seed") << QString::number(seed->getInt())
               << "--numBuildings" << QString::number(numBuildings->getInt())
               << "--getAllBuildings" << QString::number(getAllBuildings)
               << "--googKey" << apiKey->text();

	QFileInfo fileInfo(brailsData.outputFile);
	QString outputPath = fileInfo.absolutePath();
	qDebug() << "BRAILS script: " << brailsScript;
	qDebug() << "BRAILS args: " << scriptArgs;
	ModularPython* thePy = new ModularPython(outputPath);
	thePy->run(brailsScript, scriptArgs);
    parent->loadVectorLayer(brailsData.outputFile, "BRAILSGeneratedInventory");
}

void
BrailsGoogleDialog::handleBrailsFinished() {
	qDebug() << "BRAILS successfully generated the requested building inventory!\n";
}
