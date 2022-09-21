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

#include "GeneralInformationWidgetR2D.h"
#include "sectiontitle.h"
#include "SimCenterPreferences.h"
#include "SimCenterUnitsCombo.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDebug>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QMetaEnum>
#include <QPushButton>

GeneralInformationWidgetR2D::GeneralInformationWidgetR2D(QWidget *parent)
    : SimCenterWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(0);

    QHBoxLayout *theHeaderLayout = new QHBoxLayout();
    SectionTitle *label = new SectionTitle();
    label->setText(QString("General Information"));
    label->setMinimumWidth(150);

    theHeaderLayout->addWidget(label);
    QSpacerItem *spacer = new QSpacerItem(50,0);
    theHeaderLayout->addItem(spacer);
    theHeaderLayout->addStretch(1);

    auto infoLayout = this->getInfoLayout();

    mainLayout->addLayout(theHeaderLayout);
    mainLayout->addLayout(infoLayout);
    mainLayout->addStretch();

    this->setMaximumWidth(750);
}


GeneralInformationWidgetR2D::~GeneralInformationWidgetR2D()
{

}


bool GeneralInformationWidgetR2D::outputToJSON(QJsonObject &jsonObj)
{
    auto runDir = SimCenterPreferences::getInstance()->getLocalWorkDir();
    // auto appDir = SimCenterPreferences::getInstance()->getAppDir();

    jsonObj.insert("Name", nameEdit->text());
    jsonObj.insert("Author", "SimCenter");
    jsonObj.insert("WorkflowType", "Parametric Study");
    jsonObj.insert("runDir", runDir);


    //jsonObj.insert("localAppDir", appDir);

    QJsonObject unitsObj;
    unitsObj["force"] = unitsForceCombo->getCurrentUnitString();
    unitsObj["length"] = unitsLengthCombo->getCurrentUnitString();
    unitsObj["time"] = unitsTimeCombo->getCurrentUnitString();

    QJsonObject outputsObj;
    outputsObj.insert("EDP", EDPCheckBox->isChecked());
    outputsObj.insert("DM", DMCheckBox->isChecked());
    outputsObj.insert("DV", DVCheckBox->isChecked());
    outputsObj.insert("every_realization", realizationCheckBox->isChecked());
    outputsObj.insert("AIM", AIMCheckBox->isChecked());
    outputsObj.insert("IM", IMCheckBox->isChecked());


    QJsonObject assetsObj;
    assetsObj.insert("buildings", buildingsCheckBox->isChecked());
    assetsObj.insert("soil", soilCheckBox->isChecked());
    assetsObj.insert("gas", gasCheckBox->isChecked());
    assetsObj.insert("water", waterCheckBox->isChecked());
    assetsObj.insert("waste", sewerCheckBox->isChecked());
    assetsObj.insert("transportation", transportationCheckBox->isChecked());

    jsonObj.insert("units",unitsObj);
    jsonObj.insert("outputs",outputsObj);
    jsonObj.insert("assets",assetsObj);

    return true;
}


bool GeneralInformationWidgetR2D::inputFromJSON(QJsonObject &jsonObject){

    if (jsonObject.contains("Name"))
        nameEdit->setText(jsonObject["Name"].toString());

    if (jsonObject.contains("units")) {
        QJsonObject unitsObj=jsonObject["units"].toObject();

        QJsonValue unitsForceValue = unitsObj["force"];
        if(!unitsForceValue.isNull())
        {
            unitsForceCombo->setCurrentUnitString(unitsForceValue.toString());
        }

        QJsonValue unitsLengthValue = unitsObj["length"];
        if(!unitsLengthValue.isNull())
        {
            unitsLengthCombo->setCurrentUnitString(unitsLengthValue.toString());
        }

        QJsonValue unitsTimeValue = unitsObj["time"];
        if(!unitsTimeValue.isNull())
        {
            unitsTimeCombo->setCurrentUnitString(unitsTimeValue.toString());
        }
    }

    if (jsonObject.contains("outputs")) {
        QJsonObject outObj=jsonObject["outputs"].toObject();

        EDPCheckBox->setChecked(outObj["EDP"].toBool());
        DMCheckBox->setChecked(outObj["DM"].toBool());
        DVCheckBox->setChecked(outObj["DV"].toBool());
        realizationCheckBox->setChecked(outObj["every_realization"].toBool());
        AIMCheckBox->setChecked(outObj["AIM"].toBool());
        IMCheckBox->setChecked(outObj["IM"].toBool());
    }

    if (jsonObject.contains("assets")) {
        QJsonObject outObj=jsonObject["assets"].toObject();

        buildingsCheckBox->setChecked(outObj["buildings"].toBool());
        soilCheckBox->setChecked(outObj["soil"].toBool());
        gasCheckBox->setChecked(outObj["gas"].toBool());
        waterCheckBox->setChecked(outObj["water"].toBool());
        sewerCheckBox->setChecked(outObj["waste"].toBool());
        transportationCheckBox->setChecked(outObj["transportation"].toBool());
    }

    return true;
}


