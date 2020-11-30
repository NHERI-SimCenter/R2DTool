#include "StructuralModelingWidget.h"
#include "OpenSeesBuildingModel.h"
#include "OpenSeesPyBuildingModel.h"

#include <QGroupBox>
#include <QComboBox>
#include <QStackedWidget>
#include <QGridLayout>
#include <QJsonObject>

StructuralModelingWidget::StructuralModelingWidget(QWidget *parent, RandomVariablesContainer* RVContainer) : SimCenterAppWidget(parent), theRandomVariablesContainer(RVContainer)
{
    buildingModelSelectCombo = new QComboBox(this);
    buildingModelSelectCombo->addItem("OpenSeesPy Model");
    buildingModelSelectCombo->addItem("OpenSees Model");

    connect(buildingModelSelectCombo,QOverload<int>::of(&QComboBox::currentIndexChanged), this, &StructuralModelingWidget::handleBuildingModelSelectionChanged);

    QVBoxLayout* buildingModelLayout = new QVBoxLayout(this);

    theStackedWidget = new QStackedWidget(this);

    theOpenSeesModelWidget = new OpenSeesBuildingModel(theRandomVariablesContainer,false,this);
    theOpenSeesPyModelWidget = new OpenSeesPyBuildingModel(theRandomVariablesContainer,this);

    theStackedWidget->addWidget(theOpenSeesPyModelWidget);
    theStackedWidget->addWidget(theOpenSeesModelWidget);

    buildingModelLayout->addWidget(buildingModelSelectCombo);
    buildingModelLayout->addWidget(theStackedWidget);
}


StructuralModelingWidget::~StructuralModelingWidget()
{

}


bool StructuralModelingWidget::outputToJSON(QJsonObject &jsonObj)
{
    QJsonObject modelingObj;
    theOpenSeesPyModelWidget->outputToJSON(modelingObj);

    jsonObj.insert("Modeling",modelingObj);

    return true;
}



void StructuralModelingWidget::handleBuildingModelSelectionChanged(const int index)
{
    theStackedWidget->setCurrentIndex(index);
}




