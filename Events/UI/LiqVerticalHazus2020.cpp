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

// Written by: Jinyan Zhao

#include "LiqVerticalHazus2020.h"
#include "SimCenterPreferences.h"
#include "CRSSelectionWidget.h"

#include <QLabel>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QSpacerItem>
#include <QPushButton>
#include <HBoxFormLayout.h>
#include <QFileDialog>
#include <QMessageBox>
#include <QSignalMapper>
#include <QDir>
#include <QGroupBox>
#include <QPushButton>
#include <QJsonArray>


LiqVerticalHazus2020::LiqVerticalHazus2020(QWidget *parent) : SimCenterAppWidget(parent)
{
    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    messageLabel = new QLabel(tr("No input required"),this);
    //We use a grid layout for the Rupture widget
    QGridLayout* layout = new QGridLayout(this);
    layout->addWidget(messageLabel, 0, 0);

    resetToDefaultButton = new QPushButton();
    resetToDefaultButton->setText(tr("Reset to Default"));
    resetToDefaultButton->setMaximumWidth(150);

    connect(resetToDefaultButton, &QPushButton::clicked, this, &LiqVerticalHazus2020::setDefaultFilePath);

    QCheckBox* SaveCheckBox = new QCheckBox("liq_PGD_v");
    outputSaveCheckBoxes.insert("liq_PGD_v", SaveCheckBox);

    outputSaveGroupBox = new QGroupBox(this);
    outputSaveGroupBox->setTitle(tr("Save Output"));
    outputSaveGroupBox->setContentsMargins(0,0,0,0);
    QGridLayout* outputSaveGroupBoxLayout = new QGridLayout(outputSaveGroupBox);
    outputSaveGroupBox->setLayout(outputSaveGroupBoxLayout);
    int checkBoxCount = 0;
    for (auto it = outputSaveCheckBoxes.constBegin(); it != outputSaveCheckBoxes.constEnd(); ++it) {
        outputSaveGroupBoxLayout->addWidget(it.value(), 0, checkBoxCount);
        checkBoxCount ++;
    }
    layout->addWidget(outputSaveGroupBox, 1, 0);

    this->setDefaultFilePath();

    this->setLayout(layout);

}


bool LiqVerticalHazus2020::outputToJSON(QJsonObject &jsonObject){
    QJsonObject parameterObj;
    QJsonArray outputArray;
    for (auto it = outputSaveCheckBoxes.constBegin(); it != outputSaveCheckBoxes.constEnd(); ++it) {
        if (it.value()->isChecked()){
            outputArray.append(it.key());
        }
    }
    jsonObject["Output"] = outputArray;
    jsonObject["Model"] = "Hazus2020Vertical";
    jsonObject["Parameters"] = parameterObj;
    return true;
}

void LiqVerticalHazus2020::setDefaultFilePath(){
    for (auto it = outputSaveCheckBoxes.constBegin(); it != outputSaveCheckBoxes.constEnd(); ++it) {
        it.value()->setChecked(true);
    }
}
