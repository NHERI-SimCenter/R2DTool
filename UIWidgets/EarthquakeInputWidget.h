#ifndef EARTHQUAKEINPUTWIDGET_H
#define EARTHQUAKEINPUTWIDGET_H

// Written by: Stevan Gavrilovic
// Latest revision: 10.08.2020

#include "SimCenterAppWidget.h"

class QGroupBox;
class QStackedWidget;
class RandomVariablesContainer;
class GMWidget;
class ShakeMapWidget;
class UserInputGMWidget;
class VisualizationWidget;

class EarthquakeInputWidget : public  SimCenterAppWidget
{
    Q_OBJECT

public:
    EarthquakeInputWidget(QWidget *parent, VisualizationWidget* visWidget, RandomVariablesContainer * RVContainer);
    ~EarthquakeInputWidget();

    QWidget* getEarthquakesWidget(void);

private slots:

    void handleEQTypeSelection(const QString& selection);

    void shakeMapLoadingFinished(const bool value);

private:

    void createEarthquakesWidget(void);

    QWidget* theEQWidget;
    QStackedWidget* theRootStackedWidget;

    RandomVariablesContainer* theRandomVariablesContainer;

    VisualizationWidget* theVisualizationWidget;
    GMWidget* theEQSSWidget;
    ShakeMapWidget* theShakeMapWidget;
    UserInputGMWidget* theUserInputGMWidget;
};

#endif // EARTHQUAKEINPUTWIDGET_H
