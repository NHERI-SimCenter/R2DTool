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

// Written by: Kuanshi Zhong

#include "BedrockDepthWidget.h"

#include <QHBoxLayout>
#include <QGroupBox>
#include <QStringListModel>
#include <QComboBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QSizePolicy>

BedrockDepthWidget::BedrockDepthWidget(BedrockDepth& bedrockDepth, SiteConfig& siteConfig, QWidget *parent): QWidget(parent), m_bedrockDpeth(bedrockDepth)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    QGroupBox* bedrockDepthGroupBox = new QGroupBox(this);
    bedrockDepthGroupBox->setTitle("Bedrock Depth Model (if DepthToRock not provided by users)");

    QHBoxLayout* formLayout = new QHBoxLayout(bedrockDepthGroupBox);
    m_typeBox = new QComboBox(this);

    m_typeBox->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);

    QLabel* typeLabel = new QLabel(tr("Type:"),this);

    formLayout->addWidget(typeLabel);
    formLayout->addWidget(m_typeBox);
//    formLayout->addStretch(1);

    bedrockDepthGroupBox->setLayout(formLayout);

    layout->addWidget(bedrockDepthGroupBox);
    this->setLayout(layout);

    QStringList validType;
    validType = this->m_bedrockDpeth.validTypes();

    QStringListModel* typeModel = new QStringListModel(validType);
    m_typeBox->setModel(typeModel);
    m_typeBox->setCurrentIndex(validType.indexOf(m_bedrockDpeth.type()));
    this->setupConnections();
}


void BedrockDepthWidget::setupConnections()
{
    connect(this->m_typeBox, &QComboBox::currentTextChanged,
            &this->m_bedrockDpeth, &BedrockDepth::setType);

    connect(&this->m_bedrockDpeth, &BedrockDepth::typeChanged,
            this->m_typeBox, &QComboBox::setCurrentText);
}

bool BedrockDepthWidget::outputCitation(QJsonObject &jsonObject){
    QJsonObject outputToJsonObj;
    m_bedrockDpeth.outputToJSON(outputToJsonObj);
    QString bedrockDepthType = outputToJsonObj["Type"].toString();
    if (bedrockDepthType == "SoilGrid250 (Hengl et al., 2017)"){
        jsonObject.insert("citation", QString("Hengl T, Mendes de Jesus J, Heuvelink GBM, Ruiperez Gonzalez M, Kilibarda M, Blagotić A, et al. (2017) SoilGrids250m: Global gridded soil information based on machine learning. PLoS ONE 12(2): e0169748."));
    }
	
	return true;
}
