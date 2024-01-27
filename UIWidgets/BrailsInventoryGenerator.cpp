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

// Written by: Stevan Gavrilovic

#include "BrailsInventoryGenerator.h"
#include "QGISVisualizationWidget.h"
#include "SimCenterMapcanvasWidget.h"
#include "GIS_Selection.h"
#include <qgsmapcanvas.h>
#include <PlainRectangle.h>
#include <BrailsGoogleDialog.h>
//#include <PythonProcessHandler.h>
#include <SimCenterPreferences.h>

#include <QLabel>
#include <QPushButton>
#include <QStandardPaths>
#include <QGroupBox>
#include <QGridLayout>
#include <SC_DoubleLineEdit.h>
#include <QComboBox>
#include <SC_FileEdit.h>
#include <SC_ComboBox.h>
#include <SC_IntLineEdit.h>
#include <SC_DoubleLineEdit.h>
#include <QSettings>

BrailsInventoryGenerator::BrailsInventoryGenerator(VisualizationWidget* visWidget, QWidget *parent) : SimCenterAppWidget(parent)
{
    theVisualizationWidget = dynamic_cast<QGISVisualizationWidget*>(visWidget);
    assert(visWidget);
    
    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);

    minLat  = new SC_DoubleLineEdit("minLat",0.0);
    maxLat  = new SC_DoubleLineEdit("maxLat",0.0);
    minLong = new SC_DoubleLineEdit("minLong",0.0);
    maxLong = new SC_DoubleLineEdit("maxLong",0.0);    
    theOutputFile = new SC_FileEdit("outputFile");

    QString appDir = SimCenterPreferences::getInstance()->getLocalWorkDir();
    QString brailsDir = appDir + QDir::separator() + QString("brails");
    QDir dir(brailsDir);
    if (!dir.exists())
      dir.mkpath(brailsDir);

    QString brailsOutput = brailsDir + QDir::separator() + "inventory.geojson";
    theOutputFile->setFilename(brailsOutput);
    
    QGridLayout *mainLayout = new QGridLayout(this);

    int numRow = 0;
    QStringList unitList; unitList << "m" << "ft";    
    units = new SC_ComboBox("units", unitList);
    mainLayout->addWidget(new QLabel("Output Units"),numRow,0);    
    mainLayout->addWidget(units,numRow,1);
    
    numRow++;
    mainLayout->addWidget(new QLabel("Latitude:"),numRow,0);
    numRow++;
    mainLayout->addWidget(new QLabel("min:"),numRow,0);
    mainLayout->addWidget(minLat,numRow,1);    
    mainLayout->addWidget(new QLabel("max:"),numRow,2);
    mainLayout->addWidget(maxLat,numRow,3);

    numRow = 1;    
    mainLayout->addWidget(new QLabel("Longitude:"),numRow,4);
    numRow = 2;
    mainLayout->addWidget(new QLabel("min:"),numRow,4);
    mainLayout->addWidget(minLong,numRow,5);    
    mainLayout->addWidget(new QLabel("max:"),numRow,6);
    mainLayout->addWidget(maxLong,numRow,7);

    numRow++;
    
    mainLayout->addWidget(new QLabel("Export BRAILS file"),numRow,0);
    mainLayout->addWidget(theOutputFile,numRow,1,1,7);

    numRow++;
    mainLayout->addWidget(new QLabel("Image Source"),numRow,0);
    QComboBox *imageSourceCombo = new QComboBox();
    imageSourceCombo->addItem("Google");
    imageSourceCombo->addItem("Local");
    imageSourceCombo->addItem("NHERI DesignSafe");
    imageSourceCombo->addItem("NHERI Rapid");
    mainLayout->addWidget(imageSourceCombo,numRow,1,1,3);
    
    connect(imageSourceCombo, &QComboBox::currentTextChanged, this, [=](QString text) {
      imageSource = text;
    });
    
    mainLayout->addWidget(new QLabel("Imputation Algorithm"),4,0);
    QComboBox *imputationAlgoCombo = new QComboBox();
    imputationAlgoCombo->addItem("None");    
    mainLayout->addWidget(imputationAlgoCombo,4,1,1,3);

    connect(imputationAlgoCombo, &QComboBox::currentTextChanged, this, [=](QString text) {
      imputationAlgo = text;
    });

    QPushButton *runButton = new QPushButton(tr("Run BRAILS"));
    mainLayout->addWidget(runButton, 5,0,1,8);
    connect(runButton,SIGNAL(clicked()),this,SLOT(runBRAILS()));    
    
    theSelectionWidget = new GIS_Selection(theVisualizationWidget);
    mainLayout->addWidget(theSelectionWidget,6,0,1,8);
    connect(theSelectionWidget,SIGNAL(selectionGeometryChanged()), this, SLOT(coordsChanged()));

    // set current selections
    imageSource=imageSourceCombo->currentText();
    imputationAlgo=imputationAlgoCombo->currentText();
}


BrailsInventoryGenerator::~BrailsInventoryGenerator()
{

}

void BrailsInventoryGenerator::clear(void)
{

}

void BrailsInventoryGenerator::runBRAILS(void)
{

  // make sure output dir exists
  QFileInfo fileInfo(theOutputFile->getFilename());
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
  brailsData.outputFile =theOutputFile->getFilename();
  brailsData.imageSource = imageSource;
  brailsData.imputationAlgo = imputationAlgo;
  brailsData.units = units->currentText();

  
  if (imageSource == "Google") {
    errorMessage("Starting Brails .. ");
    if (theGoogleDialog == 0) {
      theGoogleDialog = new BrailsGoogleDialog(this);
    }
    theGoogleDialog->setData(brailsData);
    theGoogleDialog->show();
    theGoogleDialog->raise();
    theGoogleDialog->activateWindow();
    errorMessage("Window shown and activated .. ");    
  }
}



void BrailsInventoryGenerator::coordsChanged(void)
{
  QVector<double> points = theSelectionWidget->getSelectedPoints();
  minLat->setText(QString::number(points.at(0)));
  minLong->setText(QString::number(points.at(1)));
  maxLat->setText(QString::number(points.at(6)));
  maxLong->setText(QString::number(points.at(7)));    
}





