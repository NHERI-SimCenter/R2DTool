#ifndef INTENSITYMEASUREWIDGET_H
#define INTENSITYMEASUREWIDGET_H

#include "IntensityMeasure.h"

#include <QWidget>

class QComboBox;
class QLabel;
class QCheckBox;
class QLineEdit;
class QRegExpValidator;

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

private:
    IntensityMeasure& m_intensityMeasure;
    QComboBox* m_typeBox;
    QLineEdit* periodsLineEdit;
    QRegExpValidator* LEValidator;
    QLabel* periodLabel;

    void setupConnections();
};

#endif // INTENSITYMEASUREWIDGET_H
