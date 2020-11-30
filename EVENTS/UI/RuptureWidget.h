#ifndef RUPTUREWIDGET_H
#define RUPTUREWIDGET_H

#include "SimCenterAppWidget.h"

class QGroupBox;
class QComboBox;
class QStackedWidget;
class PointSourceRuptureWidget;
class EarthquakeRuptureForecastWidget;

class RuptureWidget : public SimCenterAppWidget
{
    Q_OBJECT
public:
    explicit RuptureWidget(QWidget *parent = nullptr);

    QJsonObject getJson(void);

signals:

public slots:
    void handleSelectionChanged(const QString& selection);

private:
    QGroupBox* ruptureGroupBox;
    QComboBox* ruptureSelectionCombo;
    QStackedWidget* theRootStackedWidget;
    PointSourceRuptureWidget* pointSourceWidget;
    EarthquakeRuptureForecastWidget* erfWidget;
};

#endif // RUPTUREWIDGET_H
