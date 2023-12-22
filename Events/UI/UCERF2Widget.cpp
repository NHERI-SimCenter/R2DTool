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

#include "UCERF2Widget.h"
#include "SC_ComboBox.h"
#include "SC_DoubleLineEdit.h"
#include "SC_CheckBox.h"

#include <QJsonObject>
#include <QGridLayout>
#include <QLabel>

UCERF2Widget::UCERF2Widget(QWidget *parent) : SimCenterAppWidget(parent)
{
    //We use a grid layout for the Rupture widget
    auto mainLayout = new QGridLayout(this);
//    mainLayout->setContentsMargins(1,1,1,1);
//    mainLayout->setSpacing(1);

    auto rupOffstLabel = new QLabel("Rupture Offset (km)");
    auto floaterTypeLabel = new QLabel("Floater Type");
    auto backgroundSeisLabel = new QLabel("Background Seismicity");
    auto backgroundSeisTypeLabel = new QLabel("Treat Background Seismicity As");
//    auto applySSkLabel = new QLabel("Apply CyberShake DDW Corr");
    auto probabilityModelLabel = new QLabel("Probability Model");


    rupOffstLE = new SC_DoubleLineEdit("Rupture Offset", 5.0);
    floaterTypeCombo = new SC_ComboBox("Floater Type",QStringList({"Along strike & centered down dip"}));
    backgroundSeisCombo = new SC_ComboBox("Background Seismicity",QStringList({"Include"}));
    backgroundSeisTypeCombo = new SC_ComboBox("Treat Background Seismicity As",QStringList({"Point Sources", "Two perpendicular faults"}));
    probabilityModelCombo = new SC_ComboBox("Probability Model",QStringList({"Poisson","WGCEP Preferred Blend"}));
//    applyCyberShakeCB = new SC_CheckBox("ApplyCyberShakeDDWCorr", "Apply CyberShake DDW Corr");

    // Turn off max width
    rupOffstLE->setMaximumWidth(QWIDGETSIZE_MAX);

    mainLayout->addWidget(rupOffstLabel,0,0);
    mainLayout->addWidget(rupOffstLE,0,1);
    mainLayout->addWidget(floaterTypeLabel,1,0);
    mainLayout->addWidget(floaterTypeCombo,1,1);
    mainLayout->addWidget(backgroundSeisLabel,2,0);
    mainLayout->addWidget(backgroundSeisCombo,2,1);
    mainLayout->addWidget(backgroundSeisTypeLabel,3,0);
    mainLayout->addWidget(backgroundSeisTypeCombo,3,1);
//    mainLayout->addWidget(applySSkLabel,4,0);
//    mainLayout->addWidget(applyCyberShakeCB,4,1);
    mainLayout->addWidget(probabilityModelLabel,4,0);
    mainLayout->addWidget(probabilityModelCombo,4,1);

}


bool UCERF2Widget::inputFromJSON(QJsonObject& /*obj*/)
{

    return true;
}


bool UCERF2Widget::outputToJSON(QJsonObject& obj)
{
    obj["Model"] = this->objectName();

    QJsonObject modelParams;

    rupOffstLE->outputToJSON(modelParams);

    floaterTypeCombo->outputToJSON(modelParams);
    backgroundSeisCombo->outputToJSON(modelParams);
    backgroundSeisTypeCombo->outputToJSON(modelParams);
//    applyCyberShakeCB->outputToJSON(modelParams);
    probabilityModelCombo->outputToJSON(modelParams);

    obj["ModelParameters"] = modelParams;

    return true;
}

