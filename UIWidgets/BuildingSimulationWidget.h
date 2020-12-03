#ifndef BuildingSimulationWidget_H
#define BuildingSimulationWidget_H

// Written by: Stevan Gavrilovic
// Latest revision: 10.08.2020

#include "SimCenterAppWidget.h"

class InputWidgetOpenSeesAnalysis;
class InputWidgetOpenSeesPyAnalysis;
class RandomVariablesContainer;

class QComboBox;
class QGroupBox;
class QStackedWidget;

class BuildingSimulationWidget : public  SimCenterAppWidget
{
    Q_OBJECT

public:
    explicit BuildingSimulationWidget(QWidget *parent, RandomVariablesContainer* RVContainer);
    virtual ~BuildingSimulationWidget();

    bool outputToJSON(QJsonObject &rvObject);

public slots:

    void handleBuildingSIMSelectionChanged(const int index);

private:
    QComboBox* buildingSIMSelectCombo;

    QStackedWidget* theStackedWidget;
    InputWidgetOpenSeesAnalysis* openSeesInputWidget;
    InputWidgetOpenSeesPyAnalysis* openSeesPyInputWidget;
    RandomVariablesContainer* theRandomVariablesContainer;
};

#endif // BuildingSimulationWidget_H
