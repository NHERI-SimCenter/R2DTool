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
class QBarSet;
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

    // Map to store the results - the QString (key) is the header text while the double is the value for that building
    QMap<QString, double> ResultsValues;

    int ID;
};

class PelicunPostProcessor : public SimCenterAppWidget
{
    Q_OBJECT

public:

    PelicunPostProcessor(QWidget *parent, VisualizationWidget* visWidget);

    int importResults(const QString& pathToResults);

    QGridLayout *getResultsGridLayout() const;

    int printToPDF(const QString& outputPath);


private slots:

    int assemblePDF(QImage screenShot);

private:

    int processDVResults(const QVector<QStringList>& DVdata);

    QString outputFilePath;

    QGridLayout* resultsGridLayout;

    QLabel* totalCasLabel;
    QLabel* totalLossLabel;
    QLabel* totalRepairTimeLabel;
    QLabel* totalFatalitiesLabel; 
    QLabel* structLossLabel;
    QLabel* nonStructLossLabel;

    QLabel* totalCasValueLabel;
    QLabel* totalLossValueLabel;
    QLabel* totalRepairTimeValueLabel;
    QLabel* totalFatalitiesValueLabel;
    QLabel* structLossValueLabel;
    QLabel* nonStructLossValueLabel;

    QTableWidget* pelicunResultsTableWidget;

    VisualizationWidget* theVisualizationWidget;

    std::unique_ptr<ResultsMapViewWidget> mapViewSubWidget;
    Esri::ArcGISRuntime::MapGraphicsView* mapViewMainWidget;

    QtCharts::QChartView *casualtiesChartView;
    QtCharts::QChartView *lossesChartView;

    int createLossesChart(QtCharts::QBarSet *structLossSet, QtCharts::QBarSet *NSAccLossSet, QtCharts::QBarSet *NSDriftLossSet);

    int createCasualtiesChart(QtCharts::QBarSet *casualtiesSet);

    QVector<Building> buildingsVec;

};

#endif // PELICUNPOSTPROCESSOR_H
