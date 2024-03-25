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

// Written by: Jinyan Zhao


#include "GroundFailureWidget.h"
#include "SimCenterAppSelection.h"
#include "LiqTriggerZhuEtAl2017.h"
#include "LiqLateralHazus2020.h"
#include "LiqVerticalHazus2020.h"
#include "LiqTriggerHazus2020.h"
#include "NoneWidget.h"
#include "SimCenterUnitsCombo.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QComboBox>
#include <QSet>
#include <QMessageBox>
#include <QGroupBox>
#include <QCheckBox>
#include <QMessageBox>


GroundFailureWidget::GroundFailureWidget(QWidget *parent) : SimCenterAppWidget(parent)
{
    auto mainLayout = new QVBoxLayout(this);

    gfGroupBox = new QGroupBox(this);
    gfGroupBox->setTitle("Ground Failure Source(s)");
    gfGroupBox->setContentsMargins(0,0,0,0);

    QGridLayout* groupBoxLayout = new QGridLayout(gfGroupBox);
    gfGroupBox->setLayout(groupBoxLayout);

    liquefactionCheckBox = new QCheckBox("Liquefaction");
//    liquefactionCheckBox->setChecked(true);

    landslideCheckBox = new QCheckBox("Landslide");
    landslideCheckBox->setEnabled(false);
    faultDispCheckBox = new QCheckBox("Fault Displacement");
    faultDispCheckBox->setEnabled(false);

//    unitsCombo = new SimCenterUnitsCombo(SimCenter::Unit::LENGTH, "PGD", this);
//    unitsCombo->setCurrentUnit(SimCenter::Unit::m);
//    QLabel* unitLabel = new QLabel("PGD Output Unit:");

    groupBoxLayout->addWidget(liquefactionCheckBox, 0, 0);
    groupBoxLayout->addWidget(landslideCheckBox, 0, 1);
    groupBoxLayout->addWidget(faultDispCheckBox, 0, 2);
//    groupBoxLayout->addWidget(unitLabel, 1, 0);
//    groupBoxLayout->addWidget(unitsCombo, 1, 1,1,2);
//    groupBoxLayout->setColumnStretch(3,1);

    this->createLiquefactionGroupBox();
    this->setConnections();

    mainLayout->addWidget(gfGroupBox);
    mainLayout->addWidget(liquefactionGroupBox);
    mainLayout->addStretch(0);
    this->setLayout(mainLayout);
//    liquefactionWidget = new LiquefactionWidget();
//    mainLayout->addWidget(liquefactionWidget);

}


void GroundFailureWidget::createLiquefactionGroupBox(){
    liquefactionGroupBox = new QGroupBox(this);
    liquefactionGroupBox->setTitle("Liquefaction Models");
    liquefactionGroupBox->setContentsMargins(0,0,0,0);
//    liquefactionGroupBox->setCheckable(true);

    QVBoxLayout* groupBoxLayout = new QVBoxLayout(gfGroupBox);
    liquefactionGroupBox->setLayout(groupBoxLayout);

    liqTriggerSelection = new SimCenterAppSelection(QString("Liquefaction Triggering Model"), QString("Triggering"), this);
    liqLateralSelection = new SimCenterAppSelection(QString("Liquefaction Lateral Spreading Model"), QString("LateralSpreading"), this);
    liqVerticalSelection = new SimCenterAppSelection(QString("Liquefaction Settlement Model"), QString("Settlement"), this);

    SimCenterAppWidget* liqTriggerZhuEtAl2017 = new LiqTriggerZhuEtAl2017;
//    Hazus2020_with_ZhuEtal2017 and ZhuEtal2017 use the same UI widget
    SimCenterAppWidget* liqTriggerHazus2020withZhuEtAl2017 = new LiqTriggerZhuEtAl2017;
    SimCenterAppWidget* liqTriggerHazus2020 = new LiqTriggerHazus2020;
    liqTriggerSelection->addComponent(QString("Zhu et al. (2017)"), QString("ZhuEtal2017"), liqTriggerZhuEtAl2017);
    liqTriggerSelection->addComponent(QString("Zhu et al. (2017) susceptibility and Hazus (2020) probability"), QString("Hazus2020_with_ZhuEtal2017"), liqTriggerHazus2020withZhuEtAl2017);
    liqTriggerSelection->addComponent(QString("Geologic Map and Hazus (2020)"), QString("Hazus2020"), liqTriggerHazus2020);

    SimCenterAppWidget *noneWidgetLiqLat = new NoneWidget(this);
    SimCenterAppWidget* liqLateralHazus2020 = new LiqLateralHazus2020;
    liqLateralSelection->addComponent(QString("None"), QString("None"), noneWidgetLiqLat);
    liqLateralSelection->addComponent(QString("Hazus (2020)"), QString("Hazus2020Lateral"), liqLateralHazus2020);
    liqLateralSelection->selectComponent("Hazus (2020)");

    SimCenterAppWidget *noneWidgetLiqVer = new NoneWidget(this);
    SimCenterAppWidget* liqVerticalHazus2020 = new LiqVerticalHazus2020;
    liqVerticalSelection->addComponent(QString("None"), QString("None"), noneWidgetLiqVer);
    liqVerticalSelection->addComponent(QString("Hazus (2020)"), QString("Hazus2020Vertical"), liqVerticalHazus2020);
    liqVerticalSelection->selectComponent("Hazus (2020)");
    groupBoxLayout->addWidget(liqTriggerSelection);
    groupBoxLayout->addWidget(liqLateralSelection);
    groupBoxLayout->addWidget(liqVerticalSelection);

    liquefactionGroupBox->hide();



}


