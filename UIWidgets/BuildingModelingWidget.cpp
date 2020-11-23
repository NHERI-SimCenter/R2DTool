#include "BuildingModelingWidget.h"

#include "CSVtoBIMModelingWidget.h"

#include <QGroupBox>
#include <QComboBox>
#include <QStackedWidget>
#include <QGridLayout>


BuildingModelingWidget::BuildingModelingWidget(QWidget *parent) : SimCenterAppWidget(parent)
{
    componentGroupBox = nullptr;
}


BuildingModelingWidget::~BuildingModelingWidget()
{

}


QGroupBox* BuildingModelingWidget::getComponentsWidget(void)
{
    if(componentGroupBox == nullptr)
        this->createComponentsBox();

    return componentGroupBox;
}


bool BuildingModelingWidget::outputToJSON(QJsonObject &jsonObj)
{

    theCSVtoBIMWidget->outputToJSON(jsonObj);

    return true;
}


void BuildingModelingWidget::createComponentsBox(void)
{

    componentGroupBox = new QGroupBox("Buildings Models");
    componentGroupBox->setFlat(true);

    buildingModelSelectCombo = new QComboBox(this);
    buildingModelSelectCombo->addItem("CSV to BIM Model Generator (Default)");

    connect(buildingModelSelectCombo,QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BuildingModelingWidget::handleBuildingModelSelectionChanged);

    QVBoxLayout* buildingModelLayout = new QVBoxLayout(componentGroupBox);

    theStackedWidget = new QStackedWidget(this);

    theCSVtoBIMWidget = new CSVtoBIMModelingWidget(this);

    theStackedWidget->addWidget(theCSVtoBIMWidget);

    buildingModelLayout->addWidget(buildingModelSelectCombo);

    buildingModelLayout->addWidget(theStackedWidget);

}

void BuildingModelingWidget::handleBuildingModelSelectionChanged(const int index)
{
    theStackedWidget->setCurrentIndex(index);
}




