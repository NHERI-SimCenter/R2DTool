#ifndef BUILDINGEDPEQWIDGET_H
#define BUILDINGEDPEQWIDGET_H

#include <SimCenterAppWidget.h>

#include <QGroupBox>
#include <QVector>

class QComboBox;
class QStackedWidget;
class StandardEarthquakeEDP;
class UserDefinedEDPR;

class RandomVariablesContainer;

class BuildingEDPEQWidget : public  SimCenterAppWidget
{
    Q_OBJECT
public:
    explicit BuildingEDPEQWidget(RandomVariablesContainer *theRandomVariableIW, QWidget *parent = 0);
    ~BuildingEDPEQWidget();

    bool outputToJSON(QJsonObject &jsonObject) override;
    bool inputFromJSON(QJsonObject &jsonObject) override;
    bool outputAppDataToJSON(QJsonObject &jsonObject) override;
    bool inputAppDataFromJSON(QJsonObject &jsonObject) override;
    bool copyFiles(QString &destName) override;

    void clear(void);

signals:

public slots:
    void edpSelectionChanged(const QString &arg1);

private:
    QComboBox* edpSelection;
    QStackedWidget* theStackedWidget;
    SimCenterAppWidget* theCurrentWidget;

    StandardEarthquakeEDP* theStandardEarthquakeEDPs;
    UserDefinedEDPR* theUserDefinedEDPs;

    RandomVariablesContainer* theRandomVariablesContainer;
};

#endif // BUILDINGEDPEQWIDGET_H
