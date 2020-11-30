#include "RecordSelectionConfig.h"

RecordSelectionConfig::RecordSelectionConfig(QObject *parent) : QObject(parent)
{
    this->m_error = ErrorMetric::RMSE;
}


QString RecordSelectionConfig::getDatabase() const
{
    QString db;

    if(m_database.compare("PEER NGA West 2") == 0)
        db =  "NGAWest2";
    else
    {
        qDebug()<<"Error, the type of database "+m_database+" is not recognized";
        db = "ERROR_DATABASE_NOT_RECOGNIZED";
    }

    return db;
}

void RecordSelectionConfig::setDatabase(const QString &database)
{
    if(m_database != database)
    {
        m_database = database;
        emit this->databaseChanged(m_database);
    }
}

RecordSelectionConfig::ErrorMetric RecordSelectionConfig::getError() const
{
    return m_error;
}

void RecordSelectionConfig::setError(const ErrorMetric &error)
{
    if(m_error != error)
    {
        m_error = error;
        emit this->errorChanged(m_error);
    }
}

QJsonObject RecordSelectionConfig::getJson()
{
    QJsonObject db;

    db.insert("Database", this->getDatabase());

    return db;
}
