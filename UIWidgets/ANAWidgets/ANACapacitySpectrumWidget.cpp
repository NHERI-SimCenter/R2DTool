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

// Written by: 
//   Stevan Gavrilovic
//   Adam Zsarnoczay

#include "ANACapacitySpectrumWidget.h"
#include "SimCenterAppSelection.h"
#include "NoArgSimCenterApp.h"
#include "CapacitySpectrumWidgets/HAZUSDemandWidget.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QPushButton>
#include <QGridLayout>
#include <QGroupBox>
#include <QIntValidator>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QJsonArray>

ANACapacitySpectrumWidget::ANACapacitySpectrumWidget(QWidget *parent): SimCenterAppWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
//    QGroupBox* DemandGroupBox = new QGroupBox(this);
//    DemandGroupBox->setTitle("Demand Spectrum Model");

//    QGroupBox* CapacityGroupBox = new QGroupBox(this);
//    CapacityGroupBox->setTitle("Capacity Curve Model");

//    QGroupBox* DampingGroupBox = new QGroupBox(this);
//    DampingGroupBox->setTitle("Damping Model");


//    QGridLayout* gridLayout = new QGridLayout(DemandGroupBox);
    DemandSelection = new SimCenterAppSelection ("Demand Spectrum Model", "DemandModel", this);
    HAZUSDemandWidget* hazusDemand = new HAZUSDemandWidget(this);
    DemandSelection->addComponent(QString("HAZUS"), QString("HAZUS"), hazusDemand);
    DemandAppNameToDisplayText = new QMap<QString, QString>();
    DemandAppNameToDisplayText->insert("HAZUS", "HAZUS");

    CapacitySelection = new SimCenterAppSelection ("Capacity Curve Model", "CapacityModel", this);
    SimCenterAppWidget* HAZUSCaoPetersonCapacity = new NoArgSimCenterApp(QString("HAZUS_cao_peterson_2006"), this);
    CapacitySelection->addComponent(QString("Cao and Peterson (2006)"), QString("HAZUS_cao_peterson_2006"), HAZUSCaoPetersonCapacity);
    CapacityAppNameToDisplayText = new QMap<QString, QString>();
    CapacityAppNameToDisplayText->insert("HAZUS_cao_peterson_2006", "Cao and Peterson (2006)");

    DampingSelection = new SimCenterAppSelection ("Damping Model", "DampingModel", this);
    SimCenterAppWidget* HAZUSCaoPetersonDamping = new NoArgSimCenterApp(QString("HAZUS_cao_peterson_2006"), this);
    DampingSelection->addComponent(QString("Cao and Peterson (2006)"), QString("HAZUS_cao_peterson_2006"), HAZUSCaoPetersonDamping);
    DampingAppNameToDisplayText = new QMap<QString, QString>();
    DampingAppNameToDisplayText->insert("HAZUS_cao_peterson_2006", "Cao and Peterson (2006)");

    layout->addWidget(DemandSelection);
    layout->addWidget(CapacitySelection);
    layout->addWidget(DampingSelection);

    layout->setStretch(3,1);

    this->clear();


}


bool ANACapacitySpectrumWidget::outputAppDataToJSON(QJsonObject &jsonObject)
{
    jsonObject.insert("Application","CapacitySpectrumMethod");
    QJsonObject appDataObj;

    DemandSelection->outputAppDataToJSON(appDataObj);

    if (CapacitySelection->getCurrentSelectionName() == "HAZUS_cao_peterson_2006"){
        QJsonObject modelObj;
        modelObj.insert("Name", "HAZUS_cao_peterson_2006");
        QJsonObject ParamObj;
        modelObj.insert("Parameters", ParamObj);
        appDataObj.insert("CapacityModel", modelObj);
    } else {
        CapacitySelection->outputAppDataToJSON(appDataObj);
    }

    if (DampingSelection->getCurrentSelectionName() == "HAZUS_cao_peterson_2006"){
        QJsonObject modelObj;
        modelObj.insert("Name", "HAZUS_cao_peterson_2006");
        QJsonObject ParamObj;
        modelObj.insert("Parameters", ParamObj);
        appDataObj.insert("DampingModel", modelObj);
    } else {
        DampingSelection->outputAppDataToJSON(appDataObj);
    }

    jsonObject.insert("ApplicationData", appDataObj);

    return true;
}


