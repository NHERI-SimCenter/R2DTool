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
class QJsonObject;

class IntensityMeasureWidget : public SimCenterAppWidget
{
    Q_OBJECT
public:
    explicit IntensityMeasureWidget(IntensityMeasure& im, QWidget *parent = nullptr);

    QComboBox *typeBox() const;
    QStringList* getSelectedIMTypes(void);
    bool outputToJSON(QJsonObject &jsonObject);

signals:
    void IMSelectionChanged(QStringList* selectedIMs);

public slots:
    QString checkPeriodsValid(const QString& input) const;

    void commitPeriods();

    void handleTypeChanged(int state);

    void handleIntensityMeasureLevels(const QString sourceType);

private:
    IntensityMeasure& m_intensityMeasure;
    QLabel* typeLabel;
    QComboBox* m_typeBox;
    QLineEdit* periodsLineEdit;
    QRegExpValidator* LEValidator;
    QLabel* periodLabel;

    QCheckBox* PGACheckBox;
    QCheckBox* SACheckBox;
    QCheckBox* PGVCheckBox;
    QCheckBox* DS575HCheckBox;
    QCheckBox* DS595HCheckBox;
    QStringList* selectedIMTypes = new QStringList();
    QList<double> SAperiods;

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
