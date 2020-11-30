#ifndef BuildingModelGeneratorWidget_H
#define BuildingModelGeneratorWidget_H

// Written by: Stevan Gavrilovic
// Latest revision: 10.08.2020

#include "SimCenterAppWidget.h"

class CSVtoBIMModelingWidget;

class QComboBox;
class QGroupBox;
class QStackedWidget;

class BuildingModelGeneratorWidget : public  SimCenterAppWidget
{
    Q_OBJECT

public:
    explicit BuildingModelGeneratorWidget(QWidget *parent);
    virtual ~BuildingModelGeneratorWidget();

    bool outputToJSON(QJsonObject &rvObject);

public slots:

    void handleBuildingModelSelectionChanged(const int index);

private:
    QComboBox* buildingModelSelectCombo;

    QStackedWidget* theStackedWidget;
    CSVtoBIMModelingWidget* theCSVtoBIMWidget;
};

#endif // BuildingModelGeneratorWidget_H
