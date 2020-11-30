#ifndef RUPTURELOCATION_H
#define RUPTURELOCATION_H

#include "Location.h"

class RuptureLocation : public Location
{
    Q_OBJECT

public:
    RuptureLocation(QObject *parent = nullptr);
    RuptureLocation(double latitude, double longitude, QObject *parent = nullptr);

    double depth() const;

signals:
    void depthChanged(double newDepth);

public slots:
    void setDepth(double depth);

private:
    double m_depth;
};

#endif // RUPTURELOCATION_H
