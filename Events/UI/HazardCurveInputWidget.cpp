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

#include "HazardCurveInputWidget.h"
#include "SC_FileEdit.h"

#include <QVBoxLayout>
#include <QStackedWidget>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>

HazardCurveInputWidget::HazardCurveInputWidget(QWidget *parent) : QWidget(parent)
{
    auto mainLayout = new QVBoxLayout(this);

    auto hazCurveLabel = new QLabel("Hazard Curve Input");
    meanPresetsCombo = new QComboBox();
    mainStackedWidget = new QStackedWidget();

    auto emptyWidget = new QWidget();
    auto NSHMCombo = new QComboBox();
    NSHMCombo->addItem("NSHM V1");
    NSHMCombo->addItem("NSHM V2");

    SC_FileEdit* userDefHazardLE = new SC_FileEdit("UserDefined");
    auto userDefLabel = new QLabel("Load a user-defined hazard curve");

    auto userDefHaz = new QWidget();
    auto horizLayout = new QHBoxLayout(userDefHaz);
    horizLayout->addWidget(userDefLabel);
    horizLayout->addWidget(userDefHazardLE);

    meanPresetsCombo->addItem("Inferred");
    meanPresetsCombo->addItem("National Seismic Hazard Map");
    meanPresetsCombo->addItem("User-defined");

    mainStackedWidget->addWidget(emptyWidget);
    mainStackedWidget->addWidget(NSHMCombo);
    mainStackedWidget->addWidget(userDefHaz);


    // Connect the combo box signal to the stacked widget slot
    QObject::connect(meanPresetsCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                     mainStackedWidget, &QStackedWidget::setCurrentIndex);

    mainLayout->addWidget(hazCurveLabel);
    mainLayout->addWidget(meanPresetsCombo);
    mainLayout->addWidget(mainStackedWidget);

}


void HazardCurveInputWidget::reset(void)
{

}


bool HazardCurveInputWidget::inputFromJSON(QJsonObject& /*obj*/)
{
    return true;
}


bool HazardCurveInputWidget::outputToJSON(QJsonObject& obj)
{


    return true;
}

