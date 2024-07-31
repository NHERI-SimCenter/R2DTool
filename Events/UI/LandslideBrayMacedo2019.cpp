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

#include "LandslideBrayMacedo2019.h"
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


LandslideBrayMacedo2019::LandslideBrayMacedo2019(QWidget *parent) : SimCenterAppWidget(parent)
{
    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    //We use a grid layout for the landslide widget
    QGridLayout* layout = new QGridLayout(this);

    //Slope input
    QLabel* slopeInputLabel = new QLabel(tr("Slope (degree):"),this);
    slopeFilenameLineEdit = new QLineEdit(this);
    slopeBrowseFileButton = new QPushButton();
    slopeBrowseFileButton->setText(tr("Browse"));
    slopeBrowseFileButton->setMaximumWidth(150);
    slopeComboBox = new SC_ComboBox("Slope", QStringList({"Defined (\"slope\") in Site File (.csv)", "Define with raster (nearest pixel)"}));

    //Slope thickness
    QLabel* slopeThickfilenameLabel = new QLabel(tr("Slope Thickness (m):"),this);
    slopeThickFilenameLineEdit = new QLineEdit(this);
    slopeThickConstLineEdit = new QLineEdit(this);
    slopeThickBrowseFileButton = new QPushButton();
    slopeThickBrowseFileButton->setText(tr("Browse"));
    slopeThickBrowseFileButton->setMaximumWidth(150);
    slopeThickComboBox = new SC_ComboBox("SlopeThickness", QStringList({"Use constant value (m)", "Defined (\"slopeThickness\") in Site File (.csv)", "Define with raster (nearest pixel)"}));

    //Gamma soil
    QLabel* gammaSoilfilenameLabel = new QLabel(tr("Soil Unit Weight (kN/m^3):"),this);
    gammaSoilFilenameLineEdit = new QLineEdit(this);
    gammaSoilConstLineEdit = new QLineEdit(this);
    gammaSoilBrowseFileButton = new QPushButton();
    gammaSoilBrowseFileButton->setText(tr("Browse"));
    gammaSoilBrowseFileButton->setMaximumWidth(150);
    gammaSoilComboBox = new SC_ComboBox("GammaSoil", QStringList({"Use constant value (kN/m^3)", "Defined (\"gammaSoil\") in Site File (.csv)", "Define with raster (nearest pixel)"}));

    //Cohesion soil
    QLabel* cohesionfilenameLabel = new QLabel(tr("Soil Cohesion (kPa):"),this);
    cohesionSoilConstLineEdit = new QLineEdit(this);
    cohesionComboBox = new SC_ComboBox("CohesionSoil", QStringList({"Use constant value (kPa)", "Defined (\"cohesionSoil\") in Site File (.csv)", "Infer from Geologic Map (Bain et al. 2022)"}));

    //Friction soil
    QLabel* frictionfilenameLabel = new QLabel(tr("Soil Friction Angle (degree):"),this);
    frictionSoilConstLineEdit = new QLineEdit(this);
    frictionComboBox = new SC_ComboBox("PhiSoil", QStringList({"Use constant value (degree)", "Defined (\"phiSoil\") in Site File (.csv)", "Infer from Geologic Map (Bain et al. 2022)"}));

    //Geologic Map
    geologicMapLabel = new QLabel(tr("Geologic Map File:"),this);
    geologicMapFileLineEdit = new QLineEdit(this);
    geologicMapBrowseFileButton = new QPushButton();
    geologicMapBrowseFileButton->setText(tr("Browse"));
    geologicMapBrowseFileButton->setMaximumWidth(150);

    //reset
    resetToDefaultButton = new QPushButton();
    resetToDefaultButton->setText(tr("Reset to Default"));
    resetToDefaultButton->setMaximumWidth(150);

    // https://stackoverflow.com/questions/5153157/passing-an-argument-to-a-slot
    QSignalMapper* signalMapper = new QSignalMapper (this) ;
    connect(slopeBrowseFileButton,SIGNAL(clicked()),signalMapper, SLOT(map()));
    connect(slopeThickBrowseFileButton,SIGNAL(clicked()),signalMapper, SLOT(map()));
    connect(gammaSoilBrowseFileButton,SIGNAL(clicked()),signalMapper, SLOT(map()));
    connect(geologicMapBrowseFileButton,SIGNAL(clicked()),signalMapper, SLOT(map()));

    signalMapper->setMapping(slopeBrowseFileButton, QString("Slope"));
    signalMapper->setMapping(slopeThickBrowseFileButton, QString("Slope Thickness"));
    signalMapper->setMapping(gammaSoilBrowseFileButton, QString("Gamma Soil"));
    signalMapper->setMapping(geologicMapBrowseFileButton, QString("Geologic Map"));


    connect(signalMapper, SIGNAL(mappedString(QString)), this, SLOT(loadFile(QString)));

    connect(resetToDefaultButton, &QPushButton::clicked, this, &LandslideBrayMacedo2019::setDefaultFilePath);

    crsSelectorWidget = new CRSSelectionWidget();

    QCheckBox* lsdPgdSaveCheckBox = new QCheckBox("lsd_PGD_h");
    outputSaveCheckBoxes.insert("lsd_PGD_h", lsdPgdSaveCheckBox);

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
    layout->addWidget(slopeInputLabel,0,0);
    layout->addWidget(slopeComboBox,0,1,1,1);
    layout->addWidget(slopeFilenameLineEdit,0,2,1,3);
    layout->addWidget(slopeBrowseFileButton,0,5);

    layout->addWidget(slopeThickfilenameLabel,1,0);
    layout->addWidget(slopeThickComboBox,1,1,1,1);
    layout->addWidget(slopeThickFilenameLineEdit,1,2,1,3);
    layout->addWidget(slopeThickConstLineEdit,1,2,1,1);
    layout->addWidget(slopeThickBrowseFileButton,1,5);

    layout->addWidget(gammaSoilfilenameLabel,2,0);
    layout->addWidget(gammaSoilComboBox,2,1,1,1);
    layout->addWidget(gammaSoilFilenameLineEdit,2,2,1,3);
    layout->addWidget(gammaSoilConstLineEdit,2,2,1,1);
    layout->addWidget(gammaSoilBrowseFileButton,2,5);

    layout->addWidget(cohesionfilenameLabel,3,0);
    layout->addWidget(cohesionComboBox,3,1,1,1);
    layout->addWidget(cohesionSoilConstLineEdit,3,2,1,1);

    layout->addWidget(frictionfilenameLabel,4,0);
    layout->addWidget(frictionComboBox,4,1,1,1);
    layout->addWidget(frictionSoilConstLineEdit,4,2,1,1);

    layout->addWidget(geologicMapLabel, 5, 0);
    layout->addWidget(geologicMapFileLineEdit,5,1,1,4);
    layout->addWidget(geologicMapBrowseFileButton,5,5);

    layout->addWidget(crsSelectorWidget,6,0, 1, 6);
    layout->addWidget(outputSaveGroupBox, 7, 0, 1, 6);
    layout->addWidget(resetToDefaultButton,8,5);

    layout->setColumnStretch(2,3);
    layout->setColumnStretch(1,1);
    layout->setRowStretch(9,1);

    this->setLayout(layout);

    //We need to set initial values
    connect(this->slopeComboBox, &QComboBox::currentTextChanged,
            this, &LandslideBrayMacedo2019::handleInputTypeChanged);
    connect(this->slopeThickComboBox, &QComboBox::currentTextChanged,
            this, &LandslideBrayMacedo2019::handleInputTypeChanged);
    connect(this->gammaSoilComboBox, &QComboBox::currentTextChanged,
            this, &LandslideBrayMacedo2019::handleInputTypeChanged);
    connect(this->cohesionComboBox, &QComboBox::currentTextChanged,
            this, &LandslideBrayMacedo2019::handleInputTypeChanged);
    connect(this->frictionComboBox, &QComboBox::currentTextChanged,
            this, &LandslideBrayMacedo2019::handleInputTypeChanged);

    slopeComboBox->setCurrentIndex(1);
    slopeThickComboBox->setCurrentIndex(0);
    gammaSoilComboBox->setCurrentIndex(0);
    cohesionComboBox->setCurrentIndex(2);
    frictionComboBox->setCurrentIndex(2);
    this->setDefaultFilePath();

    this->setupConnections();
}





