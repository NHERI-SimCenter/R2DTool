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

#include "LiqTriggerHazus2020.h"
#include "SimCenterPreferences.h"
#include "CRSSelectionWidget.h"
#include "SC_ComboBox.h"

#include <QLabel>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QSpacerItem>
#include <QPushButton>
#include <HBoxFormLayout.h>
#include <QFileDialog>
#include <QMessageBox>
#include <QSignalMapper>
#include <QDir>
#include <QMimeData>
#include "qgsrasterlayer.h"
#include "qgsvectorlayer.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsvectorfilewriter.h"
#include <QMap>
#include <QCheckBox>
#include <QGroupBox>
#include <QJsonArray>

LiqTriggerHazus2020::LiqTriggerHazus2020(QWidget *parent) : SimCenterAppWidget(parent)
{
    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    //We use a grid layout for the Rupture widget
    QGridLayout* layout = new QGridLayout(this);



    QLabel* GwDepthfilenameLabel = new QLabel(tr("Ground Water Table Depth (m):"),this);
    GwDepthFilenameLineEdit = new QLineEdit(this);
    GwDepthDefineMethodComboBox = new SC_ComboBox("GwDepth", QStringList({"Defined (\"gwDepth\") in Site File (.csv)", "Define with raster (nearest pixel)"}));
    GwDepthBrowseFileButton = new QPushButton();
    GwDepthBrowseFileButton->setText(tr("Browse"));
    GwDepthBrowseFileButton->setMaximumWidth(150);

    QLabel* GeologicMapFilenameLabel = new QLabel(tr("Liquefaction Susceptibility:"),this);
    GeologicMapFilenameLineEdit = new QLineEdit(this);
    GeologicMapBrowseFileButton = new QPushButton();
    GeologicMapBrowseFileButton->setText(tr("Browse"));
    GeologicMapBrowseFileButton->setMaximumWidth(150);
    GeoMapDefineMethodComboBox = new SC_ComboBox("LiqSusc", QStringList({"Defined (\"liqSusc\") in Site File (.csv)", "Define with vector"}));

    LiqSuscKeyLabel = new QLabel(tr("Susceptibility Field Name:"),this);
    LiqSuscKeyLineEdit = new QLineEdit(this);

    resetToDefaultButton = new QPushButton();
    resetToDefaultButton->setText(tr("Reset to Default"));
    resetToDefaultButton->setMaximumWidth(150);

    // https://stackoverflow.com/questions/5153157/passing-an-argument-to-a-slot
    QSignalMapper* signalMapper = new QSignalMapper (this) ;
    connect(GwDepthBrowseFileButton,SIGNAL(clicked()),signalMapper, SLOT(map()));
    connect(GeologicMapBrowseFileButton,SIGNAL(clicked()),signalMapper, SLOT(map()));

    signalMapper->setMapping(GwDepthBrowseFileButton, QString("Ground Water Table Depth"));
    signalMapper->setMapping(GeologicMapBrowseFileButton, QString("Geologic Map"));

    connect(signalMapper, SIGNAL(mappedString(QString)), this, SLOT(loadFile(QString)));

    connect(resetToDefaultButton, &QPushButton::clicked, this, &LiqTriggerHazus2020::setDefaultFilePath);

    crsSelectorWidget = new CRSSelectionWidget();

    QCheckBox* LiqSuscSaveCheckBox = new QCheckBox("liq_susc");
    outputSaveCheckBoxes.insert("liq_susc", LiqSuscSaveCheckBox);
    QCheckBox* LiqProbSaveCheckBox = new QCheckBox("liq_prob");
    outputSaveCheckBoxes.insert("liq_prob", LiqProbSaveCheckBox);

    outputSaveGroupBox = new QGroupBox(this);
    outputSaveGroupBox->setTitle(tr("Save Output"));
    outputSaveGroupBox->setContentsMargins(0,0,0,0);
    QGridLayout* outputSaveGroupBoxLayout = new QGridLayout(outputSaveGroupBox);
    outputSaveGroupBox->setLayout(outputSaveGroupBoxLayout);
    int checkBoxCount = 0;
    for (auto it = outputSaveCheckBoxes.constBegin(); it != outputSaveCheckBoxes.constEnd(); ++it) {
        outputSaveGroupBoxLayout->addWidget(it.value(), 0, checkBoxCount);
        checkBoxCount ++;
    }

    // Add to layout
    layout->addWidget(GwDepthfilenameLabel,0,0);
    layout->addWidget(GwDepthDefineMethodComboBox,0,1,1,1);
    layout->addWidget(GwDepthFilenameLineEdit,0,2,1,3);
    layout->addWidget(GwDepthBrowseFileButton,0,5);

    layout->addWidget(GeologicMapFilenameLabel,1,0);
    layout->addWidget(GeoMapDefineMethodComboBox,1,1,1,1);
    layout->addWidget(GeologicMapFilenameLineEdit,1,2,1,3);
    layout->addWidget(GeologicMapBrowseFileButton,1,5);

    layout->addWidget(LiqSuscKeyLabel,2,0);
    layout->addWidget(LiqSuscKeyLineEdit,2,1,1,1);

    layout->addWidget(crsSelectorWidget,3,0, 1, 6);
    layout->addWidget(outputSaveGroupBox, 4, 0, 1, 6);

    layout->addWidget(resetToDefaultButton,5,5);

    layout->setColumnStretch(2,3);
    layout->setColumnStretch(1,1);
    layout->setRowStretch(6,1);

    this->setLayout(layout);

    //We need to set initial values
    connect(this->GwDepthDefineMethodComboBox, &QComboBox::currentTextChanged,
            this, &LiqTriggerHazus2020::handleInputTypeChanged);
    connect(this->GeoMapDefineMethodComboBox, &QComboBox::currentTextChanged,
            this, &LiqTriggerHazus2020::handleInputTypeChanged);

    GwDepthDefineMethodComboBox->setCurrentIndex(1);
    GeoMapDefineMethodComboBox->setCurrentIndex(1);
    this->setDefaultFilePath();

    this->setupConnections();
}





