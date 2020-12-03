#include "BuildingDMWidget.h"
#include "BuildingDMEQWidget.h"
#include "BuildingSimulationWidget.h"
#include "StructuralModelingWidget.h"
#include "CSVtoBIMModelingWidget.h"

#include <QGroupBox>
#include <QComboBox>
#include <QTabWidget>
#include <QGridLayout>
#include <QJsonObject>

BuildingDMWidget::BuildingDMWidget(QWidget *parent) : SimCenterAppWidget(parent)
{    
    QVBoxLayout* buildingModelLayout = new QVBoxLayout(this);

    theTabbedWidget = new QTabWidget(this);

    theDMEQWidget = new BuildingDMEQWidget(nullptr,this);
    theTabbedWidget->addTab(theDMEQWidget,"Earthquake DM");

    theTabbedWidget->setTabEnabled(1,true);

    buildingModelLayout->addWidget(theTabbedWidget);
}


BuildingDMWidget::~BuildingDMWidget()
{


}


bool BuildingDMWidget::outputToJSON(QJsonObject &jsonObj)
{
    theDMEQWidget->outputToJSON(jsonObj);

    return true;
}



