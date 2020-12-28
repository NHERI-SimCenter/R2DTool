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

#include "SiteWidget.h"
#include "HBoxFormLayout.h"

#include <QtWidgets>

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
