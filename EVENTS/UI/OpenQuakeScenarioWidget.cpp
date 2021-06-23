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

#include "OpenQuakeScenarioWidget.h"
#include "OpenQuakeScenario.h"

#include <QLabel>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QSpacerItem>
#include <QPushButton>
#include <HBoxFormLayout.h>
#include <QFileDialog>
#include <QMessageBox>

OpenQuakeScenarioWidget::OpenQuakeScenarioWidget(QWidget *parent) : SimCenterWidget(parent)
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
    QLabel* areaMeshLabel = new QLabel(tr("Area Meshsize (km<sup>2</sup>):"),this);
    QLabel* distMaxLabel = new QLabel(tr("Maximum Distance (km):"),this);

    QLabel* filenameLabel = new QLabel(tr("Rupture File (.xml):"),this);
    FilenameLineEdit = new QLineEdit(this);
    browseFileButton = new QPushButton();
    browseFileButton->setText(tr("Browse"));
    browseFileButton->setMaximumWidth(150);
    connect(browseFileButton,SIGNAL(clicked()),this,SLOT(loadRupFile()));

    // if line-type ruptures are defined -> mesh size in length
    layout->addWidget(rupMeshLabel,0,0);
    layout->addWidget(m_rupMeshBox,0,1);

    // if area-type or complex rutpures are defined are defined -> mesh size in area
    layout->addWidget(areaMeshLabel,0,2);
    layout->addWidget(m_areaMeshBox,0,3);

    // maximum considered distance between the rupture and sites
    layout->addWidget(distMaxLabel,0,4);
    layout->addWidget(m_maxDistanceBox,0,5);

    // rupture definition (.xml)
    layout->addWidget(filenameLabel,1,0);
    layout->addWidget(FilenameLineEdit,1,1,1,4);
    layout->addWidget(browseFileButton,1,5);

    this->setLayout(layout);

    //We need to set initial values
    m_eqRupture = new OpenQuakeScenario(1.0, 10.0, 500.0, "", parent);

    this->m_rupMeshBox->setValue(m_eqRupture->getRupMesh());
    this->m_areaMeshBox->setValue(m_eqRupture->getAreaMesh());
    this->m_maxDistanceBox->setValue(m_eqRupture->getMaxDistance());
    //this->FilenameLineEdit->setText(m_eqRupture->getRupFilename());

    this->setupConnections();
}


OpenQuakeScenario* OpenQuakeScenarioWidget::getRuptureSource() const
{
    return m_eqRupture;
}


void OpenQuakeScenarioWidget::setupConnections()
{
    //Connecting parameters
    connect(this->m_rupMeshBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this->m_eqRupture, &OpenQuakeScenario::setRupMesh);

    connect(this->m_areaMeshBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this->m_eqRupture, &OpenQuakeScenario::setAreaMesh);

    connect(this->m_maxDistanceBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this->m_eqRupture, &OpenQuakeScenario::setMaxDistance);

    //connect(this->FilenameLineEdit, &QLineEdit::textChanged, this->m_eqRupture, &OpenQuakeScenario::setRupFilename);

}


void OpenQuakeScenarioWidget::setRupFile(QString dirPath)
{
    this->FilenameLineEdit->setText(dirPath);
    return;
}


void OpenQuakeScenarioWidget::loadRupFile()
{
    this->rupFilePath=QFileDialog::getOpenFileName(this,tr("Flatfile"));
    if(this->rupFilePath.isEmpty())
    {
        this->rupFilePath = "NULL";
        return;
    }
    this->setRupFile(this->rupFilePath);

    //Connecting the filename
    if (this->rupFilePath.compare("Null") == 0 && this->rupFilePath.contains(".xml", Qt::CaseInsensitive))
    {
        QString errMsg = "Please choose a Rupture File (.xml)";
        qDebug() << errMsg;
        this->errorMessage(errMsg);
        return;
    }
    else
    {
        this->m_eqRupture->setRupFilename(this->rupFilePath);
    }

    //this->transferFile();
}


