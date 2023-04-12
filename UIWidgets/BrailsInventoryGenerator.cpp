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

#include <qgsmapcanvas.h>
#include <qgsvectorlayer.h>
#include <qgslinesymbol.h>
#include <qgsfillsymbol.h>
#include <qgsrenderer.h>
#include <qgslayertreegroup.h>
#include <qgsmaptool.h>
#include <qgsmaptoolselectionhandler.h>
#include <qgsmaptoolextent.h>

#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QProgressBar>
#include <QPushButton>
#include <QFileDialog>
#include <QStandardPaths>
#include <QGroupBox>

BrailsInventoryGenerator::BrailsInventoryGenerator(VisualizationWidget* visWidget, QWidget *parent) : SimCenterAppWidget(parent)
{
    theVisualizationWidget = dynamic_cast<QGISVisualizationWidget*>(visWidget);
    assert(visWidget);

    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(5,0,0,0);

    layout->addWidget(this->getBrailsInventoryGenerator());


}


BrailsInventoryGenerator::~BrailsInventoryGenerator()
{

}


QStackedWidget* BrailsInventoryGenerator::getBrailsInventoryGenerator(void)
{
    theStackedWidget = new QStackedWidget();
    theStackedWidget->setContentsMargins(0,0,0,0);

    theStackedWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    //
    // file and dir input
    //

    fileInputWidget = new QWidget();
    fileInputWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    fileInputWidget->setContentsMargins(0,0,0,0);

    QVBoxLayout *mainLayout = new QVBoxLayout(fileInputWidget);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(4);

    QHBoxLayout* exportLayout = new QHBoxLayout();

    auto exportText = new QLabel("Export BRAILS file");

    exportPathLineEdit = new QLineEdit();
    exportPathLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QPushButton *exportBrowseFileButton = new QPushButton();
    exportBrowseFileButton->setText(tr("Browse"));
    exportBrowseFileButton->setMaximumWidth(150);

    exportLayout->addWidget(exportText);
    exportLayout->addWidget(exportPathLineEdit);
    exportLayout->addWidget(exportBrowseFileButton);

    mainLayout->addLayout(exportLayout);

    connect(exportBrowseFileButton,SIGNAL(clicked()),this,SLOT(chooseExportFileDialog()));

    QGroupBox* selectionGB = new QGroupBox("Selection Method");
    QHBoxLayout* selectionLayout = new QHBoxLayout(selectionGB);

    selectionGB->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

    QPushButton *rectangleSelectButton = new QPushButton(tr("Click to Select"));

    connect(rectangleSelectButton,SIGNAL(clicked()),this,SLOT(handleRectangleSelect()));

    selectionLayout->addWidget(rectangleSelectButton);

    QPushButton *clearSelection = new QPushButton(tr("Clear Selection"));
    clearSelection->setMaximumWidth(250);
    connect(clearSelection,SIGNAL(clicked()),this,SLOT(clearSelection()));
    selectionLayout->addWidget(clearSelection);


    QWidget* selectionWidget = new QWidget();
    QHBoxLayout* selectionWidgetLayout = new QHBoxLayout(selectionWidget);

    QPushButton *clearAllButton = new QPushButton(tr("Run BRAILS"));
    clearAllButton->setMaximumWidth(150);
    connect(clearAllButton,SIGNAL(clicked()),this,SLOT(handleSelectionDone()));

    selectionWidgetLayout->addWidget(clearAllButton);

    QHBoxLayout* topLayout = new QHBoxLayout();

    topLayout->addWidget(selectionGB);
    topLayout->addWidget(selectionWidget);

    mainLayout->addLayout(topLayout);

    auto mapView = theVisualizationWidget->getMapViewWidget("BrailsInventoryGenerator");
    mapViewSubWidget = std::unique_ptr<SimCenterMapcanvasWidget>(mapView);
    auto canvas = mapViewSubWidget->mapCanvas();
    extentTool = new QgsMapToolExtent(canvas);
    QObject::connect(extentTool,&QgsMapToolExtent::extentChanged,this,&BrailsInventoryGenerator::selectRectangle);


    // Enable the selection tool
    mapViewSubWidget->setShowPopUpOnSelection(false); // Do not show popup

    mainLayout->addWidget(mapViewSubWidget.get());

    //
    // progress bar
    //

    progressBarWidget = new QWidget();
    auto progressBarLayout = new QVBoxLayout(progressBarWidget);

    auto progressText = new QLabel("Running BRAILS. This may take a while.");
    progressLabel =  new QLabel("");
    progressBar = new QProgressBar();

    auto vspacer = new QSpacerItem(0,0,QSizePolicy::Minimum, QSizePolicy::Expanding);
    auto vspacer2 = new QSpacerItem(0,0,QSizePolicy::Minimum, QSizePolicy::Expanding);
    progressBarLayout->addItem(vspacer);
    progressBarLayout->addWidget(progressText,1, Qt::AlignCenter);
    progressBarLayout->addWidget(progressLabel,1, Qt::AlignCenter);
    progressBarLayout->addWidget(progressBar);
    progressBarLayout->addItem(vspacer2);
    //    progressBarLayout->addStretch(1);

    //
    // add file and progress widgets to stacked widgets, then set defaults
    //

    theStackedWidget->addWidget(fileInputWidget);
    theStackedWidget->addWidget(progressBarWidget);

    theStackedWidget->setCurrentWidget(fileInputWidget);

    return theStackedWidget;
}


