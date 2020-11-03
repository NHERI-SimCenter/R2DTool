#include "EarthquakeInputWidget.h"
#include "VisualizationWidget.h"

#include "GMWidget.h"

#include <QGroupBox>
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QStackedWidget>

EarthquakeInputWidget::EarthquakeInputWidget(QWidget *parent, VisualizationWidget* visWidget, RandomVariablesContainer * RVContainer) : SimCenterAppWidget(parent), theRandomVariablesContainer(RVContainer), theVisualizationWidget(visWidget)
{
    theEQWidget = nullptr;
    theRootStackedWidget = nullptr;
}


EarthquakeInputWidget::~EarthquakeInputWidget()
{

}


QWidget* EarthquakeInputWidget::getEarthquakesWidget(void)
{
    if(theEQWidget == nullptr)
        this->createEarthquakesWidget();

    return theEQWidget;
}



void EarthquakeInputWidget::createEarthquakesWidget(void)
{
    theEQWidget = new QWidget(this);

    QGridLayout* gridLayout = new QGridLayout();
    theEQWidget->setLayout(gridLayout);

    auto smallVSpacer = new QSpacerItem(0,10);

    QLabel* selectionText = new QLabel();
    selectionText->setText("Earthquake Hazard Type:");

    auto earthquakeSelectionCombo = new QComboBox();
    earthquakeSelectionCombo->addItem("Earthquake Scenario Simulation");
    earthquakeSelectionCombo->addItem("ShakeMap Input");

    connect(earthquakeSelectionCombo, SIGNAL(currentIndexChanged(QString)), this, SLOT(handleEQTypeSelection(QString)));

    theRootStackedWidget = new QStackedWidget();

    // Add a vertical spacer at the bottom to push everything up
    auto vspacer = new QSpacerItem(0,0,QSizePolicy::Minimum, QSizePolicy::Expanding);

    gridLayout->addItem(smallVSpacer,0,0);
    gridLayout->addWidget(selectionText,1,0);
    gridLayout->addWidget(earthquakeSelectionCombo,1,1);
    gridLayout->addWidget(theRootStackedWidget,2,0,1,2);

    gridLayout->addItem(vspacer, 3, 0);

    EQSSWidget = new GMWidget(this, theVisualizationWidget);
    theRootStackedWidget->addWidget(EQSSWidget);


    theRootStackedWidget->setCurrentWidget(EQSSWidget);
}




void EarthquakeInputWidget::handleEQTypeSelection(const QString& selection)
{

}
