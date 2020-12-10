#ifndef PELICUNPOSTPROCESSOR_H
#define PELICUNPOSTPROCESSOR_H

#include <memory>
#include <QString>

#include "ResultsMapViewWidget.h"

class VisualizationWidget;
class ResultsMapViewWidget;

class QTableWidget;
class QGridLayout;

namespace Esri
{
namespace ArcGISRuntime
{
class Map;
class MapGraphicsView;
}
}

class PelicunPostProcessor
{
public:
    PelicunPostProcessor(VisualizationWidget* visWidget);

    int importResults(const QString& pathToResults);

private:
    int processDVResults(const QVector<QStringList>& DMdata);

    QGridLayout* resultsGridLayout;

    QTableWidget* componentTableWidget;

    VisualizationWidget* theVisualizationWidget;

    std::unique_ptr<ResultsMapViewWidget> mapViewSubWidget;
    Esri::ArcGISRuntime::MapGraphicsView* mapViewMainWidget;

};

#endif // PELICUNPOSTPROCESSOR_H
