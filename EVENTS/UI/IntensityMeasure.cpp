#include "IntensityMeasure.h"

#include <QtAlgorithms>
#include <QJsonArray>
#include <QLineEdit>

IntensityMeasure::IntensityMeasure(QObject *parent) : QObject(parent)
{

}


QString IntensityMeasure::type() const
{
    return m_type;
}


bool IntensityMeasure::setType(const QString &type)
{
    if(m_type != type && validTypes().contains(type, Qt::CaseInsensitive))
    {
        m_type = type;
        emit typeChanged(m_type);
        return true;
    }
    return false;
}


QJsonObject IntensityMeasure::getJson()
{
    QJsonObject im;

    if(m_type.compare("Spectral Accelerations (SA)") == 0)
        im.insert("Type", "SA");
    else if(m_type.compare("Peak Ground Acceleration (PGA)") == 0)
        im.insert("Type", "PGA");
    else
    {
        im.insert("Type", "UNKNOWN");
        qDebug()<<"Warning, the type of intensity measure "<<m_type<<" is not recognized";
    }

    QJsonArray arrayPeriods;

    for(auto&& it : m_periods)
    {
        arrayPeriods.append(it);
    }
    im.insert("Periods", arrayPeriods);

    return im;
}


const QStringList &IntensityMeasure::validTypes()
{
    static QStringList validTypes = QStringList()
            << "Peak Ground Acceleration (PGA)"
            << "Spectral Accelerations (SA)";

    return validTypes;
}


QList<double> IntensityMeasure::periods() const
{
    return m_periods;
}


void IntensityMeasure::setPeriods(const QList<double> &periods)
{
    m_periods = periods;
}


void IntensityMeasure::setPeriods(const QString &periods)
{
    periodsText = periods;
}


void IntensityMeasure::addPeriod(double period)
{
    this->m_periods.append(period);
    qSort(this->m_periods);
}

