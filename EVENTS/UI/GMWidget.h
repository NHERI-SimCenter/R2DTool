#ifndef GMWIDGET_H
#define GMWIDGET_H

#include "SimCenterAppWidget.h"
#include "GroundMotionStation.h"
#include "PeerNgaWest2Client.h"

#include <QProcess>
#include <QJsonObject>

class GmAppConfig;
class IntensityMeasureWidget;
class SpatialCorrelationWidget;
class RuptureWidget;
class GMPEWidget;
class RecordSelectionWidget;
class RecordSelectionConfig;
class SiteConfig;
class SiteConfigWidget;
class GMPE;
class IntensityMeasure;
class VisualizationWidget;
class MapViewSubWidget;

class QDialog;
class QPlainTextEdit;
class QPushButton;
class QProgressBar;
class QStatusBar;

namespace Esri
{
namespace ArcGISRuntime
{
class MapGraphicsView;
}
}

class GMWidget : public SimCenterAppWidget
{
    Q_OBJECT

public:
    explicit GMWidget(QWidget *parent, VisualizationWidget* visWidget);
    ~GMWidget();

    bool outputAppDataToJSON(QJsonObject &jsonObject);
    bool outputToJSON(QJsonObject &jsonObject);
    bool inputFromJSON(QJsonObject &jsonObject);
    void saveAppSettings();
    void setCurrentlyViewable(bool status);

signals:
    void locationsChanged(void);
    void sceneViewChanged(void);
    void mapViewChanged(void);
    void outputDirectoryPathChanged(QString folderPath, QString gridFile);

public slots:
    void setAppConfig(void);
    void showGISWindow(void);
    void runHazardSimulation(void);
    void showInfoDialog(void);

    // Handles the results when the user is finished
    void handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

    // Brings up the dialog and tells the user that the process has started
    void handleProcessStarted(void);

    // Displays the text output of the process in the dialog
    void handleProcessTextOutput(void);

    // Download records once selected
    int downloadRecords(void);

    // Download records once selected
    void downloadRecordBatch(void);

    // Process the outfile files once the hazard simulation is complete
    int parseDownloadedRecords(QString);

private slots:

private:
    PeerNgaWest2Client peerClient;

    RuptureWidget* m_ruptureWidget;
    GMPE* m_gmpe;
    GMPEWidget* m_gmpeWidget;
    QProcess* process;
    IntensityMeasure* m_intensityMeasure;
    IntensityMeasureWidget* m_intensityMeasureWidget;
    SpatialCorrelationWidget* spatialCorrWidget;
    RecordSelectionConfig* m_selectionconfig;
    RecordSelectionWidget* m_selectionWidget;
    SiteConfig* m_siteConfig;
    SiteConfigWidget* m_siteConfigWidget;
    QPushButton* m_runButton;
    QPushButton* m_settingButton;
    GmAppConfig* m_appConfig;

    QDialog* progressDialog;
    QPlainTextEdit* progressTextEdit;
    QProgressBar* progressBar;

    VisualizationWidget* theVisualizationWidget;
    std::unique_ptr<MapViewSubWidget> mapViewSubWidget;
    Esri::ArcGISRuntime::MapGraphicsView* mapViewMainWidget;

    void setupConnections();
    void initAppConfig();

    bool simulationComplete;
    QVector<GroundMotionStation> stationList;

    int processDownloadedRecords(QString& errorMessage);

    void handleErrorMessage(const QString& errorMessage);

    int numDownloaded;
    bool downloadComplete;
    QStringList recordsListToDownload;
    QJsonObject NGA2Results;
};

#endif // GMWIDGET_H