void GroundFailureWidget::reset(void)
{

}

void GroundFailureWidget::setConnections()
{
    connect(this->liquefactionCheckBox, &QCheckBox::stateChanged, this, &GroundFailureWidget::handleSourceSelectionChanged);
}


void GroundFailureWidget::handleSourceSelectionChanged()
{
    liquefactionGroupBox->hide();
//    landslideGroupBox->hide();
//    faultDispGroupBox->hide();
    if (liquefactionCheckBox->isChecked()){
        liquefactionGroupBox->show();
    }

}

bool GroundFailureWidget::inputFromJSON(QJsonObject& /*obj*/)
{
    return true;
}


bool GroundFailureWidget::outputToJSON(QJsonObject& obj)
{
    QJsonObject groundFailureObj;
    if (liquefactionCheckBox->isChecked()){
        QJsonObject liquefactionObj;
        liqTriggerSelection->outputToJSON(liquefactionObj);
        //Hazus2020_with_ZhuEtal2017 triggerin model uses the same widget with ZhuEtal2017
        //So the model name needs to be replaced
        if (liqTriggerSelection->getCurrentSelectionName().compare("Hazus2020_with_ZhuEtal2017")==0){
            QJsonObject triggeringObj = liquefactionObj["Triggering"].toObject();
            triggeringObj["Model"] = "Hazus2020_with_ZhuEtal2017";
            liquefactionObj["Triggering"] = triggeringObj;
        }
        liqLateralSelection->outputToJSON(liquefactionObj);
        liqVerticalSelection->outputToJSON(liquefactionObj);

        bool calcSettlement = true;
        if (liquefactionObj["Settlement"].toObject().isEmpty()){
            liquefactionObj.remove("Settlement");
            calcSettlement = false;
        }
        bool calcLateralSpread = true;
        if (liquefactionObj["LateralSpreading"].toObject().isEmpty()){
            liquefactionObj.remove("LateralSpreading");
            calcLateralSpread = false;
        }
        if ((!calcSettlement) && (!calcLateralSpread)){
            QMessageBox msgBox;
            msgBox.setText("Warning:\n"
                           "No models are selected for liquefaction lateral spreading nor settlement. Only triggering model will be run.\n"
                           "Continue running?");
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);

            auto res = msgBox.exec();

            if(res != QMessageBox::Yes)
                return false;
        }

        groundFailureObj["Liquefaction"] = liquefactionObj;
    }
    obj["GroundFailure"] = groundFailureObj;
//    obj["PGDunit"] = unitsCombo->currentText();
    return true;
}

//void GroundMotionModelsWidget::addGMMforSA(bool SAenabled){

//}

