#include "EarthquakeRuptureForecastWidget.h"

#include "HBoxFormLayout.h"
#include "EarthquakeRuptureForecast.h"

#include <QLabel>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QSpacerItem>

EarthquakeRuptureForecastWidget::EarthquakeRuptureForecastWidget(QWidget *parent) : QWidget(parent)
{
    //We use a grid layout for the Rupture widget
    QGridLayout* layout = new QGridLayout(this);

    this->m_magnitudeMinBox = new QDoubleSpinBox(this);
    this->m_magnitudeMinBox->setRange(4.5, 9.0);
    this->m_magnitudeMinBox->setDecimals(2);
    this->m_magnitudeMinBox->setSingleStep(0.1);
    this->m_magnitudeMinBox->setMinimumWidth(250);

    this->m_magnitudeMaxBox = new QDoubleSpinBox(this);
    this->m_magnitudeMaxBox->setRange(4.5, 9.0);
    this->m_magnitudeMaxBox->setDecimals(2);
    this->m_magnitudeMaxBox->setSingleStep(0.1);
    this->m_magnitudeMaxBox->setMinimumWidth(250);

    this->m_maxDistanceBox = new QDoubleSpinBox(this);
    this->m_maxDistanceBox->setRange(0, 1000.0);
    this->m_maxDistanceBox->setDecimals(1);
    this->m_maxDistanceBox->setSingleStep(1.0);
    this->m_maxDistanceBox->setMinimumWidth(250);

    QLabel* magMinLabel = new QLabel(tr("Minimum Magnitude:"),this);
    QLabel* magMaxLabel = new QLabel(tr("Maximum Magnitude:"),this);
    QLabel* distMaxLabel = new QLabel(tr("Maximum Distance:"),this);

    QLabel* nameLabel = new QLabel(tr("Earthquake Name:"),this);
    QLabel* modelLabel = new QLabel(tr("Forecast Model:"),this);

    ModelTypeCombo = new QComboBox(this);
    ModelTypeCombo->addItem("WGCEP (2007) UCERF2 - Single Branch");
    ModelTypeCombo->addItem("Mean UCERF3");
    ModelTypeCombo->addItem("Mean UCERF3 FM3.1");
    ModelTypeCombo->addItem("Mean UCERF3 FM3.2");

    EQNameLineEdit = new QLineEdit(this);

    // Add a horizontal
    auto hspacer = new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Minimum);

    layout->addWidget(magMinLabel,0,0);
    layout->addWidget(m_magnitudeMinBox,0,1);

    layout->addWidget(magMaxLabel,0,2);
    layout->addWidget(m_magnitudeMaxBox,0,3);

    layout->addWidget(distMaxLabel,0,4);
    layout->addWidget(m_maxDistanceBox,0,5);

    layout->addItem(hspacer,0,6);

    layout->addWidget(nameLabel,1,0);
    layout->addWidget(EQNameLineEdit,1,1,1,6);

    layout->addWidget(modelLabel,2,0);
    layout->addWidget(ModelTypeCombo,2,1,1,6);

    this->setLayout(layout);

    //We need to set initial values
    m_eqRupture = new EarthquakeRuptureForecast(7.0,8.0,200.0, ModelTypeCombo->currentText(),"San Andreas",this);

    this->m_magnitudeMinBox->setValue(m_eqRupture->getMagnitudeMin());
    this->m_magnitudeMaxBox->setValue(m_eqRupture->getMagnitudeMax());
    this->m_maxDistanceBox->setValue(m_eqRupture->getMaxDistance());
    this->EQNameLineEdit->setText(m_eqRupture->getEQName());

    this->setupConnections();
}



EarthquakeRuptureForecast* EarthquakeRuptureForecastWidget::getRuptureSource() const
{
    return m_eqRupture;
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

    connect(this->ModelTypeCombo, &QComboBox::currentTextChanged,
            this->m_eqRupture, &EarthquakeRuptureForecast::setEQModelType);

}



