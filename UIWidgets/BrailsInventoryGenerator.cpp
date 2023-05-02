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

#include <QLabel>
#include <QPushButton>
#include <QStandardPaths>
#include <QGroupBox>
#include <QGridLayout>
#include <SC_DoubleLineEdit.h>
#include <QComboBox>
#include <SC_FileEdit.h>

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
    
    mainLayout->addWidget(new QLabel("Export BRAILS file"),2,0);
    mainLayout->addWidget(theOutputFile,2,1,1,7);

    mainLayout->addWidget(new QLabel("Image Source"),3,0);
    QComboBox *imageSourceCombo = new QComboBox();
    imageSourceCombo->addItem("Google");
    imageSourceCombo->addItem("Local");
    imageSourceCombo->addItem("NHERI DesignSafe");
    imageSourceCombo->addItem("NHERI Rapid");
    mainLayout->addWidget(imageSourceCombo,3,1,1,3);

    connect(imageSourceCombo, &QComboBox::currentTextChanged, this, [=](QString text) {
      imageSource = text;
    });
    
    mainLayout->addWidget(new QLabel("Imputation Algorithm"),4,0);
    QComboBox *fillUndefined = new QComboBox();
    fillUndefined->addItem("None");    
    fillUndefined->addItem("Sang-ri");
    fillUndefined->addItem("Aakash");
    fillUndefined->addItem("Dimitrios");
    mainLayout->addWidget(fillUndefined,4,1,1,3);    
    
    QPushButton *runButton = new QPushButton(tr("Run BRAILS"));
    mainLayout->addWidget(runButton, 5,0,1,8);
    connect(runButton,SIGNAL(clicked()),this,SLOT(runBRAILS()));

    connect(fillUndefined, &QComboBox::currentTextChanged, this, [=](QString text) {
      fillAlgorithm = text;
    });
    
    /*
    auto mapView = theVisualizationWidget->getMapViewWidget("GIS_Selection");
    mapViewSubWidget = std::unique_ptr<SimCenterMapcanvasWidget>(mapView);
    QgsMapCanvas *mapCanvas = mapViewSubWidget->mapCanvas();
    */

    theSelectionWidget = new GIS_Selection(theVisualizationWidget);
    mainLayout->addWidget(theSelectionWidget,6,0,1,8);
    
    // connect(theTool, &PlainRectangle::geometryChanged, this, &GIS_Selection::handleSelectionGeometryChange);
    connect(theSelectionWidget,SIGNAL(selectionGeometryChanged()), this, SLOT(coordsChanged()));
}


BrailsInventoryGenerator::~BrailsInventoryGenerator()
{

}

void BrailsInventoryGenerator::clear(void)
{

}


void BrailsInventoryGenerator::runBRAILS(void)
{

}



void BrailsInventoryGenerator::coordsChanged(void)
{
  QVector<double> points = theSelectionWidget->getSelectedPoints();
  qDebug() << "BRAILS_INVENTORY::coordsChanged " << points;
  minLat->setText(QString::number(points.at(0)));
  minLong->setText(QString::number(points.at(1)));
  maxLat->setText(QString::number(points.at(6)));
  maxLong->setText(QString::number(points.at(7)));  
  
}