void LandslideBrayMacedo2019::setupConnections()
{
    //Connecting parameters
    //connect(this->FilenameLineEdit, &QLineEdit::textChanged, this->m_eqRupture, &OpenQuakeScenario::setRupFilename);

}


void LandslideBrayMacedo2019::loadFile(QString fieldKey)
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
    if (fieldKey.compare("Slope Thickness")==0)
    {
        this->slopeThickFilePath = newFilePath;
        this->slopeThickFilenameLineEdit->setText(slopeThickFilePath);
    } else if (fieldKey.compare("Gamma Soil")==0)
    {
        this->gammaSoilFilePath = newFilePath;
        this->gammaSoilFilenameLineEdit->setText(gammaSoilFilePath);
    } else if (fieldKey.compare("Slope")==0)
    {
        this->slopeFilePath = newFilePath;
        this->slopeFilenameLineEdit->setText(slopeFilePath);
    } else if (fieldKey.compare("Geologic Map")==0)
    {
        this->geologicMapFilePath = newFilePath;
        this->geologicMapFileLineEdit->setText(geologicMapFilePath);
    }

}

bool LandslideBrayMacedo2019::outputToJSON(QJsonObject &jsonObject){

    QJsonObject parameterObj;
    bool CRSneeded = false;
    bool geologicMapNeeded = false;
    slopeComboBox->outputToJSON(parameterObj);
    if (slopeComboBox->currentIndex()==1){
        parameterObj["Slope"] = slopeFilePath;
        CRSneeded = true;
    }
    slopeThickComboBox->outputToJSON(parameterObj);
    if (slopeThickComboBox->currentIndex()==2){
        parameterObj["SlopeThickness"] = slopeThickFilePath;
        CRSneeded = true;
    }
    if (slopeThickComboBox->currentIndex()==0){
        bool ok;
        parameterObj["SlopeThicknessValue"] = slopeThickConstLineEdit->text().toDouble(&ok);
        if (!ok) {
            errorMessage("Slope thickness value input" + slopeThickConstLineEdit->text() + " is invalid, please input a plain number");
        }
    }
    gammaSoilComboBox->outputToJSON(parameterObj);
    if (gammaSoilComboBox->currentIndex()==2){
        parameterObj["GammaSoil"] = gammaSoilFilePath;
        CRSneeded = true;
    }
    if (gammaSoilComboBox->currentIndex()==0){
        bool ok;
        parameterObj["GammaSoilValue"] = gammaSoilConstLineEdit->text().toDouble(&ok);
        if (!ok) {
            errorMessage("Soil Unit Weight value input" + gammaSoilConstLineEdit->text() + " is invalid, please input a plain number");
        }
    }
    cohesionComboBox->outputToJSON(parameterObj);
    if (cohesionComboBox->currentIndex()==0){
        bool ok;
        parameterObj["CohesionSoilValue"] = cohesionSoilConstLineEdit->text().toDouble(&ok);
        if (!ok) {
            errorMessage("Soil cohesion value input" + cohesionSoilConstLineEdit->text() + " is invalid, please input a plain number");
        }
    }
    if (cohesionComboBox->currentIndex()==2){
        CRSneeded = true;
        geologicMapNeeded = true;
    }
    frictionComboBox->outputToJSON(parameterObj);
    if (frictionComboBox->currentIndex()==0){
        bool ok;
        parameterObj["PhiSoilValue"] = frictionSoilConstLineEdit->text().toDouble(&ok);
        if (!ok) {
            errorMessage("Soil friction value input" + frictionSoilConstLineEdit->text() + " is invalid, please input a plain number");
        }
    }
    if (frictionComboBox->currentIndex()==2){
        CRSneeded = true;
        geologicMapNeeded = true;
    }
    if (CRSneeded){
        QJsonObject crsObj;
        crsSelectorWidget->outputAppDataToJSON(crsObj);
        parameterObj["inputCRS"] = crsObj["CRS"].toString();
    }
    if (geologicMapNeeded){
        parameterObj["GeologicMap"] = geologicMapFilePath;
    }
    QJsonArray outputArray;
    for (auto it = outputSaveCheckBoxes.constBegin(); it != outputSaveCheckBoxes.constEnd(); ++it) {
        if (it.value()->isChecked()){
            outputArray.append(it.key());
        }
    }
    jsonObject["Output"] = outputArray;
    jsonObject["Model"] = "BrayMacedo2019";
    jsonObject["Parameters"] = parameterObj;
    return true;
}


