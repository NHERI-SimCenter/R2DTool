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

// Written by: Stevan Gavrilovic, Frank McKenna

#include "SpatialCorrelationWidget.h"

#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QGroupBox>
#include "SC_ComboBox.h"


SpatialCorrelationWidget::SpatialCorrelationWidget(QWidget *parent): SimCenterAppWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    QGroupBox* spatCorrGroupBox = new QGroupBox(this);
    spatCorrGroupBox->setTitle("Spatial Correlation");
    spatCorrGroupBox->setContentsMargins(0,0,0,0);

    this->setMinimumWidth(375);

    QGridLayout* gridLayout = new QGridLayout(spatCorrGroupBox);
    spatCorrGroupBox->setLayout(gridLayout);

    spatialCorrelationInterLabel = new QLabel(tr("Inter-event\nSpatial Correlation Model:"),this);
    spatialCorrelationIntraLabel = new QLabel(tr("Intra-event\nSpatial Correlation Model:"),this);

    m_correlationBoxInter = new SC_ComboBox("SaInterEvent",QStringList({"Baker & Jayaram (2008)"}));
    m_correlationBoxIntra = new SC_ComboBox("SaIntraEvent",QStringList({"Markhvida et al. (2017)",
                                                                         "Jayaram & Baker (2009)",
                                                                         "Loth & Baker (2013)"}));


    gridLayout->addWidget(spatialCorrelationInterLabel,1,0);
    gridLayout->addWidget(m_correlationBoxInter,1,1);
    gridLayout->addWidget(spatialCorrelationIntraLabel,2,0);
    gridLayout->addWidget(m_correlationBoxIntra,2,1);


    layout->addWidget(spatCorrGroupBox);

    this->setLayout(layout);
}


bool SpatialCorrelationWidget::outputToJSON(QJsonObject& obj)
{
    m_correlationBoxInter->outputToJSON(obj);
    m_correlationBoxIntra->outputToJSON(obj);

    return true;
}


bool SpatialCorrelationWidget::inputFromJSON(QJsonObject& /*obj*/)
{
    return true;
}




void SpatialCorrelationWidget::handleAvailableModel(const QString sourceType)
{
    if (sourceType.compare("OpenQuake Classical")==0 || sourceType.compare("OpenQuake User-Specified")==0)
    {
        // No IM correlation is considered for UHS in Classical PSHA
        m_correlationBoxInter->hide();
        m_correlationBoxIntra->hide();
        spatialCorrelationIntraLabel->hide();
        spatialCorrelationInterLabel->hide();
    }
    else
    {
        m_correlationBoxInter->show();
        m_correlationBoxIntra->show();
        spatialCorrelationIntraLabel->show();
        spatialCorrelationInterLabel->show();
    }
}

