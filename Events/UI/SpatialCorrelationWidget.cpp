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
#include <QJsonObject>


SpatialCorrelationWidget::SpatialCorrelationWidget(QStringList* selectedIMTypes, QWidget *parent):
    SimCenterAppWidget(parent), _selectedIMTypes(selectedIMTypes)
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    QGroupBox* interCorrGroupBox = new QGroupBox(this);
    interCorrGroupBox->setTitle("Inter-event Spatial Correlation");
    interCorrGroupBox->setContentsMargins(0,0,0,0);

    this->setMinimumWidth(375);

    QGridLayout* gridLayoutInter = new QGridLayout(interCorrGroupBox);
    interCorrGroupBox->setLayout(gridLayoutInter);

//    spatialCorrelationInterLabel = new QLabel(tr("Inter-event Spatial Correlation Model:"),this);
//    spatialCorrelationIntraLabel = new QLabel(tr("Intra-event Spatial Correlation Model:"),this);

    PGAcorrelationBoxInter = new SC_ComboBox("InterEventCorr",QStringList({"Baker & Jayaram (2008)"}));
    SAcorrelationBoxInter = new SC_ComboBox("InterEventCorr",QStringList({"Baker & Jayaram (2008)"}));
    PGVcorrelationBoxInter = new SC_ComboBox("InterEventCorr",QStringList({"Baker & Bradley (2017)"}));


    gridLayoutInter->addWidget(PGAtypeLabelInter, 0, 0);
    gridLayoutInter->addWidget(PGAcorrelationBoxInter,0,1);
    gridLayoutInter->addWidget(SAtypeLabelInter,1,0);
    gridLayoutInter->addWidget(SAcorrelationBoxInter,1,1);
    gridLayoutInter->addWidget(PGVtypeLabelInter,2,0);
    gridLayoutInter->addWidget(PGVcorrelationBoxInter,2,1);
    gridLayoutInter->setColumnStretch(1,1);

    QGroupBox* intraCorrGroupBox = new QGroupBox(this);
    intraCorrGroupBox->setTitle("Intra-event Spatial Correlation");
    intraCorrGroupBox->setContentsMargins(0,0,0,0);

    this->setMinimumWidth(375);

    QGridLayout* gridLayoutIntra = new QGridLayout(intraCorrGroupBox);
    intraCorrGroupBox->setLayout(gridLayoutInter);

    PGAcorrelationBoxIntra = new SC_ComboBox("IntraEventCorr",QStringList({"Markhvida et al. (2017)",
                                                                         "Jayaram & Baker (2009)",
                                                                         "Loth & Baker (2013)",
                                                                            "Du & Ning (2021)"}));
    SAcorrelationBoxIntra = new SC_ComboBox("IntraEventCorr",QStringList({"Markhvida et al. (2017)",
                                                                          "Jayaram & Baker (2009)",
                                                                          "Loth & Baker (2013)",
                                                                           "Du & Ning (2021)"}));
    PGVcorrelationBoxIntra = new SC_ComboBox("IntraEventCorr",QStringList({"Du & Ning (2021)"}));

    gridLayoutIntra->addWidget(PGAtypeLabelIntra, 0, 0);
    gridLayoutIntra->addWidget(PGAcorrelationBoxIntra,0,1);
    gridLayoutIntra->addWidget(SAtypeLabelIntra,1,0);
    gridLayoutIntra->addWidget(SAcorrelationBoxIntra,1,1);
    gridLayoutIntra->addWidget(PGVtypeLabelIntra,2,0);
    gridLayoutIntra->addWidget(PGVcorrelationBoxIntra,2,1);
    gridLayoutIntra->setColumnStretch(1,1);

    toggleIMselection(selectedIMTypes);

    layout->addWidget(interCorrGroupBox);
    layout->addWidget(intraCorrGroupBox);

    this->setLayout(layout);
}

