#ifndef HAZARDS_WIDGET_H
#define HAZARDS_WIDGET_H

// Written by: Stevan Gavrilovic
// Latest revision: 10.08.2020

#include "SimCenterAppWidget.h"

class RandomVariablesContainer;
class GMWidget;
class ShakeMapWidget;
class UserInputGMWidget;
class VisualizationWidget;

class QGroupBox;
class QStackedWidget;
class QCheckBox;
class QComboBox;

class HazardsWidget : public  SimCenterAppWidget
{
    Q_OBJECT

public:
    HazardsWidget(QWidget *parent, VisualizationWidget* visWidget, RandomVariablesContainer * RVContainer);
    ~HazardsWidget();

    // QWidget* getEarthquakesWidget(void);

    bool outputToJSON(QJsonObject &jsonObject);
    bool inputFromJSON(QJsonObject &jsonObject);

private slots:

    void handleEQTypeSelection(const QString& selection);
    void shakeMapLoadingFinished(const bool value);

private:

    void createWidget(void);

    QStackedWidget* theRootStackedWidget;
    QCheckBox* includeHazardCheckBox;
    QComboBox* hazardSelectionCombo;

    RandomVariablesContainer* theRandomVariablesContainer;

    VisualizationWidget* theVisualizationWidget;
    GMWidget* theEQSSWidget;
    ShakeMapWidget* theShakeMapWidget;
    UserInputGMWidget* theUserInputGMWidget;
};

#endif // HAZARDS_WIDGET_H
