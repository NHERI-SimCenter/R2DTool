#ifndef INTENSITYMEASUREWIDGET_H
#define INTENSITYMEASUREWIDGET_H

#include "IntensityMeasure.h"

#include <QWidget>

class QComboBox;
class QLabel;
class QCheckBox;
class QLineEdit;
class QRegExpValidator;
class QDoubleSpinBox;
class QGroupBox;

class IntensityMeasureWidget : public QWidget
{
    Q_OBJECT
public:
    explicit IntensityMeasureWidget(IntensityMeasure& im, QWidget *parent = nullptr);

signals:

public slots:
    QString checkPeriodsValid(const QString& input) const;

    void commitPeriods();

    void handleTypeChanged(const QString &val);

    void handleIntensityMeasureLevels(const QString sourceType);

private:
    IntensityMeasure& m_intensityMeasure;
    QLabel* typeLabel;
    QComboBox* m_typeBox;
    QLineEdit* periodsLineEdit;
    QRegExpValidator* LEValidator;
    QLabel* periodLabel;

    QLineEdit* imtLevelLineEdit;
    QLabel* imtLevelLabel;

    QComboBox* scaleBox;
    QLabel* scaleLabel;

    QDoubleSpinBox* imtTrucBox;
    QLabel* imtTrucLabel;

    QGroupBox* imGroupBox;

    void setupConnections();
};

#endif // INTENSITYMEASUREWIDGET_H
