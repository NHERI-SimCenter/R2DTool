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

#include "LiqLateralHazus2020.h"
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
#include <qgsrasterlayer.h>
#include <qgscoordinatereferencesystem.h>
#include <QJsonArray>
#include <QMap>
#include <QGroupBox>

LiqLateralHazus2020::LiqLateralHazus2020(QWidget *parent) : SimCenterAppWidget(parent)
{
    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    //We use a grid layout for the Rupture widget
    QGridLayout* layout = new QGridLayout(this);

    DistWaterfilenameLabel = new QLabel(tr("Distance (km) to Water Body (raster):"),this);
    DistWaterFilenameLineEdit = new QLineEdit(this);
    DistWaterBrowseFileButton = new QPushButton();
    DistWaterBrowseFileButton->setText(tr("Browse"));
    DistWaterBrowseFileButton->setMaximumWidth(150);
    DistWaterComboBox = new SC_ComboBox("DistWater", QStringList({"Defined (\"distWater\") in Site File (.csv)", "Define with raster (nearest pixel)", "None"}));


    resetToDefaultButton = new QPushButton();
    resetToDefaultButton->setText(tr("Reset to Default"));
    resetToDefaultButton->setMaximumWidth(150);

    // https://stackoverflow.com/questions/5153157/passing-an-argument-to-a-slot
    QSignalMapper* signalMapper = new QSignalMapper (this) ;
    connect(DistWaterBrowseFileButton,SIGNAL(clicked()),signalMapper, SLOT(map()));
    signalMapper->setMapping(DistWaterBrowseFileButton, QString("Distance to Water Body"));

    connect(signalMapper, SIGNAL(mappedString(QString)), this, SLOT(loadFile(QString)));

    connect(resetToDefaultButton, &QPushButton::clicked, this, &LiqLateralHazus2020::setDefaultFilePath);

    crsSelectorWidget = new CRSSelectionWidget();

    QCheckBox* SaveCheckBox = new QCheckBox("liq_PGD_h");
    outputSaveCheckBoxes.insert("liq_PGD_h", SaveCheckBox);

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
    layout->addWidget(DistWaterfilenameLabel,0,0);
    layout->addWidget(DistWaterComboBox,0,1,1,1);
    layout->addWidget(DistWaterFilenameLineEdit,0,2,1,3);
    layout->addWidget(DistWaterBrowseFileButton,0,5);

    layout->addWidget(crsSelectorWidget,1,0, 1, 6);
    layout->addWidget(outputSaveGroupBox, 2, 0, 1, 6);
    layout->addWidget(resetToDefaultButton,3,5);

    layout->setColumnStretch(2,3);
    layout->setColumnStretch(1,1);
    layout->setRowStretch(4,1);

    this->setLayout(layout);

    //We need to set initial values
    connect(this->DistWaterComboBox, &QComboBox::currentTextChanged,
            this, &LiqLateralHazus2020::handleInputTypeChanged);
    DistWaterComboBox->setCurrentIndex(1);
    this->setDefaultFilePath();

    this->setupConnections();
}





void LiqLateralHazus2020::setupConnections()
{
    //Connecting parameters
    //connect(this->FilenameLineEdit, &QLineEdit::textChanged, this->m_eqRupture, &OpenQuakeScenario::setRupFilename);

}


void LiqLateralHazus2020::loadFile(QString fieldKey)
{
    if (fieldKey.compare("Distance to Water Body")==0)
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
        this->DistWaterFilePath = newFilePath;
        this->DistWaterFilenameLineEdit->setText(DistWaterFilePath);
    }
}

bool LiqLateralHazus2020::outputToJSON(QJsonObject &jsonObject){


    QJsonObject parameterObj;
    bool CRSneeded = false;
    DistWaterComboBox->outputToJSON(parameterObj);
    if (DistWaterComboBox->currentIndex()==1){
        parameterObj["DistWater"] = DistWaterFilePath;
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
    jsonObject["Model"] = "Hazus2020Lateral";
    jsonObject["Parameters"] = parameterObj;
    return true;
}


void LiqLateralHazus2020::setDefaultFilePath(){

    QString backendDataBasePath = QCoreApplication::applicationDirPath() + QDir::separator() + "Databases"+ QDir::separator() + "groundFailure";

    DistWaterFilePath = backendDataBasePath + QDir::separator() +
                        "CA_DistAnyWaterNoWB_1km_WGS84_km" + QDir::separator() +
                        "CA_DistAnyWaterNoWB_1km_WGS84_km.tif";

    this->DistWaterFilenameLineEdit->setText(DistWaterFilePath);

    QgsCoordinateReferenceSystem defaultCRS("EPSG:4326");
    crsSelectorWidget->setCRS(defaultCRS);
    this->DistWaterComboBox->setCurrentIndex(1);


    for (auto it = outputSaveCheckBoxes.constBegin(); it != outputSaveCheckBoxes.constEnd(); ++it) {
        it.value()->setChecked(true);
    }
}

void LiqLateralHazus2020::handleInputTypeChanged(){
    if (DistWaterComboBox->currentIndex() == 0){
        // Define in Site File (.csv)
        DistWaterFilenameLineEdit->hide();
        DistWaterBrowseFileButton->hide();
        crsSelectorWidget->hide();
    } else if (DistWaterComboBox->currentIndex() == 1){
        // Define with raster file
        DistWaterFilenameLineEdit->show();
        DistWaterBrowseFileButton->show();
        crsSelectorWidget->show();
    } else if (DistWaterComboBox->currentIndex() == 0){
        // Define in Site File (.csv)
        DistWaterFilenameLineEdit->hide();
        DistWaterBrowseFileButton->hide();
        crsSelectorWidget->hide();
    }
}
