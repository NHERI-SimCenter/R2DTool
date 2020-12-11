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

class PelicunPostProcessor : public SimCenterAppWidget
{
public:
    PelicunPostProcessor(QWidget *parent, VisualizationWidget* visWidget);

    int importResults(const QString& pathToResults);

    QGridLayout *getResultsGridLayout() const;

private:
    int processDVResults(const QVector<QStringList>& DMdata);

    QGridLayout* resultsGridLayout;

    QLabel* aggCasLabel;
    QLabel* aggLossLabel;

    QTableWidget* componentTableWidget;

    VisualizationWidget* theVisualizationWidget;

    std::unique_ptr<ResultsMapViewWidget> mapViewSubWidget;
    Esri::ArcGISRuntime::MapGraphicsView* mapViewMainWidget;

    QtCharts::QChartView *chartView;
    QtCharts::QChartView *chartView2;

    int createLossesChart();

    int createCasualtiesChart();

};

#endif // PELICUNPOSTPROCESSOR_H
