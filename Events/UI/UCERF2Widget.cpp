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

#include <QGridLayout>
#include <QLabel>

UCERF2Widget::UCERF2Widget(QWidget *parent) : QWidget(parent)
{
    //We use a grid layout for the Rupture widget
    auto mainLayout = new QGridLayout(this);
    mainLayout->setContentsMargins(1,1,1,1);
    mainLayout->setSpacing(1);

    auto EqkLabel = new QLabel("Eqk Rup Forecast");
    auto rupOffstLabel = new QLabel("Rupture Offset (km)");
    auto floaterTypeLabel = new QLabel("Floater Type");
    auto backgroundSeisLabel = new QLabel("Background Seismicity");
    auto backgroundSeisTypeLabel = new QLabel("Treat Background Seismicity As");
    auto applySSkLabel = new QLabel("Apply CyberShake DDW Corr");
    auto probabilityModelLabel = new QLabel("Probability Model");

    EqkLabel->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum);

    EqkRupForecastCombo = new SC_ComboBox("EqkRupForecast",QStringList({"WGCEP (2007) UCERF2 - Single Branch"}));
    rupOffstLE = new SC_DoubleLineEdit("RuptureOffset", 5.0);
    floaterTypeCombo = new SC_ComboBox("FloaterType",QStringList({"Along strike & centered down dip"}));
    backgroundSeisCombo = new SC_ComboBox("BackgroundSeismicity",QStringList({"Include"}));
    backgroundSeisTypeCombo = new SC_ComboBox("BackgroundSeismicityType",QStringList({"Two perpendicular faults"}));
    applyCyberShakeCB = new SC_CheckBox("ApplyCyberShakeDDWCorr", "Apply CyberShake DDW Corr");
    probabilityModelCombo = new SC_ComboBox("PorbabilityModel",QStringList({"WGCEP Preferred Blend"}));

    EqkRupForecastCombo->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);


    mainLayout->addWidget(EqkLabel,0,0);
    mainLayout->addWidget(EqkRupForecastCombo,0,1);
    mainLayout->addWidget(rupOffstLabel,1,0);
    mainLayout->addWidget(rupOffstLE,1,1);
    mainLayout->addWidget(floaterTypeLabel,2,0);
    mainLayout->addWidget(floaterTypeCombo,2,1);
    mainLayout->addWidget(backgroundSeisLabel,3,0);
    mainLayout->addWidget(backgroundSeisCombo,3,1);
    mainLayout->addWidget(backgroundSeisTypeLabel,4,0);
    mainLayout->addWidget(backgroundSeisTypeCombo,4,1);
    mainLayout->addWidget(applySSkLabel,5,0);
    mainLayout->addWidget(applyCyberShakeCB,5,1);
    mainLayout->addWidget(probabilityModelLabel,6,0);
    mainLayout->addWidget(probabilityModelCombo,6,1);

    // Set column stretch factors
    mainLayout->setColumnStretch(0, 1); // Smaller stretch factor for the first column
    mainLayout->setColumnStretch(1, 2); // Larger stretch factor for the second column, it will be wider
}


void UCERF2Widget::reset(void)
{

}


bool UCERF2Widget::inputFromJSON(QJsonObject& /*obj*/)
{

    return true;
}


bool UCERF2Widget::outputToJSON(QJsonObject& obj)
{

    EqkRupForecastCombo->outputToJSON(obj);
    rupOffstLE->outputToJSON(obj);
    floaterTypeCombo->outputToJSON(obj);
    backgroundSeisCombo->outputToJSON(obj);
    backgroundSeisTypeCombo->outputToJSON(obj);
    applyCyberShakeCB->outputToJSON(obj);
    probabilityModelCombo->outputToJSON(obj);

    return true;
}

