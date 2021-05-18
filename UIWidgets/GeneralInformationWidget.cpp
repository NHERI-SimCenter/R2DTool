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

#include "GeneralInformationWidget.h"
#include "sectiontitle.h"
#include "SimCenterPreferences.h"

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

GeneralInformationWidget::GeneralInformationWidget(QWidget *parent)
    : SimCenterWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QHBoxLayout *theHeaderLayout = new QHBoxLayout();
    SectionTitle *label = new SectionTitle();
    label->setText(QString("General Information"));
    label->setMinimumWidth(150);

    theHeaderLayout->addWidget(label);
    QSpacerItem *spacer = new QSpacerItem(50,10);
    theHeaderLayout->addItem(spacer);
    theHeaderLayout->addStretch(1);

    auto infoLayout = this->getInfoLayout();

    mainLayout->addLayout(theHeaderLayout);
    mainLayout->addLayout(infoLayout);
    mainLayout->addStretch();

    this->setLayout(mainLayout);
    this->setMinimumWidth(640);
    this->setMaximumWidth(750);
}


GeneralInformationWidget::~GeneralInformationWidget()
{

}


bool GeneralInformationWidget::outputToJSON(QJsonObject &jsonObj)
{
    auto runDir = SimCenterPreferences::getInstance()->getLocalWorkDir();
    auto appDir = SimCenterPreferences::getInstance()->getAppDir();

    jsonObj.insert("Name", nameEdit->text());
    jsonObj.insert("Author", "SimCenter");
    jsonObj.insert("WorkflowType", "Parametric Study");
    jsonObj.insert("runDir", runDir);


    //jsonObj.insert("localAppDir", appDir);

    QJsonObject unitsObj;
    //   unitsObj.insert("force", "kips"/*unitsForceCombo->currentText()*/);
    //   unitsObj.insert("length", "in" /*unitsLengthCombo->currentText()*/);
    //   unitsObj.insert("time", "sec"/*unitsTimeCombo->currentText()*/);
    //    unitsObj.insert("temperature", unitsTemperatureCombo->currentText());

    unitsObj["force"] = unitEnumToString(unitsForceCombo->currentData().value<ForceUnit>());
    unitsObj["length"] = unitEnumToString(unitsLengthCombo->currentData().value<LengthUnit>());
    unitsObj["time"] = unitEnumToString(unitsTimeCombo->currentData().value<TimeUnit>());
    unitsObj["temperature"] = unitEnumToString(unitsTemperatureCombo->currentData().value<TemperatureUnit>());
    unitsObj["speed"] = unitEnumToString(unitsSpeedCombo->currentData().value<SpeedUnit>());

    QJsonObject outputsObj;
    outputsObj.insert("EDP", EDPCheckBox->isChecked());
    outputsObj.insert("DM", DMCheckBox->isChecked());
    outputsObj.insert("DV", DVCheckBox->isChecked());
    outputsObj.insert("every_realization", realizationCheckBox->isChecked());

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


bool GeneralInformationWidget::inputFromJSON(QJsonObject &jsonObject){

    if (jsonObject.contains("Name"))
        nameEdit->setText(jsonObject["Name"].toString());

    if (jsonObject.contains("units")) {
        QJsonObject unitsObj=jsonObject["units"].toObject();

        QJsonValue unitsForceValue = unitsObj["force"];
        ForceUnit forceUnit = unitStringToEnum<ForceUnit>(unitsForceValue.toString());
        int forceUnitIndex = unitsForceCombo->findData(forceUnit);
        unitsForceCombo->setCurrentIndex(forceUnitIndex);

        QJsonValue unitsLengthValue = unitsObj["length"];
        LengthUnit lengthUnit = unitStringToEnum<LengthUnit>(unitsLengthValue.toString());

        if(lengthUnit == LengthUnit::m || lengthUnit == LengthUnit::meter)
            unitsLengthCombo->setCurrentIndex(0);
        else if(lengthUnit == LengthUnit::cm || lengthUnit == LengthUnit::centimeter)
            unitsLengthCombo->setCurrentIndex(1);
        else if(lengthUnit == LengthUnit::mm || lengthUnit == LengthUnit::milimeter)
            unitsLengthCombo->setCurrentIndex(2);
        if(lengthUnit == LengthUnit::in || lengthUnit == LengthUnit::inch)
            unitsLengthCombo->setCurrentIndex(3);
        else if(lengthUnit == LengthUnit::ft || lengthUnit == LengthUnit::foot)
            unitsLengthCombo->setCurrentIndex(4);
        else
            unitsLengthCombo->setCurrentIndex(0);

        QJsonValue unitsSpeedValue = unitsObj["speed"];
        SpeedUnit speedUnit = unitStringToEnum<SpeedUnit>(unitsSpeedValue.toString());
        int speedUnitIndex = unitsSpeedCombo->findData(speedUnit);
        unitsSpeedCombo->setCurrentIndex(speedUnitIndex);

        QJsonValue unitsTimeValue = unitsObj["time"];

        if(!unitsTimeValue.isNull())
        {
            TimeUnit timeUnit = unitStringToEnum<TimeUnit>(unitsTimeValue.toString());
            if(timeUnit == TimeUnit::sec || timeUnit == TimeUnit::seconds)
                unitsTimeCombo->setCurrentIndex(0);
            else if(timeUnit == TimeUnit::min || timeUnit == TimeUnit::minutes)
                unitsTimeCombo->setCurrentIndex(1);
            else if(timeUnit == TimeUnit::hr || timeUnit == TimeUnit::hour)
                unitsTimeCombo->setCurrentIndex(2);
            else
                unitsTimeCombo->setCurrentIndex(0);
        }
    }

    if (jsonObject.contains("outputs")) {
        QJsonObject outObj=jsonObject["outputs"].toObject();

        EDPCheckBox->setChecked(outObj["EDP"].toBool());
        DMCheckBox->setChecked(outObj["DM"].toBool());
        DVCheckBox->setChecked(outObj["DV"].toBool());
        realizationCheckBox->setChecked(outObj["every_realization"].toBool());
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


QString GeneralInformationWidget::getAnalysisName(void)
{
    return nameEdit->text();
}


QGridLayout* GeneralInformationWidget::getInfoLayout(void)
{
    // Analysis information
    nameEdit = new QLineEdit(this);
    QGroupBox* analysisGroupBox = new QGroupBox("Analysis Name", this);
    analysisGroupBox->setContentsMargins(0,5,0,0);
    QFormLayout* analysisLayout = new QFormLayout(analysisGroupBox);
    analysisLayout->addRow(tr("Name"), nameEdit);
    analysisLayout->setAlignment(Qt::AlignLeft);
    analysisLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    analysisLayout->setRowWrapPolicy(QFormLayout::DontWrapRows);
    nameEdit->setText("R2D Analysis");

    unitsForceCombo = new QComboBox(this);
    unitsForceCombo->addItem("Newtons", ForceUnit::N);
    unitsForceCombo->addItem("Kilonewtons", ForceUnit::kN);
    unitsForceCombo->addItem("Pounds", ForceUnit::lb);
    unitsForceCombo->addItem("Kips", ForceUnit::kips);
    unitsForceCombo->setCurrentIndex(3);

    unitsLengthCombo = new QComboBox(this);
    unitsLengthCombo->addItem("Meters", LengthUnit::m);
    unitsLengthCombo->addItem("Centimeters", LengthUnit::cm);
    unitsLengthCombo->addItem("Millimeters", LengthUnit::mm);
    unitsLengthCombo->addItem("Inches", LengthUnit::inch);
    unitsLengthCombo->addItem("Feet", LengthUnit::ft);
    unitsLengthCombo->setCurrentIndex(3);

    unitsSpeedCombo = new QComboBox(this);
    unitsSpeedCombo->addItem("Miles per hour", SpeedUnit::mph);
    unitsSpeedCombo->addItem("Kilometers per hour", SpeedUnit::kph);
    unitsSpeedCombo->setCurrentIndex(0);

    unitsTemperatureCombo = new QComboBox(this);
    unitsTemperatureCombo->addItem("Celsius", TemperatureUnit::C);
    unitsTemperatureCombo->addItem("Fahrenheit", TemperatureUnit::F);
    unitsTemperatureCombo->addItem("Kelvin", TemperatureUnit::K);

    unitsTimeCombo = new QComboBox();
    unitsTimeCombo->addItem("Seconds", TimeUnit::sec);
    unitsTimeCombo->addItem("Minutes", TimeUnit::min);
    unitsTimeCombo->addItem("Hours", TimeUnit::hr);

    // Units
    QGroupBox* unitsGroupBox = new QGroupBox("Units", this);
    unitsGroupBox->setContentsMargins(0,5,0,0);
    QFormLayout* unitsFormLayout = new QFormLayout(unitsGroupBox);
    unitsFormLayout->addRow(tr("Force"), unitsForceCombo);
    unitsFormLayout->addRow(tr("Length"), unitsLengthCombo);
    unitsFormLayout->addRow(tr("Speed"), unitsSpeedCombo);
    unitsFormLayout->addRow(tr("Temperature"), unitsTemperatureCombo);
    unitsFormLayout->addRow(tr("Time"), unitsTimeCombo);
    unitsFormLayout->setAlignment(Qt::AlignLeft);
    unitsFormLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    unitsFormLayout->setRowWrapPolicy(QFormLayout::DontWrapRows);

    // Assets
    buildingsCheckBox = new QCheckBox("Buildings",this);
    soilCheckBox = new QCheckBox("Soil",this);
    gasCheckBox = new QCheckBox("Gas Network",this);
    waterCheckBox = new QCheckBox("Water Network",this);
    sewerCheckBox = new QCheckBox("Waste Network",this);
    transportationCheckBox = new QCheckBox("Transportation Network",this);

    soilCheckBox->setCheckable(false);
    gasCheckBox->setCheckable(false);
    waterCheckBox->setCheckable(false);
    sewerCheckBox->setCheckable(false);
    transportationCheckBox->setCheckable(false);

    soilCheckBox->setEnabled(false);
    gasCheckBox->setEnabled(false);
    waterCheckBox->setEnabled(false);
    sewerCheckBox->setEnabled(false);
    transportationCheckBox->setEnabled(false);

    connect(buildingsCheckBox, &QCheckBox::stateChanged, this, [=](){
        emit(assetChanged("Buildings",buildingsCheckBox->isChecked()));
    });
    connect(soilCheckBox, &QCheckBox::stateChanged, this, [=](){
        emit(assetChanged("Soil",soilCheckBox->isChecked()));
    });
    connect(gasCheckBox, &QCheckBox::stateChanged, this, [=](){
        emit(assetChanged("Gas Network",gasCheckBox->isChecked()));
    });
    connect(waterCheckBox, &QCheckBox::stateChanged, this, [=](){
        emit(assetChanged("Water Network",waterCheckBox->isChecked()));
    });
    connect(sewerCheckBox, &QCheckBox::stateChanged, this, [=](){
        this->assetChanged("Water Network",sewerCheckBox->isChecked());
    });
    connect(transportationCheckBox, &QCheckBox::stateChanged, this, [=](){
        emit(assetChanged("TransportationNetwork",transportationCheckBox->isChecked()));
    });


    QGroupBox* assetGroupBox = new QGroupBox("Asset Layers", this);
    assetGroupBox->setContentsMargins(0,5,0,0);
    QVBoxLayout* assetLayout = new QVBoxLayout(assetGroupBox);
    assetLayout->addWidget(buildingsCheckBox);
    assetLayout->addWidget(soilCheckBox);
    assetLayout->addWidget(gasCheckBox);
    assetLayout->addWidget(waterCheckBox);
    assetLayout->addWidget(sewerCheckBox);
    assetLayout->addWidget(transportationCheckBox);
    
    // Outputs
    EDPCheckBox = new QCheckBox("Engineering demand parameters (EDP)",this);
    DMCheckBox = new QCheckBox("Damage measures (DM)",this);
    DVCheckBox = new QCheckBox("Decision variables (DV)",this);
    realizationCheckBox = new QCheckBox("Output EDP, DM, and DV every sampling realization",this);

    EDPCheckBox->setChecked(true);
    DMCheckBox->setChecked(true);
    DVCheckBox->setChecked(true);
    realizationCheckBox->setChecked(false);

    QGroupBox* outputGroupBox = new QGroupBox("Output Settings", this);
    outputGroupBox->setContentsMargins(0,5,0,0);
    QVBoxLayout* outputLayout = new QVBoxLayout(outputGroupBox);
    outputLayout->addWidget(EDPCheckBox);
    outputLayout->addWidget(DMCheckBox);
    outputLayout->addWidget(DVCheckBox);
    outputLayout->addWidget(realizationCheckBox);

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


bool GeneralInformationWidget::setAssetTypeState(QString assetType, bool checkedStatus){
    if (assetType == "Buildings")
        buildingsCheckBox->setChecked(checkedStatus);
    return true;
}


template<typename UnitEnum> QString GeneralInformationWidget::unitEnumToString(UnitEnum enumValue)
{
    return QString(QMetaEnum::fromType<UnitEnum>().valueToKey(enumValue));
}


template<typename UnitEnum> UnitEnum GeneralInformationWidget::unitStringToEnum(QString unitString)
{
    return (UnitEnum)QMetaEnum::fromType<UnitEnum>().keyToValue(unitString.toStdString().c_str());
}


QString GeneralInformationWidget::getLengthUnit()
{
    return unitEnumToString(unitsLengthCombo->currentData().value<LengthUnit>());
}


QString GeneralInformationWidget::getForceUnit()
{
    return unitEnumToString(unitsForceCombo->currentData().value<ForceUnit>());
}


void GeneralInformationWidget::clear(void)
{
    nameEdit->clear();

    unitsForceCombo->setCurrentIndex(0);
    unitsLengthCombo->setCurrentIndex(0);
    unitsTemperatureCombo->setCurrentIndex(0);
    unitsTimeCombo->setCurrentIndex(0);

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
}
