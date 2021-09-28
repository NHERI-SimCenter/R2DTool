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

#include "OpenQuakeClassicalWidget.h"
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

OpenQuakeClassicalWidget::OpenQuakeClassicalWidget(QWidget *parent) : SimCenterWidget(parent)
{
    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    //We use a grid layout for the Rupture widget
    QGridLayout* layout = new QGridLayout(this);

    this->m_rupMeshBox = new QDoubleSpinBox(this);
    this->m_rupMeshBox->setRange(0.0001, 100.0);
    this->m_rupMeshBox->setDecimals(2);
    this->m_rupMeshBox->setSingleStep(0.1);
    //FMK this->m_magnitudeMinBox->setMinimumWidth(250);

    this->m_areaMeshBox = new QDoubleSpinBox(this);
    this->m_areaMeshBox->setRange(0.0001, 100.0);
    this->m_areaMeshBox->setDecimals(2);
    this->m_areaMeshBox->setSingleStep(0.1);
    //FMK this->m_magnitudeMaxBox->setMinimumWidth(250);

    this->m_maxDistanceBox = new QDoubleSpinBox(this);
    this->m_maxDistanceBox->setRange(0, 1000.0);
    this->m_maxDistanceBox->setDecimals(1);
    this->m_maxDistanceBox->setSingleStep(1.0);
    //FMK this->m_maxDistanceBox->setMinimumWidth(250);

    QLabel* rupMeshLabel = new QLabel(tr("Rupture Meshsize (km):"),this);
    QLabel* areaMeshLabel = new QLabel(tr("Area Meshsize (km):"),this);
    QLabel* distMaxLabel = new QLabel(tr("Maximum Distance (km):"),this);

    QLabel* sfilenameLabel = new QLabel(tr("Source Logic Tree (.xml):"),this);
    sourceFilenameLineEdit = new QLineEdit(this);
    browseSourceFileButton = new QPushButton();
    browseSourceFileButton->setText(tr("Browse"));
    browseSourceFileButton->setMaximumWidth(150);
    connect(browseSourceFileButton,SIGNAL(clicked()),this,SLOT(loadSourceLogicTree()));

    QLabel* gfilenameLabel = new QLabel(tr("GMPE Logic Tree (.xml):"),this);
    gmpeFilenameLineEdit = new QLineEdit(this);
    browseGMPEFileButton = new QPushButton();
    browseGMPEFileButton->setText(tr("Browse"));
    browseGMPEFileButton->setMaximumWidth(150);
    connect(browseGMPEFileButton,SIGNAL(clicked()),this,SLOT(loadGMPELogicTree()));


    QLabel* sDirLabel = new QLabel(tr("Source model folder:"),this);
    sourceDirLineEdit = new QLineEdit(this);
    browseSourceDirButton = new QPushButton();
    browseSourceDirButton->setText(tr("Browse"));
    browseSourceDirButton->setMaximumWidth(150);
    connect(browseSourceDirButton,SIGNAL(clicked()),this,SLOT(loadSourceDir()));


    // Check boxes for different PSHA output options
    indivHCCheckBox = new QCheckBox("Individual hazard curve",this);
    meanHCCheckBox = new QCheckBox("Mean hazard curve",this);
    hazMapCheckBox = new QCheckBox("Hazard map",this);
    UHSCheckBox = new QCheckBox("Uniform hazard spectra",this);

    indivHCCheckBox->setChecked(true);
    meanHCCheckBox->setChecked(true);
    hazMapCheckBox->setChecked(true);
    UHSCheckBox->setChecked(true);

    // OpenQuake Version
    QLabel* oqVerLabel = new QLabel(tr("OpenQuake Version:"),this);
    oqVersionCombo = new QComboBox(this);
    oqVersionCombo->addItem("3.12");
    oqVersionCombo->addItem("3.11");
    oqVersionCombo->addItem("3.10");
    connect(oqVersionCombo,&QComboBox::currentTextChanged,this,&OpenQuakeClassicalWidget::handleOQVersionChanged);

    // version
    layout->addWidget(oqVerLabel,0,0);
    layout->addWidget(oqVersionCombo,0,1,1,1);

    // source logic tree definition (.xml)
    layout->addWidget(sfilenameLabel,1,0);
    layout->addWidget(sourceFilenameLineEdit,1,1,1,4);
    layout->addWidget(browseSourceFileButton,1,5);

    // source model directory
    layout->addWidget(sDirLabel,2,0);
    layout->addWidget(sourceDirLineEdit,2,1,1,4);
    layout->addWidget(browseSourceDirButton,2,5);

    // if line-type ruptures are defined -> mesh size in length
    layout->addWidget(rupMeshLabel,3,0);
    layout->addWidget(m_rupMeshBox,3,1);

    // if area-type or complex rutpures are defined are defined -> mesh size in area
    layout->addWidget(areaMeshLabel,3,2);
    layout->addWidget(m_areaMeshBox,3,3);

    // maximum considered distance between the rupture and sites
    layout->addWidget(distMaxLabel,3,4);
    layout->addWidget(m_maxDistanceBox,3,5);

    // GMPE logic tree definition (.xml)
    layout->addWidget(gfilenameLabel,4,0);
    layout->addWidget(gmpeFilenameLineEdit,4,1,1,4);
    layout->addWidget(browseGMPEFileButton,4,5);

    // return period
    QLabel* rpLabel= new QLabel("Return period (yr):",this);
    rpLineEdit = new QLineEdit(this);
    rpLineEdit->setText("500");
    auto validator = new QIntValidator(1, 9900, this);
    rpLineEdit->setValidator(validator);
    layout->addWidget(rpLabel, 5,0);
    layout->addWidget(rpLineEdit,5,1);

    // random seed
    QLabel* rsLabel= new QLabel("Seed:",this);
    randSeedLineEdit = new QLineEdit(this);
    randSeedLineEdit->setText("14");
    auto validator2 = new QIntValidator(1, 1000, this);
    randSeedLineEdit->setValidator(validator2);
    layout->addWidget(rsLabel, 5,2);
    layout->addWidget(randSeedLineEdit,5,3);

    // quantile
    QLabel* quantLabel= new QLabel("Quantiles:",this);
    quantLineEdit = new QLineEdit(this);
    quantLineEdit->setText("0.1,0.5,0.9");
    layout->addWidget(quantLabel, 5,4);
    layout->addWidget(quantLineEdit,5,5);

    // investigation time period
    QLabel* ivgtTimeLabel= new QLabel("Investigation Time (yr):",this);
    ivgtTimeLineEdit = new QLineEdit(this);
    ivgtTimeLineEdit->setText("1");
    auto validator_ivgt = new QIntValidator(1, 9900, this);
    ivgtTimeLineEdit->setValidator(validator_ivgt);
    layout->addWidget(ivgtTimeLabel, 6,0);
    layout->addWidget(ivgtTimeLineEdit,6,1);

    // check boxes
    layout->addWidget(indivHCCheckBox,7,0);
    layout->addWidget(meanHCCheckBox,7,1);
    layout->addWidget(hazMapCheckBox,7,2);
    layout->addWidget(UHSCheckBox,7,3);

    this->setLayout(layout);

    //We need to set initial values
    m_eqRupture = new OpenQuakeClassical(1.0, 10.0, 500.0, "", this);

    this->m_rupMeshBox->setValue(m_eqRupture->getRupMesh());
    this->m_areaMeshBox->setValue(m_eqRupture->getAreaMesh());
    this->m_maxDistanceBox->setValue(m_eqRupture->getMaxDistance());
    //this->FilenameLineEdit->setText(m_eqRupture->getRupFilename());

    this->setupConnections();
}

