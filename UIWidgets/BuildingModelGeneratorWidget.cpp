#include "BuildingModelGeneratorWidget.h"

#include "CSVtoBIMModelingWidget.h"

#include <QGroupBox>
#include <QComboBox>
#include <QStackedWidget>
#include <QGridLayout>


BuildingModelGeneratorWidget::BuildingModelGeneratorWidget(QWidget *parent) : SimCenterAppWidget(parent)
{
    buildingModelSelectCombo = new QComboBox(this);
    buildingModelSelectCombo->addItem("CSV to BIM Model Generator (Default)");

    connect(buildingModelSelectCombo,QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BuildingModelGeneratorWidget::handleBuildingModelSelectionChanged);

    QVBoxLayout* buildingModelLayout = new QVBoxLayout(this);

    theStackedWidget = new QStackedWidget(this);

    theCSVtoBIMWidget = new CSVtoBIMModelingWidget(this);

    theStackedWidget->addWidget(theCSVtoBIMWidget);

    buildingModelLayout->addWidget(buildingModelSelectCombo);

    buildingModelLayout->addWidget(theStackedWidget);
}


BuildingModelGeneratorWidget::~BuildingModelGeneratorWidget()
{

}


bool BuildingModelGeneratorWidget::outputToJSON(QJsonObject &jsonObj)
{

    theCSVtoBIMWidget->outputToJSON(jsonObj);

    return true;
}



void BuildingModelGeneratorWidget::handleBuildingModelSelectionChanged(const int index)
{
    theStackedWidget->setCurrentIndex(index);
}