void SpatialCorrelationWidget::toggleIMselection(QStringList* selectedIMTypes){
    PGAtypeLabelInter->hide();
    PGAtypeLabelIntra->hide();
    SAtypeLabelInter->hide();
    SAtypeLabelIntra->hide();
    PGVtypeLabelInter->hide();
    PGVtypeLabelIntra->hide();
    PGAcorrelationBoxIntra->hide();
    PGAcorrelationBoxInter->hide();
    SAcorrelationBoxIntra->hide();
    SAcorrelationBoxInter->hide();
    PGVcorrelationBoxIntra->hide();
    PGVcorrelationBoxInter->hide();
    _selectedIMTypes = selectedIMTypes;
    if (selectedIMTypes->contains("PGA")){
        PGAtypeLabelInter->show();
        PGAtypeLabelIntra->show();
        PGAcorrelationBoxIntra->show();
        PGAcorrelationBoxInter->show();
    }
    if (selectedIMTypes->contains("SA")){
        SAtypeLabelInter->show();
        SAtypeLabelIntra->show();
        SAcorrelationBoxIntra->show();
        SAcorrelationBoxInter->show();
    }
    if (selectedIMTypes->contains("PGV")){
        PGVtypeLabelInter->show();
        PGVtypeLabelIntra->show();
        PGVcorrelationBoxIntra->show();
        PGVcorrelationBoxInter->show();
    }
    return;
}
bool SpatialCorrelationWidget::outputToJSON(QJsonObject& obj)
{
    if (_selectedIMTypes->size()==1){
        if (_selectedIMTypes->contains("PGA")){
            obj["SaInterEvent"] = PGAcorrelationBoxInter->currentText();
            obj["SaIntraEvent"] = PGAcorrelationBoxIntra->currentText();
        }
        if (_selectedIMTypes->contains("SA")){
            obj["SaInterEvent"] = SAcorrelationBoxInter->currentText();
            obj["SaIntraEvent"] = SAcorrelationBoxIntra->currentText();
        }
        if (_selectedIMTypes->contains("PGV")){
            obj["SaInterEvent"] = PGVcorrelationBoxInter->currentText();
            obj["SaIntraEvent"] = PGVcorrelationBoxIntra->currentText();
        }
    } else {
        if (_selectedIMTypes->contains("PGA")){
            QJsonObject PGAgmpe;
            PGAcorrelationBoxInter->outputToJSON(PGAgmpe);
            PGAcorrelationBoxIntra->outputToJSON(PGAgmpe);
            obj["PGA"] = PGAgmpe;
        }
        if (_selectedIMTypes->contains("SA")){
            QJsonObject SAgmpe;
            SAcorrelationBoxInter->outputToJSON(SAgmpe);
            SAcorrelationBoxIntra->outputToJSON(SAgmpe);
            obj["SA"] = SAgmpe;
        }
        if (_selectedIMTypes->contains("PGV")){
            QJsonObject PGVgmpe;
            PGVcorrelationBoxInter->outputToJSON(PGVgmpe);
            PGVcorrelationBoxIntra->outputToJSON(PGVgmpe);
            obj["PGV"] = PGVgmpe;
        }
    }

    return true;
}


bool SpatialCorrelationWidget::inputFromJSON(QJsonObject& /*obj*/)
{
    return true;
}




//void SpatialCorrelationWidget::handleAvailableModel(const QString sourceType)
//{
//    if (sourceType.compare("OpenQuake Classical")==0 || sourceType.compare("OpenQuake User-Specified")==0)
//    {
//        // No IM correlation is considered for UHS in Classical PSHA
//        m_correlationBoxInter->hide();
//        m_correlationBoxIntra->hide();
//        spatialCorrelationIntraLabel->hide();
//        spatialCorrelationInterLabel->hide();
//    }
//    else
//    {
//        m_correlationBoxInter->show();
//        m_correlationBoxIntra->show();
//        spatialCorrelationIntraLabel->show();
//        spatialCorrelationInterLabel->show();
//    }
//}

