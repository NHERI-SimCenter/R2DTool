#ifndef SpatialCorrelationWidget_H
#define SpatialCorrelationWidget_H

#include "IntensityMeasure.h"

#include <QWidget>

class QComboBox;
class QCheckBox;
class QLineEdit;

class SpatialCorrelationWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SpatialCorrelationWidget(QWidget *parent = nullptr);

    QJsonObject getJsonCorr();

    QJsonObject getJsonScaling();

signals:

public slots:

private:
    QComboBox* m_correlationBoxInter;
    QComboBox* m_correlationBoxIntra;

    QLineEdit* maxScalingLineEdit;
    QLineEdit* minScalingLineEdit;

};

#endif // SpatialCorrelationWidget_H