void LiqTriggerHazus2020::setupConnections()
{
    //Connecting parameters
    //connect(this->FilenameLineEdit, &QLineEdit::textChanged, this->m_eqRupture, &OpenQuakeScenario::setRupFilename);

}

void LiqTriggerHazus2020::handleInputTypeChanged()
{
    if (GwDepthDefineMethodComboBox->currentIndex() == 0){
        // Define in Site File (.csv)
        GwDepthFilenameLineEdit->hide();
        GwDepthBrowseFileButton->hide();
    } else if (GwDepthDefineMethodComboBox->currentIndex() == 1){
        // Define with raster file
        GwDepthFilenameLineEdit->show();
        GwDepthBrowseFileButton->show();
    }
    if (GeoMapDefineMethodComboBox->currentIndex() == 0){
        // Define in Site File (.csv)
        GeologicMapFilenameLineEdit->hide();
        GeologicMapBrowseFileButton->hide();
        LiqSuscKeyLineEdit->hide();
        LiqSuscKeyLabel->hide();
    } else if (GwDepthDefineMethodComboBox->currentIndex() == 1){
        // Define with raster file
        GeologicMapFilenameLineEdit->show();
        GeologicMapBrowseFileButton->show();
        LiqSuscKeyLineEdit->show();
        LiqSuscKeyLabel->show();
    }
    if (GwDepthDefineMethodComboBox->currentIndex() == 0 && GeoMapDefineMethodComboBox->currentIndex() == 0){
        crsSelectorWidget->hide();
    } else {
        crsSelectorWidget->show();
    }

    // If no raster files used, hide CRS selection
}


