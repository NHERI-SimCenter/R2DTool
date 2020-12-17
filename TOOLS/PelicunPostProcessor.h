#ifndef PELICUNPOSTPROCESSOR_H
#define PELICUNPOSTPROCESSOR_H

#include "ResultsMapViewWidget.h"
#include "BuildingDatabase.h"

#include <QString>
#include <QMainWindow>

#include <memory>
#include <set>

class VisualizationWidget;
class ResultsMapViewWidget;
class REmpiricalProbabilityDistribution;

class QDockWidget;
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


class PelicunPostProcessor : public QMainWindow
{
    Q_OBJECT

public:

    PelicunPostProcessor(QWidget *parent, VisualizationWidget* visWidget);

    void importResults(const QString& pathToResults);

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

    void processResultsSubset(const std::set<int>& selectedComponentIDs);

private slots:

    int assemblePDF(QImage screenShot);

    void sortTable(int index);

    void restoreUI(void);

private:

    int processDVResults(const QVector<QStringList>& DVResults);

    QVector<QStringList> DMdata;
    QVector<QStringList> DVdata;
    QVector<QStringList> EDPdata;

    QString outputFilePath;

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

    QWidget *tableWidget;

    QTableWidget* pelicunResultsTableWidget;

    QDockWidget* chartsDock1;
    QDockWidget* chartsDock2;
    QDockWidget* chartsDock3;

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

    QByteArray uiState;

};

#endif // PELICUNPOSTPROCESSOR_H
