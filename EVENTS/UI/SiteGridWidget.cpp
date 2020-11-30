#include "SiteGridWidget.h"
#include "HBoxFormLayout.h"

SiteGridWidget::SiteGridWidget(SiteGrid& siteGrid, QWidget *parent) : QWidget(parent), m_siteGrid(siteGrid)
{
    gridCreated = false;

    //We use a grid layout for the site widget
    QGridLayout* layout = new QGridLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    QGroupBox* latitudeGroupBox = new QGroupBox(tr("Latitude"));
    latitudeGroupBox->setFlat(true);
    latitudeGroupBox->setStyleSheet("QGroupBox { font-weight: normal;}");

    m_minLatitudeBox = new QDoubleSpinBox(latitudeGroupBox);
    m_minLatitudeBox->setRange(-90.0, 90.0);
    m_minLatitudeBox->setDecimals(3);
    m_minLatitudeBox->setSingleStep(0.1);
    m_minLatitudeBox->setValue(m_siteGrid.latitude().min());
    HBoxFormLayout* latitudeFormLayout = new HBoxFormLayout(latitudeGroupBox);
    latitudeFormLayout->setContentsMargins(0,10,0,10);
    latitudeFormLayout->addField(tr("Min:"), this->m_minLatitudeBox);

    m_maxLatitudeBox = new QDoubleSpinBox(latitudeGroupBox);
    m_maxLatitudeBox->setRange(-90.0, 90.0);
    m_maxLatitudeBox->setDecimals(3);
    m_maxLatitudeBox->setSingleStep(0.1);
    m_maxLatitudeBox->setValue(m_siteGrid.latitude().max());
    latitudeFormLayout->addField(tr("Max:"), this->m_maxLatitudeBox);

    m_latitudeDivBox = new QSpinBox(latitudeGroupBox);
    m_latitudeDivBox->setRange(1, 60);
    m_latitudeDivBox->setValue(m_siteGrid.latitude().divisions());
    latitudeFormLayout->addField(tr("Divisions:"), this->m_latitudeDivBox);

    QGroupBox* longitudeGroupBox = new QGroupBox(tr("Longitude"));
    longitudeGroupBox->setFlat(true);
    longitudeGroupBox->setStyleSheet("QGroupBox { font-weight: normal;}");

    m_minLongitudeBox = new QDoubleSpinBox(longitudeGroupBox);
    m_minLongitudeBox->setRange(-180.0, 180.0);
    m_minLongitudeBox->setDecimals(3);
    m_minLongitudeBox->setSingleStep(0.1);
    m_minLongitudeBox->setValue(m_siteGrid.longitude().min());
    HBoxFormLayout* longitudeFormLayout = new HBoxFormLayout(longitudeGroupBox);
    longitudeFormLayout->setContentsMargins(0,10,0,10);
    longitudeFormLayout->addField(tr("Min:"), this->m_minLongitudeBox);

    m_maxLongitudeBox = new QDoubleSpinBox(longitudeGroupBox);
    m_maxLongitudeBox->setRange(-180.0, 180.0);
    m_maxLongitudeBox->setDecimals(3);
    m_maxLongitudeBox->setSingleStep(0.1);
    m_maxLongitudeBox->setValue(m_siteGrid.longitude().max());
    longitudeFormLayout->addField(tr("Max:"), this->m_maxLongitudeBox);

    m_longitudeDivBox = new QSpinBox(longitudeGroupBox);
    m_longitudeDivBox->setRange(1, 60);
    m_longitudeDivBox->setValue(m_siteGrid.longitude().divisions());
    longitudeFormLayout->addField(tr("Divisions:"), this->m_longitudeDivBox);

    selectGridButton = new QPushButton(tr("&Define Grid on Map"));

    layout->addWidget(latitudeGroupBox,0,0);
    layout->addWidget(longitudeGroupBox,1,0);
    layout->addWidget(selectGridButton,2,0,Qt::AlignLeft);

    //Now we need to setup the connections
    setupConnections();
}

void SiteGridWidget::setupConnections()
{
    //Connecting Latitude
    connect(this->m_minLatitudeBox,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            &m_siteGrid.latitude(),
            &GridDivision::setMin);

    connect(this->m_maxLatitudeBox,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            &m_siteGrid.latitude(),
            &GridDivision::setMax);

    connect(this->m_latitudeDivBox,
            QOverload<int>::of(&QSpinBox::valueChanged),
            &m_siteGrid.latitude(),
            &GridDivision::setDivisions);

    connect(&m_siteGrid.latitude(),
            &GridDivision::minChanged,
            this->m_minLatitudeBox,
            &QDoubleSpinBox::setValue);

    connect(&m_siteGrid.latitude(),
            &GridDivision::maxChanged,
            this->m_maxLatitudeBox,
            &QDoubleSpinBox::setValue);

    connect(&m_siteGrid.latitude(),
            &GridDivision::divisionsChanged,
            this->m_latitudeDivBox,
            &QSpinBox::setValue);

    //Connecting Longitude
    connect(this->m_minLongitudeBox,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            &m_siteGrid.longitude(),
            &GridDivision::setMin);

    connect(this->m_maxLongitudeBox,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            &m_siteGrid.longitude(),
            &GridDivision::setMax);

    connect(this->m_longitudeDivBox,
            QOverload<int>::of(&QSpinBox::valueChanged),
            &m_siteGrid.longitude(),
            &GridDivision::setDivisions);

    connect(&m_siteGrid.longitude(),
            &GridDivision::minChanged,
            this->m_minLongitudeBox,
            &QDoubleSpinBox::setValue);

    connect(&m_siteGrid.longitude(),
            &GridDivision::maxChanged,
            this->m_maxLongitudeBox,
            &QDoubleSpinBox::setValue);

    connect(&m_siteGrid.longitude(),
            &GridDivision::divisionsChanged,
            this->m_longitudeDivBox,
            &QSpinBox::setValue);

    connect(selectGridButton,
            &QPushButton::clicked,
            this,
            &SiteGridWidget::selectGridButtonPressed);

}

bool SiteGridWidget::getGridCreated() const
{
    return gridCreated;
}

void SiteGridWidget::selectGridButtonPressed()
{
    emit selectGridOnMap();
    gridCreated = true;
}