void LandslideBrayMacedo2019::setDefaultFilePath(){

    QString backendDataBasePath = QCoreApplication::applicationDirPath() + QDir::separator() + "Databases"+ QDir::separator() + "groundFailure";

    slopeFilePath = backendDataBasePath + QDir::separator() +
                        "CA_Slope_30m_WGS84_degree" + QDir::separator() +
                        "CA_Slope_30m_WGS84_degree.tif";

    slopeThickFilePath = "";

    gammaSoilFilePath = "";

    geologicMapFilePath = backendDataBasePath + QDir::separator() +
                            "CA_GeologicMap_WillsEtal2015_WGS84" + QDir::separator() +
                            "CA_GeologicMap_WillsEtal2015_WGS84.shp";

    QgsCoordinateReferenceSystem defaultCRS("EPSG:4326");
    crsSelectorWidget->setCRS(defaultCRS);

    this->slopeFilenameLineEdit->setText(slopeFilePath);
    this->slopeThickFilenameLineEdit->setText(slopeThickFilePath);
    this->slopeThickConstLineEdit->setText(QString::number(defaultSlopeThickness));
    this->gammaSoilFilenameLineEdit->setText(gammaSoilFilePath);
    this->gammaSoilConstLineEdit->setText(QString::number(defaultGammaSoil));
    this->geologicMapFileLineEdit->setText(geologicMapFilePath);

    this->slopeComboBox->setCurrentIndex(1);
    this->slopeThickComboBox->setCurrentIndex(0);
    this->gammaSoilComboBox->setCurrentIndex(0);
    this->cohesionComboBox->setCurrentIndex(2);
    this->frictionComboBox->setCurrentIndex(2);

    for (auto it = outputSaveCheckBoxes.constBegin(); it != outputSaveCheckBoxes.constEnd(); ++it) {
        it.value()->setChecked(true);
    }
}


