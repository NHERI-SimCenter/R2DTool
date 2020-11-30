#include "GmCommon.h"
#include <QStandardPaths>
#include <QDir>

QString GmCommon::getCacheLocation()
{
    return QStandardPaths::writableLocation(QStandardPaths::StandardLocation::CacheLocation);
}

QString GmCommon::getWorkFilePath(QString filename)
{
    QString dataDirectory = QStandardPaths::writableLocation(QStandardPaths::StandardLocation::DataLocation);
    if(!QDir(dataDirectory).exists())
        QDir().mkdir(dataDirectory);
    return QDir::cleanPath(dataDirectory + QDir::separator() + filename);
}
