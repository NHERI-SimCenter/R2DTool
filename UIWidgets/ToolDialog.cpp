#include "ToolDialog.h"
#include "Utils/ProgramOutputDialog.h"
#include "WorkflowAppWidget.h"
#include "HousingUnitAllocationWidget.h"
#include "QGISVisualizationWidget.h"
#include "QGISHurricaneSelectionWidget.h"
#include "OpenQuakeSelectionWidget.h"
#include "BrailsInventoryGenerator.h"
#include "BrailsTranspInventoryGenerator.h"
#include "PyReCodesWidget.h"
#include "GMWidget.h"
#include "ResidualDemandToolWidget.h"

#include <QVBoxLayout>
#include <QStackedWidget>
#include <QPushButton>
#include <QGuiApplication>
#include <QScreen>

ToolDialog::ToolDialog(WorkflowAppWidget* parent, QGISVisualizationWidget* visWidget) : QDialog(parent), visualizationWidget(visWidget)
{
  
#ifndef OpenSRA
  QRect rec = QGuiApplication::primaryScreen()->geometry();
  int height = this->height()<int(rec.height())?int(rec.height()):this->height();
  int width  = this->width()<int(rec.width())?int(rec.width()):this->width();
  height = abs(0.75*height);
  width = abs(0.75*width);
  this->resize(width, height);
#endif
  
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

    if(theResidualDemandToolWidget != nullptr){
        theResidualDemandToolWidget->clear();
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

void ToolDialog::handleResidualDemandTool(void){
    if(theResidualDemandToolWidget == nullptr){
        theResidualDemandToolWidget = new ResidualDemandToolWidget(visualizationWidget,this);
        mainWidget->addWidget(theResidualDemandToolWidget);
    }
    mainWidget->setCurrentWidget(theResidualDemandToolWidget);
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

void ToolDialog::handlePyrecodesTool(void){
    if(thePyReCodesWidget == nullptr)
    {
        thePyReCodesWidget = new PyReCoDesWidget(visualizationWidget,this);
        mainWidget->addWidget(thePyReCodesWidget);
    }

    mainWidget->setCurrentWidget(thePyReCodesWidget);

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

