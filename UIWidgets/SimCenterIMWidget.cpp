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

// Written by: Stevan Gavrilovic, Frank McKenna

#include "SimCenterIMWidget.h"
#include "Utils/ProgramOutputDialog.h"

#include <QGridLayout>
#include <QLabel>
#include <QComboBox>

SimCenterIMWidget::SimCenterIMWidget(QString title, QWidget* parent) : QGroupBox(title, parent)
{
    mainLayout = new QGridLayout(this);

    // Create the dictionaries containing the EDPs according to each hazard
    EDPdict EQ;
    EDPdict Hurricane;
    EDPdict Tsunami;

    EQ["PID"] = "Story Drift Ratio - PID";
    EQ["PRD"] = "Roof Drift Ratio - PRD";
    EQ["DWD"] = "Damageable Wall Drift - DWD";
    EQ["RDR"] = "Racking Drift Ratio - RDR";
    EQ["PFA"] = "Peak Floor Acceleration - PFA";
    EQ["PFV"] = "Peak Floor Velocity - PFV";
    EQ["PGA"] = "Peak Ground Acceleration - PGA";
    EQ["PGV"] = "Peak Ground Velocity - PGV";
    EQ["SA" ] = "Spectral Acceleration - SA";
    EQ["SV" ] = "Spectral Velocity - SV";
    EQ["SD" ] = "Spectral Displacement - SD";
    EQ["PGD"] = "Permanent Ground Deformation - PGD";
    EQ["PMD"] = "Mega Drift Ratio - PMD";
    EQ["RID"] = "Residual Drift Ratio - RID";

    Hurricane["PWS"] = "Peak Gust Wind Speed - PWS ";
    Hurricane["PIH"] = "Peak Inundation Height - PIH ";
    Hurricane["PIH"] = "Flood Water Depth - PIH ";

    Tsunami["PIH"] = "Maximum Tsunami Depth - PIH ";
    Tsunami["PWM"] = "Maximum Tsunami Momentum Flux - PWM ";
    Tsunami["PWV"] = "Maximum Tsunami Velocity - PWV ";

    hazardDict["Earthquake"] = EQ;
    hazardDict["Hurricane"] = Hurricane;
    hazardDict["Tsunami"] = Tsunami;
}


void SimCenterIMWidget::clear(void)
{
    QLayoutItem *child;
    while ((child = mainLayout->takeAt(0)) != nullptr)
    {
        auto widget = child->widget();

        if (widget)
            widget->setParent(nullptr);

        delete child;
    }
}


bool SimCenterIMWidget::outputToJSON(QJsonObject &jsonObject)
{
    auto count = mainLayout->rowCount();

    if(count<1)
        return false;

    QJsonObject imsObj;

    auto numItems = mainLayout->count();
    for(int i = 0; i<numItems; ++i)
    {
        QLayoutItem *child = mainLayout->itemAt(i);

        auto widget = dynamic_cast<QComboBox*>(child->widget());

        if (widget)
        {
            auto currText = widget->currentText();

            auto name = widget->objectName();

            // Return false if IM undefined
            if(currText.compare("N/A") == 0)
            {
                ProgramOutputDialog::getInstance()->appendInfoMessage("Note: attribute "+name+" is not linked to an intensity measure.");
                continue;
            }

            auto currData = widget->currentData().toString();

            imsObj[name] = currData;
        }
    }

    jsonObject["intensityMeasures"] = imsObj;

    return true;
}


bool SimCenterIMWidget::inputFromJSON(QJsonObject &jsonObject)
{
    auto appsObj = jsonObject.value("intensityMeasures").toObject();

    if(appsObj.isEmpty())
        return false;

    foreach(const QString& key, appsObj.keys())
    {
        auto im = appsObj.value(key).toString();

        auto imCombo = this->findChild(key);

        if(imCombo == nullptr)
            return false;
        else
        {
            auto indx = imCombo->findData(im);

            if(indx == -1)
                return false;

            imCombo->setCurrentIndex(indx);
        }
    }

    return true;
}


void SimCenterIMWidget::reset(void)
{

}


void SimCenterIMWidget::addNewIMItem(const QString& labelText, const QString& IMName)
{

    auto i = this->getNumberOfIMs();

    QComboBox* IMCombo = new QComboBox(this);
    IMCombo->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);
    IMCombo->addItem("N/A - Select a hazard",QVariant());

    IMCombo->setObjectName(IMName);

    QLabel* IMLabel = new QLabel(labelText);

    mainLayout->addWidget(IMLabel,i,0,1,1);
    mainLayout->addWidget(IMCombo,i,1,1,1);

}


QComboBox* SimCenterIMWidget::findChild(const QString& name)
{
    auto numItems = mainLayout->count();
    for(int i = 0; i<numItems; ++i)
    {
        QLayoutItem *child = mainLayout->itemAt(i);

        auto widget = dynamic_cast<QComboBox*>(child->widget());

        if (widget)
        {
            auto widgetName = widget->objectName();

            if(widgetName.compare(name) == 0)
                return widget;
        }
    }

    return nullptr;
}


int SimCenterIMWidget::getNumberOfIMs(void)
{
    return mainLayout->count()/mainLayout->columnCount();
}


int SimCenterIMWidget::setIM(const QString& parameterName, const QString& IM)
{
    auto widget = this->findChild(parameterName);

    if(widget)
    {
        auto indx = widget->findData(IM);

        if(indx == -1)
            return -1;

        widget->setCurrentIndex(indx);
    }
    else
        return -1;


    return 0;
}


QList<QString> SimCenterIMWidget::getParameterNames()
{
    QList<QString> paramList;
    auto numItems = mainLayout->count();
    for(int i = 0; i<numItems; ++i)
    {
        QLayoutItem *child = mainLayout->itemAt(i);

        auto widget = dynamic_cast<QComboBox*>(child->widget());
        if (widget)
        {
            auto widgetName = widget->objectName();
            paramList.append(widgetName);
        }
    }

    return paramList;
}


void SimCenterIMWidget::handleHazardChange(const QString hazard)
{
    if (!hazardDict.contains(hazard))
    {
        qDebug()<<"Error, hazard not supported "<<hazard;
        return;
    }

    EDPdict edpd = hazardDict[hazard];

    auto keys = edpd.keys();
    auto values = edpd.values();
    auto numItems = mainLayout->count();

    // Swap out the
    for(int i = 0; i<numItems; ++i)
    {
        QLayoutItem *child = mainLayout->itemAt(i);

        QComboBox* widget = dynamic_cast<QComboBox*>(child->widget());

        if (widget)
        {
            widget->clear();

            for (int j=0; j<keys.size(); ++j)
            {
                auto key = keys.at(j);
                auto value = values.at(j);

                widget->addItem(value,key);
            }

            widget->addItem("N/A",QVariant());

            widget->setCurrentText("N/A");
        }
    }

}
