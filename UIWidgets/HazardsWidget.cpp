#include "HazardsWidget.h"
#include "VisualizationWidget.h"
#include "UserInputGMWidget.h"
#include "GMWidget.h"
#include "ShakeMapWidget.h"
#include "WorkflowAppRDT.h"

#include <QGroupBox>
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QStackedWidget>
#include <QCheckBox>
#include <QJsonObject>
#include <QJsonArray>

HazardsWidget::HazardsWidget(QWidget *parent,
                             VisualizationWidget* visWidget,
                             RandomVariablesContainer * RVContainer)
    : SimCenterAppSelection(QString("Hazard Selection"),QString("Hazard"), parent),
      theRandomVariablesContainer(RVContainer), theVisualizationWidget(visWidget)
{
    theShakeMapWidget = nullptr;
    theEQSSWidget = nullptr;
    theUserInputGMWidget = nullptr;

    theEQSSWidget = new GMWidget(this, theVisualizationWidget);
    // theShakeMapWidget = new ShakeMapWidget(theVisualizationWidget);
    theUserInputGMWidget = new UserInputGMWidget(theVisualizationWidget);

    this->addComponent("Earthquake Scenario Simulation", "EQSS", theEQSSWidget);
    this->addComponent("User Specified Ground Motions", "UserInputGM", theUserInputGMWidget);

    //connect(theShakeMapWidget, &ShakeMapWidget::loadingComplete, this, &HazardsWidget::shakeMapLoadingFinished);
    connect(theEQSSWidget, SIGNAL(outputDirectoryPathChanged(QString, QString)), this,  SLOT(gridFileChangedSlot(QString, QString)));
    connect(theUserInputGMWidget, SIGNAL(outputDirectoryPathChanged(QString, QString)), this,  SLOT(gridFileChangedSlot(QString, QString)));

}


HazardsWidget::~HazardsWidget()
{

}


void HazardsWidget::shakeMapLoadingFinished(const bool value)
{
    if(!value)
        return;

    // Shift the focus to the visualization widget
    auto mainWindowWidget = qobject_cast<WorkflowAppRDT*>(this->parent());

    if(!mainWindowWidget)
        return;

    // mainWindowWidget->setActiveWidget(theVisualizationWidget);

}


void HazardsWidget::gridFileChangedSlot(QString motionD, QString eventF)
{
    emit gridFileChangedSignal(motionD, eventF);
}

void HazardsWidget::setCurrentlyViewable(bool status) {
    this->SimCenterAppSelection::setCurrentlyViewable(status);
}
