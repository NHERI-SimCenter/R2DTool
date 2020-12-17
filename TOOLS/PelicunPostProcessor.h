#ifndef PELICUNPOSTPROCESSOR_H
#define PELICUNPOSTPROCESSOR_H

#include <memory>
#include <QString>

#include "ResultsMapViewWidget.h"
#include "SimCenterAppWidget.h"

class VisualizationWidget;
class ResultsMapViewWidget;

class QTableWidget;
class QGridLayout;
class QLabel;

namespace QtCharts
{
class QChartView;
}

namespace Esri
{
namespace ArcGISRuntime
{
class Map;
class MapGraphicsView;
}
}

struct Building
{
public:

    QMap<QString, double> values;

    int ID;
};

class PelicunPostProcessor : public SimCenterAppWidget
{
public:
    PelicunPostProcessor(QWidget *parent, VisualizationWidget* visWidget);

    int importResults(const QString& pathToResults);

    QGridLayout *getResultsGridLayout() const;

private:
    int processDVResults(const QVector<QStringList>& DVdata);

    QGridLayout* resultsGridLayout;

    QLabel* totalCasLabel;
    QLabel* totalLossLabel;
    QLabel* totalRepairTimeLabel;
    QLabel* totalFatalitiesLabel;

    QTableWidget* pelicunResultsTableWidget;

    VisualizationWidget* theVisualizationWidget;

    std::unique_ptr<ResultsMapViewWidget> mapViewSubWidget;
    Esri::ArcGISRuntime::MapGraphicsView* mapViewMainWidget;

    QtCharts::QChartView *casualtiesChartView;
    QtCharts::QChartView *lossesChartView;

    int createLossesChart();

    int createCasualtiesChart();

    QVector<Building> buildingsVec;

};

#endif // PELICUNPOSTPROCESSOR_H
