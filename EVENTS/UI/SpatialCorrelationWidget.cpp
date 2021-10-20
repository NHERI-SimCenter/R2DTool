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

// Written by: Stevan Gavrilovic, Frank McKenna

#include "SpatialCorrelationWidget.h"

#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QGroupBox>
#include <QCheckBox>

SpatialCorrelationWidget::SpatialCorrelationWidget(QWidget *parent): QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    QGroupBox* spatCorrGroupBox = new QGroupBox(this);
    spatCorrGroupBox->setTitle("Spatial Correlation and Scaling");
    spatCorrGroupBox->setContentsMargins(0,0,0,0);

    this->setMinimumWidth(375);

    QGridLayout* gridLayout = new QGridLayout(spatCorrGroupBox);
    spatCorrGroupBox->setLayout(gridLayout);

    QLabel* spatialCorrelationInterLabel = new QLabel(tr("Inter-event\nSpatial Correlation Model:"),this);

    m_correlationBoxInter = new QComboBox(this);
    m_correlationBoxInter->addItem("Baker & Jayaram (2008)");
    m_correlationBoxInter->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);

    QLabel* spatialCorrelationIntraLabel = new QLabel(tr("Intra-event\nSpatial Correlation Model:"),this);

    m_correlationBoxIntra = new QComboBox(this);
    m_correlationBoxIntra->addItem("Markhvida et al. (2017)");
    m_correlationBoxIntra->addItem("Jayaram & Baker (2009)");
    m_correlationBoxIntra->addItem("Loth & Baker (2013)");
    m_correlationBoxIntra->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);

    QLabel* minScalingLabel = new QLabel(tr("Minimum Scaling Factor:"),this);
    QLabel* maxScalingLabel = new QLabel(tr("Maximum Scaling Factor:"),this);

    QDoubleValidator* doubleValid = new QDoubleValidator(0.0, 100.0,2,this);

    minScalingLineEdit = new QLineEdit(this);
    minScalingLineEdit->setText("0.1");

    maxScalingLineEdit = new QLineEdit(this);
    maxScalingLineEdit->setText("20.0");

    minScalingLineEdit->setValidator(doubleValid);
    maxScalingLineEdit->setValidator(doubleValid);

    auto smallVSpacer = new QSpacerItem(0,20);


    auto Vspacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    //gridLayout->addItem(Vspacer,0,0,1,2);
    gridLayout->addWidget(spatialCorrelationInterLabel,1,0);
    gridLayout->addWidget(m_correlationBoxInter,1,1);
    gridLayout->addWidget(spatialCorrelationIntraLabel,2,0);
    gridLayout->addWidget(m_correlationBoxIntra,2,1);
    gridLayout->addItem(smallVSpacer,3,0,1,2);
    gridLayout->addWidget(minScalingLabel,4,0);
    gridLayout->addWidget(minScalingLineEdit,4,1);
    gridLayout->addWidget(maxScalingLabel,5,0);
    gridLayout->addWidget(maxScalingLineEdit,5,1);
    //gridLayout->addItem(Vspacer,6,0,1,2);
    // gridLayout->setRowStretch(6,1);

    layout->addWidget(spatCorrGroupBox);
    this->setLayout(layout);

    m_correlationBoxInter->setCurrentIndex(0);
}


QJsonObject SpatialCorrelationWidget::getJsonCorr()
{
    QJsonObject spatCorr;
    spatCorr.insert("SaInterEvent", m_correlationBoxInter->currentText());
    spatCorr.insert("SaIntraEvent", m_correlationBoxIntra->currentText());

    return spatCorr;
}


QJsonObject SpatialCorrelationWidget::getJsonScaling()
{
    QJsonObject scaling;
    scaling.insert("Maximum", maxScalingLineEdit->text().toDouble());
    scaling.insert("Minimum", minScalingLineEdit->text().toDouble());

    return scaling;
}

