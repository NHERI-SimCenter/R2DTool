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

// Written by: Stevan Gavrilovic

#include "GmAppConfigWidget.h"
#include "GmappConfig.h"
#include "GMWidget.h"

#include <QLineEdit>
#include <QPushButton>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QGroupBox>

GmAppConfigWidget::GmAppConfigWidget(GmAppConfig* appConfig, GMWidget *parent) : QWidget(parent, Qt::Dialog), appConfig(appConfig), parentWidget(parent)
{
    this->setWindowModality(Qt::WindowModal);
    this->setWindowTitle("Path Settings");
    this->setWindowIcon(QIcon(":/images/settings.png"));
    this->setAutoFillBackground(true);
    QPalette palette = QPalette();
    palette.setColor(QPalette::Background, Qt::white);
    this->setPalette(palette);

    QVBoxLayout* layout = new QVBoxLayout(this);

    QGroupBox* appsBox = new QGroupBox("Applications", this);
    layout->addWidget(appsBox);
    appsBox->setStyleSheet("QGroupBox {font-weight: bold}");

    QFormLayout* appsFormLayout = new QFormLayout(appsBox);

    QHBoxLayout* workDirLayout = new QHBoxLayout();
    workDirectoryBox = new QLineEdit(this);
    workDirectoryBox->setMinimumWidth(600);
    workDirectoryBox->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);
    workDirLayout->addWidget(workDirectoryBox, 1);
    workDirectoryButton = new QPushButton(tr("Browse..."), this);
    workDirLayout->addWidget(workDirectoryButton, 0);
    appsFormLayout->addRow("Working Directory:", workDirLayout);

    QHBoxLayout* inputDirLayout = new QHBoxLayout();
    inputDirectoryBox = new QLineEdit(this);
    inputDirectoryBox->setMinimumWidth(600);
    inputDirectoryBox->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);
    inputDirLayout->addWidget(inputDirectoryBox, 1);
    inputDirectoryButton = new QPushButton(tr("Browse..."), this);
    inputDirLayout->addWidget(inputDirectoryButton, 0);
    appsFormLayout->addRow("Input Directory:", inputDirLayout);

    QHBoxLayout* outputDirLayout = new QHBoxLayout();
    outputDirectoryBox = new QLineEdit(this);
    outputDirectoryBox->setMinimumWidth(600);
    outputDirectoryBox->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);
    outputDirLayout->addWidget(outputDirectoryBox, 1);
    outputDirectoryButton = new QPushButton(tr("Browse..."), this);
    outputDirLayout->addWidget(outputDirectoryButton, 0);
    appsFormLayout->addRow("Output Directory:", outputDirLayout);

    //    QGroupBox* dbsBox = new QGroupBox("Records Database", this);
    //    layout->addWidget(dbsBox);
    //    dbsBox->setStyleSheet("QGroupBox {font-weight: bold}");

    //    QFormLayout* dbsFormLayout = new QFormLayout(dbsBox);

    //    QHBoxLayout* DBUserLayout = new QHBoxLayout();
    //    usernameBox = new QLineEdit(this);
    //    usernameBox->setMinimumWidth(600);
    //    usernameBox->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);
    //    DBUserLayout->addWidget(usernameBox, 1);
    //    dbsFormLayout->addRow("Username:", DBUserLayout);

    //    QHBoxLayout* DBPassLayout = new QHBoxLayout();
    //    passwordBox = new QLineEdit(this);
    //    passwordBox->setMinimumWidth(600);
    //    passwordBox->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);
    //    passwordBox->setEchoMode(QLineEdit::EchoMode::Password);
    //    DBPassLayout->addWidget(passwordBox, 1);
    //    dbsFormLayout->addRow("Password:", DBPassLayout);

    closeButton = new QPushButton(tr("Close"), this);
    layout->addWidget(closeButton);
    layout->setAlignment(closeButton,Qt::AlignHCenter);

    // Reading current path
    workDirectoryBox->setText(appConfig->getWorkDirectoryPath());
    inputDirectoryBox->setText(appConfig->getInputDirectoryPath());
    outputDirectoryBox->setText(appConfig->getOutputDirectoryPath());
    //    usernameBox->setText(appConfig->getUsername());
    //    passwordBox->setText(appConfig->getPassword());

    setupConnections();
}

void GmAppConfigWidget::setupConnections()
{
    connect(workDirectoryButton, &QPushButton::released, [this](){

        QFileDialog dialog(this);
        dialog.setOption(QFileDialog::Option::ShowDirsOnly, true);
        dialog.setFileMode(QFileDialog::Directory);
        if(dialog.exec())
            appConfig->setWorkDirectoryPath(dialog.selectedFiles()[0]);
    });

    connect(appConfig, &GmAppConfig::workingDirectoryPathChanged, workDirectoryBox, &QLineEdit::setText);

    connect(inputDirectoryButton, &QPushButton::released, [this](){

        QFileDialog dialog(this);
        dialog.setOption(QFileDialog::Option::ShowDirsOnly, true);
        dialog.setFileMode(QFileDialog::Directory);
        if(dialog.exec())
            appConfig->setInputFilePath(dialog.selectedFiles()[0]);
    });

    connect(appConfig, &GmAppConfig::inputDirectoryPathChanged, inputDirectoryBox, &QLineEdit::setText);

    connect(outputDirectoryButton, &QPushButton::released, [this](){

        QFileDialog dialog(this);
        dialog.setOption(QFileDialog::Option::ShowDirsOnly, true);
        dialog.setFileMode(QFileDialog::Directory);
        if(dialog.exec())
            appConfig->setOutputFilePath(dialog.selectedFiles()[0]);
    });

    connect(appConfig, &GmAppConfig::outputDirectoryPathChanged, outputDirectoryBox, &QLineEdit::setText);

//    connect(usernameBox, &QLineEdit::textChanged, appConfig, &GmAppConfig::setUsername);
//    connect(passwordBox, &QLineEdit::textChanged, appConfig, &GmAppConfig::setPassword);

//    connect(appConfig, &GmAppConfig::usernamePathChanged, usernameBox, &QLineEdit::setText);
//    connect(appConfig, &GmAppConfig::passwordChanged, passwordBox, &QLineEdit::setText);

    connect(closeButton,&QAbstractButton::clicked,this, &QWidget::close);
    connect(closeButton,&QAbstractButton::clicked,this, &GmAppConfigWidget::saveConfig);
}


void GmAppConfigWidget::saveConfig(void)
{
    parentWidget->saveAppSettings();
}
