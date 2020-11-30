#ifndef JSONSERIALIZABLE_H
#define JSONSERIALIZABLE_H

#include <QJsonObject>

class JsonSerializable
{
public:
    virtual QJsonObject getJson() = 0;
};

#endif // JSONSERIALIZABLE_H
