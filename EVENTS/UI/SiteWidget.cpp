#include <QtWidgets>
#include "SiteWidget.h"
#include "HBoxFormLayout.h"

SiteWidget::SiteWidget(Site& site, QWidget *parent, Qt::Orientation orientation) : QWidget(parent), m_site(site)
{
    //We use a grid layout for the site widget
    QVBoxLayout* layout = new QVBoxLayout(this);

    this->m_locationGroupBox = new QGroupBox(tr("Location"));
    this->m_locationGroupBox->setFlat(true);
    this->m_locationGroupBox->setStyleSheet("QGroupBox { font-weight: normal;}");

    QVBoxLayout* locationLayout = new QVBoxLayout(this->m_locationGroupBox);

    this->m_latitudeBox = new QDoubleSpinBox(this->m_locationGroupBox);
    this->m_latitudeBox->setRange(-90.0, 90.0);
    this->m_latitudeBox->setDecimals(3);
    this->m_latitudeBox->setSingleStep(0.001);
    this->m_latitudeBox->setValue(this->m_site.location().latitude());

    this->m_longitudeBox = new QDoubleSpinBox(this->m_locationGroupBox);
    this->m_longitudeBox->setRange(-180.0, 180.0);
    this->m_longitudeBox->setDecimals(3);
    this->m_longitudeBox->setSingleStep(0.001);
    this->m_longitudeBox->setValue(this->m_site.location().longitude());



    if(orientation == Qt::Horizontal)
    {
        HBoxFormLayout* formLayout = new HBoxFormLayout();
        formLayout->addField(tr("Latitude:"), this->m_latitudeBox);
        formLayout->addField(tr("Longitude:"), this->m_longitudeBox);
        locationLayout->addLayout(formLayout);
    }
    else
    {
        QFormLayout* formLayout = new QFormLayout();
        formLayout->addRow(tr("Latitude:"), this->m_latitudeBox);
        formLayout->addRow(tr("Longitude:"), this->m_longitudeBox);
        locationLayout->addLayout(formLayout);
    }

    layout->addWidget(m_locationGroupBox);
    layout->addStretch();

    layout->setContentsMargins(0, 0, 0, 0);

    //Now we need to setup the connections
    this->setupConnections();
}

void SiteWidget::setupConnections()
{
    connect(this->m_latitudeBox,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            &this->m_site.location(),
            &Location::setLatitude);

    connect(this->m_longitudeBox,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            &this->m_site.location(),
            &Location::setLongitude);

    connect(&this->m_site.location(),
            &Location::latitudeChanged,
            this->m_latitudeBox,
            &QDoubleSpinBox::setValue);

    connect(&this->m_site.location(),
            &Location::longitudeChanged,
            this->m_longitudeBox,
            &QDoubleSpinBox::setValue);
}
