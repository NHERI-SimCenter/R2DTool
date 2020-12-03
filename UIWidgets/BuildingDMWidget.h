#ifndef BuildingDMWidget_H
#define BuildingDMWidget_H

// Written by: Stevan Gavrilovic
// Latest revision: 10.08.2020

#include "SimCenterAppWidget.h"

class BuildingDMEQWidget;

class QComboBox;
class QGroupBox;
class QTabWidget;

class BuildingDMWidget : public  SimCenterAppWidget
{
    Q_OBJECT

public:
    explicit BuildingDMWidget(QWidget *parent);
    virtual ~BuildingDMWidget();

    bool outputToJSON(QJsonObject &rvObject);

private:
    QTabWidget* theTabbedWidget;
    BuildingDMEQWidget* theDMEQWidget;

};

#endif // BuildingDMWidget_H
