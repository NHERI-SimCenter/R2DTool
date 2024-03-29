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

#include "LiqTriggerZhuEtAl2017.h"
#include "OpenQuakeScenario.h"
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
#include "qgscoordinatereferencesystem.h"
#include <QGroupBox>
#include <QGridLayout>
#include <QJsonArray>


LiqTriggerZhuEtAl2017::LiqTriggerZhuEtAl2017(QWidget *parent) : SimCenterAppWidget(parent)
{
    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    //We use a grid layout for the Rupture widget
    QGridLayout* layout = new QGridLayout(this);

    QLabel* DistWaterfilenameLabel = new QLabel(tr("Distance (km) to Water Body:"),this);
    DistWaterFilenameLineEdit = new QLineEdit(this);
    DistWaterBrowseFileButton = new QPushButton();
    DistWaterBrowseFileButton->setText(tr("Browse"));
    DistWaterBrowseFileButton->setMaximumWidth(150);
    DistWaterComboBox = new SC_ComboBox("DistWater", QStringList({"Defined (\"distWater\") in Site File (.csv)", "Define with raster (nearest pixel)"}));
//    QCheckBox* DistWaterSaveCheckBox = new QCheckBox("distWater");
//    outputSaveCheckBoxes.insert("distWater", DistWaterSaveCheckBox);

    QLabel* DistCoastfilenameLabel = new QLabel(tr("Distance (km) to Coast:"),this);
    DistCoastFilenameLineEdit = new QLineEdit(this);
    DistCoastBrowseFileButton = new QPushButton();
    DistCoastBrowseFileButton->setText(tr("Browse"));
    DistCoastBrowseFileButton->setMaximumWidth(150);
    DistCoastComboBox = new SC_ComboBox("DistCoast", QStringList({"Defined (\"distCoast\") in Site File (.csv)", "Define with raster (nearest pixel)"}));
//    QCheckBox* DistCoastSaveCheckBox = new QCheckBox("distCoast");
//    outputSaveCheckBoxes.insert("distCoast", DistCoastSaveCheckBox);

    QLabel* DistRiverfilenameLabel = new QLabel(tr("Distance (km) to River (raster):"),this);
    DistRiverFilenameLineEdit = new QLineEdit(this);
    DistRiverBrowseFileButton = new QPushButton();
    DistRiverBrowseFileButton->setText(tr("Browse"));
    DistRiverBrowseFileButton->setMaximumWidth(150);
    DistRiverComboBox = new SC_ComboBox("DistRiver", QStringList({"Defined (\"distRiver\") in Site File (.csv)", "Define with raster (nearest pixel)"}));
//    QCheckBox* DistRiverSaveCheckBox = new QCheckBox("distRiver");
//    outputSaveCheckBoxes.insert("distRiver", DistRiverSaveCheckBox);

    QLabel* GwDepthfilenameLabel = new QLabel(tr("Ground Water Table Depth (m):"),this);
    GwDepthFilenameLineEdit = new QLineEdit(this);
    GwDepthBrowseFileButton = new QPushButton();
    GwDepthBrowseFileButton->setText(tr("Browse"));
    GwDepthBrowseFileButton->setMaximumWidth(150);
    GwDepthComboBox = new SC_ComboBox("GwDepth", QStringList({"Defined (\"gwDepth\") in Site File (.csv)", "Define with raster (nearest pixel)"}));
//    QCheckBox* GwDepthSaveCheckBox = new QCheckBox("gwDepth");
//    outputSaveCheckBoxes.insert("gwDepth", GwDepthSaveCheckBox);


    QLabel* PrecipitationfilenameLabel = new QLabel(tr("Mean Annual Precipitation (mm):"),this);
    PrecipitationFilenameLineEdit = new QLineEdit(this);
    PrecipitationBrowseFileButton = new QPushButton();
    PrecipitationBrowseFileButton->setText(tr("Browse"));
    PrecipitationBrowseFileButton->setMaximumWidth(150);
    PrecipitationComboBox = new SC_ComboBox("Precipitation", QStringList({"Defined (\"precipitation\") in Site File (.csv)", "Define with raster (nearest pixel)"}));
//    QCheckBox* PrecipitationSaveCheckBox = new QCheckBox("Precipitation");
//    outputSaveCheckBoxes.insert("Precipitation", PrecipitationSaveCheckBox);

    resetToDefaultButton = new QPushButton();
    resetToDefaultButton->setText(tr("Reset to Default"));
    resetToDefaultButton->setMaximumWidth(150);

    // https://stackoverflow.com/questions/5153157/passing-an-argument-to-a-slot
    QSignalMapper* signalMapper = new QSignalMapper (this) ;
    connect(DistWaterBrowseFileButton,SIGNAL(clicked()),signalMapper, SLOT(map()));
    connect(DistCoastBrowseFileButton,SIGNAL(clicked()),signalMapper, SLOT(map()));
    connect(DistRiverBrowseFileButton,SIGNAL(clicked()),signalMapper, SLOT(map()));
    connect(GwDepthBrowseFileButton,SIGNAL(clicked()),signalMapper, SLOT(map()));
    connect(PrecipitationBrowseFileButton,SIGNAL(clicked()),signalMapper, SLOT(map()));

    signalMapper->setMapping(DistWaterBrowseFileButton, QString("Distance to Water Body"));
    signalMapper->setMapping(DistCoastBrowseFileButton, QString("Distance to Coast"));
    signalMapper->setMapping(DistRiverBrowseFileButton, QString("Distance to River"));
    signalMapper->setMapping(GwDepthBrowseFileButton, QString("Ground Water Table Depth"));
    signalMapper->setMapping(PrecipitationBrowseFileButton, QString("Mean Annual Precipitation"));

    connect(signalMapper, SIGNAL(mappedString(QString)), this, SLOT(loadFile(QString)));

    connect(resetToDefaultButton, &QPushButton::clicked, this, &LiqTriggerZhuEtAl2017::setDefaultFilePath);

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
//    outputSaveGroupBox->setFlat(true);

    // Add to layout
    layout->addWidget(DistWaterfilenameLabel,0,0);
    layout->addWidget(DistWaterComboBox,0,1,1,1);
    layout->addWidget(DistWaterFilenameLineEdit,0,2,1,3);
    layout->addWidget(DistWaterBrowseFileButton,0,5);

    layout->addWidget(DistCoastfilenameLabel,1,0);
    layout->addWidget(DistCoastComboBox,1,1,1,1);
    layout->addWidget(DistCoastFilenameLineEdit,1,2,1,3);
    layout->addWidget(DistCoastBrowseFileButton,1,5);

    layout->addWidget(DistRiverfilenameLabel,2,0);
    layout->addWidget(DistRiverComboBox,2,1,1,1);
    layout->addWidget(DistRiverFilenameLineEdit,2,2,1,3);
    layout->addWidget(DistRiverBrowseFileButton,2,5);

    layout->addWidget(GwDepthfilenameLabel,3,0);
    layout->addWidget(GwDepthComboBox,3,1,1,1);
    layout->addWidget(GwDepthFilenameLineEdit,3,2,1,3);
    layout->addWidget(GwDepthBrowseFileButton,3,5);

    layout->addWidget(PrecipitationfilenameLabel,4,0);
    layout->addWidget(PrecipitationComboBox,4,1,1,1);
    layout->addWidget(PrecipitationFilenameLineEdit,4,2,1,3);
    layout->addWidget(PrecipitationBrowseFileButton,4,5);

    layout->addWidget(crsSelectorWidget,5,0, 1, 6);
    layout->addWidget(outputSaveGroupBox, 6, 0, 1, 6);
    layout->addWidget(resetToDefaultButton,7,5);

    layout->setColumnStretch(2,3);
    layout->setColumnStretch(1,1);
    layout->setRowStretch(8,1);

    this->setLayout(layout);

    //We need to set initial values
    connect(this->DistWaterComboBox, &QComboBox::currentTextChanged,
            this, &LiqTriggerZhuEtAl2017::handleInputTypeChanged);
    connect(this->DistCoastComboBox, &QComboBox::currentTextChanged,
            this, &LiqTriggerZhuEtAl2017::handleInputTypeChanged);
    connect(this->DistRiverComboBox, &QComboBox::currentTextChanged,
            this, &LiqTriggerZhuEtAl2017::handleInputTypeChanged);
    connect(this->GwDepthComboBox, &QComboBox::currentTextChanged,
            this, &LiqTriggerZhuEtAl2017::handleInputTypeChanged);
    connect(this->PrecipitationComboBox, &QComboBox::currentTextChanged,
            this, &LiqTriggerZhuEtAl2017::handleInputTypeChanged);

    DistWaterComboBox->setCurrentIndex(1);
    DistCoastComboBox->setCurrentIndex(1);
    DistRiverComboBox->setCurrentIndex(1);
    GwDepthComboBox->setCurrentIndex(1);
    PrecipitationComboBox->setCurrentIndex(1);
    this->setDefaultFilePath();

    this->setupConnections();
}





