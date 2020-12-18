#ifndef GROUNDMOTIONSTATION_H
#define GROUNDMOTIONSTATION_H

#include "GroundMotionTimeHistory.h"

#include <QMap>
#include <QVariant>

class GroundMotionStation
{
public:
    GroundMotionStation(QString path, double lat, double lon);

    double getLatitude() const;

    double getLongitude() const;

    QString getStationFilePath() const;

    void importGroundMotions(void);

    QVector<GroundMotionTimeHistory> getStationGroundMotions() const;

private:

    void importGroundMotionTimeHistory(const QString& filePath, const double scalingFactor);

    QString stationFilePath;

    double latitude;

    double longitude;

    QVector<GroundMotionTimeHistory> stationGroundMotions;

};

#endif // GROUNDMOTIONSTATION_H