void OpenQuakeClassicalWidget::handleOQVersionChanged(const QString& selection)
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

OpenQuakeClassical* OpenQuakeClassicalWidget::getRuptureSource() const
{
    return m_eqRupture;
}


void OpenQuakeClassicalWidget::setupConnections()
{
    //Connecting parameters
    connect(this->m_rupMeshBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this->m_eqRupture, &OpenQuakeClassical::setRupMesh);

    connect(this->m_areaMeshBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this->m_eqRupture, &OpenQuakeClassical::setAreaMesh);

    connect(this->m_maxDistanceBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this->m_eqRupture, &OpenQuakeClassical::setMaxDistance);

    connect(this->indivHCCheckBox, SIGNAL(stateChanged(int)),
            this->m_eqRupture, SLOT(setIndivHC(int)));

    connect(this->meanHCCheckBox, SIGNAL(stateChanged(int)),
            this->m_eqRupture, SLOT(setMeanHC(int)));

    connect(this->hazMapCheckBox, SIGNAL(stateChanged(int)),
            this->m_eqRupture, SLOT(setHazMap(int)));

    connect(this->UHSCheckBox, SIGNAL(stateChanged(int)),
            this->m_eqRupture, SLOT(setUHS(int)));

    connect(this->rpLineEdit, SIGNAL(textChanged(QString)),
            this->m_eqRupture, SLOT(setReturnPeriod(QString)));

    connect(this->randSeedLineEdit, SIGNAL(textChanged(QString)),
            this->m_eqRupture, SLOT(setRandSeed(QString)));

    connect(this->quantLineEdit, SIGNAL(textChanged(QString)),
            this->m_eqRupture, SLOT(setQuantiles(QString)));

    connect(this->ivgtTimeLineEdit, SIGNAL(textChanged(QString)),
            this->m_eqRupture, SLOT(setTimeSpan(QString)));

}


