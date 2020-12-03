#include "BuildingModelingWidget.h"
#include "BuildingModelGeneratorWidget.h"
#include "BuildingSimulationWidget.h"
#include "StructuralModelingWidget.h"
#include "CSVtoBIMModelingWidget.h"

#include <QGroupBox>
#include <QComboBox>
#include <QTabWidget>
#include <QGridLayout>
#include <QJsonObject>

BuildingModelingWidget::BuildingModelingWidget(QWidget *parent, RandomVariablesContainer* RVContainer)
    : SimCenterAppWidget(parent), theRandomVariablesContainer(RVContainer)
{    
    QVBoxLayout* buildingModelLayout = new QVBoxLayout(this);

    theTabbedWidget = new QTabWidget(this);

    theBuildingModelGenWidget = new BuildingModelGeneratorWidget(this);
    theStructModelingWidget = new StructuralModelingWidget(this,theRandomVariablesContainer);
    theBuildingSIMWidget = new BuildingSimulationWidget(this, theRandomVariablesContainer);

    theTabbedWidget->addTab(theBuildingModelGenWidget,"Building Information Model (BIM) Generator");
    theTabbedWidget->addTab(theStructModelingWidget,"Building Response Model");
    theTabbedWidget->addTab(theBuildingSIMWidget,"Building Response Simulation");

    theTabbedWidget->setTabEnabled(1,true);

    buildingModelLayout->addWidget(theTabbedWidget);
}


BuildingModelingWidget::~BuildingModelingWidget()
{


}


bool BuildingModelingWidget::outputToJSON(QJsonObject &jsonObj)
{
    // Building model
    QJsonObject buildingObj;
    auto res1 = theBuildingModelGenWidget->outputToJSON(buildingObj);

    if(!res1)
        return res1;
    else
        jsonObj.insert("Building",buildingObj);

    // Structural model
    QJsonObject modelingObj;
    auto res2 = theStructModelingWidget->outputToJSON(modelingObj);

    if(!res2)
        return res2;
    else
        jsonObj.insert("Modeling",modelingObj);

    // SIM
    QJsonObject SIMobj;
    auto res3 = theBuildingSIMWidget->outputToJSON(SIMobj);

    if(!res3)
        return res3;
    else
        jsonObj.insert("Simulation",SIMobj);

    return true;
}



