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

#include "MeanUCERFFM3Widget.h"
#include "SC_ComboBox.h"
#include "SC_DoubleLineEdit.h"
#include "SC_CheckBox.h"

#include <QGridLayout>
#include <QLabel>

MeanUCERFFM3Widget::MeanUCERFFM3Widget(QWidget *parent) : SimCenterAppWidget(parent)
{
    //We use a grid layout for the Rupture widget
    auto mainLayout = new QGridLayout(this);

    auto applyAfterShockLabel = new QLabel("Apply Aftershock Filter");
    auto AleatoryMagStdDevLabel = new QLabel("Aleatory Mag-Area StdDev");
    auto backgroundSeisLabel = new QLabel("Background Seismicity");
    auto backgroundSeisTypeLabel = new QLabel("Treat Background Seismicity As");
    auto faultGridSpacingLabel = new QLabel("Fault Grid Spacing (km)");
    auto probabilityModelLabel = new QLabel("Probability Model");
    aperiodicityLabel = new QLabel("Aperiodicity");
    historicOpenIntLabel = new QLabel("Historic Open Interval (Years)");
    bptAvgTypeLabel = new QLabel("BPT Averaging Type");

    applyAfterShockCB = new SC_CheckBox("Apply AfterShock Filter",
                                        "Apply AfterShock Filter", false);
    AleatoryMagStdDevLE = new SC_DoubleLineEdit("Aleatory Mag-Area StdDev", 0.0);
    backgroundSeisCombo = new SC_ComboBox("Background Seismicity",QStringList({"Include","Exclude","Only"}));
    backgroundSeisTypeCombo = new SC_ComboBox("Treat Background Seismicity As",QStringList({"Point Sources","Single Random Strike Faults","Two Perpendicular Faults"}));
    faultGridSpacingLE = new SC_DoubleLineEdit("Fault Grid Spacing (km)",1.0);
    probabilityModelCombo = new SC_ComboBox("Probability Model",QStringList({"Poisson", "UCERF3 BPT", "UCERF3 Preferred Blend", "WG02 BPT"}));
    aperiodicityModelCombo = new SC_ComboBox("Aperiodicity",QStringList({"0.5,0.4,0.3,0.2",
"0.4,0.3,0.2,0.1","0.6,0.5,0.4,0.3","All 0.1","All 0.2","All 0.3","All 0.4","All 0.5","All 0.6","All 0.7","All 0.8"}));
    historicOpenIntLE = new SC_DoubleLineEdit("Historic Open Interval", 0.0);
    bptAvgTypeCombo = new SC_ComboBox("BPT Averaging Type",QStringList({"AveRI and AveNormTimeSince","AveRI and AveTimeSince","AveRate and AveNormTimeSince"}));

    // Turn off max width
    AleatoryMagStdDevLE->setMaximumWidth(QWIDGETSIZE_MAX);

    mainLayout->addWidget(applyAfterShockLabel,0,0);
    mainLayout->addWidget(applyAfterShockCB,0,1);
    mainLayout->addWidget(AleatoryMagStdDevLabel,1,0);
    mainLayout->addWidget(AleatoryMagStdDevLE,1,1);
    mainLayout->addWidget(backgroundSeisLabel,2,0);
    mainLayout->addWidget(backgroundSeisCombo,2,1);
    mainLayout->addWidget(backgroundSeisTypeLabel,3,0);
    mainLayout->addWidget(backgroundSeisTypeCombo,3,1);
    mainLayout->addWidget(faultGridSpacingLabel,4,0);
    mainLayout->addWidget(faultGridSpacingLE,4,1);
    mainLayout->addWidget(probabilityModelLabel,5,0);
    mainLayout->addWidget(probabilityModelCombo,5,1);
    mainLayout->addWidget(aperiodicityLabel,6,0);
    mainLayout->addWidget(aperiodicityModelCombo,6,1);
    mainLayout->addWidget(historicOpenIntLabel,7,0);
    mainLayout->addWidget(historicOpenIntLE,7,1);
    mainLayout->addWidget(bptAvgTypeLabel,8,0);
    mainLayout->addWidget(bptAvgTypeCombo,8,1);

    historicOpenIntLabel->hide();
    historicOpenIntLE->hide();
    aperiodicityLabel->hide();
    aperiodicityModelCombo->hide();
    bptAvgTypeLabel->hide();
    bptAvgTypeCombo->hide();

//    QComboBox* probabilityCombo = static_cast<QComboBox*>(probabilityModelCombo);

    QObject::connect(probabilityModelCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MeanUCERFFM3Widget::showAdditionalInputForProbabilityModel);


}


bool MeanUCERFFM3Widget::inputFromJSON(QJsonObject& /*obj*/)
{

    return true;
}


bool MeanUCERFFM3Widget::outputToJSON(QJsonObject& obj)
{

    AleatoryMagStdDevLE->outputToJSON(obj);
    backgroundSeisCombo->outputToJSON(obj);
    backgroundSeisTypeCombo->outputToJSON(obj);
    applyAfterShockCB->outputToJSON(obj);

    faultGridSpacingLE->outputToJSON(obj);
    probabilityModelCombo->outputToJSON(obj);
    aperiodicityModelCombo->outputToJSON(obj);
    historicOpenIntLE->outputToJSON(obj);
    bptAvgTypeCombo->outputToJSON(obj);

    return true;
}

bool MeanUCERFFM3Widget::showAdditionalInputForProbabilityModel(int currentIndex){
    if (currentIndex == 0){
        // Poisson
        historicOpenIntLabel->hide();
        historicOpenIntLE->hide();
        aperiodicityLabel->hide();
        aperiodicityModelCombo->hide();
        bptAvgTypeLabel->hide();
        bptAvgTypeCombo->hide();
    }
    else if (currentIndex == 1){
//        UCERF3 BPT
        historicOpenIntLabel->show();
        historicOpenIntLE->show();
        aperiodicityLabel->show();
        aperiodicityModelCombo->show();
        bptAvgTypeLabel->show();
        bptAvgTypeCombo->show();
    } else if (currentIndex == 2){
        //UCERF3 Preferred Blend
        historicOpenIntLabel->show();
        historicOpenIntLE->show();
        aperiodicityLabel->hide();
        aperiodicityModelCombo->hide();
        bptAvgTypeLabel->show();
        bptAvgTypeCombo->show();
    } else if (currentIndex == 3) {
        // WG02 BPT
        historicOpenIntLabel->show();
        historicOpenIntLE->show();
        aperiodicityLabel->show();
        aperiodicityModelCombo->show();
        bptAvgTypeLabel->hide();
        bptAvgTypeCombo->hide();
    }
}
