/* *****************************************************************************
Copyright (c) 2016-2017, The Regents of the University of California (Regents).
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

#include "RecordSelectionWidget.h"

RecordSelectionWidget::RecordSelectionWidget(RecordSelectionConfig& selectionConfig, QWidget *parent) : QWidget(parent), m_selectionConfig(selectionConfig)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    QGroupBox* selectionGroupBox = new QGroupBox(this);
    selectionGroupBox->setTitle("Record Selection");
    selectionGroupBox->setContentsMargins(0,0,0,0);

    //selectionGroupBox->setMinimumWidth(400);
    //selectionGroupBox->setMaximumWidth(500);

    QGridLayout* formLayout = new QGridLayout(selectionGroupBox);

    QLabel* databaseLabel = new QLabel(tr("Database:"),this);
    m_dbBox = new QComboBox(this);
    m_dbBox->addItem("PEER NGA West 2");
    connect(this->m_dbBox, &QComboBox::currentTextChanged, &this->m_selectionConfig, &RecordSelectionConfig::setDatabase);
    m_dbBox->setCurrentText("PEER NGA West 2");
    m_selectionConfig.setDatabase("PEER NGA West 2");
    m_dbBox->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);

    formLayout->addWidget(databaseLabel,0,0);
    formLayout->addWidget(m_dbBox,0,1);

    selectionGroupBox->setLayout(formLayout);

    layout->addWidget(selectionGroupBox);

    this->setLayout(layout);
    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);

}


