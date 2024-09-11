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
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
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

#include "BuildingEDPEQWidget.h"
#include "StandardEarthquakeEDP.h"
#include "UserDefinedEDPR.h"
#include "SectionTitle.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QComboBox>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QDebug>

BuildingEDPEQWidget::BuildingEDPEQWidget(QWidget *parent)
    : SimCenterAppWidget(parent), theCurrentWidget(0)
{
    QVBoxLayout *layout = new QVBoxLayout();

    // the selection part
    QHBoxLayout *theSelectionLayout = new QHBoxLayout();
    SectionTitle *label = new SectionTitle();
    label->setMinimumWidth(250);
    label->setText(QString("Engineering Demand Paramater Selection"));
    edpSelection = new QComboBox();
    edpSelection->addItem(tr("Standard Earthquake"));
    edpSelection->addItem(tr("User Defined"));
    edpSelection->setObjectName("EDPSelectionComboBox");

    theSelectionLayout->addWidget(label);
    QSpacerItem *spacer = new QSpacerItem(50,10);
    theSelectionLayout->addItem(spacer);
    theSelectionLayout->addWidget(edpSelection,1);
    theSelectionLayout->addStretch(1);
    layout->addLayout(theSelectionLayout);

    // Create the stacked widget
    theStackedWidget = new QStackedWidget();

    // Create the individual widgets add to stacked widget
    theStandardEarthquakeEDPs = new StandardEarthquakeEDP();
    theStackedWidget->addWidget(theStandardEarthquakeEDPs);

    theUserDefinedEDPs = new UserDefinedEDPR();
    theStackedWidget->addWidget(theUserDefinedEDPs);

    layout->addWidget(theStackedWidget);
    this->setLayout(layout);
    theCurrentWidget=theStandardEarthquakeEDPs;
    layout->setMargin(0);

    connect(edpSelection, SIGNAL(currentIndexChanged(QString)), this,
            SLOT(edpSelectionChanged(QString)));

    edpSelection->setCurrentText("User Defined");
}


BuildingEDPEQWidget::~BuildingEDPEQWidget()
{

}


bool BuildingEDPEQWidget::outputToJSON(QJsonObject &jsonObject)
{
    theCurrentWidget->outputToJSON(jsonObject);
    return true;
}


bool BuildingEDPEQWidget::inputFromJSON(QJsonObject &jsonObject)
{

    if (theCurrentWidget != nullptr)
    {
        return theCurrentWidget->inputFromJSON(jsonObject);
    }
    else
    {
        this->errorMessage("EDP_SElection no current EDP selected");
    }

    return false;
}


void BuildingEDPEQWidget::edpSelectionChanged(const QString &arg1)
{

    // switch stacked widgets depending on text
    // note type output in json and name in pull down are not the same and hence the ||

    if (arg1 == "Standard Earthquake")
    {
        theStackedWidget->setCurrentIndex(0);
        theCurrentWidget = theStandardEarthquakeEDPs;
    }
    else if(arg1 == "User Defined")
    {
        theStackedWidget->setCurrentIndex(1);
        theCurrentWidget = theUserDefinedEDPs;
    }

    else
    {
        qDebug() << "ERROR ..  BuildingEDPEQWidget selection .. type unknown: " << arg1;
    }
}


bool BuildingEDPEQWidget::outputAppDataToJSON(QJsonObject &jsonObject)
{
    theCurrentWidget->outputAppDataToJSON(jsonObject);
    return true;
}


bool BuildingEDPEQWidget::inputAppDataFromJSON(QJsonObject &jsonObject)
{

    // get name from "Application" key

    QString type;
    if (jsonObject.contains("Application"))
    {
        QJsonValue theName = jsonObject["Application"];
        type = theName.toString();
    }
    else
    {
        this->errorMessage(" BuildingEDPEQWidget - no Application key found");
        return false;
    }

    // based on application name value set edp type
    int index = 0;
    if ((type == QString("Standard Earthquake EDPs")) ||
            (type == QString("StandardEarthquakeEDP")))
    {
        index = 0;
    } else if ((type == QString("UserDefinedEDP")) ||
               (type == QString("User Defined EDPs")))
    {
        index = 1;
    }
    else
    {
        this->errorMessage(" BuildingEDPEQWidget - no valid type found");
        return false;
    }

    edpSelection->setCurrentIndex(index);

    // invoke inputAppDataFromJSON on new type

    if (theCurrentWidget != nullptr)
    {
        return theCurrentWidget->inputAppDataFromJSON(jsonObject);
    }

    return true;
}


bool BuildingEDPEQWidget::copyFiles(QString &destDir) {

    if (theCurrentWidget != nullptr)
    {
        return  theCurrentWidget->copyFiles(destDir);
    }

    return false;
}


void BuildingEDPEQWidget::clear(void)
{
    theStandardEarthquakeEDPs->clear();
    theUserDefinedEDPs->clear();
}
