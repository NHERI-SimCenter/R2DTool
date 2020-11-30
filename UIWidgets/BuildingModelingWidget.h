#ifndef BuildingModelingWidget_H
#define BuildingModelingWidget_H

// Written by: Stevan Gavrilovic
// Latest revision: 10.08.2020

#include "SimCenterAppWidget.h"

class BuildingModelGeneratorWidget;
class BuildingSimulationWidget;
class StructuralModelingWidget;
class RandomVariablesContainer;

class QComboBox;
class QGroupBox;
class QTabWidget;

class BuildingModelingWidget : public  SimCenterAppWidget
{
    Q_OBJECT

public:
    explicit BuildingModelingWidget(QWidget *parent, RandomVariablesContainer* RVContainer);
    virtual ~BuildingModelingWidget();

    bool outputToJSON(QJsonObject &rvObject);

private:
    RandomVariablesContainer* theRandomVariablesContainer;
    QTabWidget* theTabbedWidget;
    BuildingModelGeneratorWidget* theBuildingModelGenWidget;
    StructuralModelingWidget* theStructModelingWidget;
    BuildingSimulationWidget* theBuildingSIMWidget;
};

#endif // BuildingModelingWidget_H
