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

// Written by: Barbaros Cetiner & Stevan Gavrilovic

#include "BrailsTranspInventoryGenerator.h"
#include "QGISVisualizationWidget.h"
#include "SimCenterMapcanvasWidget.h"
#include "GIS_Selection.h"
#include <qgsmapcanvas.h>
#include <PlainRectangle.h>
#include <BrailsGoogleDialog.h>
#include <SimCenterPreferences.h>
#include "ModularPython.h"

#include <QLabel>
#include <QPushButton>
#include <QStandardPaths>
#include <QGroupBox>
#include <QGridLayout>
#include <SC_DoubleLineEdit.h>
#include <QComboBox>
#include <SC_FileEdit.h>
#include <SC_IntLineEdit.h>
#include <SC_DoubleLineEdit.h>
#include <QSettings>

BrailsTranspInventoryGenerator::BrailsTranspInventoryGenerator(QGISVisualizationWidget* visWidget, QWidget *parent) : SimCenterAppWidget(parent), theVisualizationWidget(visWidget)
{   
    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);

    minLat  = new SC_DoubleLineEdit("minLat",0.0);
    maxLat  = new SC_DoubleLineEdit("maxLat",0.0);
    minLong = new SC_DoubleLineEdit("minLong",0.0);
    maxLong = new SC_DoubleLineEdit("maxLong",0.0);    
    theOutputDir = new SC_FileEdit("outputDirectory");

    QString appDir = SimCenterPreferences::getInstance()->getLocalWorkDir();
    QString brailsDir = appDir + QDir::separator() + QString("brails");
    QDir dir(brailsDir);
    if (!dir.exists())
      dir.mkpath(brailsDir);

    theOutputDir->setFilename(brailsDir);
    
    QGridLayout *mainLayout = new QGridLayout(this);
    mainLayout->addWidget(new QLabel("Latitude:"),0,0);
    mainLayout->addWidget(new QLabel("min:"),1,0);
    mainLayout->addWidget(minLat,1,1);    
    mainLayout->addWidget(new QLabel("max:"),1,2);
    mainLayout->addWidget(maxLat,1,3);

    mainLayout->addWidget(new QLabel("Longitude:"),0,4);
    mainLayout->addWidget(new QLabel("min:"),1,4);
    mainLayout->addWidget(minLong,1,5);    
    mainLayout->addWidget(new QLabel("max:"),1,6);
    mainLayout->addWidget(maxLong,1,7);
    
    mainLayout->addWidget(new QLabel("BRAILS output directory"),2,0);
    mainLayout->addWidget(theOutputDir,2,1,1,7);
    
    QPushButton *runButton = new QPushButton(tr("Run BRAILS"));
    mainLayout->addWidget(runButton, 5,0,1,8);
    connect(runButton,SIGNAL(clicked()),this,SLOT(runBRAILS()));    
    
    theSelectionWidget = new GIS_Selection(theVisualizationWidget);
    mainLayout->addWidget(theSelectionWidget,6,0,1,8);
    connect(theSelectionWidget,SIGNAL(selectionGeometryChanged()), this, SLOT(coordsChanged()));
}

BrailsTranspInventoryGenerator::~BrailsTranspInventoryGenerator()
{

}

void BrailsTranspInventoryGenerator::clear(void)
{

}

void BrailsTranspInventoryGenerator::runBRAILS(void)
{
  // make sure output dir exists
  QFileInfo fileInfo(theOutputDir->getFilename());
  QString outputPath = fileInfo.absolutePath();
  QString fileName = fileInfo.baseName();  
  QDir dir(outputPath);
    if (!dir.exists())
      dir.mkpath(outputPath);
    
  BrailsData brailsData;
  brailsData.minLat = minLat->getDouble();
  brailsData.maxLat = maxLat->getDouble();
  brailsData.minLong = minLong->getDouble();
  brailsData.maxLong = maxLong->getDouble();
  brailsData.outputFile =theOutputDir->getFilename();

  QString appDir = SimCenterPreferences::getInstance()->getAppDir();
  QDir scriptDir(appDir + QDir::separator());
  scriptDir.cd("applications");
  scriptDir.cd("tools");
  scriptDir.cd("BRAILS");
  QString brailsScript = scriptDir.absoluteFilePath("runBrailsTransp.py");  

  
  QStringList scriptArgs;
  scriptArgs << QString("--latMin")  << QString::number(brailsData.minLat)
	     << QString("--latMax")  << QString::number(brailsData.maxLat)
	     << QString("--longMin") << QString::number(brailsData.minLong)
	     << QString("--longMax") << QString::number(brailsData.maxLong)
	     << "--outputFile" << brailsData.outputFile;

  qDebug() << "BRAILS script: " << brailsScript;
  qDebug() << "BRAILS args: " << scriptArgs;
  ModularPython *thePy = new ModularPython(outputPath);
  thePy->run(brailsScript,scriptArgs);

  this->hide();		 
}

void BrailsTranspInventoryGenerator::coordsChanged(void)
{
  QVector<double> points = theSelectionWidget->getSelectedPoints();
  minLat->setText(QString::number(points.at(0)));
  minLong->setText(QString::number(points.at(1)));
  maxLat->setText(QString::number(points.at(6)));
  maxLong->setText(QString::number(points.at(7)));    
}

