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

#include "RecordSelectionWidget.h"
#include "SC_DoubleLineEdit.h"
#include "SC_IntLineEdit.h"
#include "SC_ComboBox.h"

#include <QVBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QGroupBox>
#include <QLineEdit>

RecordSelectionWidget::RecordSelectionWidget(RecordSelectionConfig& selectionConfig, QWidget *parent) : QWidget(parent), m_selectionConfig(selectionConfig)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    QGroupBox* selectionGroupBox = new QGroupBox();
    selectionGroupBox->setTitle("Record Selection");
    selectionGroupBox->setContentsMargins(0,0,0,0);


    QGridLayout* formLayout = new QGridLayout(selectionGroupBox);

    QLabel* databaseLabel = new QLabel(tr("Database:"));
    m_dbBox = new SC_ComboBox("GMDatabase",QStringList({"None","PEER NGA West 2"}));
    connect(this->m_dbBox, &QComboBox::currentTextChanged, &this->m_selectionConfig, &RecordSelectionConfig::setDatabase);

    m_selectionConfig.setDatabase("PEER NGA West 2");
    m_dbBox->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);

    connect(this->m_dbBox, &QComboBox::currentTextChanged, this, &RecordSelectionWidget::handleDBSelection);

    formLayout->addWidget(databaseLabel,0,0);
    formLayout->addWidget(m_dbBox,0,1);

    QLabel* numGMLabel = new QLabel(tr("Number of ground motions per site:"));
    auto validator = new QIntValidator(1, 99999999);
    numGMLineEdit = new SC_IntLineEdit("NumGmPerSite",1);
    numGMLineEdit->setValidator(validator);

    formLayout->addItem(new QSpacerItem(0,20),3,0,1,2);

    formLayout->addWidget(numGMLabel,4,0);
    formLayout->addWidget(numGMLineEdit,4,1);

    formLayout->addItem(new QSpacerItem(0,20),5,0,1,2);

    scalingMinLabel = new QLabel("Minimum Scaling Factor");
    scalingMaxLabel = new QLabel("Maximum Scaling Factor");
    scalingMin = new SC_DoubleLineEdit("ScalingMin",0.1);
    scalingMax = new SC_DoubleLineEdit("ScalingMax",20.0);

    formLayout->addWidget(scalingMinLabel,6,0);
    formLayout->addWidget(scalingMin,6,1);

    formLayout->addWidget(scalingMaxLabel,7,0);
    formLayout->addWidget(scalingMax,7,1);

    selectionGroupBox->setLayout(formLayout);

    layout->addWidget(selectionGroupBox);

    this->setLayout(layout);
    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
    setScalingVisibility(false);

}


void RecordSelectionWidget::reset(void)
{

}


bool RecordSelectionWidget::inputFromJSON(QJsonObject& /*obj*/)
{

    return true;
}


bool RecordSelectionWidget::outputToJSON(QJsonObject& obj)
{
    m_dbBox->outputToJSON(obj);
    numGMLineEdit->outputToJSON(obj);
    scalingMin->outputToJSON(obj);
    scalingMax->outputToJSON(obj);

    return true;
}


int RecordSelectionWidget::getNumberOfGMPerSite(void)
{
    auto res = false;
    int numGM = numGMLineEdit->text().toInt(&res);

    if(res == true)
        return numGM;

    return -1;
}

void RecordSelectionWidget::setScalingVisibility(bool val)
{
    scalingMin->setVisible(val);
    scalingMax->setVisible(val);
    scalingMinLabel->setVisible(val);
    scalingMaxLabel->setVisible(val);
}


void RecordSelectionWidget::handleDBSelection(const QString& selection)
{
    if(selection == "PEER NGA West 2")
        setScalingVisibility(true);
    else
        setScalingVisibility(false);

}

