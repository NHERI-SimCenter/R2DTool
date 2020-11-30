#include "BuildingSimulationWidget.h"

//#include "CSVtoBIMModelingWidget.h"

#include <QGroupBox>
#include <QComboBox>
#include <QStackedWidget>
#include <QGridLayout>


BuildingSimulationWidget::BuildingSimulationWidget(QWidget *parent) : SimCenterAppWidget(parent)
{
    buildingModelSelectCombo = new QComboBox(this);
    buildingModelSelectCombo->addItem("OpenSeesPy Simulation");

    connect(buildingModelSelectCombo,QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BuildingSimulationWidget::handleBuildingModelSelectionChanged);

    QVBoxLayout* buildingModelLayout = new QVBoxLayout(this);

    theStackedWidget = new QStackedWidget(this);

//    theCSVtoBIMWidget = new CSVtoBIMModelingWidget(this);

//    theStackedWidget->addWidget(theCSVtoBIMWidget);

    buildingModelLayout->addWidget(buildingModelSelectCombo);

    buildingModelLayout->addWidget(theStackedWidget);
}


BuildingSimulationWidget::~BuildingSimulationWidget()
{

}


bool BuildingSimulationWidget::outputToJSON(QJsonObject &jsonObj)
{

//    theCSVtoBIMWidget->outputToJSON(jsonObj);

    return true;
}



void BuildingSimulationWidget::handleBuildingModelSelectionChanged(const int index)
{
    theStackedWidget->setCurrentIndex(index);
}




