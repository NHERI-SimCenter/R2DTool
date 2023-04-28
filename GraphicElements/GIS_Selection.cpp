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

#include "GIS_Selection.h"
#include "QGISVisualizationWidget.h"
#include "SimCenterMapcanvasWidget.h"
#include <PlainRectangle.h>
#include "GridNode.h"
#include <qgsmapcanvas.h>
/*
#include <qgsvectorlayer.h>
#include <qgslinesymbol.h>
#include <qgsfillsymbol.h>
#include <qgsrenderer.h>
#include <qgslayertreegroup.h>
*/

#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QProgressBar>
#include <QPushButton>
#include <QFileDialog>
#include <QStandardPaths>
#include <QGroupBox>

GIS_Selection::GIS_Selection(VisualizationWidget* visWidget, QWidget *parent) : SimCenterAppWidget(parent)
{
    theVisualizationWidget = dynamic_cast<QGISVisualizationWidget*>(visWidget);
    assert(visWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(4);

    QGroupBox* selectionGB = new QGroupBox("Selection Method");
    QHBoxLayout* selectionLayout = new QHBoxLayout(selectionGB);

    selectionGB->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

    //QPushButton *radiusSelectButton = new QPushButton(tr("Radius"));
    QPushButton *rectangleSelectButton = new QPushButton(tr("Rectangle"));
    //QPushButton *freehandSelectButton = new QPushButton(tr("Freehand"));
    //QPushButton *polygonSelectButton = new QPushButton(tr("Polygon"));

    //QPushButton *noneSelectButton = new QPushButton(tr("None (Pan)"));

    //connect(radiusSelectButton,SIGNAL(clicked()),this,SLOT(handleRadiusSelect()));
    connect(rectangleSelectButton,SIGNAL(clicked()),this,SLOT(handleRectangleSelect()));
    //connect(freehandSelectButton,SIGNAL(clicked()),this,SLOT(handleFreehandSelect()));
    //connect(noneSelectButton,SIGNAL(clicked()),this,SLOT(handleNoneSelect()));
    //connect(polygonSelectButton,SIGNAL(clicked()),this,SLOT(handlePolygonSelect()));

    
    //selectionLayout->addWidget(polygonSelectButton);
    selectionLayout->addWidget(rectangleSelectButton);
    //selectionLayout->addWidget(radiusSelectButton);
    //selectionLayout->addWidget(freehandSelectButton);
    //selectionLayout->addWidget(noneSelectButton);

    QPushButton *clearAllButton = new QPushButton(tr("Clear All"));
    clearAllButton->setMaximumWidth(150);
    connect(clearAllButton,SIGNAL(clicked()),this,SLOT(clear()));


    QPushButton *clearSelection = new QPushButton(tr("Clear Selection"));
    clearSelection->setMaximumWidth(150);
    connect(clearSelection,SIGNAL(clicked()),this,SLOT(clearSelection()));

    selectionLayout->addWidget(clearSelection);
    selectionLayout->addWidget(clearAllButton);

    mainLayout->addWidget(selectionGB);

    auto mapView = theVisualizationWidget->getMapViewWidget("GIS_Selection");
    mapViewSubWidget = std::unique_ptr<SimCenterMapcanvasWidget>(mapView);
    mainLayout->addWidget(mapViewSubWidget.get());

    this->setLayout(mainLayout);
}


GIS_Selection::~GIS_Selection()
{

}

void GIS_Selection::showEvent(QShowEvent *e)
{
    auto mainCanvas = mapViewSubWidget->getMainCanvas();
    auto mainExtent = mainCanvas->extent();

    mapViewSubWidget->mapCanvas()->zoomToFeatureExtent(mainExtent);
    SimCenterAppWidget::showEvent(e);
}


void GIS_Selection::clear(void)
{

}


void GIS_Selection::clearSelection(void)
{

}

void GIS_Selection::handleSelectionDone(void) {

}

void GIS_Selection::handleRectangleSelect(void)
{
    QgsMapCanvas *mapCanvas = mapViewSubWidget->mapCanvas();

    if (userGrid == 0) {
      userGrid = new PlainRectangle(mapCanvas);
      
      mapCanvas->setMapTool(userGrid);
      userGrid->createGrid();
      
      userGrid->setVisualizationWidget(theVisualizationWidget);
      
      // if we change userGrid or map changes location (can only seem to capture a refresh!) redo geometry
      connect(userGrid, &PlainRectangle::geometryChanged, this, &GIS_Selection::handleSelectionGeometryChange);
      connect(mapCanvas, &QgsMapCanvas::mapCanvasRefreshed, this, &GIS_Selection::handleSelectionGeometryChange);

    }
    
    userGrid->show();
    handleSelectionGeometryChange();
}

/*
void GIS_Selection::handleRadiusSelect(void)
{
    mapViewSubWidget->enableRadiusSelectionTool();
}

void GIS_Selection::handleFreehandSelect(void)
{
    mapViewSubWidget->enableFreehandSelectionTool();
}


void GIS_Selection::handleNoneSelect(void)
{
    mapViewSubWidget->enablePanTool();
}


void GIS_Selection::handlePolygonSelect(void)
{
    mapViewSubWidget->enablePolygonSelectionTool();
}
*/

QVector<double>
GIS_Selection::getSelectedPoints(void){
  return selectedPoints;
}

void
GIS_Selection::handleSelectionGeometryChange(void)
{
    selectedPoints.clear();
    
    if (userGrid != 0) {
      
      auto gridNodeVec = userGrid->getGridNodeVec();
      QgsMapCanvas *mapCanvas = mapViewSubWidget->mapCanvas();

        for(int i = 0; i<gridNodeVec.size(); ++i)
        {

            auto gridNode = gridNodeVec.at(i);
            auto screenPoint = gridNode->getPoint();

            // The latitude and longitude
            auto longitude = theVisualizationWidget->getLongFromScreenPoint(screenPoint,mapCanvas);
            auto latitude = theVisualizationWidget->getLatFromScreenPoint(screenPoint,mapCanvas);
            selectedPoints.append(latitude);
            selectedPoints.append(longitude);
             qDebug() << i << " " << latitude << " " << longitude;
        }

        emit selectionGeometryChanged();
    }
}









