#include "ToolDialog.h"
#include "Utils/ProgramOutputDialog.h"
#include "WorkflowAppWidget.h"
#include "HousingUnitAllocationWidget.h"
#include "QGISVisualizationWidget.h"
#include "QGISHurricaneSelectionWidget.h"
#include "OpenQuakeSelectionWidget.h"
#include "BrailsInventoryGenerator.h"
#include "BrailsTranspInventoryGenerator.h"
#include "GMWidget.h"

#include <QVBoxLayout>
#include <QStackedWidget>
#include <QPushButton>

ToolDialog::ToolDialog(WorkflowAppWidget* parent, QGISVisualizationWidget* visWidget) : QDialog(parent), visualizationWidget(visWidget)
{
    mainLayout = new QVBoxLayout(this);
    mainWidget = new QStackedWidget();
    mainLayout->addWidget(mainWidget);

    QPushButton* closeButton = new QPushButton("Close");
    closeButton->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum);
    connect(closeButton,&QPushButton::clicked,this,&QDialog::close);
    mainLayout->addWidget(closeButton);
}


ToolDialog::~ToolDialog()
{

}


void ToolDialog::clear()
{
    if(housingUnitAllocation)
        housingUnitAllocation->clear();

    if(theHurricaneSimWidget)
        theHurricaneSimWidget->clear();

    if(theEQSSWidget != nullptr)
    {
        theEQSSWidget->clear();
    }
}


void ToolDialog::handleShowCensusAppTool(void)
{
    if(housingUnitAllocation == nullptr)
    {
        housingUnitAllocation = new HousingUnitAllocationWidget(this, visualizationWidget);
        mainWidget->addWidget(housingUnitAllocation);
    }

    mainWidget->setCurrentWidget(housingUnitAllocation);

    this->showMaximized();
}


void ToolDialog::handleShowEQGMSimTool(void)
{
    if(theEQSSWidget == nullptr)
    {
        theEQSSWidget = new GMWidget(visualizationWidget,this);
        mainWidget->addWidget(theEQSSWidget);
    }

    mainWidget->setCurrentWidget(theEQSSWidget);

    this->showMaximized();
}


void ToolDialog::handleShowOpenquakeSelectionTool(void)
{
    if(theOpenQuakeSelectionWidget == nullptr)
    {
        theOpenQuakeSelectionWidget = new OpenQuakeSelectionWidget(visualizationWidget,this);
        mainWidget->addWidget(theOpenQuakeSelectionWidget);
    }

    mainWidget->setCurrentWidget(theOpenQuakeSelectionWidget);

    this->showMaximized();
}


void ToolDialog::handleBrailsInventoryTool(void)
{
    if(theBrailsInventoryGeneratorWidget == nullptr)
    {
        theBrailsInventoryGeneratorWidget = new BrailsInventoryGenerator(visualizationWidget,this);
        mainWidget->addWidget(theBrailsInventoryGeneratorWidget);
    }

    mainWidget->setCurrentWidget(theBrailsInventoryGeneratorWidget);

    this->showMaximized();
}

void ToolDialog::handleBrailsTranspInventoryTool(void)
{
    if(theBrailsTranspInventoryGeneratorWidget == nullptr)
    {
        theBrailsTranspInventoryGeneratorWidget = new BrailsTranspInventoryGenerator(visualizationWidget,this);
        mainWidget->addWidget(theBrailsTranspInventoryGeneratorWidget);
    }

    mainWidget->setCurrentWidget(theBrailsTranspInventoryGeneratorWidget);

    this->showMaximized();
}


void ToolDialog::handleShowHurricaneSimTool(void)
{
    if(theHurricaneSimWidget == nullptr)
    {
        theHurricaneSimWidget = new QGISHurricaneSelectionWidget(visualizationWidget,this);
        mainWidget->addWidget(theHurricaneSimWidget);
    }

    mainWidget->setCurrentWidget(theHurricaneSimWidget);

    this->showMaximized();
}

