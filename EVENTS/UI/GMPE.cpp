#include "GMPE.h"


GMPE::GMPE(QObject *parent) : QObject(parent)
{
    this->m_type = "Chiou & Youngs (2014)";
}

QString GMPE::type() const
{
    return m_type;
}

bool GMPE::setType(const QString &type)
{
    if(m_type!= type && this->validTypes().contains(type, Qt::CaseInsensitive))
    {
        m_type = type;
        emit typeChanged(m_type);
        return true;
    }

    return false;
}

QJsonObject GMPE::getJson()
{
    QJsonObject gmpe;
    gmpe.insert("Type", m_type);
    gmpe.insert("Parameters", QJsonObject());

    return gmpe;
}

const QStringList &GMPE::validTypes()
{
    static QStringList validTypes = QStringList()
            << "Abrahamson, Silva & Kamai (2014)"
            << "Boore, Stewart, Seyhan & Atkinson (2014)"
            << "Campbell & Bozorgnia (2014)"
            << "Chiou & Youngs (2014)";
//            << "Idriss (2014)"
//            << "Campbell & Bozorgnia (2008)"
//            << "Boore & Atkinson (2008)"
//            << "Abrahamson & Silva (2008)"
//            << "Chiou & Youngs (2008)"
//            << "Zhao Et Al. (2006) - Intraslab"
//            << "Boore & Atkinson (2006)"
//            << "Chiou & Youngs (2006)"
//            << "Campbell & Bozorgnia (2006)"
//            << "Campbell and Bozorgnia (2003)"
//            << "Atkinson and Boore (2003) Subduction"
//            << "Field (2000)"
//            << "Campbell (1997) w/ erratum (2000) changes"
//            << "Abrahamson & Silva (1997)"
//            << "Boore, Joyner & Fumal (1997)";

    return validTypes;
}