void BrailsInventoryGenerator::showEvent(QShowEvent *e)
{
    auto mainCanvas = mapViewSubWidget->getMainCanvas();

    auto mainExtent = mainCanvas->extent();

    mapViewSubWidget->mapCanvas()->zoomToFeatureExtent(mainExtent);
    SimCenterAppWidget::showEvent(e);
}


void BrailsInventoryGenerator::chooseExportFileDialog(void)
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::AnyFile);

    QString oldPath;

    if(!exportPathLineEdit->text().isEmpty())
        oldPath = exportPathLineEdit->text();
    else
        oldPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

    auto exportPathfile = dialog.getSaveFileName(this, tr("Where to export new .xml file"), oldPath,".xml");

    if(!exportPathfile.endsWith(".xml"))
        exportPathfile += ".xml";

    dialog.close();

    // Set file name & entry in line edit
    exportPathLineEdit->setText(exportPathfile);

    return;
}


void BrailsInventoryGenerator::clear(void)
{
    exportPathLineEdit->clear();
    boundingPoints.clear();
}


void BrailsInventoryGenerator::clearSelection(void)
{
    boundingPoints.clear();
    auto mainCanvas = mapViewSubWidget->mapCanvas();
    mainCanvas->unsetMapTool(extentTool);
}


QgsRectangle BrailsInventoryGenerator::rectToLatLonCoordinates(QgsRectangle rect)
{
    try
    {
        auto canvas = mapViewSubWidget->mapCanvas();
        auto mapcrs = canvas->mapSettings().destinationCrs();

        const QgsCoordinateTransform coordTrans(mapcrs, QgsCoordinateReferenceSystem("EPSG:4326"), QgsProject::instance());
        if (coordTrans.isValid())
            rect = coordTrans.transform(rect);
    }
    catch (QgsCsException &cse)
    {
        errorMessage(QObject::tr("Transform error: %1").arg( cse.what() )+QObject::tr("CRS") );
    }

    return rect;
}


void BrailsInventoryGenerator::selectRectangle(const QgsRectangle &extent)
{
    auto transformedRect = this->rectToLatLonCoordinates(extent);

    // Gives the bottom-left and top-right points in string format
    boundingPoints = transformedRect.asWktCoordinates();

}


void BrailsInventoryGenerator::handleRectangleSelect(void)
{
    auto mainCanvas = mapViewSubWidget->mapCanvas();
    mainCanvas->setMapTool(extentTool);
}



void BrailsInventoryGenerator::handleSelectionDone(void)
{

    auto filePathToSave = exportPathLineEdit->text();

    if(filePathToSave.isEmpty())
    {
        this->statusMessage("Please select a file name/location to save new .xml file");
        return;
    }



}





