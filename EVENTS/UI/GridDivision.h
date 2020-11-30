#ifndef GRIDDIVISION_H
#define GRIDDIVISION_H

#include <QObject>

class GridDivision : public QObject
{
    Q_OBJECT
    Q_PROPERTY(double min READ min WRITE setMin NOTIFY minChanged)
    Q_PROPERTY(double max READ max WRITE setMax NOTIFY maxChanged)
    Q_PROPERTY(int divisions READ divisions WRITE setDivisions NOTIFY divisionsChanged)

public:
    explicit GridDivision(QObject *parent = nullptr);
    double getStep();

    double min();
    double max();
    int divisions();
    void set(double min, double max, int divisions);

signals:
    void minChanged(double newMin);
    void maxChanged(double newMax);
    void divisionsChanged(double newDivisions);

public slots:
    void setMin(double min);
    void setMax(double max);
    void setDivisions(int numDivisions);

private:
    double m_min;
    double m_max;
    int m_divisions;
};

#endif // GRIDDIVISION_H
