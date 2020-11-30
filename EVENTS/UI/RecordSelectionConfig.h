#ifndef RECORDSELECTIONCONFIG_H
#define RECORDSELECTIONCONFIG_H

#include <QObject>
#include "JsonSerializable.h"

class RecordSelectionConfig : public QObject, JsonSerializable
{
    Q_OBJECT

public:
    explicit RecordSelectionConfig(QObject *parent = nullptr);

    enum ErrorMetric {AbsSum = 0, RMSE = 1, MSE = 2, MAPE = 3};

    QString getDatabase() const;

    void setError(const ErrorMetric &error);
    ErrorMetric getError() const;

    QJsonObject getJson();

signals:
    void databaseChanged(QString newDatabase);
    void errorChanged(ErrorMetric error);

public slots:
    void setDatabase(const QString &database);

private:
    QString m_database;
    ErrorMetric m_error;

};

#endif // RECORDSELECTIONCONFIG_H
