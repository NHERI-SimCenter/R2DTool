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

    // Function to convert a QString and QVariant to double
    // Throws an error exception if conversion fails
    template <typename T>
    auto objectToDouble(T obj)
    {
        // Assume a zero value if the string is empty
        if(obj.isNull())
            return 0.0;

        bool OK;
        auto val = obj.toDouble(&OK);

        if(!OK)
            throw QString("Could not convert the object to a double");

        return val;
    }


private:

    void importGroundMotionTimeHistory(const QString& filePath, const double scalingFactor);

    QString stationFilePath;

    double latitude;

    double longitude;

    QVector<GroundMotionTimeHistory> stationGroundMotions;

    QMap<QString,QVariant> attributes;

};

#endif // GROUNDMOTIONSTATION_H
