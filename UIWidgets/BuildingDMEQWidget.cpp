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

#include "BuildingDMEQWidget.h"
#include "PelicunDLWidget.h"
#include "sectiontitle.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QComboBox>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QDebug>

BuildingDMEQWidget::BuildingDMEQWidget(RandomVariablesContainer *theRandomVariableIW, QWidget *parent)
    : SimCenterAppWidget(parent), theCurrentWidget(0), theRandomVariablesContainer(theRandomVariableIW)
{
    QVBoxLayout *layout = new QVBoxLayout();

    // The selection part
    QHBoxLayout *theSelectionLayout = new QHBoxLayout();
    SectionTitle *label = new SectionTitle();
    label->setMinimumWidth(250);
    label->setText(QString("Engineering Demand Paramater Selection"));
    edpSelection = new QComboBox();
    edpSelection->addItem(tr("Pelicun"));
    edpSelection->setObjectName("DMSelectionComboBox");

    theSelectionLayout->addWidget(label);
    QSpacerItem *spacer = new QSpacerItem(50,10);
    theSelectionLayout->addItem(spacer);
    theSelectionLayout->addWidget(edpSelection,1);
    theSelectionLayout->addStretch(1);
    layout->addLayout(theSelectionLayout);

    // Create the stacked widget
    theStackedWidget = new QStackedWidget();

    // Create the individual widgets add to stacked widget
    thePelicunDLWidget = new PelicunDLWidget(this);
    theStackedWidget->addWidget(thePelicunDLWidget);

    layout->addWidget(theStackedWidget);
    this->setLayout(layout);
    layout->setMargin(0);

    connect(edpSelection, SIGNAL(currentIndexChanged(QString)), this,
            SLOT(edpSelectionChanged(QString)));

    this->edpSelectionChanged("Pelicun");

}


BuildingDMEQWidget::~BuildingDMEQWidget()
{

}


bool BuildingDMEQWidget::outputToJSON(QJsonObject &jsonObject)
{
    theCurrentWidget->outputToJSON(jsonObject);
    return true;
}


bool BuildingDMEQWidget::inputFromJSON(QJsonObject &jsonObject) {

    if (theCurrentWidget != 0)
    {
        return theCurrentWidget->inputFromJSON(jsonObject);
    }
    else
    {
        this->errorMessage("EDP_SElection no current EDP selected");
    }

    return false;
}


void BuildingDMEQWidget::edpSelectionChanged(const QString &arg1)
{
    //
    // switch stacked widgets depending on text
    // note type output in json and name in pull down are not the same and hence the ||
    //

    if (arg1 == "Pelicun")
    {
        theStackedWidget->setCurrentIndex(0);
        theCurrentWidget = thePelicunDLWidget;
    }
    else
    {
        qDebug() << "ERROR ..  BuildingDMEQWidget selection .. type unknown: " << arg1;
        theCurrentWidget = nullptr;
    }
}


bool BuildingDMEQWidget::outputAppDataToJSON(QJsonObject &jsonObject)
{
    theCurrentWidget->outputAppDataToJSON(jsonObject);
    return true;
}


bool BuildingDMEQWidget::inputAppDataFromJSON(QJsonObject &jsonObject)
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
        this->errorMessage(" BuildingDMEQWidget - no Application key found");
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
        this->errorMessage("BuildingDMEQWidget - no valid type found");
        return false;
    }

    edpSelection->setCurrentIndex(index);

    // invoke inputAppDataFromJSON on new type
    if (theCurrentWidget != 0)
    {
        return theCurrentWidget->inputAppDataFromJSON(jsonObject);
    }

    return true;
}


bool BuildingDMEQWidget::copyFiles(QString &destDir) {

    if (theCurrentWidget != nullptr)
    {
        return  theCurrentWidget->copyFiles(destDir);
    }

    return false;
}


void BuildingDMEQWidget::clear(void)
{
    thePelicunDLWidget->clear();
}
