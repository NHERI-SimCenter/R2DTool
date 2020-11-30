#ifndef INTENSITYMEASURE_H
#define INTENSITYMEASURE_H

#include <QObject>
#include "JsonSerializable.h"

class IntensityMeasure : public QObject, JsonSerializable
{
    Q_OBJECT
public:
    explicit IntensityMeasure(QObject *parent = nullptr);

    QString type() const;

    const QStringList& validTypes();

    QList<double> periods() const;
    void setPeriods(const QList<double> &periods);
    void setPeriods(const QString &periods);
    void addPeriod(double period);

signals:
    void typeChanged(QString newType);

public slots:
    bool setType(const QString &type);

private:
    QString m_type;
    QList<double> m_periods;

    QString periodsText;
    // JsonSerializable interface
public:
    QJsonObject getJson();
};

#endif // INTENSITYMEASURE_H
