#include "BuildingSimulationWidget.h"
#include "InputWidgetOpenSeesAnalysis.h"
#include "InputWidgetOpenSeesPyAnalysis.h"

#include <QGroupBox>
#include <QComboBox>
#include <QStackedWidget>
#include <QGridLayout>
#include <QDebug>

BuildingSimulationWidget::BuildingSimulationWidget(QWidget *parent, RandomVariablesContainer* RVContainer) : SimCenterAppWidget(parent), theRandomVariablesContainer(RVContainer)
{
    buildingSIMSelectCombo = new QComboBox(this);
    buildingSIMSelectCombo->addItem("OpenSees Simulation");
    buildingSIMSelectCombo->addItem("OpenSeesPy Simulation");

    connect(buildingSIMSelectCombo,QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BuildingSimulationWidget::handleBuildingSIMSelectionChanged);

    QVBoxLayout* buildingModelLayout = new QVBoxLayout(this);

    theStackedWidget = new QStackedWidget(this);

    openSeesInputWidget = new InputWidgetOpenSeesAnalysis(theRandomVariablesContainer, this);
    openSeesPyInputWidget = new InputWidgetOpenSeesPyAnalysis(theRandomVariablesContainer, this);

    theStackedWidget->addWidget(openSeesInputWidget);
    theStackedWidget->addWidget(openSeesPyInputWidget);

    buildingModelLayout->addWidget(buildingSIMSelectCombo);
    buildingModelLayout->addWidget(theStackedWidget);

    buildingSIMSelectCombo->setCurrentText("OpenSeesPy Simulation");
}


BuildingSimulationWidget::~BuildingSimulationWidget()
{

}


bool BuildingSimulationWidget::outputToJSON(QJsonObject &jsonObj)
{

    auto currentSelection = buildingSIMSelectCombo->currentText();

    if(currentSelection.compare("OpenSees Simulation") == 0)
    {
        return openSeesInputWidget->outputToJSON(jsonObj);
    }
    else if(currentSelection.compare("OpenSeesPy Simulation") == 0)
    {
        return openSeesPyInputWidget->outputToJSON(jsonObj);
    }
    else
    {
        qDebug()<<"Warning, selection "<<currentSelection<<" not handled";
        return false;
    }

    return true;
}


void BuildingSimulationWidget::handleBuildingSIMSelectionChanged(const int index)
{
    theStackedWidget->setCurrentIndex(index);
}




