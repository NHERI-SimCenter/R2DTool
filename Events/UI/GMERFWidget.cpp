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

#include "GMERFWidget.h"
#include "RuptureWidget.h"
#include "JsonSerializable.h"
#include "VisualizationWidget.h"
#include "PointSourceRuptureWidget.h"
#include "OpenQuakeUserSpecifiedWidget.h"
#include "OpenQuakeScenarioWidget.h"
#include "ModularPython.h"
#include "SimCenterPreferences.h"
#include "QGISVisualizationWidget.h"
#include "GmAppConfig.h"
#include "qgsvectorlayer.h"

#include <QVBoxLayout>
#include <QGroupBox>
#include <QComboBox>
#include <QPushButton>
#include <QStackedWidget>

GMERFWidget::GMERFWidget(QGISVisualizationWidget* visWidget, GmAppConfig* appConfig, QString jsonKey, QWidget *parent) : SimCenterAppSelection("Earthquake Rupture",jsonKey,parent), theVisualizationWidget(visWidget), m_appConfig(appConfig), jsonKey(jsonKey)
{

    // Create the widgets that will be in the stacked box
    ERFruptureWidget = new RuptureWidget("EqRupture");
    pointSourceWidget = new PointSourceRuptureWidget();
    oqcpuWidget = new OpenQuakeUserSpecifiedWidget();
    oqsbWidget = new OpenQuakeScenarioWidget();

    // OpenQuake classical PSHA
    //    oqcpWidget = new OpenQuakeClassicalWidget(parent);

    // Hazard Occurrence (KZ-08/22)
    //    hoWidget = new HazardOccurrenceWidget(parent);
    //    hoWidget->setToolTip("Hazard occurrence models reduce the number of earthquake scenarios and/or \nground motion maps to be analyzed in regional risk assessment");
    //    hoWidget->setToolTipDuration(5000);


    this->addComponent("OpenSHA ERF","OpenSHAEQ", ERFruptureWidget);
    this->addComponent("Point Source","PointEQ", pointSourceWidget);
    this->addComponent("OpenQuake Source Model","OpenQSource", oqcpuWidget);
    this->addComponent("OpenQuake Scenario-based","OpenQScenario", oqsbWidget);
    //    theStackedWidget->addWidget(oqcpWidget);
    //    theStackedWidget->addWidget(hoWidget);

    forecastRupScenButton = new QPushButton("Forecast Rupture Scenarios");

    auto mapView = theVisualizationWidget->getMapViewWidget("RuptureScenarioView");
    mapViewSubWidget = std::unique_ptr<SimCenterMapcanvasWidget>(mapView);

    auto* earthquakeLayout = qobject_cast<QVBoxLayout*>(this->layout());
    earthquakeLayout->addWidget(forecastRupScenButton);
    earthquakeLayout->addWidget(mapViewSubWidget.get());

}


bool GMERFWidget::inputFromJSON(QJsonObject& /*obj*/)
{

    return true;
}


bool GMERFWidget::outputToJSON(QJsonObject& obj)
{

    if(!SimCenterAppSelection::outputToJSON(obj))
        return false;

    // Add some additional things to the object - TODO check if this is redundant
    QJsonObject scenarioObj = obj[jsonKey].toObject();

    scenarioObj.insert("Type", "Earthquake");
    scenarioObj.insert("Generator", "Selection");

    obj[jsonKey] = scenarioObj;

    return true;
}


RuptureWidget *GMERFWidget::ruptureWidget() const
{
    return ERFruptureWidget;
}


void GMERFWidget::run_button_pressed(const QJsonObject& siteObj)
{

    QJsonObject configFile;

    configFile["Site"] = siteObj;

    this->outputToJSON(configFile);

    auto pathToInputDir = m_appConfig->getInputDirectoryPath() + QDir::separator();
    auto pathToOutputDir = m_appConfig->getOutputDirectoryPath() + QDir::separator();

    configFile["Directory"] = pathToOutputDir;

    // Assemble the path and save the config file
    QString pathToConfigFile = pathToInputDir + QDir::separator() + "EQScenarioConfiguration.json";

    QFile file(pathToConfigFile);

    QString strFromObj = QJsonDocument(configFile).toJson(QJsonDocument::Indented);

    if(!file.open(QIODevice::WriteOnly))
    {
        file.close();
    }
    else
    {
        QTextStream out(&file); out << strFromObj;
        file.close();
    }

    // Assemble the path and save the EqRupture file
    QJsonObject eqRupFile = configFile["Scenario"].toObject()["EqRupture"].toObject();
    QString pathToEqRupFile = pathToOutputDir + QDir::separator() + "EQRupture.json";

    QFile file2(pathToEqRupFile);

    QString strFromObj2 = QJsonDocument(eqRupFile).toJson(QJsonDocument::Indented);

    if(!file2.open(QIODevice::WriteOnly))
    {
        file2.close();
    }
    else
    {
        QTextStream out(&file2); out << strFromObj2;
        file2.close();
    }

    // Set up and run the script
    QString scriptPath = SimCenterPreferences::getInstance()->getAppDir() + QDir::separator()
                         + "applications" + QDir::separator() + "performRegionalEventSimulation" + QDir::separator()
                         + "regionalGroundMotion" + QDir::separator() + "ScenarioForecast.py";



    QStringList scriptArgs;
    scriptArgs << QString("--hazard_config")  << pathToConfigFile;

    std::unique_ptr<ModularPython> thePy = std::make_unique<ModularPython>(pathToOutputDir);

    connect(thePy.get(),SIGNAL(runComplete()),this,SLOT(processRuptureScenarioResults()));

    thePy->run(scriptPath,scriptArgs);

    disconnect(thePy.get(),SIGNAL(runComplete()),this,SLOT(processRuptureScenarioResults()));

}


QPushButton *GMERFWidget::getForecastRupScenButton() const
{
    return forecastRupScenButton;
}


void GMERFWidget::clear(void)
{
    mainLayer = nullptr;
}


void GMERFWidget::processRuptureScenarioResults(void)
{
    auto pathToRupturesFile = m_appConfig->getOutputDirectoryPath() + QDir::separator() + "Output" + QDir::separator() + "RupFile.geojson";

    if(!QFile::exists(pathToRupturesFile))
    {
        this->errorMessage("Error, could not find the rupture files");
        return;
    }

    this->infoMessage("Processing rupture file");

    // Clear the old layers if any
    if(mainLayer != nullptr)
        theVisualizationWidget->removeLayer(mainLayer);

    mainLayer = theVisualizationWidget->addVectorLayer(pathToRupturesFile, "Earthquake Ruptures", "ogr");

    if(mainLayer == nullptr)
    {
        this->errorMessage("Error, failed to add the ruptures layer");
    }

    auto numFeat = mainLayer->featureCount();

    this->infoMessage("Loaded "+QString::number(numFeat)+" ruptures ");
}
