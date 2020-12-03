#include "BuildingEDPWidget.h"
#include "BuildingEDPEQWidget.h"
#include "BuildingSimulationWidget.h"
#include "StructuralModelingWidget.h"
#include "CSVtoBIMModelingWidget.h"

#include <QGroupBox>
#include <QComboBox>
#include <QTabWidget>
#include <QGridLayout>
#include <QJsonObject>

BuildingEDPWidget::BuildingEDPWidget(QWidget *parent) : SimCenterAppWidget(parent)
{    
    QVBoxLayout* buildingModelLayout = new QVBoxLayout(this);

    theTabbedWidget = new QTabWidget(this);

    theEDPEQWidget = new BuildingEDPEQWidget(nullptr,this);
    theTabbedWidget->addTab(theEDPEQWidget,"Earthquake EDP");

    theTabbedWidget->setTabEnabled(1,true);

    buildingModelLayout->addWidget(theTabbedWidget);
}


BuildingEDPWidget::~BuildingEDPWidget()
{


}


bool BuildingEDPWidget::outputToJSON(QJsonObject &jsonObj)
{
    theEDPEQWidget->outputToJSON(jsonObj);

    return true;
}



