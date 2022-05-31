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

// Written by: Kuanshi Zhong, Frank McKenna

#include "EventGMDirWidget.h"

#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QGroupBox>
#include <QCheckBox>
#include <QPushButton>
#include <QDir>
#include <QFileDialog>
#include <QDebug>

EventGMDirWidget::EventGMDirWidget(QWidget *parent): QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    QGroupBox* evtGMDirGroupBox = new QGroupBox(this);
    evtGMDirGroupBox->setTitle("Output Directory");
    evtGMDirGroupBox->setContentsMargins(0,0,0,0);

    this->setMinimumWidth(375);

    QGridLayout* gridLayout = new QGridLayout(evtGMDirGroupBox);
    evtGMDirGroupBox->setLayout(gridLayout);

    QLabel* gmDirLabel = new QLabel(tr("Event and/or GM"),this);
    motionDirLineEdit = new QLineEdit();
    motionDirLineEdit->setText(motionDir);

    useEvtFolderButton = new QPushButton(tr("Browser"),this);
    connect(useEvtFolderButton, &QPushButton::clicked, this, &EventGMDirWidget::browseEventFolderDir);

    gridLayout->addWidget(gmDirLabel,1,0);
    gridLayout->addWidget(motionDirLineEdit,1,1);
    gridLayout->addWidget(useEvtFolderButton,1,2);

    layout->addWidget(evtGMDirGroupBox);
    this->setLayout(layout);
}

void EventGMDirWidget::setEventFile(const QString &value)
{
    eventFile = value;
    this->sendEventFileMotionDir();
}

void EventGMDirWidget::setMotionDir(const QString &value)
{
    motionDir = value;
    motionDirLineEdit->setText(motionDir);
    this->sendEventFileMotionDir();
}

void EventGMDirWidget::sendEventFileMotionDir(void)
{
    emit useEventFileMotionDir(this->eventFile, this->motionDir);
}

void EventGMDirWidget::browseEventFolderDir(void)
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::Directory);
    QString tmp = dialog.getExistingDirectory(this,tr("Event Folder"),QString(QDir(motionDir).absolutePath()));
    dialog.close();
    if(!tmp.isEmpty())
    {
        this->setMotionDir(tmp);
        this->setEventFile(tmp + QDir::separator() + "EventGrid.csv");
    }
}