void LiqTriggerHazus2020::loadFile(QString fieldKey)
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::AnyFile);
    QString newFilePath;
    if (dialog.exec()){
        newFilePath = dialog.selectedFiles()[0];
    }
    if (newFilePath.isEmpty())
    {
        return;
    }
    // Check if raster
    QString retError;
    QgsVectorLayer testLayer = QgsVectorLayer(newFilePath, "testLayer", "ogr");
    if(!testLayer.isValid()){
        QString errMsg = "The file selected for " + fieldKey + "is not a valid vector file.";
        qDebug() << errMsg;
        this->errorMessage(errMsg);
        return;
    }
    if (fieldKey.compare("Ground Water Table Depth")==0)
    {
        this->GwDepthFilePath = newFilePath;
        this->GwDepthFilenameLineEdit->setText(GwDepthFilePath);
    } else if (fieldKey.compare("Geologic Map")==0)
    {
        this->GeologicMapFilePath = newFilePath;
        this->GeologicMapFilenameLineEdit->setText(GeologicMapFilePath);
    }
}

bool LiqTriggerHazus2020::outputToJSON(QJsonObject &jsonObject){

    QJsonObject parameterObj;
    GeoMapDefineMethodComboBox->outputToJSON(parameterObj);
    if (GeoMapDefineMethodComboBox->currentIndex()==1){
        parameterObj["SusceptibilityFile"] = GeologicMapFilePath;
        parameterObj["SusceptibilityKey"] = LiqSuscKeyLineEdit->text();
    }
    GwDepthDefineMethodComboBox->outputToJSON(parameterObj);
    if (GwDepthDefineMethodComboBox->currentIndex()==1){
        parameterObj["GwDepth"] = GwDepthFilePath;
    }
    if (GeoMapDefineMethodComboBox->currentIndex()==1 || GwDepthDefineMethodComboBox->currentIndex()==1){
        QJsonObject crsObj;
        crsSelectorWidget->outputAppDataToJSON(crsObj);
        parameterObj["inputCRS"] = crsObj["CRS"].toString();
    }
    QJsonArray outputArray;
    for (auto it = outputSaveCheckBoxes.constBegin(); it != outputSaveCheckBoxes.constEnd(); ++it) {
        if (it.value()->isChecked()){
            outputArray.append(it.key());
        }
    }
    jsonObject["Output"] = outputArray;
    jsonObject["Model"] = "Hazus2020";
    jsonObject["Parameters"] = parameterObj;
    return true;
}


void LiqTriggerHazus2020::setDefaultFilePath(){

    QString backendDataBasePath = QCoreApplication::applicationDirPath() + QDir::separator() + "Databases"+ QDir::separator() + "groundFailure";
    GwDepthFilePath = backendDataBasePath + QDir::separator() +
                      "CA_WaterTableDepth_1km_WGS84_meter" + QDir::separator() +
                      "CA_WaterTableDepth_1km_WGS84_meter.tif";
    // QFile GwDepthFile(GwDepthFilePath);
    // if (!GwDepthFile.exists()){
    //     GwDepthFilePath = "";
    // }
    GeologicMapFilePath = backendDataBasePath + QDir::separator() +
                            "CA_GeologicMap_WillsEtal2015_WGS84" + QDir::separator() +
                            "CA_GeologicMap_WillsEtal2015_WGS84.shp";
    // QFile GeologicMapFile(GeologicMapFilePath);
    // if (!GeologicMapFile.exists()){
    //     GeologicMapFilePath = "";
    // }

    QgsCoordinateReferenceSystem defaultCRS("EPSG:4326");
    crsSelectorWidget->setCRS(defaultCRS);

    LiqSuscKey = "LIQ";

    this->GwDepthFilenameLineEdit->setText(GwDepthFilePath);
    this->LiqSuscKeyLineEdit->setText(LiqSuscKey);
    this->GeologicMapFilenameLineEdit->setText(GeologicMapFilePath);

    this->GwDepthDefineMethodComboBox->setCurrentIndex(1);
    this->GeoMapDefineMethodComboBox->setCurrentIndex(1);

    for (auto it = outputSaveCheckBoxes.constBegin(); it != outputSaveCheckBoxes.constEnd(); ++it) {
        it.value()->setChecked(true);
    }
}
