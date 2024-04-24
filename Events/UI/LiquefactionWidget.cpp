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


#include "LiquefactionWidget.h"
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


LiquefactionWidget::LiquefactionWidget(QWidget *parent) : SimCenterAppWidget(parent)
{
    auto mainLayout = new QVBoxLayout(this);

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

    mainLayout->addWidget(liqTriggerSelection);
    mainLayout->addWidget(liqLateralSelection);
    mainLayout->addWidget(liqVerticalSelection);
    mainLayout->addStretch(0);
    this->setLayout(mainLayout);
//    liquefactionWidget = new LiquefactionWidget();
//    mainLayout->addWidget(liquefactionWidget);

}


void LiquefactionWidget::reset(void)
{

}

//void LiquefactionWidget::setConnections()
//{
//    connect(this->liquefactionCheckBox, &QCheckBox::stateChanged, this, &GroundFailureWidget::handleSourceSelectionChanged);
//}


/*void LiquefactionWidget::handleSourceSelectionChanged()
{
    liquefactionGroupBox->hide();
//    landslideGroupBox->hide();
//    faultDispGroupBox->hide();
    if (liquefactionCheckBox->isChecked()){
        liquefactionGroupBox->show();
    }

}*/

bool LiquefactionWidget::inputFromJSON(QJsonObject& /*obj*/)
{
    return true;
}


bool LiquefactionWidget::outputToJSON(QJsonObject& obj)
{
    liqTriggerSelection->outputToJSON(obj);
    //Hazus2020_with_ZhuEtal2017 triggerin model uses the same widget with ZhuEtal2017
    //So the model name needs to be replaced
    if (liqTriggerSelection->getCurrentSelectionName().compare("Hazus2020_with_ZhuEtal2017")==0){
        QJsonObject triggeringObj = obj["Triggering"].toObject();
        triggeringObj["Model"] = "Hazus2020_with_ZhuEtal2017";
        obj["Triggering"] = triggeringObj;
    }
    liqLateralSelection->outputToJSON(obj);
    liqVerticalSelection->outputToJSON(obj);

    bool calcSettlement = true;
    if (obj["Settlement"].toObject().isEmpty()){
        obj.remove("Settlement");
        calcSettlement = false;
    }
    bool calcLateralSpread = true;
    if (obj["LateralSpreading"].toObject().isEmpty()){
        obj.remove("LateralSpreading");
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

    return true;
}

//void GroundMotionModelsWidget::addGMMforSA(bool SAenabled){

//}

