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

#include "Vs30Widget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QSizePolicy>

Vs30Widget::Vs30Widget(Vs30& vs30, SiteConfig& siteConfig, QWidget *parent): QWidget(parent), m_vs30(vs30)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    QGroupBox* vs30GroupBox = new QGroupBox(this);
    vs30GroupBox->setTitle("Vs30 Model (if no Vs30 provided by users)");

    QHBoxLayout* formLayout = new QHBoxLayout(vs30GroupBox);
    m_typeBox = new QComboBox(this);

    m_typeBox->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);

    QLabel* typeLabel = new QLabel(tr("Type:"),this);

    formLayout->addWidget(typeLabel);
    formLayout->addWidget(m_typeBox);
//    formLayout->addStretch(1);

    vs30GroupBox->setLayout(formLayout);
    //FMK gmpeGroupBox->setMinimumWidth(400);
    //FMK gmpeGroupBox->setMaximumWidth(500);

    layout->addWidget(vs30GroupBox);
    this->setLayout(layout);

    QStringList validType;
    validType = this->m_vs30.validTypes();

    QStringListModel* typeModel = new QStringListModel(validType);
    m_typeBox->setModel(typeModel);
    m_typeBox->setCurrentIndex(validType.indexOf(m_vs30.type()));
    this->setupConnections();
}


void Vs30Widget::setupConnections()
{
    connect(this->m_typeBox, &QComboBox::currentTextChanged,
            &this->m_vs30, &Vs30::setType);

    connect(&this->m_vs30, &Vs30::typeChanged,
            this->m_typeBox, &QComboBox::setCurrentText);
}
