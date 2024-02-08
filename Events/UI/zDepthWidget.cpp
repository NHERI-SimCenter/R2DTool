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


#include "zDepthWidget.h"
#include "zDepthUserInputWidget.h"

#include <QHBoxLayout>
#include <QGroupBox>
#include <QComboBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QSizePolicy>
#include <QStackedWidget>

zDepthWidget::zDepthWidget(QString type, QWidget *parent): type(type), QWidget(parent)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    m_z1Combo = new QComboBox(this);
    m_z1Combo->addItem("OpenSHA default model");
    m_z1Combo->addItem("User-specified");
    m_z1Combo->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);

    auto comboLayout = new QHBoxLayout(this);

    QLabel* typeLabel = new QLabel(QString("%1:").arg(type),this);

    comboLayout->addWidget(typeLabel);
    comboLayout->addWidget(m_z1Combo);

    z1StackedWidget = new QStackedWidget(this);
    QString lableText = QString("Will get %1 from OpenSHA").arg(type);
    if (type == "Z1pt0"){
        lableText = lableText + QString("\nThe value will be inferred from Vs30 (Chiou and Youngs, 2013) if it can not be interpolated from") +
                    QString(" SCEC Community Velocity Models nor USGS Bay Area Velocity Model.\nThis model is only applicable for California");
    } else {
        lableText = lableText + QString("\nThe value will be inferred from Vs30 (Campbell and Bozorgnia (2014)) if it can not be interpolated from") +
                    QString(" SCEC Community Velocity Models nor USGS Bay Area Velocity Model,\nThis model is only applicable for California");
    }

    auto labelZ1 = new QLabel(lableText);
    z1StackedWidget->addWidget(labelZ1);

    userInputZ1 = new zDepthUserInputWidget(type);
    z1StackedWidget->addWidget(userInputZ1);

    mainLayout->addLayout(comboLayout);
    mainLayout->addWidget(z1StackedWidget);

    QObject::connect(m_z1Combo, SIGNAL(currentIndexChanged(int)), z1StackedWidget, SLOT(setCurrentIndex(int)));
}


bool zDepthWidget::outputToJSON(QJsonObject &jsonObject)
{
    jsonObject.insert("Type", m_z1Combo->currentText());
    if (m_z1Combo->currentText() == "User-specified"){
        QJsonObject parameters;
        parameters.insert("value", userInputZ1->getValue());
        jsonObject.insert("Parameters", parameters);
    }
    return true;
}



