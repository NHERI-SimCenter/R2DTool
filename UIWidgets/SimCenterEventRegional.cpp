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

// Written by: Frank McKenna

#include "SimCenterEventRegional.h"
#include "RandomVariablesContainer.h"

#include <QVBoxLayout>

SimCenterEventRegional::SimCenterEventRegional(QWidget *parent)
    :SimCenterAppWidget(parent)\
{
    unitsCombo = new QComboBox(this);
    unitsCombo->addItem("Gravitational constant (g)","g");
    unitsCombo->addItem("Meter per second squared","mps2");
    unitsCombo->addItem("Feet per second squared","ftps2");
    unitsCombo->addItem("Inches per second squared","inchps2");
    unitsCombo->addItem("Feet","ft");
    unitsCombo->addItem("Meter","m");

    unitsCombo->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Maximum);

    eventTypeCombo = new QComboBox(this);
    eventTypeCombo->addItem("Earthquake","Earthquake");
    eventTypeCombo->addItem("Hurricane","Hurricane");
    eventTypeCombo->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Maximum);

    QVBoxLayout *Vlayout = new QVBoxLayout(this);

    QHBoxLayout *Unitslayout = new QHBoxLayout();
    QLabel* unitsLabel = new QLabel("Event Units:",this);
    Unitslayout->addWidget(unitsLabel);
    Unitslayout->addWidget(unitsCombo,Qt::AlignLeft);

    QHBoxLayout *Eventslayout = new QHBoxLayout();
    QLabel* eventTypeLabel = new QLabel("Event Type:",this);
    Eventslayout->addWidget(eventTypeLabel);
    Eventslayout->addWidget(eventTypeCombo,Qt::AlignLeft);

    Vlayout->addLayout(Unitslayout);
    Vlayout->addLayout(Eventslayout);
}


SimCenterEventRegional::~SimCenterEventRegional()
{

}


void SimCenterEventRegional::clear(void)
{

}


bool SimCenterEventRegional::outputToJSON(QJsonObject &jsonObject)
{
    // just need to send the class type here.. type needed in object in case user screws up
    jsonObject["type"]= eventTypeCombo->currentText();

    return true;
}


bool SimCenterEventRegional::inputFromJSON(QJsonObject &jsonObject)
{
    Q_UNUSED(jsonObject);

    return true;
}


bool SimCenterEventRegional::outputAppDataToJSON(QJsonObject &jsonObject)
{
    //
    // per API, need to add name of application to be called in AppLication
    // and all data to be used in ApplicationDate
    //

    jsonObject["Application"] = "SimCenterEvent";
    jsonObject["EventClassification"] = eventTypeCombo->currentText();
    QJsonObject dataObj;

    auto units = unitsCombo->currentData().toString();

    dataObj["inputUnit"]=units;

    jsonObject["ApplicationData"] = dataObj;

    return true;
}


bool SimCenterEventRegional::inputAppDataFromJSON(QJsonObject &jsonObject) {


    if (jsonObject.contains("EventClassification"))
    {
        auto eventType = jsonObject["EventClassification"].toString();

        if(eventType.compare("Earthquake") == 0)
        {
            eventTypeCombo->setCurrentText("Earthquake");
        }
        else if(eventType.compare("Hurricane") == 0)
        {
            eventTypeCombo->setCurrentText("Hurricane");
        }
        else
        {
            this->errorMessage("The event classification type "+eventType+" is not recognized");
            return false;
        }
    }
    else
    {
        this->errorMessage("Missing the event classification type in the input file");
        return false;
    }


    if (jsonObject.contains("ApplicationData"))
    {
        QJsonObject appData = jsonObject["ApplicationData"].toObject();

        if (appData.contains("inputUnit"))
        {
            auto unit = appData["inputUnit"].toString();

            if(unit.compare("mps2") == 0)
            {
                unitsCombo->setCurrentText("Meter per second squared");
            }
            else if(unit.compare("ftps2") == 0)
            {
                unitsCombo->setCurrentText("Feet per second squared");
            }
            else if(unit.compare("inchps2") == 0)
            {
                unitsCombo->setCurrentText("Inches per second squared");
            }
            else if(unit.compare("g") == 0)
            {
                unitsCombo->setCurrentText("Gravitational constant (g)");
            }
            else if(unit.compare("ft") == 0)
            {
                unitsCombo->setCurrentText("Feet");
            }
            else if(unit.compare("m") == 0)
            {
                unitsCombo->setCurrentText("Meter");
            }
            else
            {
                this->errorMessage("The unit type "+unit+" is not recognized");
                return false;
            }
        }
        else
        {
            // Default is g
            unitsCombo->setCurrentText("Gravitational constant (g)");
        }

        return true;
    }

    return false;
}


bool SimCenterEventRegional::copyFiles(QString &dirName) {
    Q_UNUSED(dirName);
    return true;
}


void SimCenterEventRegional::setEventType(QString newType)
{
    eventTypeCombo->setCurrentText(newType);
}

