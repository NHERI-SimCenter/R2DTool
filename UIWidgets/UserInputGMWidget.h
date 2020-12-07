#ifndef UserInputGMWidget_H
#define UserInputGMWidget_H

#include "GroundMotionStation.h"
#include "SimCenterAppWidget.h"

#include <memory>

#include <QMap>

class VisualizationWidget;

class QStackedWidget;
class QLineEdit;
class QProgressBar;
class QLabel;

namespace Esri
{
namespace ArcGISRuntime
{
class ArcGISMapImageLayer;
class GroupLayer;
class FeatureCollectionLayer;
class KmlLayer;
class Layer;
}
}


class UserInputGMWidget : public SimCenterAppWidget
{
    Q_OBJECT

public:
    UserInputGMWidget(VisualizationWidget* visWidget, QWidget *parent = nullptr);
    ~UserInputGMWidget();

    void showUserGMLayers(bool state);

    QStackedWidget* getUserInputGMWidget(void);

    bool outputToJSON(QJsonObject &jsonObj);

public slots:

    void showUserGMSelectDialog(void);

private slots:

    void loadUserGMData(void);
    void chooseUserFileDialog(void);

signals:
    void outputDirectoryPathChanged(QString newPath);
    void loadingComplete(const bool value);

private:

    std::unique_ptr<QStackedWidget> userGMStackedWidget;

    VisualizationWidget* theVisualizationWidget;
    QLineEdit *filePathLineEdit;
    QLabel* progressLabel;
    QWidget* progressBarWidget;
    QWidget* fileInputWidget;
    QProgressBar* progressBar;
    QString pathToUserGMFile;
    QVector<GroundMotionStation> stationList;

};

#endif // UserInputGMWidget_H
