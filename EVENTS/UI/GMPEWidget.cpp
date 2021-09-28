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

#include "GMPEWidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QSizePolicy>

GMPEWidget::GMPEWidget(GMPE& gmpe, QWidget *parent): QWidget(parent), m_gmpe(gmpe)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    gmpeGroupBox = new QGroupBox(this);
    gmpeGroupBox->setTitle("Ground Motion Prediction Equation");

    QHBoxLayout* formLayout = new QHBoxLayout(gmpeGroupBox);
    m_typeBox = new QComboBox(this);

    QLabel* typeLabel = new QLabel(tr("Type:"),this);

    formLayout->addWidget(typeLabel);
    formLayout->addWidget(m_typeBox);
//    formLayout->addStretch(1);

    gmpeGroupBox->setLayout(formLayout);
    //FMK gmpeGroupBox->setMinimumWidth(400);
    //FMK gmpeGroupBox->setMaximumWidth(500);

    layout->addWidget(gmpeGroupBox);
    this->setLayout(layout);

    const QStringList validType = this->m_gmpe.validTypes();

    QStringListModel* typeModel = new QStringListModel(validType);
    m_typeBox->setModel(typeModel);
    m_typeBox->setCurrentIndex(validType.indexOf(m_gmpe.type()));
    this->setupConnections();
}


void GMPEWidget::setupConnections()
{
    connect(this->m_typeBox, &QComboBox::currentTextChanged,
            &this->m_gmpe, &GMPE::setType);

    connect(&this->m_gmpe, &GMPE::typeChanged,
            this->m_typeBox, &QComboBox::setCurrentText);
}


void GMPEWidget::handleAvailableGMPE(const QString sourceType)
{
    if (sourceType.compare("OpenQuake Classical")==0)
    {
        // users are expected to upload a GMPE logic tree, so the GMPE
        // widget needs to be hiden
        m_typeBox->hide();
        gmpeGroupBox->hide();
        this->setVisible(false);
    }
    else
    {
        m_typeBox->show();
        gmpeGroupBox->show();
        this->setVisible(true);
    }
}
