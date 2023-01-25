#ifndef ToolDialog_H
#define ToolDialog_H

#include <QDialog>

class WorkflowAppWidget;
class QVBoxLayout;
class QGISVisualizationWidget;
class HousingUnitAllocationWidget;
class QGISHurricaneSelectionWidget;
class GMWidget;
class QStackedWidget;

class ToolDialog : public QDialog
{
    Q_OBJECT

public:
    ToolDialog(WorkflowAppWidget* parent, QGISVisualizationWidget* visWidget);
    ~ToolDialog();

    void clear(void);

public slots:

     void handleShowCensusAppTool(void);
     void handleShowEQGMSimTool(void);
     void handleShowHurricaneSimTool(void);

private:

    QStackedWidget* mainWidget = nullptr;

    QGISVisualizationWidget* visualizationWidget = nullptr;
    QVBoxLayout* mainLayout = nullptr;

    HousingUnitAllocationWidget* housingUnitAllocation = nullptr;
    QGISHurricaneSelectionWidget* theHurricaneSimWidget = nullptr;
    GMWidget* theEQSSWidget = nullptr;

};

#endif // ToolDialog_H
