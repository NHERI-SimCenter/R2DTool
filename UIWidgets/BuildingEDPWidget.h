#ifndef BuildingEDPWidget_H
#define BuildingEDPWidget_H

// Written by: Stevan Gavrilovic
// Latest revision: 10.08.2020

#include "SimCenterAppWidget.h"

class BuildingEDPEQWidget;

class QComboBox;
class QGroupBox;
class QTabWidget;

class BuildingEDPWidget : public  SimCenterAppWidget
{
    Q_OBJECT

public:
    explicit BuildingEDPWidget(QWidget *parent);
    virtual ~BuildingEDPWidget();

    bool outputToJSON(QJsonObject &rvObject);

private:
    QTabWidget* theTabbedWidget;
    BuildingEDPEQWidget* theEDPEQWidget;

};

#endif // BuildingEDPWidget_H
