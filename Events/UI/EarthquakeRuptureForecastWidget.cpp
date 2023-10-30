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

#include "EarthquakeRuptureForecastWidget.h"
#include "EarthquakeRuptureForecast.h"

#include <QLabel>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QSpacerItem>
#include <HBoxFormLayout.h>

EarthquakeRuptureForecastWidget::EarthquakeRuptureForecastWidget(QWidget *parent) : QWidget(parent)
{
    //We use a grid layout for the Rupture widget
    QGridLayout* layout = new QGridLayout(this);

    this->m_magnitudeMinBox = new QDoubleSpinBox(this);
    this->m_magnitudeMinBox->setRange(4.5, 9.0);
    this->m_magnitudeMinBox->setDecimals(2);
    this->m_magnitudeMinBox->setSingleStep(0.1);
    //FMK this->m_magnitudeMinBox->setMinimumWidth(250);

    this->m_magnitudeMaxBox = new QDoubleSpinBox(this);
    this->m_magnitudeMaxBox->setRange(4.5, 9.0);
    this->m_magnitudeMaxBox->setDecimals(2);
    this->m_magnitudeMaxBox->setSingleStep(0.1);
    //FMK this->m_magnitudeMaxBox->setMinimumWidth(250);

    this->m_maxDistanceBox = new QDoubleSpinBox(this);
    this->m_maxDistanceBox->setRange(0, 1000.0);
    this->m_maxDistanceBox->setDecimals(1);
    this->m_maxDistanceBox->setSingleStep(1.0);
    //FMK this->m_maxDistanceBox->setMinimumWidth(250);

    QLabel* magMinLabel = new QLabel(tr("Minimum Magnitude:"),this);
    QLabel* magMaxLabel = new QLabel(tr("Maximum Magnitude:"),this);
    QLabel* distMaxLabel = new QLabel(tr("Maximum Distance (km):"),this);

    QLabel* nameLabel = new QLabel(tr("Earthquake Name:"),this);
    QLabel* modelLabel = new QLabel(tr("Forecast Model:"),this);

    ModelTypeCombo = new QComboBox(this);
    ModelTypeCombo->addItem("WGCEP (2007) UCERF2 - Single Branch");
    ModelTypeCombo->addItem("Mean UCERF3");
    ModelTypeCombo->addItem("Mean UCERF3 FM3.1");
    ModelTypeCombo->addItem("Mean UCERF3 FM3.2");

    EQNameLineEdit = new QLineEdit(this);

    // number of scenarios
    QLabel* NumScenarioLabel= new QLabel(tr("Number of scenarios:"),this);
    NumScenarioLineEdit = new QLineEdit(this);
    NumScenarioLineEdit->setText("1");
    auto validator = new QIntValidator(1, 9999, this);
    NumScenarioLineEdit->setValidator(validator);

    // sampling methods
    QLabel* SamplingLabel = new QLabel(tr("Sampling method:"),this);
    SamplingMethodCombo = new QComboBox(this);
    SamplingMethodCombo->addItem("Random");
    SamplingMethodCombo->addItem("MAF");
    SamplingMethodCombo->setCurrentIndex(0); // default at Random
    // hide it for now (KZ-08/22)
    SamplingLabel->hide();
    SamplingMethodCombo->hide();

    // Add a horizontal
    auto hspacer = new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Minimum);

    layout->addWidget(NumScenarioLabel,0,0);
    layout->addWidget(NumScenarioLineEdit,0,1);

    //layout->addWidget(SamplingLabel,0,2);
    //layout->addWidget(SamplingMethodCombo,0,3);

    layout->addWidget(magMinLabel,1,0);
    layout->addWidget(m_magnitudeMinBox,1,1);

    layout->addWidget(magMaxLabel,1,2);
    layout->addWidget(m_magnitudeMaxBox,1,3);

    layout->addWidget(distMaxLabel,1,4);
    layout->addWidget(m_maxDistanceBox,1,5);

    layout->addItem(hspacer,1,6);

    layout->addWidget(nameLabel,2,0);
    layout->addWidget(EQNameLineEdit,2,1,1,6);

    layout->addWidget(modelLabel,3,0);
    layout->addWidget(ModelTypeCombo,3,1,1,6);

    this->setLayout(layout);

    //We need to set initial values
    m_eqRupture = new EarthquakeRuptureForecast(7.0,8.0,200.0, ModelTypeCombo->currentText(),"San Andreas",SamplingMethodCombo->currentText(),NumScenarioLineEdit->text().toInt(),this);

    this->m_magnitudeMinBox->setValue(m_eqRupture->getMagnitudeMin());
    this->m_magnitudeMaxBox->setValue(m_eqRupture->getMagnitudeMax());
    this->m_maxDistanceBox->setValue(m_eqRupture->getMaxDistance());
    this->EQNameLineEdit->setText(m_eqRupture->getEQName());
    this->NumScenarioLineEdit->setText(QString::number(m_eqRupture->getNumScen()));

    this->setupConnections();
}



EarthquakeRuptureForecast* EarthquakeRuptureForecastWidget::getRuptureSource() const
{
    return m_eqRupture;
}

// KZ: adding getNumScen to return number of scenarios
QString EarthquakeRuptureForecastWidget::getNumScen() const
{
    return this->NumScenarioLineEdit->text();
}



void EarthquakeRuptureForecastWidget::setupConnections()
{
    //Connecting magnitude
    connect(this->m_magnitudeMinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this->m_eqRupture, &EarthquakeRuptureForecast::setMagnitudeMin);

    connect(this->m_magnitudeMaxBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this->m_eqRupture, &EarthquakeRuptureForecast::setMagnitudeMax);

    connect(this->m_maxDistanceBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this->m_eqRupture, &EarthquakeRuptureForecast::setMaxDistance);

    connect(this->EQNameLineEdit, &QLineEdit::textChanged,
            this->m_eqRupture, &EarthquakeRuptureForecast::setEQName);

    connect(this->NumScenarioLineEdit, &QLineEdit::textChanged,
            this->m_eqRupture, &EarthquakeRuptureForecast::setNumScen);

    connect(this->ModelTypeCombo, &QComboBox::currentTextChanged,
            this->m_eqRupture, &EarthquakeRuptureForecast::setEQModelType);

    connect(this->SamplingMethodCombo, &QComboBox::currentTextChanged,
            this->m_eqRupture, &EarthquakeRuptureForecast::setSamplingMethod);


}