void LiqTriggerZhuEtAl2017::setupConnections()
{
    //Connecting parameters
    //connect(this->FilenameLineEdit, &QLineEdit::textChanged, this->m_eqRupture, &OpenQuakeScenario::setRupFilename);

}


void LiqTriggerZhuEtAl2017::loadFile(QString fieldKey)
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::AnyFile);
    QString newFilePath;
    if (dialog.exec()){
        newFilePath = dialog.selectedFiles()[0];
    }
    if(newFilePath.isEmpty())
    {
        return;
    }
    // Check if raster
    QString retError;
    if(!QgsRasterLayer::isValidRasterFileName(newFilePath, retError)){
        QString errMsg = "The file selected for " + fieldKey + "is not a valid raster file.";
        qDebug() << errMsg;
        this->errorMessage(errMsg);
        return;
    }
    if (fieldKey.compare("Ground Water Table Depth")==0)
    {
        this->GwDepthFilePath = newFilePath;
        this->GwDepthFilenameLineEdit->setText(GwDepthFilePath);
    } else if (fieldKey.compare("Distance to Coast")==0)
    {
        this->DistCoastFilePath = newFilePath;
        this->DistCoastFilenameLineEdit->setText(DistCoastFilePath);
    } else if (fieldKey.compare("Distance to River")==0)
    {
        this->DistRiverFilePath = newFilePath;
        this->DistRiverFilenameLineEdit->setText(DistRiverFilePath);
    } else if (fieldKey.compare("Distance to Water Body")==0)
    {
        this->DistWaterFilePath = newFilePath;
        this->DistWaterFilenameLineEdit->setText(DistWaterFilePath);
    } else if (fieldKey.compare("Mean Annual Precipitation")==0)
    {
        this->PrecipitationFilePath = newFilePath;
        this->PrecipitationFilenameLineEdit->setText(PrecipitationFilePath);
    }

}

