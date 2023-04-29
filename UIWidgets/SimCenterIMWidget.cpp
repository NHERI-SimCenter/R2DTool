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
#include "SimCenterUnitsCombo.h"
#include "Utils/ProgramOutputDialog.h"

#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QJsonArray>
#include <QSignalMapper>
#include <QPushButton>

SimCenterIMWidget::SimCenterIMWidget(QString title, QWidget* parent)
    : QGroupBox(title, parent)
{
    mainLayout = new QGridLayout(this);
    numIMs = 0;

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

    theSignalMapper = new QSignalMapper();
}


void SimCenterIMWidget::clear(void)
{
    // qDebug() << "Layout count after clearing it: " << mainLayout->count();
    QLayoutItem *child;
    while ((child = mainLayout->takeAt(0)) != nullptr)
    {
        auto widget = child->widget();

        if (widget)
            widget->setParent(nullptr);

        delete child;
    }
    //qDebug() << "Layout count after clearing it: " << mainLayout->count();
    //qDebug() << "Layout count after clearing it numROWS: " << mainLayout->rowCount() << " " << mainLayout->columnCount();
    numIMs = 0;
}


bool SimCenterIMWidget::outputToJSON(QJsonObject &jsonObject)
{
    // we have to put into 2 objects as units is not an array and if JsonObject the bands could get interleaved
    QJsonObject unitObject; // im and unit
    QJsonArray imArray;     // ordered array of im
    QJsonArray labelArray;     // ordered array of labels for IM
    
    for(int i = 0; i<numIMs; ++i) {
        QLayoutItem *labelItem = mainLayout->itemAtPosition(i,0);      
        QLayoutItem *imItem = mainLayout->itemAtPosition(i,1);
        QLayoutItem *unitItem = mainLayout->itemAtPosition(i,2);

        QLabel *labelWidget = dynamic_cast<QLabel*>(labelItem->widget());	
        QComboBox *imWidget = dynamic_cast<QComboBox*>(imItem->widget());
        SimCenterUnitsCombo *unitWidget = dynamic_cast<SimCenterUnitsCombo*>(unitItem->widget());

        if (imWidget && unitWidget) {

            QString currLabel = labelWidget->text();	  
            QString currIM = imWidget->currentData().toString();
            QString currUnit = unitWidget->getCurrentUnitString();

            if(currUnit.compare("UNDEFINED") == 0) {
                ProgramOutputDialog::getInstance()->appendErrorMessage("Warning unit undefined! Please set the unit");
                continue;
            }

            unitObject[currIM] = currUnit;
            imArray.append(currIM);
            labelArray.append(currIM);	    
        }
    }

    jsonObject["intensityMeasures"] = imArray;
    jsonObject["intensityLabels"]=labelArray;
    jsonObject["units"]=unitObject;

    return true;
}


bool SimCenterIMWidget::inputFromJSON(QJsonObject &jsonObject)
{
    this->clear();

    QJsonArray labelArray;    
    QJsonArray imArray;
    QJsonObject  unitsObject;

    if (jsonObject.contains("intensityMeasures")) {
        QJsonValue theValue = jsonObject["intensityMeasures"];
        if (theValue.isArray())
            imArray = theValue.toArray();
    }

    if (jsonObject.contains("intensityLabels")) {
        QJsonValue theValue = jsonObject["intensityLabels"];
        if (theValue.isArray())
            labelArray = theValue.toArray();
    }    

    if (jsonObject.contains("units")) {
        QJsonValue theValue = jsonObject["units"];
        if (theValue.isObject())
            unitsObject = theValue.toObject();
    }

    if(imArray.isEmpty())
        return false;

    for (auto im : imArray) {
        // get im text and unit text
        QString imString = im.toString();
	
        QString labelString;

	qDebug() << "FOUND LABEL: " << labelArray.isEmpty();
	
	if (!labelArray.isEmpty())
	  labelString= labelArray.at(numIMs).toString();
	else
	  labelString = "Layer " + QString::number(numIMs);
	
	    
        QString unitString = unitsObject[imString].toString();
	

        // create the combo boxes
        QComboBox* IMCombo = new QComboBox(this);
        SimCenterUnitsCombo *unitsCombo = new SimCenterUnitsCombo(SimCenter::Unit::ALL,"unit", this);

        IMCombo->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);

        if (hazardDict.contains(hazard)) {
            EDPdict edpd = hazardDict[hazard];
            auto keys = edpd.keys();
            auto values = edpd.values();
            for (int j=0; j<keys.size(); ++j) {
                auto key = keys.at(j);
                auto value = values.at(j);

                IMCombo->addItem(value,key);
            }
        }

        IMCombo->addItem("N/A",QVariant());

        // set the combo boxes correct values from the input
        unitsCombo->setCurrentUnitString(unitString);

        auto indx = IMCombo->findData(imString);
        if(indx != -1)
            IMCombo->setCurrentIndex(indx);
        else
            qDebug() << "ERROR: Hazard Intensity Measue Not Found:" <<  imString;

        mainLayout->addWidget(new QLabel(labelString),numIMs,0);
        mainLayout->addWidget(IMCombo,numIMs,1);
        mainLayout->addWidget(unitsCombo,numIMs,2);
        numIMs++;
    }

    return true;
}


