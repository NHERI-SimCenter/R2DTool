#ifndef XMLADAPTOR_H
#define XMLADAPTOR_H

#include <QString>

class QObject;

namespace Esri
{
namespace ArcGISRuntime
{
class FeatureCollectionLayer;
}
}

class XMLAdaptor
{
public:
    XMLAdaptor();

    Esri::ArcGISRuntime::FeatureCollectionLayer* parseXMLFile(const QString& filePath, QString& errMessage, QObject* parent = nullptr);

    QString getEventName() const;

private:
    QString eventName;

    QString shakemapID;
};

#endif // XMLADAPTOR_H
