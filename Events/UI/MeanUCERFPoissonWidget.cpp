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

#include "MeanUCERFPoissonWidget.h"
#include "SC_ComboBox.h"
#include "SC_DoubleLineEdit.h"
#include "SC_CheckBox.h"

#include <QGridLayout>
#include <QLabel>

MeanUCERFPoissonWidget::MeanUCERFPoissonWidget(QWidget *parent) : QWidget(parent)
{
    //We use a grid layout for the Rupture widget
    auto mainLayout = new QGridLayout(this);

    auto applyAfterShockLabel = new QLabel("Apply Aftershock Filter");
    auto AleatoryMagStdDevLabel = new QLabel("Aleatory Mag-Area StdDev");

    auto backgroundSeisLabel = new QLabel("Background Seismicity");
    auto backgroundSeisTypeLabel = new QLabel("Treat Background Seismicity As");

    applyAfterShockCB = new SC_CheckBox("ApplyAfterShockFilter", "Apply Aftershock Filter");
    AleatoryMagStdDevLE = new SC_DoubleLineEdit("RuptureOffset");

    backgroundSeisCombo = new SC_ComboBox("BackgroundSeismicity",QStringList({"Include"}));
    backgroundSeisTypeCombo = new SC_ComboBox("BackgroundSeismicityType",QStringList({"Point Sources"}));


    mainLayout->addWidget(applyAfterShockLabel,0,0);
    mainLayout->addWidget(applyAfterShockCB,0,1);
    mainLayout->addWidget(AleatoryMagStdDevLabel,1,0);
    mainLayout->addWidget(AleatoryMagStdDevLE,1,1);
    mainLayout->addWidget(backgroundSeisLabel,2,0);
    mainLayout->addWidget(backgroundSeisCombo,2,1);
    mainLayout->addWidget(backgroundSeisTypeLabel,3,0);
    mainLayout->addWidget(backgroundSeisTypeCombo,3,1);

}


void MeanUCERFPoissonWidget::reset(void)
{

}


bool MeanUCERFPoissonWidget::inputFromJSON(QJsonObject& /*obj*/)
{

    return true;
}


bool MeanUCERFPoissonWidget::outputToJSON(QJsonObject& obj)
{

    AleatoryMagStdDevLE->outputToJSON(obj);
    backgroundSeisCombo->outputToJSON(obj);
    backgroundSeisTypeCombo->outputToJSON(obj);
    applyAfterShockCB->outputToJSON(obj);

    return true;
}

