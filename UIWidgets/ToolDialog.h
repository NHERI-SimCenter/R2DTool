#ifndef ToolDialog_H
#define ToolDialog_H

#include <QDialog>

class WorkflowAppWidget;
class QVBoxLayout;
class QGISVisualizationWidget;
class OpenQuakeSelectionWidget;
class BrailsInventoryGenerator;
class BrailsTranspInventoryGenerator;
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
     void handleShowOpenquakeSelectionTool(void);
     void handleBrailsInventoryTool(void);
	 void handleBrailsTranspInventoryTool(void);

private:

    QStackedWidget* mainWidget = nullptr;

    QGISVisualizationWidget* visualizationWidget = nullptr;
    QVBoxLayout* mainLayout = nullptr;

    HousingUnitAllocationWidget* housingUnitAllocation = nullptr;
    QGISHurricaneSelectionWidget* theHurricaneSimWidget = nullptr;
    GMWidget* theEQSSWidget = nullptr;
    OpenQuakeSelectionWidget* theOpenQuakeSelectionWidget = nullptr;
    BrailsInventoryGenerator* theBrailsInventoryGeneratorWidget = nullptr;
    BrailsTranspInventoryGenerator* theBrailsTranspInventoryGeneratorWidget = nullptr;

};

#endif // ToolDialog_H
