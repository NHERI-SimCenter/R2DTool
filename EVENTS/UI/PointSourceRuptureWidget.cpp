#include "PointSourceRuptureWidget.h"
#include "HBoxFormLayout.h"
#include "PointSourceRupture.h"

PointSourceRuptureWidget::PointSourceRuptureWidget(QWidget *parent, Qt::Orientation orientation) : QWidget(parent)
{

    this->m_magnitudeBox = new QDoubleSpinBox(this);
    this->m_magnitudeBox->setRange(4.5, 9.0);
    this->m_magnitudeBox->setDecimals(3);
    this->m_magnitudeBox->setSingleStep(0.001);
    this->m_magnitudeBox->setMinimumWidth(200);

    QVBoxLayout* siteBoxLayout = new QVBoxLayout(this);
    siteBoxLayout->setContentsMargins(0,0,0,0);
    siteBoxLayout->setMargin(0);

    QHBoxLayout* magnitudeLayout = new QHBoxLayout();

    QLabel* magLabel = new QLabel(tr("Magnitude:"),this);
    auto hspacer = new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Minimum);

    magnitudeLayout->addWidget(magLabel);
    magnitudeLayout->addWidget(m_magnitudeBox);
    magnitudeLayout->addItem(hspacer);

    this->m_locationGroupBox = new QGroupBox(tr("Location"));
    this->m_locationGroupBox->setContentsMargins(0,0,0,0);
    this->m_locationGroupBox->setFlat(true);

    this->m_latitudeBox = new QDoubleSpinBox(this->m_locationGroupBox);
    this->m_latitudeBox->setRange(-90.0, 90.0);
    this->m_latitudeBox->setDecimals(3);
    this->m_latitudeBox->setSingleStep(0.1);

    this->m_longitudeBox = new QDoubleSpinBox(this->m_locationGroupBox);
    this->m_longitudeBox->setRange(-180.0, 180.0);
    this->m_longitudeBox->setDecimals(3);
    this->m_longitudeBox->setSingleStep(0.1);

    this->m_depthBox = new QDoubleSpinBox(this->m_locationGroupBox);
    this->m_depthBox->setRange(0.0, 100.0);
    this->m_depthBox->setDecimals(3);
    this->m_depthBox->setSingleStep(0.01);


    HBoxFormLayout* locationFormLayout = new HBoxFormLayout(this->m_locationGroupBox);
    locationFormLayout->setContentsMargins(0,10,0,10);
    locationFormLayout->addField(tr("Latitude:"), this->m_latitudeBox);
    locationFormLayout->addField(tr("Longitude:"), this->m_longitudeBox);
    locationFormLayout->addField(tr("Depth:"), this->m_depthBox);


    this->m_geometryGroupBox = new QGroupBox(tr("Geometry"));
    this->m_geometryGroupBox->setContentsMargins(0,0,0,0);
    this->m_geometryGroupBox->setFlat(true);

    this->m_dipBox = new QDoubleSpinBox(this->m_geometryGroupBox);
    this->m_dipBox->setRange(0.0, 90.0);
    this->m_dipBox->setDecimals(2);
    this->m_dipBox->setSingleStep(0.01);

    this->m_rakeBox = new QDoubleSpinBox(this->m_geometryGroupBox);
    this->m_rakeBox->setRange(-180.0, 180.0);
    this->m_rakeBox->setDecimals(2);
    this->m_rakeBox->setSingleStep(0.01);

    HBoxFormLayout* geometryFormLayout = new HBoxFormLayout(this->m_geometryGroupBox);
    geometryFormLayout->setContentsMargins(0,10,0,10);
    geometryFormLayout->addField(tr("Average Dip:"), this->m_dipBox);
    geometryFormLayout->addField(tr("Average Rake:"), this->m_rakeBox);

    auto smallVSpacer = new QSpacerItem(0,10);
    auto VSpacer = new QSpacerItem(0,0,QSizePolicy::Minimum,QSizePolicy::Expanding);

    siteBoxLayout->addItem(smallVSpacer);
    siteBoxLayout->addLayout(magnitudeLayout);
    siteBoxLayout->addWidget(this->m_locationGroupBox);
    siteBoxLayout->addWidget(this->m_geometryGroupBox);
    siteBoxLayout->addItem(VSpacer);

    this->setLayout(siteBoxLayout);

    //We need to set initial values
    RuptureLocation location(37.9, -122.3);
    m_eqRupture = new PointSourceRupture(6.0, location, 0.0, 90.0, this);

    this->m_magnitudeBox->setValue(m_eqRupture->magnitude());
    this->m_latitudeBox->setValue(this->m_eqRupture->location().latitude());
    this->m_longitudeBox->setValue(this->m_eqRupture->location().longitude());
    this->m_depthBox->setValue(this->m_eqRupture->location().depth());

    this->m_dipBox->setValue(this->m_eqRupture->averageDip());
    this->m_rakeBox->setValue(this->m_eqRupture->averageRake());

    //Now, we will connect the ui to the data
    this->setupConnections();
}


void PointSourceRuptureWidget::setLocation(const double latitude, const double longitude)
{
    m_eqRupture->location().set(latitude,longitude);
}

PointSourceRupture* PointSourceRuptureWidget::getRuptureSource() const
{
    return m_eqRupture;
}

void PointSourceRuptureWidget::setupConnections()
{
    //Connecting magnitude
    connect(this->m_magnitudeBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this->m_eqRupture, &PointSourceRupture::setMagnitude);

    connect(this->m_eqRupture, &PointSourceRupture::magnitudeChanged,
            this->m_magnitudeBox, &QDoubleSpinBox::setValue);

    //Connecting latitude
    connect(this->m_latitudeBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            &this->m_eqRupture->location(), &Location::setLatitude);

    connect(&this->m_eqRupture->location(), &Location::latitudeChanged,
            this->m_latitudeBox, &QDoubleSpinBox::setValue);

    //Connecting longitude
    connect(this->m_longitudeBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            &this->m_eqRupture->location(), &Location::setLongitude);

    connect(&this->m_eqRupture->location(), &Location::longitudeChanged,
            this->m_longitudeBox, &QDoubleSpinBox::setValue);

    //Connecting depth
    connect(this->m_depthBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            &this->m_eqRupture->location(), &RuptureLocation::setDepth);

    connect(&this->m_eqRupture->location(), &RuptureLocation::depthChanged,
            this->m_depthBox, &QDoubleSpinBox::setValue);

    //Connecting dip
    connect(this->m_dipBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this->m_eqRupture, &PointSourceRupture::setAverageDip);

    connect(this->m_eqRupture, &PointSourceRupture::dipChanged,
            this->m_dipBox, &QDoubleSpinBox::setValue);

    //Connecting rake
    connect(this->m_rakeBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this->m_eqRupture, &PointSourceRupture::setAverageRake);

    connect(this->m_eqRupture, &PointSourceRupture::rakeChanged,
            this->m_rakeBox, &QDoubleSpinBox::setValue);
}
