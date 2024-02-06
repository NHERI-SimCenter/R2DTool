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
#include "SC_ComboBox.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QStringListModel>

GMPEWidget::GMPEWidget(GMPE& gmpe, QWidget *parent): SimCenterAppWidget(parent), m_gmpe(gmpe)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    gmpeGroupBox = new QGroupBox();
    gmpeGroupBox->setTitle("Ground Motion Model");

    const QStringList validType = this->m_gmpe.validTypes();


    QHBoxLayout* formLayout = new QHBoxLayout(gmpeGroupBox);
    m_typeBox = new SC_ComboBox("Type",validType);

    m_typeBox->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);

    QLabel* typeLabel = new QLabel(tr("Type:"));

    formLayout->addWidget(typeLabel);
    formLayout->addWidget(m_typeBox);

    layout->addWidget(gmpeGroupBox);


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


bool GMPEWidget::inputFromJSON(QJsonObject& /*obj*/)
{

    return true;
}


bool GMPEWidget::outputToJSON(QJsonObject& obj)
{
    m_typeBox->outputToJSON(obj);

    obj["Parameters"] = QJsonObject();

    return true;
}



void GMPEWidget::handleAvailableGMPE(const QString sourceType)
{
    if (sourceType.compare("OpenQuake Classical")==0 || sourceType.compare("OpenQuake User-Specified")==0)
    {
        // users are expected to upload a GMPE logic tree, so the GMPE
        // widget needs to be hiden
        m_typeBox->hide();
        gmpeGroupBox->hide();
        this->setVisible(false);
    }
    else if (sourceType.compare("OpenQuake User-Specified")==0)
    {
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