void LandslideBrayMacedo2019::handleInputTypeChanged()
{
    bool CRSneeded = false;
    bool geologicNeeded = false;
    // Slope
    if (slopeComboBox->currentIndex() == 0){
        // Define in Site File (.csv)
        slopeFilenameLineEdit->hide();
        slopeBrowseFileButton->hide();
    } else if (slopeComboBox->currentIndex() == 1){
        // Define with raster file
        slopeFilenameLineEdit->show();
        slopeBrowseFileButton->show();
        CRSneeded = true;
    }
    // Slope thickness
    if (slopeThickComboBox->currentIndex() == 0){
        // Use Constant value
        slopeThickFilenameLineEdit->hide();
        slopeThickBrowseFileButton->hide();
        slopeThickConstLineEdit->show();
    } else if (slopeThickComboBox->currentIndex() == 1){
        // Define in Site File (.csv)
        slopeThickFilenameLineEdit->hide();
        slopeThickBrowseFileButton->hide();
        slopeThickConstLineEdit->hide();
    } else if (slopeThickComboBox->currentIndex() == 2){
        // Define with raster file
        slopeThickFilenameLineEdit->show();
        slopeThickBrowseFileButton->show();
        slopeThickConstLineEdit->hide();
        CRSneeded = true;
    }
    // Gamma soil
    if (gammaSoilComboBox->currentIndex() == 0){
        // Use Constant value
        gammaSoilFilenameLineEdit->hide();
        gammaSoilBrowseFileButton->hide();
        gammaSoilConstLineEdit->show();
    } else if (gammaSoilComboBox->currentIndex() == 1){
        // Define in Site File (.csv)
        gammaSoilFilenameLineEdit->hide();
        gammaSoilBrowseFileButton->hide();
        gammaSoilConstLineEdit->hide();
    } else if (gammaSoilComboBox->currentIndex() == 2){
        // Define with raster file
        gammaSoilFilenameLineEdit->show();
        gammaSoilBrowseFileButton->show();
        gammaSoilConstLineEdit->hide();
        CRSneeded = true;
    }

    // Cohesion soil
    if (cohesionComboBox->currentIndex() == 0){
        // Define in as constant
        cohesionSoilConstLineEdit->show();
    } else if (cohesionComboBox->currentIndex() == 1){
        // Define with site file
        cohesionSoilConstLineEdit->hide();
    } else if (cohesionComboBox->currentIndex() == 2){
        // Define with geologic map
        cohesionSoilConstLineEdit->hide();
        CRSneeded = true;
        geologicNeeded = true;
    }

    // Friction soil
    if (frictionComboBox->currentIndex() == 0){
        // Define as constant
        frictionSoilConstLineEdit->show();
    } else if (frictionComboBox->currentIndex() == 1){
        // Define with site file
        frictionSoilConstLineEdit->hide();
    } else if (frictionComboBox->currentIndex() == 2){
        // Define with geologic map
        frictionSoilConstLineEdit->hide();
        CRSneeded = true;
        geologicNeeded = true;
    }


    if (!CRSneeded){
        crsSelectorWidget->hide();
    } else {
        crsSelectorWidget->show();
    }

    if (geologicNeeded){
        geologicMapLabel->show();
        geologicMapFileLineEdit->show();
        geologicMapBrowseFileButton->show();
    } else {
        geologicMapLabel->hide();
        geologicMapFileLineEdit->hide();
        geologicMapBrowseFileButton->hide();
    }
}
