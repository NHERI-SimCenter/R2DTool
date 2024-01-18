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

#include "RuptureWidget.h"
#include "UCERF2Widget.h"
#include "MeanUCERFWidget.h"
#include "SC_DoubleLineEdit.h"

#include <QVBoxLayout>
#include <QJsonObject>
#include <QStackedWidget>
#include <QGroupBox>
#include <QComboBox>

RuptureWidget::RuptureWidget(QString jsonKey, QWidget *parent) : SimCenterAppSelection("OpenSHA",jsonKey,parent), jsonKey(jsonKey)
{
    this->setContentsMargins(0,0,0,0);

    ucerfWidget = new UCERF2Widget();
    meanUcerfWidget = new MeanUCERFWidget("ModelParameters");

    ucerfWidget->setObjectName("WGCEP (2007) UCERF2 - Single Branch");
    meanUcerfWidget->setObjectName("Mean UCERF3");

    this->addComponent(ucerfWidget->objectName(),"UCERF2",ucerfWidget);
    this->addComponent(meanUcerfWidget->objectName(),"MEANUCERF2",meanUcerfWidget);

    auto maxDistLabel = new QLabel("Maximum Distance (km)");
    auto maxMagLabel= new QLabel("Maximum Magnitude");
    auto minMagLabel= new QLabel("Minimum Magnitude");

    maxDistLE = new SC_DoubleLineEdit("max_Dist", 200.0);
    maxMagLE= new SC_DoubleLineEdit("max_Mag", 10.0);
    minMagLE= new SC_DoubleLineEdit("min_Mag", 2.5);

    // Turn off max width
    maxDistLE->setMaximumWidth(QWIDGETSIZE_MAX);
    maxMagLE->setMaximumWidth(QWIDGETSIZE_MAX);
    minMagLE->setMaximumWidth(QWIDGETSIZE_MAX);

    auto gridLayout = new QGridLayout();
    gridLayout->setContentsMargins(0,5,0,5);
    gridLayout->addWidget(maxDistLabel,0,0);
    gridLayout->addWidget(maxDistLE,0,1);
    gridLayout->addWidget(maxMagLabel,1,0);
    gridLayout->addWidget(maxMagLE,1,1);
    gridLayout->addWidget(minMagLabel,2,0);
    gridLayout->addWidget(minMagLE,2,1);

    auto thisLayout = qobject_cast<QVBoxLayout*>(this->layout());
    thisLayout->addLayout(gridLayout);

}


bool RuptureWidget::outputToJSON(QJsonObject &jsonObject)
{
    if(!SimCenterAppSelection::outputToJSON(jsonObject))
        return false;

    auto jsonKeyObj = jsonObject[jsonKey].toObject();

    jsonKeyObj["Type"] = "ERF";

    maxDistLE->outputToJSON(jsonKeyObj);
    maxMagLE->outputToJSON(jsonKeyObj);
    minMagLE->outputToJSON(jsonKeyObj);

    jsonObject[jsonKey] = jsonKeyObj;

    return true;
}


bool RuptureWidget::inputFromJSON(QJsonObject &/*jsonObject*/)
{
    return true;
}

