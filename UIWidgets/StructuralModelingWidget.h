#ifndef StructuralModelingWidget_H
#define StructuralModelingWidget_H

// Written by: Stevan Gavrilovic
// Latest revision: 10.08.2020

#include "SimCenterAppWidget.h"

class OpenSeesBuildingModel;
class OpenSeesPyBuildingModel;
class RandomVariablesContainer;
class RandomVariablesContainer;

class QComboBox;
class QGroupBox;
class QStackedWidget;

class StructuralModelingWidget : public  SimCenterAppWidget
{
    Q_OBJECT

public:
    explicit StructuralModelingWidget(QWidget *parent, RandomVariablesContainer* RVContainer);
    virtual ~StructuralModelingWidget();

    bool outputToJSON(QJsonObject &rvObject);

public slots:

    void handleBuildingModelSelectionChanged(const int index);

private:
    QComboBox* buildingModelSelectCombo;

    QStackedWidget* theStackedWidget;
    OpenSeesPyBuildingModel* theOpenSeesPyModelWidget;
    OpenSeesBuildingModel* theOpenSeesModelWidget;
    RandomVariablesContainer* theRandomVariablesContainer;
};

#endif // StructuralModelingWidget_H