void SimCenterIMWidget::reset(void)
{

}


void SimCenterIMWidget::addNewIMItem(const QString& labelText, const QString& IMName)
{
    QComboBox* IMCombo = new QComboBox(this);
    IMCombo->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);

    if (hazardDict.contains(hazard)) {
        EDPdict edpd = hazardDict[hazard];
        auto keys = edpd.keys();
        auto values = edpd.values();

        for (int j=0; j<keys.size(); ++j) {
            auto key = keys.at(j);
            auto value = values.at(j);

            IMCombo->addItem(value,key);
        }
    }
    
    IMCombo->addItem("N/A",QVariant());
    IMCombo->setCurrentText("N/A");
    IMCombo->setObjectName(IMName);    

    SimCenterUnitsCombo *unitsCombo = new SimCenterUnitsCombo(SimCenter::Unit::ALL,"unit", this);

    QLabel* IMLabel = new QLabel(labelText);

    mainLayout->addWidget(IMLabel,numIMs,0);
    mainLayout->addWidget(IMCombo,numIMs,1);
    mainLayout->addWidget(unitsCombo,numIMs,2);

    QPushButton *removeButton = new QPushButton("Remove");
    mainLayout->addWidget(removeButton,numIMs,3);
    connect(removeButton, &QPushButton::clicked, [this, labelText] {
      // remove(labelText);
      for (int i=0; i<numIMs; i++) {
        QLayoutItem *labelItem = mainLayout->itemAtPosition(i,0);      
        QLabel *labelWidget = dynamic_cast<QLabel*>(labelItem->widget());	
	QString currLabel = labelWidget->text();
	if (currLabel == labelText) {
	  // remove row
	  for (int j=0; j<4; j++) {
	    QLayoutItem* item = mainLayout->itemAtPosition(i,j);
	    QWidget* widget = item->widget();
	    mainLayout->removeItem(item);
	    if (widget)
	      delete widget;
	  }
	  // reduce numberIMs and break out
	  i=numIMs;	  
	  numIMs--;
	}
      }
    });
    
    numIMs++;
}

QComboBox*
SimCenterIMWidget::findChild(const QString& name)
{
    auto numItems = numIMs;
    for(int i = 0; i<numItems; ++i)
    {
      QLayoutItem *child = mainLayout->itemAtPosition(i,1);

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
    return numIMs; // mainLayout->rowCount();
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

QStringList
SimCenterIMWidget::getSelectedIMs() {

    QStringList selectedParam;
    for(int i = 0; i<numIMs; ++i) {
        QLayoutItem *imItem = mainLayout->itemAtPosition(i,1);
        QComboBox *imWidget = dynamic_cast<QComboBox*>(imItem->widget());    
	QString currIM = imWidget->currentData().toString();	
	selectedParam.append(currIM);
    }
    return selectedParam;
}

/*
QList<QString> SimCenterIMWidget::getParameterNames()
{
    QList<QString> paramList;
    for(int i = 0; i<numIMs; ++i) {
      QLayoutItem *child = mainLayout->itemAtPosition(i,1);

        auto widget = dynamic_cast<QComboBox*>(child->widget());
        if (widget)
        {
            auto widgetName = widget->objectName();
            paramList.append(widgetName);
        }
    }

    return paramList;
}
*/

void SimCenterIMWidget::handleHazardChange(const QString hazardType)
{    
    if (!hazardDict.contains(hazardType))
    {
        qDebug()<<"Error, hazard not supported "<<hazard;
        return;
    }

    hazard = hazardType;
    EDPdict edpd = hazardDict[hazard];

    auto keys = edpd.keys();
    auto values = edpd.values();

    // Swap out the
    for(int i = 0; i<numIMs; ++i)
    {
      QLayoutItem *child = mainLayout->itemAtPosition(i,1);
      
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
