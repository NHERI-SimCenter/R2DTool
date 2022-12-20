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
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
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

#include "GISAboveGroundGasComponentInputWidget.h"
#include "QGISVisualizationWidget.h"
#include "GISAssetInputWidget.h"

#include <qgslinesymbol.h>
#include <qgsmarkersymbol.h>

#include <QFileDialog>
#include <QSplitter>
#include <QGroupBox>
#include <QVBoxLayout>

#ifdef OpenSRA
#include "WorkflowAppOpenSRA.h"
#include "WidgetFactory.h"
#include "JsonGroupBoxWidget.h"
#endif


GISAboveGroundGasComponentInputWidget::GISAboveGroundGasComponentInputWidget(QWidget *parent, VisualizationWidget* visWidget) : SimCenterAppWidget(parent)
{
    theVisualizationWidget = static_cast<QGISVisualizationWidget*>(visWidget);
    assert(theVisualizationWidget);

    theAboveGroundWidget = new GISAssetInputWidget(this, theVisualizationWidget, "Above Ground Gas Infrastructure");

    theAboveGroundWidget->setLabel1("Load above ground component information from a GIS file");

    connect(theAboveGroundWidget,&GISAssetInputWidget::doneLoadingComponents,this,&GISAboveGroundGasComponentInputWidget::handleAssetsLoaded);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    mainLayout->addWidget(theAboveGroundWidget);
}


GISAboveGroundGasComponentInputWidget::~GISAboveGroundGasComponentInputWidget()
{

}


bool GISAboveGroundGasComponentInputWidget::copyFiles(QString &destName)
{

    // The file containing the above ground infrastructure
    auto res = theAboveGroundWidget->copyFiles(destName);

    return res;
}

#ifdef OpenSRA

bool GISAboveGroundGasComponentInputWidget::outputToJSON(QJsonObject &rvObject)
{
    return theAboveGroundWidget->outputToJSON(rvObject);
}


bool GISAboveGroundGasComponentInputWidget::inputFromJSON(QJsonObject &rvObject)
{

    return theAboveGroundWidget->inputFromJSON(rvObject);
}

#endif


bool GISAboveGroundGasComponentInputWidget::outputAppDataToJSON(QJsonObject &jsonObject)
{
    jsonObject["Application"]="GIS_to_ABOVE_GROUND";

    QJsonObject data;

    // The file containing the network pipelines
    theAboveGroundWidget->outputAppDataToJSON(data);

    jsonObject["ApplicationData"] = data;

    return true;
}


bool GISAboveGroundGasComponentInputWidget::inputAppDataFromJSON(QJsonObject &jsonObject)
{

    // Check the app type
    if (jsonObject.contains("Application")) {
        if ("GIS_to_ABOVE_GROUND" != jsonObject["Application"].toString()) {
            this->errorMessage("GISAboveGroundGasComponentInputWidget::inputFromJSON app name conflict");
            return false;
        }
    }


    if (!jsonObject.contains("ApplicationData"))
    {
        this->errorMessage("GISAboveGroundGasComponentInputWidget::inputFromJSON app name conflict");
        return false;
    }

    return true;
}


int GISAboveGroundGasComponentInputWidget::loadAboveGroundVisualization()
{
    aboveGroundMainLayer = theAboveGroundWidget->getMainLayer();

    if(aboveGroundMainLayer==nullptr)
        return -1;


    QgsMarkerSymbol* markerSymbol = new QgsMarkerSymbol();

    markerSymbol->setColor(Qt::darkBlue);
    theVisualizationWidget->createSimpleRenderer(markerSymbol,aboveGroundMainLayer);

    //    auto numFeat = mainLayer->featureCount();

    theVisualizationWidget->zoomToLayer(aboveGroundMainLayer);

    auto tableHeadings = aboveGroundMainLayer->fields().names();

    emit headingValuesChanged(tableHeadings);

    return 0;
}


void GISAboveGroundGasComponentInputWidget::clear()
{
    theAboveGroundWidget->clear();

    emit headingValuesChanged(QStringList{"N/A"});
}


void GISAboveGroundGasComponentInputWidget::handleAssetsLoaded()
{
    if(theAboveGroundWidget->isEmpty())
        return;

    auto res = this->loadAboveGroundVisualization();

    if(res != 0)
    {
        this->errorMessage("Error, failed to load the wells and caprocks visualization");
        return;
    }

}

