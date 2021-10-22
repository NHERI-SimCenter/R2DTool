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

#include "SimCenterUnitsWidget.h"
#include "SimCenterUnitsCombo.h"
#include "Utils/PythonProgressDialog.h"

#include <QGridLayout>
#include <QLabel>

SimCenterUnitsWidget::SimCenterUnitsWidget(QString title, QWidget* parent) : QGroupBox(title, parent)
{
    unitsLayout = new QGridLayout(this);
}


void SimCenterUnitsWidget::clear(void)
{
    QLayoutItem *child;
    while ((child = unitsLayout->takeAt(0)) != nullptr)
    {
        auto widget = child->widget();

        if (widget)
            widget->setParent(nullptr);

        delete child;
    }
}


bool SimCenterUnitsWidget::outputToJSON(QJsonObject &jsonObject)
{
    auto count = unitsLayout->rowCount();

    if(count<1)
        return false;

    QJsonObject unitsObj;

    auto numItems = unitsLayout->count();
    for(int i = 0; i<numItems; ++i)
    {
        QLayoutItem *child = unitsLayout->itemAt(i);

        auto widget = dynamic_cast<SimCenterUnitsCombo*>(child->widget());

        if (widget)
        {
            auto name = widget->getName();

            if(name.isEmpty())
                return false;

            auto unit = widget->getCurrentUnitString();

            // Return false if unit undefined
            if(unit.compare("UNDEFINED") == 0)
            {
                PythonProgressDialog::getInstance()->appendErrorMessage("Warning unit undefined! Please set the unit");
                return false;
            }

            unitsObj[name] = unit;
        }
    }

    jsonObject["Units"] = unitsObj;

    return true;
}


bool SimCenterUnitsWidget::inputFromJSON(QJsonObject &jsonObject)
{

    auto unitsObj = jsonObject.value("Units").toObject();

    if(unitsObj.isEmpty())
        return false;

    foreach(const QString& key, unitsObj.keys())
    {
        auto unit = unitsObj.value(key).toString();

        auto unitCombo = this->findChild(key);

        if(unitCombo == nullptr)
            return false;
        else
        {
            auto res = unitCombo->setCurrentUnitString(unit);

            if(!res)
                return false;
        }
    }

    return true;
}


void SimCenterUnitsWidget::reset(void)
{

}


void SimCenterUnitsWidget::addNewUnitItem(const QString& name)
{
    auto i = this->getNumberOfUnits();

    SimCenterUnitsCombo* unitsCombo = new SimCenterUnitsCombo(SimCenter::Unit::ALL,name);

    QLabel* unitLabel = new QLabel(name);

    unitsLayout->addWidget(unitLabel,i,0,1,1);
    unitsLayout->addWidget(unitsCombo,i,1,1,1);
}


SimCenterUnitsCombo* SimCenterUnitsWidget::findChild(const QString& name)
{
    auto numItems = unitsLayout->count();
    for(int i = 0; i<numItems; ++i)
    {
        QLayoutItem *child = unitsLayout->itemAt(i);

        auto widget = dynamic_cast<SimCenterUnitsCombo*>(child->widget());

        if (widget)
        {
            auto widgetName = widget->getName();

            if(widgetName.compare(name) == 0)
                return widget;
        }
    }

    return nullptr;
}


int SimCenterUnitsWidget::getNumberOfUnits(void)
{
    return unitsLayout->count()/unitsLayout->columnCount();
}


int SimCenterUnitsWidget::setUnit(const QString& parameterName, const QString& unit)
{
    auto widget = this->findChild(parameterName);

    if(widget)
    {
        auto res = widget->setCurrentUnitString(unit);

        if(!res)
            return -1;
    }
    else
        return -1;


    return 0;
}


QList<QString> SimCenterUnitsWidget::getParameterNames()
{
    QList<QString> paramList;
    auto numItems = unitsLayout->count();
    for(int i = 0; i<numItems; ++i)
    {
        QLayoutItem *child = unitsLayout->itemAt(i);

        auto widget = dynamic_cast<SimCenterUnitsCombo*>(child->widget());
        if (widget)
        {
            auto widgetName = widget->getName();
            paramList.append(widgetName);
        }
    }

    return paramList;
}
