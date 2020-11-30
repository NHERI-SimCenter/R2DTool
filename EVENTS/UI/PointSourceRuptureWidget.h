#ifndef PointSourceRuptureWidget_H
#define PointSourceRuptureWidget_H

#include <QWidget>
#include <QtWidgets>

class PointSourceRupture;

class PointSourceRuptureWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PointSourceRuptureWidget(QWidget *parent = nullptr, Qt::Orientation orientation = Qt::Horizontal);

    void setLocation(const double latitude, const double longitude);

    PointSourceRupture* getRuptureSource() const;

signals:

public slots:

private:
    PointSourceRupture* m_eqRupture;

    QDoubleSpinBox* m_magnitudeBox;

    QGroupBox* m_locationGroupBox;
    QDoubleSpinBox* m_latitudeBox;
    QDoubleSpinBox* m_longitudeBox;
    QDoubleSpinBox* m_depthBox;

    QGroupBox* m_geometryGroupBox;
    QDoubleSpinBox* m_dipBox;
    QDoubleSpinBox* m_rakeBox;

    void setupConnections();
};

#endif // PointSourceRuptureWidget_H
