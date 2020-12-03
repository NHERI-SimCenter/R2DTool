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
    buildingModelSelectCombo->addItem("OpenSees Model");
    buildingModelSelectCombo->addItem("OpenSeesPy Model");

    connect(buildingModelSelectCombo,QOverload<int>::of(&QComboBox::currentIndexChanged), this, &StructuralModelingWidget::handleBuildingModelSelectionChanged);

    QVBoxLayout* buildingModelLayout = new QVBoxLayout(this);

    theStackedWidget = new QStackedWidget(this);

    theOpenSeesModelWidget = new OpenSeesBuildingModel(theRandomVariablesContainer,false,this);
    theOpenSeesPyModelWidget = new OpenSeesPyBuildingModel(theRandomVariablesContainer,this);

    theStackedWidget->addWidget(theOpenSeesPyModelWidget);
    theStackedWidget->addWidget(theOpenSeesModelWidget);

    buildingModelLayout->addWidget(buildingModelSelectCombo);
    buildingModelLayout->addWidget(theStackedWidget);

    buildingModelSelectCombo->setCurrentText("OpenSeesPy Model");

}


StructuralModelingWidget::~StructuralModelingWidget()
{

}


bool StructuralModelingWidget::outputToJSON(QJsonObject &jsonObj)
{

    auto currentSelection = buildingModelSelectCombo->currentText();

    if(currentSelection.compare("OpenSeesPy Model") == 0)
    {
        return theOpenSeesPyModelWidget->outputToJSON(jsonObj);
    }
    else if(currentSelection.compare("OpenSees Model") == 0)
    {
        return theOpenSeesModelWidget->outputToJSON(jsonObj);
    }
    else
    {
        qDebug()<<"Warning, selection "<<currentSelection<<" not handled";
        return false;
    }


    return true;
}



void StructuralModelingWidget::handleBuildingModelSelectionChanged(const int index)
{
    theStackedWidget->setCurrentIndex(index);
}




