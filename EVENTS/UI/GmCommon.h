#ifndef GMCOMMON_H
#define GMCOMMON_H
#include <QString>

class GmCommon
{
public:
    static QString getCacheLocation();
    static QString getWorkFilePath(QString filename);
};

#endif // GMCOMMON_H
