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
#include <QStringListModel>

GMPEWidget::GMPEWidget(GMPE& gmpe, QStringList* selectedIMTypes, QWidget *parent): SimCenterAppWidget(parent), m_gmpe(gmpe)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    gmpeGroupBox = new QGroupBox();
    gmpeGroupBox->setTitle("Ground Motion Model");

    const QStringList validType = this->m_gmpe.validTypes();


    QGridLayout* formLayout = new QGridLayout(gmpeGroupBox);
    PGAtypeBox = new SC_ComboBox("Type",validType);
    PGAtypeBox->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);
    PGVtypeBox = new SC_ComboBox("Type",validType);
    PGVtypeBox->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);
    SAtypeBox = new SC_ComboBox("Type",validType);
    SAtypeBox->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);

    formLayout->addWidget(PGAtypeLabel,0, 0);
    formLayout->addWidget(PGAtypeBox,0,1);

    formLayout->addWidget(SAtypeLabel,1, 0);
    formLayout->addWidget(SAtypeBox,1,1);

    formLayout->addWidget(PGVtypeLabel,2, 0);
    formLayout->addWidget(PGVtypeBox,2,1);


    toggleIMselection(selectedIMTypes);
    _selectedIMTypes = selectedIMTypes;

    layout->addWidget(gmpeGroupBox);


    QStringListModel* typeModel = new QStringListModel(validType);
    PGAtypeBox->setModel(typeModel);
    PGAtypeBox->setCurrentIndex(validType.indexOf(m_gmpe.type()));
    PGVtypeBox->setModel(typeModel);
    PGVtypeBox->setCurrentIndex(validType.indexOf(m_gmpe.type()));
    SAtypeBox->setModel(typeModel);
    SAtypeBox->setCurrentIndex(validType.indexOf(m_gmpe.type()));
    this->setupConnections();
}

void GMPEWidget::toggleIMselection(QStringList* selectedIMTypes){
    PGAtypeLabel->hide();
    PGAtypeBox->hide();
    SAtypeLabel->hide();
    SAtypeBox->hide();
    PGVtypeLabel->hide();
    PGVtypeBox->hide();
    _selectedIMTypes = selectedIMTypes;
    if (selectedIMTypes->contains("PGA")){
        PGAtypeLabel->show();
        PGAtypeBox->show();
    }
    if (selectedIMTypes->contains("SA")){
        SAtypeLabel->show();
        SAtypeBox->show();
    }
    if (selectedIMTypes->contains("PGV")){
        PGVtypeLabel->show();
        PGVtypeBox->show();
    }
    return;
}

void GMPEWidget::setupConnections()
{
    connect(this->PGAtypeBox, &QComboBox::currentTextChanged,
            &this->m_gmpe, &GMPE::setType);

    connect(&this->m_gmpe, &GMPE::typeChanged,
            this->PGAtypeBox, &QComboBox::setCurrentText);
}


bool GMPEWidget::inputFromJSON(QJsonObject& /*obj*/)
{

    return true;
}


bool GMPEWidget::outputToJSON(QJsonObject& obj)
{
    if (_selectedIMTypes->size()==1){
        if (_selectedIMTypes->contains("PGA")){
            PGAtypeBox->outputToJSON(obj);
        }
        if (_selectedIMTypes->contains("SA")){
            SAtypeBox->outputToJSON(obj);
        }
        if (_selectedIMTypes->contains("PGV")){
            PGVtypeBox->outputToJSON(obj);
        }
    } else {
        if (_selectedIMTypes->contains("PGA")){
            QJsonObject PGAgmpe;
            PGAtypeBox->outputToJSON(PGAgmpe);
            obj["PGA"] = PGAgmpe;
        }
        if (_selectedIMTypes->contains("SA")){
            QJsonObject SAgmpe;
            SAtypeBox->outputToJSON(SAgmpe);
            obj["SA"] = SAgmpe;
        }
        if (_selectedIMTypes->contains("PGV")){
            QJsonObject PGVgmpe;
            PGVtypeBox->outputToJSON(PGVgmpe);
            obj["PGV"] = PGVgmpe;
        }
    }
    obj["Parameters"] = QJsonObject();

    return true;
}



void GMPEWidget::handleAvailableGMPE(const QString sourceType)
{
    if (sourceType.compare("OpenQuake Classical")==0 || sourceType.compare("OpenQuake User-Specified")==0)
    {
        // users are expected to upload a GMPE logic tree, so the GMPE
        // widget needs to be hiden
        PGAtypeBox->hide();
        gmpeGroupBox->hide();
        this->setVisible(false);
    }
    else if (sourceType.compare("OpenQuake User-Specified")==0)
    {
        PGAtypeBox->hide();
        gmpeGroupBox->hide();
        this->setVisible(false);
    }
    else
    {
        PGAtypeBox->show();
        gmpeGroupBox->show();
        this->setVisible(true);
    }
}
