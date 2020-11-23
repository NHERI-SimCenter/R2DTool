#ifndef BuildingModelingWidget_H
#define BuildingModelingWidget_H

// Written by: Stevan Gavrilovic
// Latest revision: 10.08.2020

#include "SimCenterAppWidget.h"

class CSVtoBIMModelingWidget;

class QComboBox;
class QGroupBox;
class QStackedWidget;

class BuildingModelingWidget : public  SimCenterAppWidget
{
    Q_OBJECT

public:
    explicit BuildingModelingWidget(QWidget *parent);
    virtual ~BuildingModelingWidget();

    QGroupBox* getComponentsWidget(void);

    bool outputToJSON(QJsonObject &rvObject);

public slots:

    void handleBuildingModelSelectionChanged(const int index);

private:
    QGroupBox* componentGroupBox;
    QComboBox* buildingModelSelectCombo;

    void createComponentsBox(void);

    QStackedWidget* theStackedWidget;
    CSVtoBIMModelingWidget* theCSVtoBIMWidget;
};

#endif // BuildingModelingWidget_H
