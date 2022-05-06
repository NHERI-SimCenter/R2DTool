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

// Written by: Kuanshi Zhong

#include "OpenQuakeUserSpecifiedWidget.h"
#include "OpenQuakeClassical.h"

#include <QLabel>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QSpacerItem>
#include <QPushButton>
#include <HBoxFormLayout.h>
#include <QFileDialog>
#include <QMessageBox>
#include <QCheckBox>
#include <QSettings>

OpenQuakeUserSpecifiedWidget::OpenQuakeUserSpecifiedWidget(QWidget *parent) : SimCenterWidget(parent)
{
    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    //We use a grid layout for the Rupture widget
    QGridLayout* layout = new QGridLayout(this);

    QLabel* sDirLabel = new QLabel(tr("Input folder:"),this);
    sourceDirLineEdit = new QLineEdit(this);
    browseSourceDirButton = new QPushButton();
    browseSourceDirButton->setText(tr("Browse"));
    browseSourceDirButton->setMaximumWidth(150);
    connect(browseSourceDirButton,SIGNAL(clicked()),this,SLOT(loadSourceDir()));

    QLabel* cFileLabel = new QLabel(tr("Configuration File (.ini):"),this);
    iniFilePathLineEdit = new QLineEdit(this);
    browseIniFileButton = new QPushButton();
    browseIniFileButton->setText(tr("Browse"));
    browseIniFileButton->setMaximumWidth(150);
    connect(browseIniFileButton,SIGNAL(clicked()),this,SLOT(loadConfigFile()));

    // OpenQuake Version
    QLabel* oqVerLabel = new QLabel(tr("OpenQuake Version:"),this);
    oqVersionCombo = new QComboBox(this);
    oqVersionCombo->addItem("3.12");
    oqVersionCombo->addItem("3.11");
    oqVersionCombo->addItem("3.10");
    connect(oqVersionCombo,&QComboBox::currentTextChanged,this,&OpenQuakeUserSpecifiedWidget::handleOQVersionChanged);

    // version
    layout->addWidget(oqVerLabel,0,0);
    layout->addWidget(oqVersionCombo,0,1,1,1);

    layout->addWidget(cFileLabel,1,0);
    layout->addWidget(iniFilePathLineEdit,1,1,1,4);
    layout->addWidget(browseIniFileButton,1,5);

    layout->addWidget(sDirLabel,2,0);
    layout->addWidget(sourceDirLineEdit,2,1,1,4);
    layout->addWidget(browseSourceDirButton,2,5);

    this->setLayout(layout);

    //We need to set initial values
    m_eqRupture = new OpenQuakeClassical(1, 1.0, 10.0, 500.0, "", this);
}

void OpenQuakeUserSpecifiedWidget::handleOQVersionChanged(const QString& selection)
{
    if(selection.compare("3.10") == 0)
    {
        oqVersion = "3.10.1";
    }
    else if(selection.compare("3.11") == 0)
    {
        oqVersion = "3.11.4";
    }
    else if(selection.compare("3.12") == 0)
    {
        oqVersion = "3.12.0"; // this is the latest version 09/27/21
    }
    m_eqRupture->setOQVersion(oqVersion);
}

OpenQuakeClassical* OpenQuakeUserSpecifiedWidget::getRuptureSource() const
{
    return m_eqRupture;
}

void OpenQuakeUserSpecifiedWidget::setSourceDir(QString dirPath)
{
    this->sourceDirLineEdit->setText(dirPath);
    return;
}

void OpenQuakeUserSpecifiedWidget::setIniFile(QString dirPath)
{
    this->iniFilePathLineEdit->setText(dirPath);
    return;
}

void OpenQuakeUserSpecifiedWidget::loadSourceDir()
{
    this->sourceDir=QFileDialog::getExistingDirectory(this,tr("Source Model Directory"));
    if(this->sourceDir.isEmpty())
    {
        this->sourceDir = "NULL";
        return;
    }
    this->setSourceDir(this->sourceDir);

    //Connecting the filename
    if (this->sourceDir.compare("Null") == 0)
    {
        QString errMsg = "Please choose a valid Source Model Directory";
        qDebug() << errMsg;
        this->errorMessage(errMsg);
        return;
    }
    else
    {
        this->m_eqRupture->setSourceModelDir(this->sourceDir);
    }
    this->parseConfigFile();
}

void OpenQuakeUserSpecifiedWidget::loadConfigFile()
{
    this->iniFilePath=QFileDialog::getOpenFileName(this,tr("Configuration File"));
    if(this->iniFilePath.isEmpty())
    {
        this->iniFilePath = "NULL";
        return;
    }
    this->setIniFile(this->iniFilePath);

    //Connecting the filename
    if (this->iniFilePath.compare("Null") == 0 && this->iniFilePath.contains(".ini", Qt::CaseInsensitive))
    {
        QString errMsg = "Please choose a configuration file in .ini";
        qDebug() << errMsg;
        this->errorMessage(errMsg);
        return;
    }
    else
    {
        this->m_eqRupture->setConfigFilename(this->iniFilePath);
        this->parseConfigFile();
    }
}


void OpenQuakeUserSpecifiedWidget::parseConfigFile()
{
    //qDebug() << this->iniFilePath;
    QString sourcePath;
    if (this->sourceDirLineEdit->text().compare("Null") == 0)
        sourcePath = QFileInfo(this->iniFilePath).absoluteDir().absolutePath();
    else
        sourcePath = this->sourceDirLineEdit->text();
    iniConfigSetting = new QSettings(this->iniFilePath, QSettings::IniFormat);
    // random seed
    int random_seed = iniConfigSetting->value("random_seed", -1).toInt();
    if (random_seed > 0) {
        this->m_eqRupture->setRandSeed(QString::number(random_seed));
    }

    // uhs tag
    bool uhs = iniConfigSetting->value("output/uniform_hazard_spectra", false).toBool();
    this->m_eqRupture->setUHS(uhs);

    // individual_curves tag
    bool individual_curves = iniConfigSetting->value("output/individual_curves", false).toBool();
    this->m_eqRupture->setIndivHC(individual_curves);

    // hc_mean tag
    bool hc_mean = iniConfigSetting->value("output/mean", false).toBool();
    this->m_eqRupture->setMeanHC(hc_mean);

    // hazard_maps tag
    bool hazard_maps = iniConfigSetting->value("output/hazard_maps", false).toBool();
    this->m_eqRupture->setHazMap(hazard_maps);

    // quantiles tag
    QString quantiles = iniConfigSetting->value("output/quantiles", "0.1 0.5 0.9").toString().replace(" ",",");
    this->m_eqRupture->setQuantiles(quantiles);

    // source_model_logic_tree_file
    QString smlt_file = iniConfigSetting->value("calculation/source_model_logic_tree_file").toString();
    this->m_eqRupture->setSourceFilename(sourcePath+'/'+smlt_file);

    // gmpe_logic_tree_file
    QString gsim_file = iniConfigSetting->value("calculation/gsim_logic_tree_file").toString();
    this->m_eqRupture->setGMPEFilename(sourcePath+'/'+gsim_file);

    QStringList keys = iniConfigSetting->allKeys();
    //qDebug() << keys;
}