void OpenQuakeClassicalWidget::setSourceFile(QString dirPath)
{
    this->sourceFilenameLineEdit->setText(dirPath);
    return;
}


void OpenQuakeClassicalWidget::setGMPEFile(QString dirPath)
{
    this->gmpeFilenameLineEdit->setText(dirPath);
    return;
}

void OpenQuakeClassicalWidget::setSourceDir(QString dirPath)
{
    this->sourceDirLineEdit->setText(dirPath);
    return;
}


void OpenQuakeClassicalWidget::loadSourceLogicTree()
{
    this->sourceFilePath=QFileDialog::getOpenFileName(this,tr("Source Logic Tree"));
    if(this->sourceFilePath.isEmpty())
    {
        this->sourceFilePath = "NULL";
        return;
    }
    this->setSourceFile(this->sourceFilePath);

    //Connecting the filename
    if (this->sourceFilePath.compare("Null") == 0 && this->sourceFilePath.contains(".xml", Qt::CaseInsensitive))
    {
        QString errMsg = "Please choose a Source Logic Tree (.xml)";
        qDebug() << errMsg;
        this->errorMessage(errMsg);
        return;
    }
    else
    {
        this->m_eqRupture->setSourceFilename(this->sourceFilePath);
    }

    //this->transferFile();
}

void OpenQuakeClassicalWidget::loadGMPELogicTree()
{
    this->gmpeFilePath=QFileDialog::getOpenFileName(this,tr("GMPE Logic Tree"));
    if(this->gmpeFilePath.isEmpty())
    {
        this->gmpeFilePath = "NULL";
        return;
    }
    this->setGMPEFile(this->gmpeFilePath);

    //Connecting the filename
    if (this->gmpeFilePath.compare("Null") == 0 && this->gmpeFilePath.contains(".xml", Qt::CaseInsensitive))
    {
        QString errMsg = "Please choose a GMPE Logic Tree (.xml)";
        qDebug() << errMsg;
        this->errorMessage(errMsg);
        return;
    }
    else
    {
        this->m_eqRupture->setGMPEFilename(this->gmpeFilePath);
    }

}


void OpenQuakeClassicalWidget::loadSourceDir()
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

    //this->transferFile();
}
