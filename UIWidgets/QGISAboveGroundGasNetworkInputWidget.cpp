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

#include "QGISAboveGroundGasNetworkInputWidget.h"
#include "QGISVisualizationWidget.h"
#include "ComponentTableView.h"
#include "ComponentDatabaseManager.h"
#include "AssetFilterDelegate.h"
#include "LineAssetInputWidget.h"

#include "QFileInfo"

#include <qgsfield.h>
#include <qgsfillsymbol.h>
#include <qgsvectorlayer.h>
#include <qgsmarkersymbol.h>

QGISAboveGroundGasNetworkInputWidget::QGISAboveGroundGasNetworkInputWidget(QWidget *parent, VisualizationWidget* visWidget, QString assetType, QString appType) : PointAssetInputWidget(parent, visWidget, assetType, appType)
{
    QGISAboveGroundGasNetworkInputWidget::createComponentsBox();
}

void QGISAboveGroundGasNetworkInputWidget::createComponentsBox(void)
{
    QVBoxLayout* inputLayout = new QVBoxLayout();

    // box for LON and LAT headers in file
    // CPT data columns
    QGroupBox* locationWidget = new QGroupBox("Note: Headers to use for 'Latitude' and 'Longitude' in CSV files");
    QGridLayout* vboxLayout = new QGridLayout(locationWidget);
    QLabel* lonLabel = new QLabel("1. Header to use for longitude: LON");
    QLabel* latLabel = new QLabel("2. Header to use for latitude: LAT");
    vboxLayout->addWidget(lonLabel,0,0,Qt::AlignLeft);
    vboxLayout->addWidget(latLabel,1,0,Qt::AlignLeft);
    inputLayout->addWidget(locationWidget);

    auto insPoint = mainWidgetLayout->count();

    mainWidgetLayout->insertLayout(insPoint-3,inputLayout);
}


bool QGISAboveGroundGasNetworkInputWidget::inputFromJSON(QJsonObject &rvObject)
{
    return PointAssetInputWidget::inputFromJSON(rvObject);
}


bool QGISAboveGroundGasNetworkInputWidget::outputToJSON(QJsonObject &rvObject)
{
   auto res = PointAssetInputWidget::outputToJSON(rvObject);

   if(!res)
   {
       this->errorMessage("Error output to json in "+QString(__FUNCTION__));
       return false;
   }

   return true;
}


void QGISAboveGroundGasNetworkInputWidget::clear()
{
    PointAssetInputWidget::clear();
}