bool ANACapacitySpectrumWidget::inputAppDataFromJSON(QJsonObject &jsonObject)
{
    if (jsonObject.contains("ApplicationData")){
        QJsonObject appData = jsonObject["ApplicationData"].toObject();

        if (appData.contains("DemandModel")){
            QJsonObject ModelObj = appData["DemandModel"].toObject();
            if(ModelObj.contains("Name")){
                QString dispName = DemandAppNameToDisplayText->value(ModelObj["Name"].toString(), "None");
                DemandSelection->selectComponent(dispName);
                if(ModelObj.contains("Parameters")){
                    QJsonObject paraObj = ModelObj["Parameters"].toObject();
                    DemandSelection->getCurrentSelection()->inputAppDataFromJSON(paraObj);
                }
            }
        }

        if (appData.contains("CapacityModel")){
            QJsonObject ModelObj = appData["CapacityModel"].toObject();
            if(ModelObj.contains("Name")){
                QString dispName = CapacityAppNameToDisplayText->value(ModelObj["Name"].toString(), "None");
                CapacitySelection->selectComponent(dispName);
                if(ModelObj.contains("Parameters")){
                    QJsonObject paraObj = ModelObj["Parameters"].toObject();
                    CapacitySelection->getCurrentSelection()->inputAppDataFromJSON(paraObj);
                }
            }
        }

        if (appData.contains("DampingModel")){
            QJsonObject ModelObj = appData["DampingModel"].toObject();
            if(ModelObj.contains("Name")){
                QString dispName = DampingAppNameToDisplayText->value(ModelObj["Name"].toString(), "None");
                DampingSelection->selectComponent(dispName);
                if(ModelObj.contains("Parameters")){
                    QJsonObject paraObj = ModelObj["Parameters"].toObject();
                    DampingSelection->getCurrentSelection()->inputAppDataFromJSON(paraObj);
                }
            }
        }
    }

    return true;
}


void ANACapacitySpectrumWidget::clear(void)
{
    DemandSelection->clear();
    CapacitySelection->clear();
    DampingSelection->clear();
}




bool ANACapacitySpectrumWidget::copyFiles(QString &destName)
{

    return true;
}


bool
ANACapacitySpectrumWidget::outputCitation(QJsonObject &jsonObject)
{
  QJsonObject citationPelicun;
  citationPelicun.insert("citation",
"Adam Zsarnoczay, John Vouvakis Manousakis, Jinyan Zhao, Kuanshi Zhong, \
Pouria Kourehpaz (2024). NHERI-SimCenter/pelicun: v3.3. \
Zenodo. https://doi.org/10.5281/zenodo.10896145");
  citationPelicun.insert("description",
"This reference indicates the version of the tool used for the simulation.");

  QJsonObject citationPelicunMarker;
  citationPelicunMarker.insert("citation",
"Adam Zsarnoczay, Gregory G. Deierlein, \
PELICUN - A Computational Framework for Estimating Damage, Loss, and Community \
Resilience, \
Proceedings of the 17th World Conference on Earthquake Engineering, Japan, 2020");
  citationPelicunMarker.insert("description",
"This paper describes the Pelicun damage and loss assessment framework. Please \
reference it if your work results from using the Pelicun engine in the SimCenter \
tools.");

  QJsonArray citationsArray;
  citationsArray.push_back(citationPelicun);
  citationsArray.push_back(citationPelicunMarker);

  jsonObject.insert("citations", citationsArray);
  
  return true;
}

