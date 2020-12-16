#ifndef PELICUNPOSTPROCESSOR_H
#define PELICUNPOSTPROCESSOR_H

#include <memory>
#include <QString>

#include "ResultsMapViewWidget.h"
#include "SimCenterAppWidget.h"
#include "BuildingDatabase.h"

class VisualizationWidget;
class ResultsMapViewWidget;
class REmpiricalProbabilityDistribution;

class QTableWidget;
class QGridLayout;
class QLabel;
class QComboBox;

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


class PelicunPostProcessor : public SimCenterAppWidget
{
    Q_OBJECT

public:

    PelicunPostProcessor(QWidget *parent, VisualizationWidget* visWidget);

    int importResults(const QString& pathToResults);

    QGridLayout *getResultsGridLayout() const;

    int printToPDF(const QString& outputPath);

    // Function to convert a QString and QVariant to double
    // Throws an error exception if conversion fails
    template <typename T>
    auto objectToDouble(T obj)
    {
        // Assume a zero value if the string is empty
        if(obj.isNull())
            return 0.0;

        bool OK;
        auto val = obj.toDouble(&OK);

        if(!OK)
            throw QString("Could not convert the object to a double");

        return val;
    }

private slots:

    int assemblePDF(QImage screenShot);

    void sortTable(int index);

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

    QComboBox* sortComboBox;

    std::unique_ptr<ResultsMapViewWidget> mapViewSubWidget;
    Esri::ArcGISRuntime::MapGraphicsView* mapViewMainWidget;

    QtCharts::QChartView *casualtiesChartView;
    QtCharts::QChartView *lossesChartView;
    QtCharts::QChartView *lossesHistogram;

    int createHistogramChart(REmpiricalProbabilityDistribution* probDist);

    int createLossesChart(QtCharts::QBarSet *structLossSet, QtCharts::QBarSet *NSAccLossSet, QtCharts::QBarSet *NSDriftLossSet);

    int createCasualtiesChart(QtCharts::QBarSet *casualtiesSet);

    QVector<Building> buildingsVec;

};

#endif // PELICUNPOSTPROCESSOR_H
