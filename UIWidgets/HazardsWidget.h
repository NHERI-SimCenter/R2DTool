#ifndef HAZARDS_WIDGET_H
#define HAZARDS_WIDGET_H

// Written by: Stevan Gavrilovic
// modified: fmk

#include "SimCenterAppSelection.h"

class RandomVariablesContainer;
class GMWidget;
class ShakeMapWidget;
class UserInputGMWidget;
class VisualizationWidget;

class QGroupBox;

class HazardsWidget : public  SimCenterAppSelection
{
    Q_OBJECT

public:
    HazardsWidget(QWidget *parent, VisualizationWidget* visWidget, RandomVariablesContainer * RVContainer);
    ~HazardsWidget();

    void setCurrentlyViewable(bool status);

signals:
    void gridFileChangedSignal(QString motionDir, QString eventFile);

private slots:

    void shakeMapLoadingFinished(const bool value);
    void gridFileChangedSlot(QString motionDir, QString eventFile);

private:

    RandomVariablesContainer* theRandomVariablesContainer;

    VisualizationWidget* theVisualizationWidget;
    GMWidget* theEQSSWidget;
    ShakeMapWidget* theShakeMapWidget;
    UserInputGMWidget* theUserInputGMWidget;
};

#endif // HAZARDS_WIDGET_H