QString GeneralInformationWidgetR2D::getAnalysisName(void)
{
    return nameEdit->text();
}


QGridLayout* GeneralInformationWidgetR2D::getInfoLayout(void)
{
    // Analysis information
    nameEdit = new QLineEdit();
    QGroupBox* analysisGroupBox = new QGroupBox("Analysis Name");
    analysisGroupBox->setContentsMargins(0,5,0,0);
    QFormLayout* analysisLayout = new QFormLayout(analysisGroupBox);
    analysisLayout->addRow(tr("Name"), nameEdit);
    analysisLayout->setAlignment(Qt::AlignLeft);
    analysisLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    analysisLayout->setRowWrapPolicy(QFormLayout::DontWrapRows);
    nameEdit->setText("R2D Analysis");

    unitsForceCombo = new SimCenterUnitsCombo(SimCenter::Unit::FORCE,"force");
    unitsLengthCombo = new SimCenterUnitsCombo(SimCenter::Unit::LENGTH,"length");
    unitsTimeCombo = new SimCenterUnitsCombo(SimCenter::Unit::TIME,"time");

    // Units
    QGroupBox* unitsGroupBox = new QGroupBox("Units");
    unitsGroupBox->setContentsMargins(0,5,0,0);
    QFormLayout* unitsFormLayout = new QFormLayout(unitsGroupBox);
    unitsFormLayout->addRow(tr("Force"), unitsForceCombo);
    unitsFormLayout->addRow(tr("Length"), unitsLengthCombo);
    unitsFormLayout->addRow(tr("Time"), unitsTimeCombo);
    unitsFormLayout->setAlignment(Qt::AlignLeft);
    unitsFormLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    unitsFormLayout->setRowWrapPolicy(QFormLayout::DontWrapRows);

    // Assets
    buildingsCheckBox = new QCheckBox("Buildings");
    soilCheckBox = new QCheckBox("Soil");
    gasCheckBox = new QCheckBox("Gas Network");
    waterCheckBox = new QCheckBox("Water Network");
    sewerCheckBox = new QCheckBox("Waste Network");
    transportationCheckBox = new QCheckBox("Transportation Network");

    soilCheckBox->setCheckable(false);
    gasCheckBox->setCheckable(true);
    waterCheckBox->setCheckable(true);
    sewerCheckBox->setCheckable(false);
    transportationCheckBox->setCheckable(false);

    soilCheckBox->setEnabled(false);
    gasCheckBox->setEnabled(true);
    waterCheckBox->setEnabled(true);
    sewerCheckBox->setEnabled(false);
    transportationCheckBox->setEnabled(false);

    connect(buildingsCheckBox, &QCheckBox::stateChanged, this, [=](){
        emit assetChanged("Buildings",buildingsCheckBox->isChecked());
    });
    connect(soilCheckBox, &QCheckBox::stateChanged, this, [=](){
        emit assetChanged("Soil",soilCheckBox->isChecked());
    });
    connect(gasCheckBox, &QCheckBox::stateChanged, this, [=](){
        emit assetChanged("Gas Network",gasCheckBox->isChecked());
    });
    connect(waterCheckBox, &QCheckBox::stateChanged, this, [=](){
        emit assetChanged("Water Network",waterCheckBox->isChecked());
    });
    connect(sewerCheckBox, &QCheckBox::stateChanged, this, [=](){
       emit assetChanged("Sewer Network",sewerCheckBox->isChecked());
    });
    connect(transportationCheckBox, &QCheckBox::stateChanged, this, [=](){
        emit assetChanged("TransportationNetwork",transportationCheckBox->isChecked());
    });


    QGroupBox* assetGroupBox = new QGroupBox("Asset Layers");
    assetGroupBox->setContentsMargins(0,5,0,0);
    QVBoxLayout* assetLayout = new QVBoxLayout(assetGroupBox);
    assetLayout->addWidget(buildingsCheckBox);
    assetLayout->addWidget(soilCheckBox);
    assetLayout->addWidget(gasCheckBox);
    assetLayout->addWidget(waterCheckBox);
    assetLayout->addWidget(sewerCheckBox);
    assetLayout->addWidget(transportationCheckBox);
    
    // Outputs
    EDPCheckBox = new QCheckBox("Engineering demand parameters (EDP)");
    DMCheckBox = new QCheckBox("Damage measures (DM)");
    DVCheckBox = new QCheckBox("Decision variables (DV)");
    realizationCheckBox = new QCheckBox("Output EDP, DM, and DV every sampling realization");
    AIMCheckBox = new QCheckBox("Output Asset Information Model (AIM)");
    IMCheckBox = new QCheckBox("Output site IM");

    EDPCheckBox->setChecked(true);
    DMCheckBox->setChecked(true);
    DVCheckBox->setChecked(true);
    realizationCheckBox->setChecked(false);
    AIMCheckBox->setChecked(false);
    IMCheckBox->setChecked(false);

    QGroupBox* outputGroupBox = new QGroupBox("Output Settings");
    outputGroupBox->setContentsMargins(0,5,0,0);
    QVBoxLayout* outputLayout = new QVBoxLayout(outputGroupBox);
    outputLayout->addWidget(EDPCheckBox);
    outputLayout->addWidget(DMCheckBox);
    outputLayout->addWidget(DVCheckBox);
    outputLayout->addWidget(realizationCheckBox);
    outputLayout->addWidget(AIMCheckBox);
    outputLayout->addWidget(IMCheckBox);


    /*
    QGroupBox* regionalMappingGroupBox = new QGroupBox("Regional Mapping", this);
    regionalMappingGroupBox->setContentsMargins(0,5,0,0);

    regionalMappingGroupBox->setLayout(theRegionalMappingWidget->layout());

    //    regionalMappingGroupBox->addWidget(theRegionalMappingWidget);
    */

    QSpacerItem *spacer = new QSpacerItem(0,20);

    // Layout the UI components
    QGridLayout* layout = new QGridLayout();
    layout->addWidget(analysisGroupBox,0,0);
    layout->addItem(spacer,1,0);
    layout->addWidget(unitsGroupBox,2,0);
    layout->addItem(spacer,3,0);
    layout->addWidget(assetGroupBox,4,0);
    layout->addItem(spacer,5,0);
    layout->addWidget(outputGroupBox,6,0);
    layout->addItem(spacer,7,0);
    //layout->addWidget(regionalMappingGroupBox,8,0);

    return layout;
}


bool GeneralInformationWidgetR2D::setAssetTypeState(QString assetType, bool checkedStatus){
    if (assetType == "Buildings")
        buildingsCheckBox->setChecked(checkedStatus);
    return true;
}



void GeneralInformationWidgetR2D::clear(void)
{
    nameEdit->clear();

    unitsForceCombo->setCurrentUnit(SimCenter::Unit::lb);
    unitsLengthCombo->setCurrentUnit(SimCenter::Unit::ft);
    unitsTimeCombo->setCurrentUnit(SimCenter::Unit::sec);

    buildingsCheckBox->setChecked(true);
    soilCheckBox->setChecked(false);
    waterCheckBox->setChecked(false);
    sewerCheckBox->setChecked(false);
    gasCheckBox->setChecked(false);
    transportationCheckBox->setChecked(false);

    EDPCheckBox->setChecked(false);
    DMCheckBox->setChecked(false);
    DVCheckBox->setChecked(false);
    realizationCheckBox->setChecked(false);
    AIMCheckBox->setChecked(false);
    IMCheckBox->setChecked(false);
}
