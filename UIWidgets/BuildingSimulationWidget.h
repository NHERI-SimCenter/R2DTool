#ifndef BuildingSimulationWidget_H
#define BuildingSimulationWidget_H

// Written by: Stevan Gavrilovic
// Latest revision: 10.08.2020

#include "SimCenterAppWidget.h"

class CSVtoBIMModelingWidget;

class QComboBox;
class QGroupBox;
class QStackedWidget;

class BuildingSimulationWidget : public  SimCenterAppWidget
{
    Q_OBJECT

public:
    explicit BuildingSimulationWidget(QWidget *parent);
    virtual ~BuildingSimulationWidget();

    bool outputToJSON(QJsonObject &rvObject);

public slots:

    void handleBuildingModelSelectionChanged(const int index);

private:
    QComboBox* buildingModelSelectCombo;

    QStackedWidget* theStackedWidget;
//    CSVtoBIMModelingWidget* theCSVtoBIMWidget;
};

#endif // BuildingSimulationWidget_H
