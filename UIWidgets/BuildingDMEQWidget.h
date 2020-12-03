#ifndef BuildingDMEQWidget_H
#define BuildingDMEQWidget_H

#include <SimCenterAppWidget.h>

#include <QGroupBox>
#include <QVector>

class QComboBox;
class QStackedWidget;
class PelicunDLWidget;

class RandomVariablesContainer;

class BuildingDMEQWidget : public  SimCenterAppWidget
{

    Q_OBJECT

public:
    explicit BuildingDMEQWidget(RandomVariablesContainer *theRandomVariableIW, QWidget *parent = 0);
    ~BuildingDMEQWidget();

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

    PelicunDLWidget* thePelicunDLWidget;

    RandomVariablesContainer* theRandomVariablesContainer;
};

#endif // BuildingDMEQWidget_H