bool LiqTriggerZhuEtAl2017::outputToJSON(QJsonObject &jsonObject){

    QJsonObject parameterObj;
    bool CRSneeded = false;
    DistWaterComboBox->outputToJSON(parameterObj);
    if (DistWaterComboBox->currentIndex()==1){
        parameterObj["DistWater"] = DistWaterFilePath;
        CRSneeded = true;
    }
    DistCoastComboBox->outputToJSON(parameterObj);
    if (DistCoastComboBox->currentIndex()==1){
        parameterObj["DistCoast"] = DistCoastFilePath;
        CRSneeded = true;
    }
    DistRiverComboBox->outputToJSON(parameterObj);
    if (DistRiverComboBox->currentIndex()==1){
        parameterObj["DistRiver"] = DistRiverFilePath;
        CRSneeded = true;
    }
    GwDepthComboBox->outputToJSON(parameterObj);
    if (GwDepthComboBox->currentIndex()==1){
        parameterObj["GwDepth"] = GwDepthFilePath;
        CRSneeded = true;
    }
    PrecipitationComboBox->outputToJSON(parameterObj);
    if (PrecipitationComboBox->currentIndex()==1){
        parameterObj["Precipitation"] = PrecipitationFilePath;
        CRSneeded = true;
    }
    if (CRSneeded){
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
    jsonObject["Model"] = "ZhuEtal2017";
    jsonObject["Parameters"] = parameterObj;
    return true;
}


void LiqTriggerZhuEtAl2017::setDefaultFilePath(){

    QString backendDataBasePath = QCoreApplication::applicationDirPath() + QDir::separator() + "Databases"+ QDir::separator() + "groundFailure";

    DistWaterFilePath = backendDataBasePath + QDir::separator() +
                        "CA_DistAnyWaterWB_1km_WGS84_km" + QDir::separator() +
                        "CA_DistAnyWaterWB_1km_WGS84_km.tif";
    DistCoastFilePath = backendDataBasePath + QDir::separator() +
                        "CA_DistCoast_1km_WGS84_km" + QDir::separator() +
                        "CA_DistCoast_1km_WGS84_km.tif";
    DistRiverFilePath = backendDataBasePath + QDir::separator() +
                        "CA_DistRiver_1km_WGS84_km" + QDir::separator() +
                        "CA_DistRiver_1km_WGS84_km.tif";
    GwDepthFilePath = backendDataBasePath + QDir::separator() +
                      "CA_WaterTableDepth_1km_WGS84_meter" + QDir::separator() +
                      "CA_WaterTableDepth_1km_WGS84_meter.tif";
    PrecipitationFilePath = backendDataBasePath + QDir::separator() +
                            "CA_Precip_1981-2010_1km_WGS84_mm" + QDir::separator() +
                            "CA_Precip_1981-2010_1km_WGS84_mm.tif";

    QgsCoordinateReferenceSystem defaultCRS("EPSG:4326");
    crsSelectorWidget->setCRS(defaultCRS);

    this->DistWaterFilenameLineEdit->setText(DistWaterFilePath);
    this->DistCoastFilenameLineEdit->setText(DistCoastFilePath);
    this->DistRiverFilenameLineEdit->setText(DistRiverFilePath);
    this->GwDepthFilenameLineEdit->setText(GwDepthFilePath);
    this->PrecipitationFilenameLineEdit->setText(PrecipitationFilePath);

    this->DistWaterComboBox->setCurrentIndex(1);
    this->DistCoastComboBox->setCurrentIndex(1);
    this->DistRiverComboBox->setCurrentIndex(1);
    this->GwDepthComboBox->setCurrentIndex(1);
    this->PrecipitationComboBox->setCurrentIndex(1);

    for (auto it = outputSaveCheckBoxes.constBegin(); it != outputSaveCheckBoxes.constEnd(); ++it) {
        it.value()->setChecked(true);
    }
}


void LiqTriggerZhuEtAl2017::handleInputTypeChanged()
{
    bool CRSneeded = false;
    if (DistWaterComboBox->currentIndex() == 0){
        // Define in Site File (.csv)
        DistWaterFilenameLineEdit->hide();
        DistWaterBrowseFileButton->hide();
    } else if (DistWaterComboBox->currentIndex() == 1){
        // Define with raster file
        DistWaterFilenameLineEdit->show();
        DistWaterBrowseFileButton->show();
        CRSneeded = true;
    }
    if (DistCoastComboBox->currentIndex() == 0){
        // Define in Site File (.csv)
        DistCoastFilenameLineEdit->hide();
        DistCoastBrowseFileButton->hide();
    } else if (DistCoastComboBox->currentIndex() == 1){
        // Define with raster file
        DistCoastFilenameLineEdit->show();
        DistCoastBrowseFileButton->show();
        CRSneeded = true;
    }
    if (DistRiverComboBox->currentIndex() == 0){
        // Define in Site File (.csv)
        DistRiverFilenameLineEdit->hide();
        DistRiverBrowseFileButton->hide();
    } else if (DistRiverComboBox->currentIndex() == 1){
        // Define with raster file
        DistRiverFilenameLineEdit->show();
        DistRiverBrowseFileButton->show();
        CRSneeded = true;
    }
    if (GwDepthComboBox->currentIndex() == 0){
        // Define in Site File (.csv)
        GwDepthFilenameLineEdit->hide();
        GwDepthBrowseFileButton->hide();
    } else if (GwDepthComboBox->currentIndex() == 1){
        // Define with raster file
        GwDepthFilenameLineEdit->show();
        GwDepthBrowseFileButton->show();
        CRSneeded = true;
    }
    if (PrecipitationComboBox->currentIndex() == 0){
        // Define in Site File (.csv)
        PrecipitationFilenameLineEdit->hide();
        PrecipitationBrowseFileButton->hide();
    } else if (PrecipitationComboBox->currentIndex() == 1){
        // Define with raster file
        PrecipitationFilenameLineEdit->show();
        PrecipitationBrowseFileButton->show();
        CRSneeded = true;
    }
    if (!CRSneeded){
        crsSelectorWidget->hide();
    } else {
        crsSelectorWidget->show();
    }
}
