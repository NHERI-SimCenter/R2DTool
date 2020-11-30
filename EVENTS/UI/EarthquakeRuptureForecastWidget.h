#ifndef EarthquakeRuptureForecastWidget_H
#define EarthquakeRuptureForecastWidget_H

#include <QWidget>

class EarthquakeRuptureForecast;

class QComboBox;
class QLineEdit;
class QDoubleSpinBox;

class EarthquakeRuptureForecastWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EarthquakeRuptureForecastWidget(QWidget *parent = nullptr);

    EarthquakeRuptureForecast* getRuptureSource() const;

signals:

public slots:

private:
    EarthquakeRuptureForecast* m_eqRupture;
    QComboBox* ModelTypeCombo;
    QLineEdit* EQNameLineEdit;
    QDoubleSpinBox* m_magnitudeMinBox;
    QDoubleSpinBox* m_magnitudeMaxBox;
    QDoubleSpinBox* m_maxDistanceBox;

    void setupConnections();

};

#endif // EarthquakeRuptureForecastWidget_H
