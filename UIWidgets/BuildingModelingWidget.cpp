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

BuildingModelingWidget::BuildingModelingWidget(QWidget *parent, RandomVariablesContainer* RVContainer) : SimCenterAppWidget(parent), theRandomVariablesContainer(RVContainer)
{    
    QVBoxLayout* buildingModelLayout = new QVBoxLayout(this);

    theTabbedWidget = new QTabWidget(this);

    theBuildingModelGenWidget = new BuildingModelGeneratorWidget(this);
    theStructModelingWidget = new StructuralModelingWidget(this,theRandomVariablesContainer);
    theBuildingSIMWidget = new BuildingSimulationWidget(this);

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

    QJsonObject buildingObj;
    theBuildingModelGenWidget->outputToJSON(buildingObj);
    jsonObj.insert("Building",buildingObj);


    theStructModelingWidget->outputToJSON(jsonObj);

    return true;
}



